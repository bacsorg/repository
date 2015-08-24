#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/tester_util.hpp>
#include <bacs/system/single/worker.hpp>

#include <yandex/contest/invoker/flowctl/interactive/SimpleBroker.hpp>

#include <boost/assert.hpp>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;
using Broker = flowctl::interactive::SimpleBroker;

static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000);

constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

class interactive_invoker_flowctl_interactive_simple_tester : public tester {
 public:
  interactive_invoker_flowctl_interactive_simple_tester(
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
    return std::make_unique<
        interactive_invoker_flowctl_interactive_simple_tester>(
        container, std::move(mapper), std::move(checker));
  }

 private:
  tester_util m_util;
  const result_mapper_uptr m_mapper;
  const checker_uptr m_checker;
  builder_ptr m_builder;
  executable_ptr m_solution;
};

BUNSAN_PLUGIN_AUTO_REGISTER(
    tester, interactive_invoker_flowctl_interactive_simple_tester,
    interactive_invoker_flowctl_interactive_simple_tester::make_instance)

bool interactive_invoker_flowctl_interactive_simple_tester::test(
    const problem::single::process::Settings &settings,
    const test::storage::test &test, problem::single::TestResult &result) {
  m_util.reset();

  const boost::filesystem::path testing_path =
      m_util.create_directory("testing", unistd::access::Id{0, 0}, 0555);
  const boost::filesystem::path logging_path =
      m_util.create_directory("logging", unistd::access::Id{0, 0}, 0555);

  const boost::filesystem::path judge_to_solution_log =
      logging_path / "judge_to_solution";
  m_util.touch_test_file(judge_to_solution_log, INTERACTOR_OWNER_ID, 0600);

  const boost::filesystem::path solution_to_judge_log =
      logging_path / "solution_to_judge";
  m_util.touch_test_file(solution_to_judge_log, INTERACTOR_OWNER_ID, 0600);

  const ProcessPointer solution = m_solution->create(
      m_util.process_group(), settings.execution().argument());
  solution->setName("solution");
  m_util.setup(solution, settings);
  solution->setTerminateGroupOnCrash(false);

  const ProcessPointer interactor =
      m_util.create_process(worker::PROBLEM_BIN / "interactor");
  interactor->setName("interactor");
  interactor->setTerminateGroupOnCrash(false);

  const ProcessPointer broker = m_util.create_process(
      "yandex_contest_invoker_flowctl_interactive_simple_broker");
  broker->setArguments(broker->executable(),
                       "--notifier", "3",
                       "--interactor-source", "4",
                       "--interactor-sink", "5",
                       "--solution-source", "6",
                       "--solution-sink", "7",
                       "--output-limit-bytes", "67108864",              // FIXME
                       "--termination-real-time-limit-millis", "4000",  // FIXME
                       "--dump-judge", judge_to_solution_log.string(),
                       "--dump-solution", solution_to_judge_log.string());
  broker->setName("broker");

  m_util.set_test(test);

  broker->setStream(3, m_util.add_notifier());

  // connect interactor and broker
  {
    const Pipe source = m_util.create_pipe();
    const Pipe sink = m_util.create_pipe();

    broker->setStream(5, sink.writeEnd());
    interactor->setStream(0, sink.readEnd());

    interactor->setStream(1, source.writeEnd());
    broker->setStream(4, source.readEnd());
  }

  // connect solution and broker
  {
    const Pipe source = m_util.create_pipe();
    const Pipe sink = m_util.create_pipe();

    broker->setStream(7, sink.writeEnd());
    solution->setStream(0, sink.readEnd());

    solution->setStream(1, source.writeEnd());
    broker->setStream(6, source.readEnd());
  }

  const boost::filesystem::path broker_log = logging_path / "broker_log";
  m_util.touch_test_file(broker_log, INTERACTOR_OWNER_ID, 0600);
  broker->setStream(2, File(broker_log, AccessMode::WRITE_ONLY));

  const boost::filesystem::path interactor_log =
      logging_path / "interactor_log";
  m_util.touch_test_file(interactor_log, INTERACTOR_OWNER_ID, 0600);
  interactor->setStream(2, File(interactor_log, AccessMode::WRITE_ONLY));

  // interactor files
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
  broker->setOwnerId(INTERACTOR_OWNER_ID);
  interactor->setOwnerId(INTERACTOR_OWNER_ID);
  solution->setOwnerId(SOLUTION_OWNER_ID);
  // note: ignore current_path from settings.execution()
  // note: current path is not used
  // note: arguments is already set

  m_util.synchronized_call();
  const Process::Result solution_result = solution->result();
  const Process::Result interactor_result = interactor->result();
  const Process::Result broker_result = broker->result();

  // fill result
  problem::single::JudgeResult &judge = *result.mutable_judge();

  m_util.parse_result(solution_result, *result.mutable_execution());
  for (const problem::single::process::File &file : settings.file()) {
    if (file.id() == "stdin") {
      m_util.send_file_if_requested(result, file, judge_to_solution_log);
    } else if (file.id() == "stdout") {
      m_util.send_file_if_requested(result, file, solution_to_judge_log);
    }
  }

  // utilities
  const bool interactor_execution_success = m_util.parse_result(
      interactor_result,
      *judge.mutable_utilities()->mutable_interactor()->mutable_execution());
  const bacs::process::ExecutionResult &interactor_execution =
      result.judge().utilities().interactor().execution();
  judge.mutable_utilities()->mutable_interactor()->set_output(
      m_util.read_first(interactor_log, MAX_MESSAGE_SIZE));

  problem::single::JudgeResult::AuxiliaryUtility &broker_utility =
      *judge.mutable_utilities()->add_auxiliary();
  broker_utility.set_name("broker");
  const bool broker_execution_success = m_util.parse_result(
      broker_result, *broker_utility.mutable_utility()->mutable_execution());
  const bacs::process::ExecutionResult &broker_execution =
      broker_utility.utility().execution();
  broker_utility.mutable_utility()->set_output(
      m_util.read_first(broker_log, MAX_MESSAGE_SIZE));

  // analyze
  if (result.execution().status() ==
      bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED) {
    judge.set_status(problem::single::JudgeResult::SKIPPED);
    goto return_;
  }
  if (broker_execution.term_sig()) goto failed_;
  switch (broker_execution.status()) {
    case bacs::process::ExecutionResult::OK:
    case bacs::process::ExecutionResult::ABNORMAL_EXIT:
      switch (broker_execution.exit_status()) {
        case Broker::SOLUTION_TERMINATION_REAL_TIME_LIMIT_EXCEEDED:
          if (interactor_execution.term_sig()) {
            judge.set_status(problem::single::JudgeResult::
                                 TERMINATION_REAL_TIME_LIMIT_EXCEEDED);
          } else if (m_mapper->map(interactor_execution.exit_status()) ==
                     problem::single::JudgeResult::OK) {
            if (result.execution().status() ==
                bacs::process::ExecutionResult::OK) {
              judge.set_status(problem::single::JudgeResult::
                                   TERMINATION_REAL_TIME_LIMIT_EXCEEDED);
            } else {
              judge.set_status(problem::single::JudgeResult::SKIPPED);
            }
          } else {
            goto broker_ok_;
          }
          break;
        case Broker::OK:
        broker_ok_:
          if (interactor_execution.term_sig()) goto failed_;
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
          break;
        case Broker::SOLUTION_OUTPUT_LIMIT_EXCEEDED:
          judge.set_status(
              problem::single::JudgeResult::OUTPUT_LIMIT_EXCEEDED);
          break;
        case Broker::SOLUTION_EXCESS_DATA:
          judge.set_status(problem::single::JudgeResult::EXCESS_DATA);
          break;
        case Broker::FAILED:
        case Broker::INTERACTOR_OUTPUT_LIMIT_EXCEEDED:
        case Broker::INTERACTOR_TERMINATION_REAL_TIME_LIMIT_EXCEEDED:
        case Broker::INTERACTOR_EXCESS_DATA:
        default:
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
