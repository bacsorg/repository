#include "interpretable.hpp"

namespace bacs{namespace system{namespace single{namespace builders
{
    compilable::name_type interpretable::name(const std::string &/*source*/)
    {
        static const std::string script = "script";
        return {.source = script, .executable = script};
    }

    interpretable_solution::interpretable_solution(
        const ContainerPointer &container,
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
        process->setArguments(process->executable(), this->arguments(), arguments);
        return process;
    }

    std::vector<std::string> interpretable_solution::arguments() const
    {
        std::vector<std::string> arguments_ = flags();
        arguments_.push_back(executable().string());
        return arguments_;
    }

    std::vector<std::string> interpretable_solution::flags() const
    {
        return m_flags;
    }
}}}}
