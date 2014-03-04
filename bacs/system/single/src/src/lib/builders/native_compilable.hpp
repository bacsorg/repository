#pragma once

#include "compilable.hpp"

namespace bacs{namespace system{namespace single{namespace builders
{
    class native_compilable: public compilable
    {
    protected:
        solution_ptr create_solution(const ContainerPointer &container,
                                     bunsan::tempfile &&tmpdir,
                                     const name_type &name) override;
    };

    class native_compilable_solution: public compilable_solution
    {
    public:
        using compilable_solution::compilable_solution;

        ProcessPointer create(
            const ProcessGroupPointer &process_group,
            const ProcessArguments &arguments) override;
    };
}}}}
