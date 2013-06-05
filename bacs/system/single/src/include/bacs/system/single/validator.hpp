#pragma once

#include "bacs/system/single/common.hpp"

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace bacs{namespace system{namespace single
{
    /// \note must be implemented in problem
    class validator: private boost::noncopyable
    {
    public:
        struct result
        {
            enum status_type
            {
                OK,
                FAIL
            };

            status_type status = OK;
            boost::optional<std::string> message;
        };

    public:
        validator();
        ~validator();

        result validate(const file_map &test_files);

    private:
        class impl;

        impl *pimpl;
    };
}}}
