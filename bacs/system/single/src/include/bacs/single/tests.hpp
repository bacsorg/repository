#pragma once

#include <string>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace bacs{namespace single
{
    /// \note must be implemented in problem
    class tests: private boost::noncopyable
    {
    public:
        tests();
        ~tests();

        void create(const std::string &test_id, const std::string &data_id,
                    const boost::filesystem::path &path);

        std::unordered_set<std::string> data_set();

        std::unordered_set<std::string> test_set();

    private:
        class impl;

        impl *pimpl;
    };
}}
