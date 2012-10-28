#include "bacs/single/testing.hpp"
#include "bacs/single/detail/file.hpp"

#include "bacs/single/api/pb/resource.pb.h"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    bool testing::test(const api::pb::settings::ProcessSettings &settings,
                       const std::string &test_id,
                       api::pb::result::TestResult &result)
    {
        m_intermediate.set_test_id(test_id);
        send_intermediate();
        // TODO
        const ProcessGroupPointer process_group = m_container->createProcessGroup();
        const ProcessPointer process;// = process_group->createProcess(); // TODO somehow with executable, arguments initialized
        setup(settings.resource_limits(), process_group, process);
        const boost::filesystem::path current_path = "/tmp/testiing";
        const unistd::access::Id owner{1000, 1000};
        boost::filesystem::create_directories(m_container->filesystem().keepInRoot(current_path));
        // Files
        std::unordered_map<std::string, boost::filesystem::path> files;
        struct receive_type
        {
            boost::filesystem::path path;
            api::pb::settings::Range range;
        };
        std::vector<receive_type> receive;
        for (const api::pb::settings::File &file: process)
        {
            if (files.find(file.id()) != files.end())
                BOOST_THROW_EXCEPTION(error() << error::message("Duplicate file ids."));
            const boost::filesystem::path location = files[file.id()] =
                "/" / current_path / file::detail::to_path(file.path()).filename(); // note: strip to filename
            const boost::filesystem::path path = m_container->filesystem().keepInRoot(location);
            if (file.has_init())
                m_tests->create(test_id, file.init(), path);
            else
                detail::file::touch(path);
            m_container->filesystem().setOwner(current_path, owner);
            m_container->filesystem().setMode(current_path, 0400);
            m_container->fileststem().setOwner(location, owner);
            m_container->filesystem().setMode(location, detail::file::mode(file.permissions()));
            if (file.has_receive())
                receive.push_back({path, file.receive()});
        }
        // Execution
        process->setOwnerId(owner);
        // note: ignore current_path from settings.execution()
        process->setCurrentPath(current_path);
        {
            ProcessArguments arguments = process->arguments();
            arguments.insert(arguments.end(), begin(settings.execution().arguments()),
                             end(settings.execution().arguments()));
            process->setArguments(arguments);
        }
        for (const api::pb::settings::Execution::Redirection &redirection: settings.execution().redirections())
        {
            const auto iter = files.find(redirection.file_id());
            if (iter == files.end())
                BOOST_THROW_EXCEPTION(error() << error::message("Invalid file id."));
            const boost::filesystem::path path = iter->second;
            switch (redirection.stream())
            {
            case api::pb::settings::Execution::STDIN:
                process->setStream(0, File(path, AccessMode::READ_ONLY));
                break;
            case api::pb::settings::Execution::STDOUT:
                process->setStream(1, File(path, AccessMode::WRITE_ONLY));
                break;
            case api::pb::settings::Execution::STDERR:
                process->setStream(2, File(path, AccessMode::WRITE_ONLY));
                break;
            }
        }
        // Execute
        const ProcessGroup::Result process_group_result = process_group->syncrhonizedCall();
        const Process::Result process_result = process->result();
        // fill result
        result.set_id(test_id);
        // TODO execution result
        // TODO files
        if (process_result)
        {
            file_map test_files, solution_files;
            // TODO initialize files
            const checker::result checker_result = m_checker->check(test_files, solution_files);
            // fill checker result
            api::pb::result::TestResult::Checking &checking = *result.mutable_checking();
            // TODO
        }
    }
}}
