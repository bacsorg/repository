#include "bacs/system/single/testing.hpp"
#include "bacs/system/single/error.hpp"
#include "bacs/system/single/detail/process.hpp"
#include "bacs/system/single/detail/file.hpp"
#include "bacs/system/single/detail/result.hpp"

#include "bacs/problem/single/resource.pb.h"

#include "bunsan/filesystem/fstream.hpp"
#include "bunsan/tempfile.hpp"

#include "yandex/contest/invoker/All.hpp"

#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    static const unistd::access::Id owner_id{1000, 1000};

    bool testing::build(const problem::single::task::Solution &solution)
    {
        m_intermediate.set_state(problem::single::intermediate::BUILDING);
        send_intermediate();
        m_builder = builder::instance(solution.build().builder());
        m_solution = m_builder->build(m_container,
                                      owner_id,
                                      solution.source(),
                                      solution.build().resource_limits(),
                                      *m_result.mutable_build());
        return static_cast<bool>(m_solution);
    }

    static const boost::filesystem::path testing_path = "/tmp/testing";

    bool testing::test(const problem::single::settings::ProcessSettings &settings,
                       const std::string &test_id,
                       problem::single::result::TestResult &result)
    {
        m_intermediate.set_test_id(test_id);
        send_intermediate();
        // preinitialization
        const boost::filesystem::path container_testing_path = m_container->filesystem().keepInRoot(testing_path);
        boost::filesystem::create_directories(container_testing_path);
        // initialize working directory
        const bunsan::tempfile tmpdir = bunsan::tempfile::in_dir(container_testing_path);
        BOOST_VERIFY(boost::filesystem::create_directory(tmpdir.path()));
        const boost::filesystem::path current_path = testing_path / tmpdir.path().filename();
        m_container->filesystem().setOwnerId(current_path, owner_id);
        m_container->filesystem().setMode(current_path, 0500);
        // initialize process
        const ProcessGroupPointer process_group = m_container->createProcessGroup();
        const ProcessPointer process = m_solution->create(process_group, settings.execution().arguments());
        detail::process::setup(settings.resource_limits(), process_group, process);
        // files
        file_map test_files, solution_files;
        for (const std::string &data_id: m_tests.data_set())
        {
            test_files[data_id] = m_tests.location(test_id, data_id);
        }
        struct receive_type
        {
            std::string id;
            boost::filesystem::path path;
            problem::single::settings::File::Range range;
        };
        std::vector<receive_type> receive;
        for (const problem::single::settings::File &file: settings.files())
        {
            if (solution_files.find(file.id()) != solution_files.end())
                BOOST_THROW_EXCEPTION(error() << error::message("Duplicate file ids."));
            const boost::filesystem::path location = solution_files[file.id()] =
                "/" / current_path / (file.has_path() ?
                    detail::file::to_path(file.path()).filename() /* note: strip to filename */ : boost::filesystem::unique_path());
            const boost::filesystem::path path = m_container->filesystem().keepInRoot(location);
            if (file.has_init())
                m_tests.copy(test_id, file.init(), path);
            else
                detail::file::touch(path);
            m_container->filesystem().setOwnerId(location, owner_id);
            m_container->filesystem().setMode(location, detail::file::mode(file.permissions()) & 0700);
            if (file.has_receive())
                receive.push_back({file.id(), path, file.receive()});
        }
        // execution
        process->setOwnerId(owner_id);
        // note: ignore current_path from settings.execution()
        process->setCurrentPath(current_path);
        // note: arguments is already set
        for (const problem::single::settings::Execution::Redirection &redirection: settings.execution().redirections())
        {
            const auto iter = solution_files.find(redirection.file_id());
            if (iter == solution_files.end())
                BOOST_THROW_EXCEPTION(error() << error::message("Invalid file id."));
            const boost::filesystem::path path = iter->second;
            switch (redirection.stream())
            {
            case problem::single::settings::Execution::Redirection::STDIN:
                process->setStream(0, File(path, AccessMode::READ_ONLY));
                break;
            case problem::single::settings::Execution::Redirection::STDOUT:
                process->setStream(1, File(path, AccessMode::WRITE_ONLY));
                break;
            case problem::single::settings::Execution::Redirection::STDERR:
                process->setStream(2, File(path, AccessMode::WRITE_ONLY));
                break;
            }
        }
        // execute
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        // fill result
        result.set_id(test_id);
        const bool execution_success = detail::result::parse(
            process_group_result, process_result, *result.mutable_execution());
        for (const receive_type &r: receive)
        {
            BOOST_ASSERT(boost::filesystem::exists(r.path));
            problem::single::result::TestResult::File &file = *result.add_files();
            file.set_id(r.id);
            std::string &data = *file.mutable_data();
            bunsan::filesystem::ifstream fin(r.path, std::ios::binary);
            BUNSAN_FILESYSTEM_FSTREAM_WRAP_BEGIN(fin)
            {
                switch (r.range.whence())
                {
                case problem::single::settings::File::Range::BEGIN:
                    fin.seekg(r.range.offset(), std::ios::beg);
                    break;
                case problem::single::settings::File::Range::END:
                    fin.seekg(r.range.offset(), std::ios::end);
                    break;
                }
                char buf[4096];
                while (fin && data.size() < r.range.size())
                {
                    fin.read(buf, std::min(sizeof(buf), r.range.size() - data.size()));
                    data.insert(data.end(), buf, buf + fin.gcount());
                }
            }
            BUNSAN_FILESYSTEM_FSTREAM_WRAP_END(fin)
            fin.close();
        }
        if (execution_success)
        {
            // note: solution_files paths are relative to container's root
            for (file_map::value_type &data_id_path: solution_files)
                data_id_path.second = m_container->filesystem().keepInRoot(data_id_path.second);
            const checker::result checker_result = m_checker.check(test_files, solution_files);
            // fill checker result
            problem::single::result::TestResult::Checking &checking = *result.mutable_checking();
            switch (checker_result.status)
            {
            case checker::result::OK:
                checking.set_status(problem::single::result::TestResult::Checking::OK);
                break;
            case checker::result::WRONG_ANSWER:
                checking.set_status(problem::single::result::TestResult::Checking::WRONG_ANSWER);
                break;
            case checker::result::PRESENTATION_ERROR:
                checking.set_status(problem::single::result::TestResult::Checking::PRESENTATION_ERROR);
                break;
            case checker::result::FAIL_TEST:
                checking.set_status(problem::single::result::TestResult::Checking::FAIL_TEST);
                break;
            case checker::result::FAILED:
                checking.set_status(problem::single::result::TestResult::Checking::FAILED);
                break;
            }
            // TODO change protobuffers to appropriate format
            if (checker_result.message)
                checking.set_output(checker_result.message.get());
            else
                checking.set_output("");
        }
        return result.execution().status() == problem::single::result::Execution::OK &&
            result.checking().status() == problem::single::result::TestResult::Checking::OK;
    }
}}}
