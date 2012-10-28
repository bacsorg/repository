#include "bacs/single/builder.hpp"

namespace bacs{namespace single
{
    BUNSAN_FACTORY_DEFINE(builder)

    builder_ptr builder::instance(const api::pb::task::Builder &config)
    {
        return instance(config.type(), std::vector<std::string>{
            begin(config.arguments()),
            end(config.arguments())
        });
    }
}}
