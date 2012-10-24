#pragma once

#include "bacs/single/api/pb/result.pb.h"

#include "bunsan/factory_helper.hpp"

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

namespace bacs{namespace single{namespace callback
{
    class result: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(result, const std::vector<std::string> &arguments)
    public:
        virtual void call(const api::pb::result::Result &result)=0;
    BUNSAN_FACTORY_END(result)

    class intermediate: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(intermediate, const std::vector<std::string> &arguments)
    public:
        virtual void call(const std::vector<std::string> &state)=0;

        template <typename ... Args>
        void call(Args &&...args)
        {
            const std::vector<std::string> &state = {
                boost::lexical_cast<std::string>(args)...
            };
            call(state);
        }
    BUNSAN_FACTORY_END(intermediate)
}}}
