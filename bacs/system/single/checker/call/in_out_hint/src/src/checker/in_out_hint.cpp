#include <bacs/system/single/checker.hpp>

#include <bacs/system/single/testing.hpp>

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

    checker::result checker::check(
        const file_map &test_files,
        const file_map &solution_files)
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
        result result_;
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
        switch (process_group_result.completionStatus)
        {
        case ProcessGroup::Result::CompletionStatus::OK:
        case ProcessGroup::Result::CompletionStatus::ABNORMAL_EXIT:
            switch (process_result.completionStatus)
            {
            case Process::Result::CompletionStatus::OK:
            case Process::Result::CompletionStatus::ABNORMAL_EXIT:
                if (process_result.exitStatus)
                {
                    // FIXME don't like hardcoded exit statuses
                    switch (process_result.exitStatus.get())
                    {
                    case 0:
                        result_.status = checker::result::OK;
                        break;
                    case 2:
                    case 5:
                        result_.status = checker::result::WRONG_ANSWER;
                        break;
                    case 4:
                        result_.status = checker::result::PRESENTATION_ERROR;
                        break;
                    default:
                        result_.status = checker::result::FAILED;
                    }
                    // TODO load message
                    // TODO load log
                }
                else
                {
                    result_.status = checker::result::FAILED;
                    // TODO load log
                }
                break;
            case Process::Result::CompletionStatus::TERMINATED_BY_SYSTEM:
            case Process::Result::CompletionStatus::MEMORY_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::TIME_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::SYSTEM_TIME_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::USER_TIME_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::OUTPUT_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::START_FAILED:
            case Process::Result::CompletionStatus::STOPPED:
                result_.status = checker::result::FAILED;
                break;
            }
            break;
        case ProcessGroup::Result::CompletionStatus::REAL_TIME_LIMIT_EXCEEDED:
        case ProcessGroup::Result::CompletionStatus::STOPPED:
            result_.status = checker::result::FAILED;
            break;
        }
        return result_;
    }
}}}
