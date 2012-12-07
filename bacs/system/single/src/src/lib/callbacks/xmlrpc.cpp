#include "xmlrpc.hpp"

#include "bunsan/enable_error_info.hpp"

#define BUNSAN_EXCEPTIONS_WRAP_END_XMLRPC() \
    BUNSAN_EXCEPTIONS_WRAP_END_EXCEPT(::girerr::error)

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
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
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
        BUNSAN_EXCEPTIONS_WRAP_END_XMLRPC()
    }

    void xmlrpc::call(const data_type &data)
    {
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            xmlrpc_c::paramList argv(m_arguments);
            argv.addc(data);
            xmlrpc_c::value result;
            m_proxy.call(m_uri, m_method, argv, &result);
        }
        BUNSAN_EXCEPTIONS_WRAP_END_XMLRPC()
    }
}}}}
