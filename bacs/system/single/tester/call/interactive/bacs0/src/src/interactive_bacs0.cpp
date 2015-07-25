#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/tester_util.hpp>
#include <bacs/system/single/worker.hpp>

#include <boost/assert.hpp>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;

static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000);

constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

class interactive_bacs0_tester : public tester {
 public:
  interactive_bacs0_tester(
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
    return std::make_unique<interactive_bacs0_tester>(
        container, std::move(mapper), std::move(checker));
  }

 private:
  tester_util m_util;
  const result_mapper_uptr m_mapper;
  const checker_uptr m_checker;
  builder_ptr m_builder;
  executable_ptr m_solution;
};

BUNSAN_PLUGIN_AUTO_REGISTER(tester, interactive_bacs0_tester,
                            interactive_bacs0_tester::make_instance)

bool interactive_bacs0_tester::test(
    const problem::single::process::Settings &settings,
    const test::storage::test &test, problem::single::TestResult &result) {
  m_util.reset();

  const boost::filesystem::path testing_path =
      m_util.create_directory("testing", unistd::access::Id{0, 0}, 0555);
  const boost::filesystem::path logging_path =
      m_util.create_directory("logging", unistd::access::Id{0, 0}, 0555);

  const ProcessPointer process = m_solution->create(
      m_util.process_group(), settings.execution().argument());
  m_util.setup(process, settings);
  process->setTerminateGroupOnCrash(false);

  const ProcessPointer interactor =
      m_util.create_process(worker::PROBLEM_BIN / "interactor");
  interactor->setTerminateGroupOnCrash(false);

  const ProcessPointer pipectl =
      m_util.create_process("yandex_contest_invoker_flowctl_pipectl");
  pipectl->setTerminateGroupOnCrash(false);

  m_util.set_test(test);

  // connect interactor and solution
  {
    const Pipe pipe01 = m_util.create_pipe();
    const Pipe pipe02 = m_util.create_pipe();
    process->setStream(0, pipe01.readEnd());
    pipectl->setStream(1, pipe01.writeEnd());
    pipectl->setStream(0, pipe02.readEnd());
    interactor->setStream(1, pipe02.writeEnd());

    const Pipe pipe1 = m_util.create_pipe();
    interactor->setStream(0, pipe1.readEnd());
    process->setStream(1, pipe1.writeEnd());
  }

  const auto data_set = test.data_set();

  if (data_set.find("in") == data_set.end())
    BOOST_THROW_EXCEPTION(
        error() << error::message("Test should contain \"in\" data"));
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

  // execution
  process->setOwnerId(SOLUTION_OWNER_ID);
  interactor->setOwnerId(INTERACTOR_OWNER_ID);
  // note: ignore current_path from settings.execution()
  // note: current path is not used
  // note: arguments is already set
  // execute
  m_util.synchronized_call();
  const Process::Result process_result = process->result();
  const Process::Result interactor_result = interactor->result();
  const Process::Result pipectl_result = pipectl->result();
  // fill result
  const bool execution_success =
      m_util.parse_result(process_result, *result.mutable_execution());

  const bool interactor_execution_success =
      m_util.parse_result(interactor_result, *result.mutable_judge()
                                                  ->mutable_utilities()
                                                  ->mutable_interactor()
                                                  ->mutable_execution());
  // TODO interactor's output

  {
    problem::single::JudgeResult::AuxiliaryUtility &utility =
        *result.mutable_judge()->mutable_utilities()->add_auxiliary();
    utility.set_name("pipectl [interactor -> solution]");
    m_util.parse_result(pipectl_result,
                        *utility.mutable_utility()->mutable_execution());
  }

  // analyze
  const bacs::process::ExecutionResult &interactor_execution =
      result.judge().utilities().interactor().execution();

  problem::single::JudgeResult &judge = *result.mutable_judge();

  switch (interactor_execution.status()) {
    case bacs::process::ExecutionResult::OK:
      judge.set_status(problem::single::JudgeResult::OK);
      break;
    case bacs::process::ExecutionResult::ABNORMAL_EXIT:
      if (interactor_execution.has_exit_status()) {
        judge.set_status(m_mapper->map(interactor_execution.exit_status()));
        switch (judge.status()) {
          case problem::single::JudgeResult::INSUFFICIENT_DATA:
            if (result.execution().status() !=
                bacs::process::ExecutionResult::OK)
              judge.set_status(problem::single::JudgeResult::SKIPPED);
            break;
          case problem::single::JudgeResult::CUSTOM_FAILURE:
            judge.set_message(m_util.read_first(interactor_custom_message,
                                                MAX_MESSAGE_SIZE));
            break;
        }
      } else {
        judge.set_status(problem::single::JudgeResult::FAILED);
      }
      break;
    case bacs::process::ExecutionResult::MEMORY_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::TIME_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::OUTPUT_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::TERMINATED_BY_SYSTEM:
    case bacs::process::ExecutionResult::FAILED:
      judge.set_status(problem::single::JudgeResult::FAILED);
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
