#include "bacs/single/detail/checker.hpp"

#include <boost/filesystem/fstream.hpp>

namespace bacs{namespace single
{
    checker::checker() {}
    checker::~checker() {}

    checker::result checker::check(const file_map &test_files, const file_map &solution_files)
    {
        result result_;
        boost::filesystem::ifstream hint(test_files.at("out"), std::ios::binary),
            out(solution_files.at("stdout"), std::ios::binary);
        hint.exceptions(std::ios::badbit);
        out.exceptions(std::ios::badbit);
        result_.status = detail::checker::equal(out, hint);
        return result_;
    }
}}

