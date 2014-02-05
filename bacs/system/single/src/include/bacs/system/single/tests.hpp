#pragma once

#include <bacs/system/single/error.hpp>

#include <bacs/problem/single/testing.pb.h>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>

#include <string>
#include <unordered_set>

namespace bacs{namespace system{namespace single
{
    struct invalid_test_query_error: virtual error {};

    class test;

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

        /// \note implemented
        single::test test(const std::string &test_id);

    private:
        class impl;

        impl *pimpl;
    };

    /*!
     * Wrapper class for single test
     *
     * \warning Stores mutable reference to tests class.
     */
    class test
    {
    public:
        test()=default;
        test(const test &)=default;
        test &operator=(const test &)=default;

        test(tests &tests_, const std::string &test_id);

        void copy(const std::string &data_id, const boost::filesystem::path &path) const;

        boost::filesystem::path location(const std::string &data_id) const;

        std::unordered_set<std::string> data_set() const;

    private:
        tests *m_tests = nullptr;
        std::string m_test_id;
    };
}}}
