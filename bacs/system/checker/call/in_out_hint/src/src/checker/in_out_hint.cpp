#include "bacs/single/checker.hpp"

#include <boost/assert.hpp>

namespace bacs{namespace single
{
    checker::checker() {}
    checker::~checker() {}

    checker::result checker::check(const file_map &test_files, const file_map &solution_files)
    {
        const boost::filesystem::path in = test_files.at("in"),
            out = solution_files.at("stdout"), hint = test_files.at("out");
        BOOST_ASSERT_MSG(test_files.size() == 2, "keys(test_files) == {'in', 'out'}");
        result result_;
        // TODO
        return result_;
    }
}}
