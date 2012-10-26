#pragma once

#include "bacs/single/error.hpp"

#include "bacs/single/api/pb/task.pb.h"
#include "bacs/single/api/pb/result.pb.h"
#include "bacs/single/api/pb/intermediate.pb.h"

#include "bunsan/factory_helper.hpp"

#include <string>
#include <vector>
#include <iterator>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iterator/transform_iterator.hpp>

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

    class base: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(base, const std::vector<std::string> &/*arguments*/)
    public:
        virtual ~base() {}

    public:
        static base_ptr instance(const api::pb::task::Callback &config);

    public:
        typedef std::vector<unsigned char> data_type;

    public:
        virtual void call(const data_type &data)=0;

        template <typename T>
        void call(const T &obj)
        {
            call(begin(obj), end(obj));
        }

        template <typename Iter>
        void call(const Iter &begin, const Iter &end)
        {
            typedef std::iterator_traits<Iter> traits;
            static_assert(sizeof(traits::value_type) == sizeof(data_type::value_type), "Sizes should be equal.");
            const auto to_uc =
                [](const typename traits::value_type c)
                {
                    return data_type::value_type(c);
                };
            call(data_type(boost::make_transform_iterator(begin, to_uc),
                           boost::make_transform_iterator(end, to_uc)));
        }
    BUNSAN_FACTORY_END(base)

    template <typename T>
    class interface
    {
    public:
        interface()=default;
        interface(const interface &)=default;
        interface &operator=(const interface &)=default;

        explicit interface(const base_ptr &base_): m_base(base_) {}

        explicit interface(const api::pb::task::Callback &config):
            interface(base::instance(config)) {}

        template <typename Y>
        void assign(Y &&y)
        {
            interface(y).swap(*this);
        }

        void swap(interface &iface) noexcept
        {
            using boost::swap;
            swap(m_base, iface.m_base);
        }

        void call(const T &obj) const
        {
            call_(obj);
        }

    private:
        void call(const google::protobuf::MessageLite &message) const
        {
            if (m_base)
                m_base->call(message.SerializeAsString());
        }

    private:
        base_ptr m_base;
    };

    template <typename T>
    void swap(interface<T> &a, interface<T> &b) noexcept
    {
        a.swap(b);
    }

    typedef interface<api::pb::result::Result> result;
    typedef interface<api::pb::intermediate::IntermediateResult> intermediate;
}}}
