#pragma once

#include "bacs/single/callback.hpp"

#include <utility>

namespace bacs{namespace single{namespace callback{namespace detail
{
    class xmlrpc_base: private boost::noncopyable
    {
    public:
        explicit xmlrpc_base(const std::vector<std::string> &arguments);

    };

    class xmlrpc_result: public result, public xmlrpc_base
    {
    public:
        template <typename ... Args>
        explicit xmlrpc_result(Args &&...args):
            xmlrpc_base(std::forward<Args>(args)...) {}

        void call(const api::pb::result::Result &result) override;
    };

    class xmlrpc_intermediate: public intermediate, public xmlrpc_base
    {
    public:
        template <typename ... Args>
        explicit xmlrpc_intermediate(Args &&...args):
            xmlrpc_base(std::forward<Args>(args)...) {}

        void call(const std::vector<std::string> &state) override;
    };
}}}}
