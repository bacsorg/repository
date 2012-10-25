#pragma once

#include "bacs/single/checker.hpp"

namespace bacs{namespace single{namespace detail{namespace checker
{
    typedef single::checker::result result;

    result::status_type equal(std::istream &out, std::istream &hint);
    result::status_type seek_eof(std::istream &in);
}}}}
