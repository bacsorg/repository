#pragma once

#include "bacs/single/builder.hpp"

#include "bunsan/tempfile.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single{namespace builders
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
        solution_ptr build(const ContainerPointer &container,
                           const unistd::access::Id &owner_id,
                           const std::string &source,
                           const api::pb::ResourceLimits &resource_limits,
                           api::pb::result::BuildResult &result) override;

    protected:
        virtual name_type name(const std::string &source);

        virtual ProcessPointer create_process(const ProcessGroupPointer &process_group,
                                              const name_type &name)=0;

        virtual solution_ptr create_solution(const ContainerPointer &container,
                                             bunsan::tempfile &&tmpdir,
                                             const name_type &name)=0;
    };

    class compilable_solution: public solution
    {
    public:
        compilable_solution(const ContainerPointer &container,
                            bunsan::tempfile &&tmpdir,
                            const compilable::name_type &name);

    protected:
        ContainerPointer container();
        boost::filesystem::path dir();
        boost::filesystem::path source();
        boost::filesystem::path executable();

    private:
        const ContainerPointer m_container;
        const bunsan::tempfile m_tmpdir;
        const compilable::name_type m_name;
    };
}}}
