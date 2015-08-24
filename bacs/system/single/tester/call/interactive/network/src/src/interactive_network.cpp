#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/tester_util.hpp>
#include <bacs/system/single/worker.hpp>

#include <boost/assert.hpp>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;

static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000);  // FIXME

constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

class interactive_network_tester : public tester {
 public:
  interactive_network_tester(
      const yandex::contest::invoker::ContainerPointer &container,
      result_mapper_uptr mapper, checker_uptr checker)
      : m_util(container, "/testing"),
        m_mapper(std::move(mapper)),
        m_checker(std::move(checker)) {
    BOOST_ASSERT(m_mapper);
    BOOST_ASSERT(m_checker);
  }

  bool build(const bacs::process::Buildable &solution,
             bacs::process::BuildResult &result) override {
    m_builder = builder::instance(solution.build_settings().config());
    m_solution = m_builder->build(
        m_util.container(), SOLUTION_OWNER_ID, solution.source(),
        solution.build_settings().resource_limits(), result);
    return static_cast<bool>(m_solution);
  }

  bool test(const problem::single::process::Settings &settings,
            const test::storage::test &test,
            problem::single::TestResult &result) override;

  static tester_uptr make_instance(
      const yandex::contest::invoker::ContainerPointer &container,
      result_mapper_uptr mapper, checker_uptr checker) {
    return std::make_unique<interactive_network_tester>(
        container, std::move(mapper), std::move(checker));
  }

 private:
  tester_util m_util;
  const result_mapper_uptr m_mapper;
  const checker_uptr m_checker;
  builder_ptr m_builder;
  executable_ptr m_solution;
};

BUNSAN_PLUGIN_AUTO_REGISTER(tester, interactive_network_tester,
                            interactive_network_tester::make_instance)

bool interactive_network_tester::test(
    const problem::single::process::Settings &settings,
    const test::storage::test &test,
    problem::single::TestResult &result) override {
  m_util.reset();

  const boost::filesystem::path testing_path =
      m_util.create_directory("testing", unistd::access::Id{0, 0}, 0555);
  const boost::filesystem::path logging_path =
      m_util.create_directory("logging", unistd::access::Id{0, 0}, 0555);

  const ProcessPointer process = m_solution->create(
      m_util.process_group(), settings.execution().argument());
  process->setName("solution");
  m_util.setup(process, settings);
  process->setOwnerId(SOLUTION_OWNER_ID);
  process->setTerminateGroupOnCrash(false);
  process->setGroupWaitsForTermination(false);

  const ProcessPointer interactor =
      m_util.create_process(worker::PROBLEM_BIN / "interactor");
  interactor->setName("interactor");
  interactor->setOwnerId(INTERACTOR_OWNER_ID);
  interactor->setTerminateGroupOnCrash(true);

  interactor->setStream(
      0, m_util.add_notifier(NotificationStream::Protocol::PLAIN_TEXT));

  // note: ignore current_path from settings.execution()
  process->setCurrentPath(testing_path);

  m_util.set_test_files(process, settings, test, testing_path,
                        SOLUTION_OWNER_ID);

  const auto data_set = test.data_set();

  if (data_set.find("in") == data_set.end())
    BOOST_THROW_EXCEPTION(error()
                          << error::message("Test should contain \"in\" data"));
  const boost::filesystem::path test_in = testing_path / "in";
  m_util.copy_test_file(test, "in", test_in, INTERACTOR_OWNER_ID, 0400);
  interactor->setEnvironment("JUDGE_TEST_IN", test_in.string());

  if (data_set.find("out") != data_set.end()) {
    const boost::filesystem::path test_hint = testing_path / "hint";
    m_util.copy_test_file(test, "out", test_hint, INTERACTOR_OWNER_ID, 0400);
    interactor->setEnvironment("JUDGE_TEST_HINT", test_hint.string());
  }

  const boost::filesystem::path interactor_output = testing_path / "output";
  m_util.touch_test_file(interactor_output, INTERACTOR_OWNER_ID, 0600);
  interactor->setEnvironment("JUDGE_INTERACTOR_OUTPUT",
                             interactor_output.string());

  const boost::filesystem::path interactor_custom_message =
      testing_path / "custom_message";
  m_util.touch_test_file(interactor_custom_message, INTERACTOR_OWNER_ID, 0600);
  interactor->setEnvironment("JUDGE_INTERACTOR_CUSTOM_MESSAGE",
                             interactor_custom_message.string());

  const boost::filesystem::path interactor_log_path =
      testing_path / "interactor_log";
  m_util.touch_test_file(interactor_log_path, INTERACTOR_OWNER_ID, 0600);
  interactor->setStream(2, File(interactor_log_path, AccessMode::WRITE_ONLY));

  m_util.synchronized_call();
  m_util.send_test_files(result);

  problem::single::JudgeResult &judge = *result.mutable_judge();
  const bool execution_success =
      m_util.parse_result(process, *result.mutable_execution());
  const bool interactor_execution_success =
      m_util.parse_result(interactor, *result.mutable_judge()
                                           ->mutable_utilities()
                                           ->mutable_interactor()
                                           ->mutable_execution());
  const bacs::process::ExecutionResult &interactor_execution =
      judge.utilities().interactor().execution();
  judge.mutable_utilities()->mutable_interactor()->set_output(
      m_util.read_first(interactor_log_path, MAX_MESSAGE_SIZE));

  // analyze
  if (result.execution().status() ==
      bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED) {
    judge.set_status(problem::single::JudgeResult::SKIPPED);
    goto return_;
  }
  if (interactor_execution.term_sig()) goto failed_;
  judge.set_status(m_mapper->map(interactor_execution.exit_status()));
  switch (judge.status()) {
    case problem::single::JudgeResult::INSUFFICIENT_DATA:
      if (result.execution().status() != bacs::process::ExecutionResult::OK)
        judge.set_status(problem::single::JudgeResult::SKIPPED);
      break;
    case problem::single::JudgeResult::CUSTOM_FAILURE:
      judge.set_message(
          m_util.read_first(interactor_custom_message, MAX_MESSAGE_SIZE));
      break;
  }

  m_util.use_solution_file("stdout", interactor_output);
  m_util.run_checker_if_ok(*m_checker, result);

  goto return_;

failed_:
  judge.set_status(problem::single::JudgeResult::FAILED);

return_:
  return m_util.fill_status(result);
}

}  // namespace single
}  // namespace system
}  // namespace bacs
