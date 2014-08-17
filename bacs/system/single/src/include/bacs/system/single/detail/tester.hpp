#pragma once

#include <bacs/system/file.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/detail/file.hpp>
#include <bacs/system/single/testing.hpp>

#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <bunsan/filesystem/fstream.hpp>
#include <bunsan/tempfile.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/noncopyable.hpp>

#include <utility>

namespace bacs{namespace system{namespace single{namespace detail
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    class tester_mkdir_hook: private boost::noncopyable
    {
    public:
        tester_mkdir_hook(const ContainerPointer &container,
                          const boost::filesystem::path &testing_root)
        {
            const boost::filesystem::path path =
                container->filesystem().keepInRoot(testing_root);
            // TODO permissions
            boost::filesystem::create_directories(path);
        }
    };

    class tester: private tester_mkdir_hook
    {
    public:
        struct receive_type
        {
            std::string id;
            boost::filesystem::path path;
            bacs::file::Range range;
        };

    public:
        tester(const ContainerPointer &container,
               const boost::filesystem::path &testing_root):
            tester_mkdir_hook(container, testing_root),
            m_container(container),
            m_testing_root(testing_root),
            m_container_testing_root(
                m_container->filesystem().keepInRoot(m_testing_root)
            )
        {
            reset();
        }

        ContainerPointer container() { return m_container; }
        ProcessGroupPointer process_group() { return m_process_group; }

        void reset()
        {
            m_tmpdir = bunsan::tempfile::directory_in_directory(
                m_container_testing_root
            );
            m_current_root =
                m_container->filesystem().containerPath(m_tmpdir.path());
            m_container->filesystem().setOwnerId(m_current_root, {0, 0});
            m_container->filesystem().setMode(m_current_root, 0500);

            m_process_group = m_container->createProcessGroup();
            m_receive.clear();
            m_test_files.clear();
            m_solution_files.clear();
            m_container_solution_files.clear();
        }

        template <typename ... Args>
        ProcessPointer create_process(Args &&...args)
        {
            return m_process_group->createProcess(std::forward<Args>(args)...);
        }

        boost::filesystem::path create_directory(
            const boost::filesystem::path &filename,
            const unistd::access::Id &owner_id,
            const mode_t mode=0700)
        {
            const boost::filesystem::path path = m_current_root / filename;
            boost::filesystem::create_directory(
                m_container->filesystem().keepInRoot(path)
            );
            m_container->filesystem().setOwnerId(path, owner_id);
            m_container->filesystem().setMode(path, mode);
            return path;
        }

        void setup(const ProcessPointer &process,
                   const problem::single::settings::ProcessSettings &settings)
        {
            process::setup(
                m_process_group,
                process,
                settings.resource_limits()
            );
        }

        void set_test_files(
            const ProcessPointer &process,
            const problem::single::settings::ProcessSettings &settings,
            const test &test_,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mask=0777)
        {
            add_test_files(settings, test_, destination, owner_id, mask);
            set_redirections(process, settings);
        }

        void add_test_files(
            const problem::single::settings::ProcessSettings &settings,
            const test &test_,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mask=0777)
        {
            BOOST_ASSERT(destination.is_absolute());

            for (const std::string &data_id: test_.data_set())
                m_test_files[data_id] = test_.location(data_id);

            for (const problem::single::settings::File &file: settings.file())
            {
                if (m_solution_files.find(file.id()) != m_solution_files.end())
                    BOOST_THROW_EXCEPTION(
                        error() <<
                        error::message("Duplicate file ids."));
                // note: strip to filename
                const boost::filesystem::path location =
                    m_solution_files[file.id()] =
                    destination / (
                        file.has_path() ?
                            detail::file::to_path(file.path()).filename() :
                            boost::filesystem::unique_path());
                m_container_solution_files[file.id()] =
                    m_container->filesystem().keepInRoot(location);
                const mode_t mode = detail::file::mode(file.permission()) & mask;
                if (file.has_init())
                    copy_test_file(test_, file.init(), location, owner_id, mode);
                else
                    touch_test_file(location, owner_id, mode);
                if (file.has_receive())
                    m_receive.push_back({file.id(), location, file.receive()});
            }
        }

        void set_redirections(
            const ProcessPointer &process,
            const problem::single::settings::ProcessSettings &settings)
        {
            for (const auto &redirection: settings.execution().redirection())
            {
                const auto iter = m_solution_files.find(redirection.file_id());
                if (iter == m_solution_files.end())
                    BOOST_THROW_EXCEPTION(
                        error() <<
                        error::message("Invalid file id."));
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
        }

        void copy_test_file(
            const test &test_,
            const std::string &data_id,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mode)
        {
            test_.copy(
                data_id,
                m_container->filesystem().keepInRoot(destination)
            );
            m_container->filesystem().setOwnerId(destination, owner_id);
            m_container->filesystem().setMode(destination, mode);
        }

        void touch_test_file(
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mode)
        {
            detail::file::touch(
                m_container->filesystem().keepInRoot(destination)
            );
            m_container->filesystem().setOwnerId(destination, owner_id);
            m_container->filesystem().setMode(destination, mode);
        }

        std::string read(const boost::filesystem::path &path,
                         const bacs::file::Range &range)
        {
            return bacs::system::file::read(
                m_container->filesystem().keepInRoot(path),
                range
            );
        }

        std::string read_first(
            const boost::filesystem::path &path,
            const std::uint64_t size)
        {
            return bacs::system::file::read_first(
                m_container->filesystem().keepInRoot(path),
                size
            );
        }

        std::string read_last(
            const boost::filesystem::path &path,
            const std::uint64_t size)
        {
            return bacs::system::file::read_last(
                m_container->filesystem().keepInRoot(path),
                size
            );
        }

        void send_file(
            bacs::problem::single::result::TestResult &result,
            const std::string &id,
            const bacs::file::Range &range,
            const boost::filesystem::path &path)
        {
            problem::single::result::File &file = *result.add_file();
            file.set_id(id);
            *file.mutable_data() = bacs::system::file::read(
                m_container->filesystem().keepInRoot(path),
                range
            );
        }

        void send_file_if_requested(
            bacs::problem::single::result::TestResult &result,
            const problem::single::settings::File &file,
            const boost::filesystem::path &path)
        {
            if (!file.has_receive())
                return;
            send_file(result, file.id(), file.receive(), path);
        }

        void send_test_files(
            bacs::problem::single::result::TestResult &result)
        {
            for (const receive_type &r: m_receive)
                send_file(result, r.id, r.range, r.path);
        }

        Pipe create_pipe()
        {
            return m_process_group->createPipe();
        }

        Pipe::End add_notifier(const NotificationStream::Protocol protocol)
        {
            return m_process_group->addNotifier(protocol);
        }

        void redirect(
            const ProcessPointer &from,
            const int from_fd,
            const ProcessPointer &to,
            const int to_fd)
        {
            const Pipe pipe = create_pipe();
            from->setStream(from_fd, pipe.writeEnd());
            to->setStream(to_fd, pipe.readEnd());
        }

        ProcessGroup::Result synchronized_call()
        {
            return m_process_group->synchronizedCall();
        }

        bool parse_result(
            const Process::Result &process_result,
            bacs::process::ExecutionResult &result)
        {
            return process::parse_result(
                m_process_group->result(),
                process_result,
                result
            );
        }

        bool parse_result(
            const ProcessPointer &process,
            bacs::process::ExecutionResult &result)
        {
            return parse_result(process->result(), result);
        }

        void use_solution_file(
            const std::string &data_id,
            const boost::filesystem::path &path)
        {
            m_solution_files[data_id] = path;
            m_container_solution_files[data_id] =
                m_container->filesystem().keepInRoot(path);
        }

        bool ok(const problem::single::result::TestResult &result) const
        {
            return
                result.execution().status() ==
                    bacs::process::ExecutionResult::OK &&
                result.judge().status() ==
                    problem::single::result::Judge::OK;
        }

        bool run_checker_if_ok(
            checker &checker_,
            problem::single::result::TestResult &result)
        {
            if (!ok(result))
                return false;
            return checker_.check(
                m_test_files,
                m_container_solution_files,
                *result.mutable_judge()
            );
        }

        bool fill_status(problem::single::result::TestResult &result)
        {
            result.set_status(ok(result) ?
                problem::single::result::TestResult::OK :
                problem::single::result::TestResult::FAILED
            );
            return result.status() == problem::single::result::TestResult::OK;
        }

    private:
        const ContainerPointer m_container;
        const boost::filesystem::path m_testing_root;
        const boost::filesystem::path m_container_testing_root;
        bunsan::tempfile m_tmpdir;
        boost::filesystem::path m_current_root;

        ProcessGroupPointer m_process_group;

        std::vector<receive_type> m_receive;
        file_map m_test_files,
                 m_solution_files,
                 m_container_solution_files;
    };
}}}}
