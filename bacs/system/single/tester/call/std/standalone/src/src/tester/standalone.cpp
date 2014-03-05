#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/detail/file.hpp>
#include <bacs/system/single/testing.hpp>

#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <bunsan/filesystem/fstream.hpp>
#include <bunsan/tempfile.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    static const boost::filesystem::path testing_path = "/testing";

    static const unistd::access::Id OWNER_ID(1000, 1000);

    class tester::impl
    {
    public:
        explicit impl(const ContainerPointer &container_):
            container(container_),
            checker_(container_) {}

        ContainerPointer container;
        builder_ptr builder;
        executable_ptr solution;
        checker checker_;
    };

    tester::tester(const yandex::contest::invoker::ContainerPointer &container):
        pimpl(new impl(container)) {}

    tester::~tester() { /* implicit destructor */ }

    bool tester::build(
        const bacs::process::Buildable &solution,
        bacs::process::BuildResult &result)
    {
        pimpl->builder = builder::instance(solution.build_settings().builder());
        pimpl->solution = pimpl->builder->build(
            pimpl->container,
            OWNER_ID,
            solution.source(),
            solution.build_settings().resource_limits(),
            result
        );
        return static_cast<bool>(pimpl->solution);
    }

    bool tester::test(const problem::single::settings::ProcessSettings &settings,
                      const single::test &test_,
                      problem::single::result::TestResult &result)
    {
        // preinitialization
        const boost::filesystem::path container_testing_path =
            pimpl->container->filesystem().keepInRoot(testing_path);
        boost::filesystem::create_directories(container_testing_path);
        // initialize working directory
        const bunsan::tempfile tmpdir =
            bunsan::tempfile::directory_in_directory(container_testing_path);
        const boost::filesystem::path current_path = testing_path / tmpdir.path().filename();
        pimpl->container->filesystem().setOwnerId(current_path, OWNER_ID);
        pimpl->container->filesystem().setMode(current_path, 0500);
        // initialize process
        const ProcessGroupPointer process_group = pimpl->container->createProcessGroup();
        const ProcessPointer process = pimpl->solution->create(
            process_group, settings.execution().argument());
        process::setup(process_group, process, settings.resource_limits());
        // files
        file_map test_files, solution_files;
        for (const std::string &data_id: test_.data_set())
            test_files[data_id] = test_.location(data_id);
        struct receive_type
        {
            std::string id;
            boost::filesystem::path path;
            problem::single::settings::File::Range range;
        };
        std::vector<receive_type> receive;
        for (const problem::single::settings::File &file: settings.file())
        {
            if (solution_files.find(file.id()) != solution_files.end())
                BOOST_THROW_EXCEPTION(error() << error::message("Duplicate file ids."));
            const boost::filesystem::path location = solution_files[file.id()] =
                "/" / current_path / (
                    file.has_path() ?
                        detail::file::to_path(file.path()).filename() /* note: strip to filename */ :
                        boost::filesystem::unique_path());
            const boost::filesystem::path path = pimpl->container->filesystem().keepInRoot(location);
            if (file.has_init())
                test_.copy(file.init(), path);
            else
                detail::file::touch(path);
            pimpl->container->filesystem().setOwnerId(location, OWNER_ID);
            pimpl->container->filesystem().setMode(location, detail::file::mode(file.permission()) & 0700);
            if (file.has_receive())
                receive.push_back({file.id(), path, file.receive()});
        }
        // execution
        process->setOwnerId(OWNER_ID);
        // note: ignore current_path from settings.execution()
        process->setCurrentPath(current_path);
        // note: arguments is already set
        for (const problem::single::settings::Execution::Redirection &redirection:
             settings.execution().redirection())
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
        const bool execution_success = process::parse_result(
            process_group_result, process_result, *result.mutable_execution());
        for (const receive_type &r: receive)
        {
            BOOST_ASSERT(boost::filesystem::exists(r.path));
            problem::single::result::File &file = *result.add_file();
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
                data_id_path.second = pimpl->container->filesystem().keepInRoot(data_id_path.second);
            pimpl->checker_.check(test_files, solution_files, *result.mutable_judge());
        }
        return result.execution().status() == bacs::process::ExecutionResult::OK &&
            result.judge().status() == problem::single::result::Judge::OK;
    }
}}}
