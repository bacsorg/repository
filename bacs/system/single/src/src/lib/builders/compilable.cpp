#include "compilable.hpp"

#include <bacs/system/single/detail/process.hpp>
#include <bacs/system/single/detail/result.hpp>

#include <bunsan/filesystem/fstream.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single{namespace builders
{
    static const boost::filesystem::path solutions_path = "/solutions";

    solution_ptr compilable::build(
        const ContainerPointer &container,
        const unistd::access::Id &owner_id,
        const std::string &source,
        const problem::single::ResourceLimits &resource_limits,
        problem::single::result::BuildResult &result)
    {
        const boost::filesystem::path solutions =
            container->filesystem().keepInRoot(solutions_path);
        boost::filesystem::create_directories(solutions);
        bunsan::tempfile tmpdir =
            bunsan::tempfile::directory_in_directory(solutions);
        container->filesystem().setOwnerId(
            solutions_path / tmpdir.path().filename(), owner_id);
        const name_type name_ = name(source);
        bunsan::filesystem::ofstream fout(tmpdir.path() / name_.source);
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_BEGIN(fout)
        {
            fout << source;
        }
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_END(fout)
        fout.close();
        container->filesystem().setOwnerId(
            solutions_path / tmpdir.path().filename() / name_.source, owner_id);
        const ProcessGroupPointer process_group = container->createProcessGroup();
        const ProcessPointer process = create_process(process_group, name_);
        detail::process::setup(resource_limits, process_group, process);
        process->setCurrentPath(solutions_path / tmpdir.path().filename());
        process->setOwnerId(owner_id);
        process->setStream(2, FDAlias(1));
        process->setStream(1, File("log", AccessMode::WRITE_ONLY));
        const ProcessGroup::Result process_group_result =
            process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        const bool success = detail::result::parse(
            process_group_result, process_result, *result.mutable_execution());
        bunsan::filesystem::ifstream fin(tmpdir.path() / "log");
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_BEGIN(fin)
        {
            std::string &output = *result.mutable_output();
            char buf[4096];
            fin.read(buf, sizeof(buf));
            output.assign(buf, fin.gcount());
        }
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_END(fin)
        fin.close();
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

    const compilable::name_type &compilable_solution::name() const
    {
        return m_name;
    }

    boost::filesystem::path compilable_solution::dir() const
    {
        return solutions_path / m_tmpdir.path().filename();
    }

    boost::filesystem::path compilable_solution::source() const
    {
        return dir() / m_name.source;
    }

    boost::filesystem::path compilable_solution::executable() const
    {
        return dir() / m_name.executable;
    }
}}}}
