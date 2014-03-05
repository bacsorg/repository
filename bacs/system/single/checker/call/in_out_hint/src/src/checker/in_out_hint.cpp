#include <bacs/system/single/checker.hpp>

#include <bacs/system/single/testing.hpp>

#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    const unistd::access::Id ownerId(1000, 1000);

    class checker::impl
    {
    public:
        ContainerPointer container;
    };

    checker::checker(const yandex::contest::invoker::ContainerPointer &container): pimpl(new impl)
    {
        BOOST_ASSERT(container);
        pimpl->container = container;
    }

    checker::~checker() { /* implicit destructor */ }

    bool checker::check(
        const file_map &test_files,
        const file_map &solution_files,
        problem::single::result::Judge &result)
    {
        const boost::filesystem::path checking_path = testing::PROBLEM_ROOT;
        const boost::filesystem::path checking_log = checking_path / "log";

        const boost::filesystem::path in = test_files.at("in");
        const boost::filesystem::path out = solution_files.at("stdout");
        const auto hint_iter = test_files.find("out");
        const std::string hint_arg =
            hint_iter == test_files.end() ?
                "/dev/null" :
                "hint";
        BOOST_ASSERT_MSG(
            (hint_iter == test_files.end() && test_files.size() == 1) ||
            (hint_iter != test_files.end() && test_files.size() == 2),
            "keys(test_files) == {'in', 'out'} || keys(test_files) == {'in'}"
        );
        const ProcessGroupPointer process_group = pimpl->container->createProcessGroup();
        // TODO process_group->setResourceLimits()
        const ProcessPointer process = process_group->createProcess(testing::PROBLEM_BIN / "checker");
        pimpl->container->filesystem().push(in, checking_path / "in", {0, 0}, 0444);
        pimpl->container->filesystem().push(out, checking_path / "out", {0, 0}, 0444);
        if (hint_iter != test_files.end())
        {
            pimpl->container->filesystem().push(
                hint_iter->second, checking_path / "hint", {0, 0}, 0444);
        }
        process->setArguments(process->executable(), "in", "out", hint_arg);
        process->setCurrentPath(checking_path);
        process->setStream(2, FDAlias(1));
        process->setStream(1, File(checking_log, AccessMode::WRITE_ONLY));
        // TODO process->setResourceLimits()
        // execute
        const ProcessGroup::Result process_group_result =
            process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        const bool success = process::parse_result(
            process_group_result,
            process_result,
            *result.mutable_utilities()->mutable_checker()->mutable_execution()
        );
        const bacs::process::ExecutionResult &checker_execution =
            result.utilities().checker().execution();
        switch (checker_execution.status())
        {
        case bacs::process::ExecutionResult::OK:
            result.set_status(problem::single::result::Judge::OK);
            break;
        case bacs::process::ExecutionResult::ABNORMAL_EXIT:
            if (checker_execution.has_exit_status())
            {
                switch (checker_execution.exit_status())
                {
                case 0:
                    result.set_status(problem::single::result::Judge::OK);
                    break;
                case 2:
                case 5:
                    result.set_status(problem::single::result::Judge::WRONG_ANSWER);
                    break;
                case 4:
                    result.set_status(problem::single::result::Judge::PRESENTATION_ERROR);
                    break;
                default:
                    result.set_status(problem::single::result::Judge::FAILED);
                }
            }
            else
            {
                result.set_status(problem::single::result::Judge::FAILED);
            }
            break;
        case bacs::process::ExecutionResult::MEMORY_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::TIME_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::OUTPUT_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::TERMINATED_BY_SYSTEM:
        case bacs::process::ExecutionResult::FAILED:
            result.set_status(problem::single::result::Judge::FAILED);
            break;
        }
        return result.status() == problem::single::result::Judge::OK;
    }
}}}
