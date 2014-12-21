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
        struct error: virtual single::error {};

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

        struct reset_error: virtual error {};
        void reset()
        {
            try
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
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    reset_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct create_process_error: virtual error {};
        template <typename ... Args>
        ProcessPointer create_process(Args &&...args)
        {
            try
            {
                return m_process_group->createProcess(std::forward<Args>(args)...);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    create_process_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct create_directory_error: virtual error {};
        boost::filesystem::path create_directory(
            const boost::filesystem::path &filename,
            const unistd::access::Id &owner_id,
            const mode_t mode=0700)
        {
            try
            {
                const boost::filesystem::path path = m_current_root / filename;
                boost::filesystem::create_directory(
                    m_container->filesystem().keepInRoot(path)
                );
                m_container->filesystem().setOwnerId(path, owner_id);
                m_container->filesystem().setMode(path, mode);
                return path;
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    create_directory_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct setup_error: virtual error {};
        void setup(const ProcessPointer &process,
                   const problem::single::settings::ProcessSettings &settings)
        {
            try
            {
                process::setup(
                    m_process_group,
                    process,
                    settings.resource_limits()
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    setup_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct set_test_files_error: virtual error {};
        void set_test_files(
            const ProcessPointer &process,
            const problem::single::settings::ProcessSettings &settings,
            const test &test_,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mask=0777)
        {
            try
            {
                add_test_files(settings, test_, destination, owner_id, mask);
                set_redirections(process, settings);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    set_test_files_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct add_test_files_error: virtual error {};
        void add_test_files(
            const problem::single::settings::ProcessSettings &settings,
            const test &test_,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mask=0777)
        {
            try
            {
                BOOST_ASSERT(destination.is_absolute());

                for (const std::string &data_id: test_.data_set())
                    m_test_files[data_id] = test_.location(data_id);

                for (const problem::single::settings::File &file:
                     settings.file())
                {
                    if (m_solution_files.find(file.id()) !=
                        m_solution_files.end())
                    {
                        BOOST_THROW_EXCEPTION(
                            error() <<
                            error::message("Duplicate file ids."));
                    }
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
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    add_test_files_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct set_redirections_error: virtual error {};
        void set_redirections(
            const ProcessPointer &process,
            const problem::single::settings::ProcessSettings &settings)
        {
            try
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
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    set_redirections_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct copy_test_file_error: virtual error {};
        void copy_test_file(
            const test &test_,
            const std::string &data_id,
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mode)
        {
            try
            {
                test_.copy(
                    data_id,
                    m_container->filesystem().keepInRoot(destination)
                );
                m_container->filesystem().setOwnerId(destination, owner_id);
                m_container->filesystem().setMode(destination, mode);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    copy_test_file_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct touch_test_file_error: virtual error {};
        void touch_test_file(
            const boost::filesystem::path &destination,
            const unistd::access::Id &owner_id,
            const mode_t mode)
        {
            try
            {
                detail::file::touch(
                    m_container->filesystem().keepInRoot(destination)
                );
                m_container->filesystem().setOwnerId(destination, owner_id);
                m_container->filesystem().setMode(destination, mode);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    touch_test_file_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct read_error: virtual error {};
        std::string read(const boost::filesystem::path &path,
                         const bacs::file::Range &range)
        {
            try
            {
                return bacs::system::file::read(
                    m_container->filesystem().keepInRoot(path),
                    range
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    read_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct read_first_error: virtual error {};
        std::string read_first(
            const boost::filesystem::path &path,
            const std::uint64_t size)
        {
            try
            {
                return bacs::system::file::read_first(
                    m_container->filesystem().keepInRoot(path),
                    size
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    read_first_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct read_last_error: virtual error {};
        std::string read_last(
            const boost::filesystem::path &path,
            const std::uint64_t size)
        {
            try
            {
                return bacs::system::file::read_last(
                    m_container->filesystem().keepInRoot(path),
                    size
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    read_last_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct send_file_error: virtual error {};
        void send_file(
            bacs::problem::single::result::TestResult &result,
            const std::string &id,
            const bacs::file::Range &range,
            const boost::filesystem::path &path)
        {
            try
            {
                problem::single::result::File &file = *result.add_file();
                file.set_id(id);
                *file.mutable_data() = bacs::system::file::read(
                    m_container->filesystem().keepInRoot(path),
                    range
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    send_file_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct send_file_if_requested_error: virtual error {};
        void send_file_if_requested(
            bacs::problem::single::result::TestResult &result,
            const problem::single::settings::File &file,
            const boost::filesystem::path &path)
        {
            try
            {
                if (!file.has_receive())
                    return;
                send_file(result, file.id(), file.receive(), path);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    send_file_if_requested_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct send_test_files_error: virtual error {};
        void send_test_files(
            bacs::problem::single::result::TestResult &result)
        {
            try
            {
                for (const receive_type &r: m_receive)
                    send_file(result, r.id, r.range, r.path);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    send_test_files_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct create_pipe_error: virtual error {};
        Pipe create_pipe()
        {
            try
            {
                return m_process_group->createPipe();
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    create_pipe_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct add_notifier_error: virtual error {};
        Pipe::End add_notifier(const NotificationStream::Protocol protocol)
        {
            try
            {
                return m_process_group->addNotifier(protocol);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    add_notifier_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct redirect_error: virtual error {};
        void redirect(
            const ProcessPointer &from,
            const int from_fd,
            const ProcessPointer &to,
            const int to_fd)
        {
            try
            {
                const Pipe pipe = create_pipe();
                from->setStream(from_fd, pipe.writeEnd());
                to->setStream(to_fd, pipe.readEnd());
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    redirect_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct synchronized_call_error: virtual error {};
        ProcessGroup::Result synchronized_call()
        {
            try
            {
                return m_process_group->synchronizedCall();
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    synchronized_call_error() <<
                    bunsan::enable_nested_current());
            }
        }

        struct parse_result_error: virtual error {};
        bool parse_result(
            const Process::Result &process_result,
            bacs::process::ExecutionResult &result)
        {
            try
            {
                return process::parse_result(
                    m_process_group->result(),
                    process_result,
                    result
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    parse_result_error() <<
                    bunsan::enable_nested_current());
            }
        }

        bool parse_result(
            const ProcessPointer &process,
            bacs::process::ExecutionResult &result)
        {
            return parse_result(process->result(), result);
        }

        struct use_solution_file_error: virtual error {};
        void use_solution_file(
            const std::string &data_id,
            const boost::filesystem::path &path)
        {
            try
            {
                m_solution_files[data_id] = path;
                m_container_solution_files[data_id] =
                    m_container->filesystem().keepInRoot(path);
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    use_solution_file_error() <<
                    bunsan::enable_nested_current());
            }
        }

        bool ok(const problem::single::result::TestResult &result) const
        {
            return
                result.execution().status() ==
                    bacs::process::ExecutionResult::OK &&
                result.judge().status() ==
                    problem::single::result::Judge::OK;
        }

        struct run_checker_if_ok_error: virtual error {};
        bool run_checker_if_ok(
            checker &checker_,
            problem::single::result::TestResult &result)
        {
            try
            {
                if (!ok(result))
                    return false;
                return checker_.check(
                    m_test_files,
                    m_container_solution_files,
                    *result.mutable_judge()
                );
            }
            catch (std::exception &)
            {
                BOOST_THROW_EXCEPTION(
                    run_checker_if_ok_error() <<
                    bunsan::enable_nested_current());
            }
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
