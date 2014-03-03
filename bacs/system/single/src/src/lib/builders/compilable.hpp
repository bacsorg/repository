#pragma once

#include <bacs/system/single/builder.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <bunsan/tempfile.hpp>

namespace bacs{namespace system{namespace single{namespace builders
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    class compilable: public builder
    {
    public:
        struct name_type
        {
            boost::filesystem::path source, executable;
        };

    public:
        solution_ptr build(
            const ContainerPointer &container,
            const unistd::access::Id &owner_id,
            const std::string &source,
            const problem::single::ResourceLimits &resource_limits,
            problem::single::result::BuildResult &result) override;

    protected:
        virtual name_type name(const std::string &source);

        virtual ProcessPointer create_process(
            const ProcessGroupPointer &process_group,
            const name_type &name)=0;

        virtual solution_ptr create_solution(
            const ContainerPointer &container,
            bunsan::tempfile &&tmpdir,
            const name_type &name)=0;
    };

    class compilable_solution: public solution
    {
    public:
        compilable_solution(
            const ContainerPointer &container,
            bunsan::tempfile &&tmpdir,
            const compilable::name_type &name);

    protected:
        ContainerPointer container();
        const compilable::name_type &name() const;
        boost::filesystem::path dir() const;
        boost::filesystem::path source() const;
        boost::filesystem::path executable() const;

    private:
        const ContainerPointer m_container;
        const bunsan::tempfile m_tmpdir;
        const compilable::name_type m_name;
    };
}}}}
