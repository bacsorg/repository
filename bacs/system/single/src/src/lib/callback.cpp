#include <bacs/system/single/callback.hpp>

namespace bacs{namespace system{namespace single{namespace callback
{
    BUNSAN_FACTORY_DEFINE(base)

    base_ptr base::instance(const problem::single::task::Callback &config)
    {
        const std::vector<std::string> arguments(
            config.argument().begin(),
            config.argument().end());
        return instance(config.type(), arguments);
    }
}}}}
