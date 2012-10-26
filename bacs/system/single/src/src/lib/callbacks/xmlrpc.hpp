#pragma once

#include "bacs/single/callback.hpp"

#include <utility>

#include <xmlrpc-c/client_simple.hpp>

namespace bacs{namespace single{namespace callback{namespace callbacks
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
        static bool factory_reg_hook;
    };
}}}}
