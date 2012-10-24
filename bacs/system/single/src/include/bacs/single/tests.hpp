#pragma once

#include <string>
#include <unordered_set>

namespace bacs{namespace single{namespace tests
{
    /// \note must be implemented in problem
    void create(const std::string &test_id, const std::string &data_id,
                const boost::filesystem::path &path);

    /// \note must be implemented in problem
    std::unordered_set<std::string> data_set();

    /// \note must be implemented in problem
    std::unordered_set<std::string> test_set();
}}}
