#include <bacs/system/single/checker.hpp>

#include <bacs/system/single/worker.hpp>

#include <bacs/system/file.hpp>
#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;

static const boost::filesystem::path checking_path = "/checking";
static const unistd::access::Id owner_id(1000, 1000);

constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

class in_out_hint_checker : public checker {
 public:
  in_out_hint_checker(
      const yandex::contest::invoker::ContainerPointer &container,
      result_mapper_uptr mapper)
      : m_container(container), m_mapper(std::move(mapper)) {
    BOOST_ASSERT(m_container);
    BOOST_ASSERT(m_mapper);
  }
  bool check(const file_map &test_files, const file_map &solution_files,
             problem::single::JudgeResult &result) override;
  static checker_uptr make_instance(
      const yandex::contest::invoker::ContainerPointer &container,
      result_mapper_uptr mapper) {
    return std::make_unique<in_out_hint_checker>(container, std::move(mapper));
  }

 private:
  ContainerPointer m_container;
  result_mapper_uptr m_mapper;
};

BUNSAN_PLUGIN_AUTO_REGISTER(checker, in_out_hint_checker,
                            in_out_hint_checker::make_instance)

bool in_out_hint_checker::check(const file_map &test_files,
                                const file_map &solution_files,
                                problem::single::JudgeResult &result) {
  // files
  const boost::filesystem::path container_checking_path =
      m_container->filesystem().keepInRoot(checking_path);
  boost::filesystem::create_directories(container_checking_path);
  const boost::filesystem::path checking_log = checking_path / "log";
  const boost::filesystem::path container_checking_log =
      m_container->filesystem().keepInRoot(checking_log);

  // permissions
  m_container->filesystem().setOwnerId(checking_path, owner_id);
  m_container->filesystem().setMode(checking_path, 0500);
  const boost::filesystem::path in = test_files.at("in");
  const boost::filesystem::path out = solution_files.at("stdout");
  const auto hint_iter = test_files.find("out");
  const std::string hint_arg =
      hint_iter == test_files.end() ? "/dev/null" : "hint";
  BOOST_ASSERT_MSG(
      (hint_iter == test_files.end() && test_files.size() == 1) ||
          (hint_iter != test_files.end() && test_files.size() == 2),
      "keys(test_files) == {'in', 'out'} || keys(test_files) == {'in'}");
  m_container->filesystem().push(in, checking_path / "in", owner_id, 0600);
  m_container->filesystem().push(out, checking_path / "out", owner_id, 0600);
  if (hint_iter != test_files.end()) {
    m_container->filesystem().push(hint_iter->second, checking_path / "hint",
                                   owner_id, 0600);
  }

  const ProcessGroupPointer process_group = m_container->createProcessGroup();
  // TODO process_group->setResourceLimits()

  // execution
  const ProcessPointer process =
      process_group->createProcess(worker::PROBLEM_BIN / "checker");
  process->setOwnerId(owner_id);
  process->setArguments(process->executable(), "in", "out", hint_arg);
  process->setCurrentPath(checking_path);
  process->setStream(2, FdAlias(1));
  process->setStream(1, File(checking_log, AccessMode::WRITE_ONLY));
  // TODO process->setResourceLimits()

  // execute
  const ProcessGroup::Result process_group_result =
      process_group->synchronizedCall();

  // process result
  const Process::Result process_result = process->result();
  process::parse_result(
      process_group_result, process_result,
      *result.mutable_utilities()->mutable_checker()->mutable_execution());
  result.mutable_utilities()->mutable_checker()->set_output(
      bacs::system::file::read_first(container_checking_log, MAX_MESSAGE_SIZE));
  const bacs::process::ExecutionResult &checker_execution =
      result.utilities().checker().execution();
  switch (checker_execution.status()) {
    case bacs::process::ExecutionResult::OK:
      result.set_status(problem::single::JudgeResult::OK);
      break;
    case bacs::process::ExecutionResult::ABNORMAL_EXIT:
      if (checker_execution.has_exit_status()) {
        result.set_status(m_mapper->map(checker_execution.exit_status()));
      } else {
        result.set_status(problem::single::JudgeResult::FAILED);
      }
      break;
    case bacs::process::ExecutionResult::MEMORY_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::TIME_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::OUTPUT_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED:
    case bacs::process::ExecutionResult::TERMINATED_BY_SYSTEM:
    case bacs::process::ExecutionResult::FAILED:
      result.set_status(problem::single::JudgeResult::FAILED);
      break;
  }
  return result.status() == problem::single::JudgeResult::OK;
}

}  // namespace single
}  // namespace system
}  // namespace bacs
