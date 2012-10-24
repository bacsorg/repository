#pragma once

#include "bacs/single/common.hpp"

#include <boost/optional.hpp>

namespace bacs{namespace single{namespace validator
{
    struct result
    {
        enum status_type
        {
            OK,
            FAIL
        };

        status_type status;
        boost::optional<std::string> message;
    };

    /// \note must be implemented in problem
    result validate(const file_map &test_files);
}}}
