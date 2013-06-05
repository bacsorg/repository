#pragma once

#include "bacs/system/single/checker.hpp"

namespace bacs{namespace system{namespace single{namespace detail{namespace checker
{
    typedef single::checker::result result;

    result::status_type equal(std::istream &out, std::istream &hint);
    result::status_type seek_eof(std::istream &in);
}}}}}
