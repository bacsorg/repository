#pragma once

#include "interpretable.hpp"

#include <memory>

namespace bacs{namespace system{namespace single{namespace builders
{
    class mono: public interpretable
    {
    public:
        explicit mono(const std::vector<std::string> &arguments);

    protected:
        name_type name(const std::string &source) override;

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
