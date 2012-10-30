#include "xmlrpc.hpp"

namespace bacs{namespace single{namespace callback{namespace callbacks
{
    const bool xmlrpc::factory_reg_hook = base::register_new("xmlrpc",
        [](const std::vector<std::string> &arguments)
        {
            const base_ptr tmp(new xmlrpc(arguments));
            return tmp;
        });

    xmlrpc::xmlrpc(const std::vector<std::string> &arguments)
    {
        for (std::size_t i = 0; i < arguments.size(); ++i)
        {
            switch (i)
            {
            case 0:
                m_uri = arguments[i];
                break;
            case 1:
                m_method = arguments[i];
                break;
            default:
                m_arguments.addc(arguments[i]);
            }
        }
    }

    void xmlrpc::call(const data_type &data)
    {
        xmlrpc_c::paramList argv(m_arguments);
        argv.addc(data);
        xmlrpc_c::value result;
        m_proxy.call(m_uri, m_method, argv, &result);
    }
}}}}
