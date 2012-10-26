#include "bacs/single/callback.hpp"

namespace bacs{namespace single{namespace callback
{
    BUNSAN_FACTORY_DEFINE(base)

    base_ptr base::instance(const api::pb::task::Callback &config)
    {
        const std::vector<std::string> arguments(
            config.arguments().begin(),
            config.arguments().end());
        return instance(config.type(), arguments);
    }
}}}
