#include "bacs/single/builder.hpp"

#include "bunsan/tempfile.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single{namespace builders
{
    using namespace yandex::contest::invoker;

    class gcc: public builder
    {
    public:
        explicit gcc(const std::vector<std::string> &arguments);

        solution_ptr build(const ContainerPointer &container,
                           const std::string &source,
                           const api::pb::ResourceLimits &resource_limits,
                           api::pb::result::BuildResult &result) override;

    private:
        std::vector<std::string> m_flags;

    private:
        static const bool factory_reg_hook;
    };

    class gcc_solution: public solution
    {
    public:
        gcc_solution(const ContainerPointer &container,
                     bunsan::tempfile &&tmpdir);

        ProcessPointer create(
            const ProcessGroupPointer &process_group,
            const ProcessArguments &arguments) override;

    private:
        const ContainerPointer m_container;
        const bunsan::tempfile m_tmpdir;
    };
}}}
