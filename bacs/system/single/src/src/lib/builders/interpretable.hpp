#include "compilable.hpp"

namespace bacs{namespace single{namespace builders
{
    class interpretable: public compilable
    {
    };

    class interpretable_solution: public compilable_solution
    {
    public:
        interpretable_solution(const ContainerPointer &container,
                               bunsan::tempfile &&tmpdir,
                               const compilable::name_type &name,
                               const boost::filesystem::path &executable,
                               const std::vector<std::string> &flags);

        ProcessPointer create(
            const ProcessGroupPointer &process_group,
            const ProcessArguments &arguments) override;

    private:
        const boost::filesystem::path m_executable;
        const std::vector<std::string> m_flags;
    };
}}}
