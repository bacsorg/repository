#include "bacs/single/validator.hpp"

namespace bacs{namespace single
{
    validator::validator() {}
    validator::~validator() {}

    validator::result validator::validate(const file_map &/*test_files*/)
    {
        return result();
    }
}}}}
