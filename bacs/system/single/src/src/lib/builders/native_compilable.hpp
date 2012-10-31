#pragma once

#include "compilable.hpp"

namespace bacs{namespace single{namespace builders
{
    class native_compilable: public compilable
    {
    protected:
        solution_ptr create_solution(const ContainerPointer &container,
                                     bunsan::tempfile &&tmpdir) override;
    };

    class native_compilable_solution: public compilable_solution
    {
    public:
        template <typename ... Args>
        explicit native_compilable_solution(Args &&...args):
            compilable_solution(std::forward<Args>(args)...) {}

        ProcessPointer create(
            const ProcessGroupPointer &process_group,
            const ProcessArguments &arguments) override;
    };
}}}
