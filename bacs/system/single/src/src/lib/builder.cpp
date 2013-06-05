#include "bacs/system/single/builder.hpp"

namespace bacs{namespace system{namespace single
{
    BUNSAN_FACTORY_DEFINE(builder)

    builder_ptr builder::instance(const problem::single::task::Builder &config)
    {
        return instance(config.type(), std::vector<std::string>{
            begin(config.arguments()),
            end(config.arguments())
        });
    }
}}}
