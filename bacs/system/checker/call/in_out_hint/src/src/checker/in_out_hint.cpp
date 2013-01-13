#include "bacs/single/checker.hpp"

#include "yandex/contest/invoker/All.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    const unistd::access::Id ownerId(1000, 1000);

    const boost::filesystem::path checking_path = "/tmp/checking";
    const boost::filesystem::path checking_mount_path = checking_path / "package";
    const boost::filesystem::path checking_log = checking_path / "log";

    class checker::impl
    {
    public:
        ContainerPointer container;
    };

    checker::checker(): pimpl(new impl)
    {
        ContainerConfig cfg = ContainerConfig::fromEnvironment();
        if (!cfg.lxcConfig.mount)
            cfg.lxcConfig.mount = yandex::contest::system::lxc::MountConfig();
        if (!cfg.lxcConfig.mount->entries)
            cfg.lxcConfig.mount->entries = std::vector<unistd::MountEntry>();
        cfg.lxcConfig.mount->entries->push_back(
            unistd::MountEntry::bindRO(
                boost::filesystem::current_path(),
                checking_mount_path));
        filesystem::Directory checking_dir;
        checking_dir.path = checking_path;
        checking_dir.mode = 0555; // rx
        cfg.filesystemConfig.createFiles.push_back(filesystem::CreateFile(checking_dir));
        pimpl->container = Container::create(cfg);
    }

    checker::~checker()
    {
        delete pimpl;
    }

    checker::result checker::check(const file_map &test_files, const file_map &solution_files)
    {
        const boost::filesystem::path in = test_files.at("in"),
            out = solution_files.at("stdout"), hint = test_files.at("out");
        BOOST_ASSERT_MSG(test_files.size() == 2, "keys(test_files) == {'in', 'out'}");
        result result_;
        const ProcessGroupPointer process_group = pimpl->container->createProcessGroup();
        // TODO process_group->setResourceLimits()
        const ProcessPointer process = process_group->createProcess(checking_mount_path / "bin" / "checker");
        pimpl->container->filesystem().push(in, checking_path / "in", {0, 0}, 0444);
        pimpl->container->filesystem().push(out, checking_path / "out", {0, 0}, 0444);
        pimpl->container->filesystem().push(hint, checking_path / "hint", {0, 0}, 0444);
        process->setArguments(process->executable(), "in", "out", "hint");
        process->setCurrentPath(checking_path);
        process->setStream(2, FDAlias(1));
        process->setStream(1, File(checking_log, AccessMode::WRITE_ONLY));
        // TODO process->setResourceLimits()
        // execute
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
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
                        result_.status = checker::result::FAIL_TEST;
                    }
                    // TODO load log
                    result_.message = "TODO";
                }
                else
                {
                    result_.status = checker::result::FAIL_TEST;
                    result_.message = "CRASH";
                }
                break;
            case Process::Result::CompletionStatus::TERMINATED_BY_SYSTEM:
            case Process::Result::CompletionStatus::MEMORY_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::USER_TIME_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::OUTPUT_LIMIT_EXCEEDED:
            case Process::Result::CompletionStatus::START_FAILED:
            case Process::Result::CompletionStatus::STOPPED:
                result_.status = checker::result::FAIL_TEST;
                result_.message = boost::lexical_cast<std::string>(process_result.completionStatus);
                break;
            }
            break;
        case ProcessGroup::Result::CompletionStatus::REAL_TIME_LIMIT_EXCEEDED:
        case ProcessGroup::Result::CompletionStatus::STOPPED:
            result_.status = checker::result::FAIL_TEST;
            result_.message = boost::lexical_cast<std::string>(process_result.completionStatus);
            break;
        }
        return result_;
    }
}}
