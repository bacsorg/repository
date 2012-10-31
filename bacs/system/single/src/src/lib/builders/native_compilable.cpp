#include "native_compilable.hpp"

namespace bacs{namespace single{namespace builders
{
    solution_ptr native_compilable::create_solution(const ContainerPointer &container,
                                                    bunsan::tempfile &&tmpdir,
                                                    const name_type &name)
    {
        solution_ptr tmp(new native_compilable_solution(container, std::move(tmpdir), name));
        return tmp;
    }

    ProcessPointer native_compilable_solution::create(
        const ProcessGroupPointer &process_group,
        const ProcessArguments &arguments)
    {
        const ProcessPointer process = process_group->createProcess(executable());
        process->setArguments(process->executable(), arguments);
        return process;
    }
}}}
