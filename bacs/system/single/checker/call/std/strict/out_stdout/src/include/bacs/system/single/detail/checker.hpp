#pragma once

#include "bacs/system/single/checker.hpp"

namespace bacs{namespace system{namespace single{namespace detail{namespace checker
{
    typedef single::checker::result result;

    single::checker::result::status_type equal(std::istream &out, std::istream &hint);
    single::checker::result::status_type seek_eof(std::istream &in);
}}}}}
