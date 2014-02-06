#pragma once

#include <bacs/system/single/checker.hpp>

namespace bacs{namespace system{namespace single{namespace detail{namespace checker
{
    problem::single::result::Judge::Status equal(std::istream &out, std::istream &hint);
    problem::single::result::Judge::Status seek_eof(std::istream &in);
}}}}}
