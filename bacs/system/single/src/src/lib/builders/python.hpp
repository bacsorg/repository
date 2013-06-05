#pragma once

#include "interpretable.hpp"

namespace bacs{namespace system{namespace single{namespace builders
{
    class python: public interpretable
    {
    public:
        explicit python(const std::vector<std::string> &arguments);

    protected:
        ProcessPointer create_process(const ProcessGroupPointer &process_group,
                                      const name_type &name) override;

        solution_ptr create_solution(const ContainerPointer &container,
                                     bunsan::tempfile &&tmpdir,
                                     const name_type &name) override;

    private:
        std::string m_lang;
        std::vector<std::string> m_flags;

    private:
        static const bool factory_reg_hook;
    };
}}}}
