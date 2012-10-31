#include "native_compilable.hpp"

namespace bacs{namespace single{namespace builders
{
    class gcc: public native_compilable
    {
    public:
        explicit gcc(const std::vector<std::string> &arguments);

    protected:
        ProcessPointer create_process(const ProcessGroupPointer &process_group,
                                      const boost::filesystem::path &source_name) override;

    private:
        std::vector<std::string> m_flags;

    private:
        static const bool factory_reg_hook;
    };
}}}
