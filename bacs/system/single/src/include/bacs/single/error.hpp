#pragma once

#include "bunsan/error.hpp"

namespace bacs{namespace single
{
    struct error: virtual bunsan::error {};

    struct invalid_argument_error: virtual error
    {
        typedef boost::error_info<struct tag_argument, std::string> argument;
    };
}}
