#include "compilable.hpp"

#include "bacs/single/detail/process.hpp"
#include "bacs/single/detail/result.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
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
        const boost::filesystem::path source_name_ = source_name(source);
        {
            boost::filesystem::ofstream fout(tmpdir.path() / source_name_);
            fout.exceptions(std::ios::badbit);
            fout << source;
            fout.close();
        }
        container->filesystem().setOwnerId(solutions_path / tmpdir.path().filename() / source_name_, owner_id);
        const ProcessGroupPointer process_group = container->createProcessGroup();
        const ProcessPointer process = create_process(process_group, source_name_);
        detail::process::setup(resource_limits, process_group, process);
        process->setCurrentPath(solutions_path / tmpdir.path().filename());
        process->setOwnerId(owner_id);
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
        if (success)
            return create_solution(container, std::move(tmpdir));
        else
            return solution_ptr();
    }

    boost::filesystem::path compilable::source_name(const std::string &/*source*/)
    {
        return "source";
    }

    compilable_solution::compilable_solution(const ContainerPointer &container,
                                             bunsan::tempfile &&tmpdir):
        m_container(container), m_tmpdir(std::move(tmpdir)) {}

    ContainerPointer compilable_solution::container()
    {
        return m_container;
    }

    boost::filesystem::path compilable_solution::dir()
    {
        return solutions_path / m_tmpdir.path().filename();
    }
}}}
