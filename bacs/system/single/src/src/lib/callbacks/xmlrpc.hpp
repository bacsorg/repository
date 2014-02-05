#pragma once

#include <bacs/system/single/callback.hpp>

#include <xmlrpc-c/client_simple.hpp>

#include <utility>

namespace bacs{namespace system{namespace single{namespace callback{namespace callbacks
{
    class xmlrpc: public base
    {
    public:
        explicit xmlrpc(const std::vector<std::string> &arguments);

        void call(const data_type &data) override;

    private:
        xmlrpc_c::clientSimple m_proxy;
        std::string m_uri;
        std::string m_method;
        xmlrpc_c::paramList m_arguments;

    private:
        static const bool factory_reg_hook;
    };
}}}}}
