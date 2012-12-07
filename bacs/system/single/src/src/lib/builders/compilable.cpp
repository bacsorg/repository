#include "compilable.hpp"

#include "bacs/single/detail/process.hpp"
#include "bacs/single/detail/result.hpp"

#include "bunsan/enable_error_info.hpp"
#include "bunsan/filesystem/fstream.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace single{namespace builders
{
    static const boost::filesystem::path solutions_path = "/tmp/solutions";

    solution_ptr compilable::build(const ContainerPointer &container,
                                   const unistd::access::Id &owner_id,
                                   const std::string &source,
                                   const api::pb::ResourceLimits &resource_limits,
                                   api::pb::result::BuildResult &result)
    {
        const boost::filesystem::path solutions = container->filesystem().keepInRoot(solutions_path);
        boost::filesystem::create_directories(solutions);
        bunsan::tempfile tmpdir = bunsan::tempfile::in_dir(solutions);
        BOOST_VERIFY(boost::filesystem::create_directory(tmpdir.path()));
        container->filesystem().setOwnerId(solutions_path / tmpdir.path().filename(), owner_id);
        const name_type name_ = name(source);
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            bunsan::filesystem::ofstream fout(tmpdir.path() / name_.source);
            fout << source;
            fout.close();
        }
        BUNSAN_EXCEPTIONS_WRAP_END()
        container->filesystem().setOwnerId(solutions_path / tmpdir.path().filename() / name_.source, owner_id);
        const ProcessGroupPointer process_group = container->createProcessGroup();
        const ProcessPointer process = create_process(process_group, name_);
        detail::process::setup(resource_limits, process_group, process);
        process->setCurrentPath(solutions_path / tmpdir.path().filename());
        process->setOwnerId(owner_id);
        process->setStream(2, FDAlias(1));
        process->setStream(1, File("log", AccessMode::WRITE_ONLY));
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        const bool success = detail::result::parse(process_group_result, process_result, *result.mutable_execution());
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            bunsan::filesystem::ifstream fin(tmpdir.path() / "log");
            std::string &output = *result.mutable_output();
            char buf[4096];
            fin.read(buf, sizeof(buf));
            output.assign(buf,fin.gcount());
            fin.close();
        }
        BUNSAN_EXCEPTIONS_WRAP_END()
        if (success)
            return create_solution(container, std::move(tmpdir), name_);
        else
            return solution_ptr();
    }

    compilable::name_type compilable::name(const std::string &/*source*/)
    {
        return {.source = "source", .executable = "executable"};
    }

    compilable_solution::compilable_solution(const ContainerPointer &container,
                                             bunsan::tempfile &&tmpdir,
                                             const compilable::name_type &name):
        m_container(container), m_tmpdir(std::move(tmpdir)), m_name(name) {}

    ContainerPointer compilable_solution::container()
    {
        return m_container;
    }

    boost::filesystem::path compilable_solution::dir()
    {
        return solutions_path / m_tmpdir.path().filename();
    }

    boost::filesystem::path compilable_solution::source()
    {
        return dir() / m_name.source;
    }

    boost::filesystem::path compilable_solution::executable()
    {
        return dir() / m_name.executable;
    }
}}}
