#pragma once

#include "bacs/single/api/pb/testing.pb.h"

#include "bacs/single/error.hpp"

#include <string>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace bacs{namespace single
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
            const google::protobuf::RepeatedPtrField<api::pb::testing::TestQuery> &test_query);

    private:
        class impl;

        impl *pimpl;
    };
}}
