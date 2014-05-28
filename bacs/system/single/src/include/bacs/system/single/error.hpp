#pragma once

#include <bacs/system/error.hpp>

namespace bacs{namespace system{namespace single
{
    struct error: virtual bacs::system::error {};

    struct invalid_argument_error: virtual error
    {
        typedef boost::error_info<struct tag_argument, std::string> argument;
    };
}}}
