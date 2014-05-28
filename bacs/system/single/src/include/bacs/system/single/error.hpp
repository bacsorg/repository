#pragma once

#include <bacs/system/error.hpp>

namespace bacs{namespace system{namespace single
{
    struct error: virtual bacs::system::error {};

    struct invalid_argument_error: virtual error
    {
        typedef boost::error_info<
            struct tag_argument,
            std::string
        > argument;
    };

    struct test_group_error: virtual error
    {
        typedef boost::error_info<
            struct tag_test_group,
            std::string
        > test_group;
    };
    struct test_group_dependency_error:
        virtual test_group_error
    {
        typedef boost::error_info<
            struct tag_test_group,
            std::string
        > test_group_dependency;
    };
    struct test_group_dependency_not_found_error:
        virtual test_group_dependency_error {};
    struct test_group_circular_dependencies_error:
        virtual test_group_dependency_error {};
}}}
