#pragma once

#include "bacs/system/single/error.hpp"

#include "bacs/problem/single/testing.pb.h"

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_set>

namespace bacs{namespace system{namespace single
{
    struct invalid_test_query_error: virtual error {};

    /// \note must be implemented in problem
    class tests: private boost::noncopyable
    {
    public:
        tests();
        ~tests();

        void copy(const std::string &test_id, const std::string &data_id,
                  const boost::filesystem::path &path);

        boost::filesystem::path location(const std::string &test_id, const std::string &data_id);

        std::unordered_set<std::string> data_set();

        std::unordered_set<std::string> test_set();

        /// \note implemented
        std::unordered_set<std::string> test_set(
            const google::protobuf::RepeatedPtrField<problem::single::testing::TestQuery> &test_query);

    private:
        class impl;

        impl *pimpl;
    };
}}}
