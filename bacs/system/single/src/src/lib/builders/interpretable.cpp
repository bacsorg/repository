#include "interpretable.hpp"

namespace bacs{namespace single{namespace builders
{
    interpretable_solution::interpretable_solution(const ContainerPointer &container,
                                                   bunsan::tempfile &&tmpdir,
                                                   const compilable::name_type &name,
                                                   const boost::filesystem::path &executable,
                                                   const std::vector<std::string> &flags):
            compilable_solution(container, std::move(tmpdir), name),
            m_executable(executable), m_flags(flags) {}

    ProcessPointer interpretable_solution::create(
        const ProcessGroupPointer &process_group,
        const ProcessArguments &arguments)
    {
        const ProcessPointer process = process_group->createProcess(m_executable);
        process->setArguments(process->executable(), m_flags, arguments);
        return process;
    }
}}}
