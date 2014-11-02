#include <bacs/system/single/checker.hpp>

namespace bacs{namespace system{namespace single
{
    problem::single::result::Judge::Status checker::return_cast(const int exit_status)
    {
        BOOST_ASSERT_MSG(false, "Not implemented");
        return problem::single::result::Judge::FAILED;
    }
}}}
