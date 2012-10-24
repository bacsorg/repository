#pragma once

#include "bacs/single/error.hpp"

#include "bacs/single/api/pb/result.pb.h"

#include "bunsan/factory_helper.hpp"

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

namespace bacs{namespace single{namespace callback
{
    struct error: virtual single::error {};
    struct serialization_error: virtual error {};

    struct result_error: virtual error {};
    struct result_serialization_error:
        virtual result_error, virtual serialization_error {};

    struct intermediate_error: virtual error {};
    struct intermediate_serialization_error:
        virtual intermediate_error, virtual serialization_error {};

    class result: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(result, const std::vector<std::string> &/*arguments*/)
    public:
        virtual ~result() {}

    public:
        virtual void call(const api::pb::result::Result &result)=0;
    BUNSAN_FACTORY_END(result)

    class intermediate: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(intermediate, const std::vector<std::string> &/*arguments*/)
    public:
        virtual ~intermediate() {}

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
