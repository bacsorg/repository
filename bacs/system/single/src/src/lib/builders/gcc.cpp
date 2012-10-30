#include "gcc.hpp"

#include "bacs/single/detail/process.hpp"
#include "bacs/single/detail/result.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace single{namespace builders
{
    const bool gcc::factory_reg_hook = builder::register_new("gcc",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new gcc(arguments));
            return tmp;
        });

    static const boost::regex positional("^[^=]+$"), key_value("^([^=]+)=(.*)$");

    gcc::gcc(const std::vector<std::string> &arguments)
    {
        for (const std::string &arg: arguments)
        {
            boost::smatch match;
            if (boost::regex_match(arg, match, positional))
            {
                BOOST_ASSERT(match.size() == 1);
                m_flags.push_back("-" + arg);
            }
            else if (boost::regex_match(arg, match, key_value))
            {
                BOOST_ASSERT(match.size() == 2);
                const std::string key = match[1].str(), value = match[2].str();
                if (key == "optimize")
                {
                    m_flags.push_back("-O" + value);
                }
                else if (key == "lang")
                {
                    m_flags.push_back("-x");
                    m_flags.push_back(value);
                }
                else if (key == "std")
                {
                    m_flags.push_back("-std=" + value);
                }
                else
                {
                    BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                          invalid_argument_error::argument(arg));
                }
            }
            else
            {
                BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                      invalid_argument_error::argument(arg));
            }
        }
    }

    static const boost::filesystem::path solutions_path = "/tmp/solutions";

    solution_ptr gcc::build(const ContainerPointer &container,
                            const std::string &source,
                            const api::pb::ResourceLimits &resource_limits,
                            api::pb::result::BuildResult &result)
    {
        const boost::filesystem::path solutions = container->filesystem().keepInRoot(solutions_path);
        boost::filesystem::create_directories(solutions);
        bunsan::tempfile tmpdir = bunsan::tempfile::in_dir(solutions);
        BOOST_VERIFY(boost::filesystem::create_directory(tmpdir.path()));
        {
            boost::filesystem::ofstream fout(tmpdir.path() / "source");
            fout.exceptions(std::ios::badbit);
            fout << source;
            fout.close();
        }
        const ProcessGroupPointer process_group = container->createProcessGroup();
        const ProcessPointer process = process_group->createProcess("gcc");
        detail::process::setup(resource_limits, process_group, process);
        process->setArguments(process->executable(), m_flags, "source", "-o", "executable");
        process->setCurrentPath(solutions_path / tmpdir.path().filename());
        process->setStream(2, FDAlias(1));
        process->setStream(1, File("log", AccessMode::WRITE_ONLY));
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        const bool success = detail::result::parse(process_group_result, process_result, *result.mutable_execution());
        {
            boost::filesystem::ifstream fin(tmpdir.path() / "log");
            fin.exceptions(std::ios::badbit);
            std::string &output = *result.mutable_output();
            char buf[4096];
            fin.read(buf, sizeof(buf));
            output.assign(buf,fin.gcount());
            fin.close();
        }
        solution_ptr solution;
        if (success)
            solution.reset(new gcc_solution(container, std::move(tmpdir)));
        return solution;
    }

    gcc_solution::gcc_solution(const ContainerPointer &container,
                               bunsan::tempfile &&tmpdir):
        m_container(container), m_tmpdir(std::move(tmpdir)) {}

    ProcessPointer gcc_solution::create(
            const ProcessGroupPointer &process_group,
            const ProcessArguments &arguments)
    {
        const ProcessPointer process = process_group->createProcess(
            solutions_path / m_tmpdir.path().filename() / "executable");
        process->setArguments(process->executable(), arguments);
        return process;
    }
}}}
