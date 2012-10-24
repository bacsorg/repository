#include "xmlrpc.hpp"

namespace bacs{namespace single{namespace callback{namespace detail
{
    xmlrpc_base::xmlrpc_base(const std::vector<std::string> &arguments)
    {
    }

    void xmlrpc_result::call(const api::pb::result::Result &result)
    {
        std::string data;
        if (!result.SerializeToString(&data))
            BOOST_THROW_EXCEPTION(result_serialization_error());
        // TODO send
    }

    void xmlrpc_intermediate::call(const std::vector<std::string> &state)
    {
        // TODO send
    }
}}}}
