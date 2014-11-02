#include <bacs/system/single/tester.hpp>

namespace bacs{namespace system{namespace single
{
    problem::single::result::Judge::Status tester::return_cast(const int exit_status)
    {
        BOOST_ASSERT_MSG(false, "Not implemented");
        return problem::single::result::Judge::FAILED;
    }
}}}
