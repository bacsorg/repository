#include <bacs/system/single/tester.hpp>

namespace bacs {
namespace system {
namespace single {

problem::single::result::Judge::Status tester::return_cast(
    const int exit_status) {
  switch (exit_status) {
    case 0:
      return problem::single::result::Judge::OK;
    case 1:
      return problem::single::result::Judge::WRONG_ANSWER;
    case 2:
      return problem::single::result::Judge::PRESENTATION_ERROR;
    case 3:
      return problem::single::result::Judge::QUERIES_LIMIT_EXCEEDED;
    case 4:
      return problem::single::result::Judge::INCORRECT_REQUEST;
    case 5:
      return problem::single::result::Judge::INSUFFICIENT_DATA;
    case 100:
      return problem::single::result::Judge::CUSTOM_FAILURE;
    case 200:
      return problem::single::result::Judge::FAIL_TEST;
    case 250:
      return problem::single::result::Judge::SKIPPED;
    default:
      return problem::single::result::Judge::FAILED;
  }
}

}  // namespace single
}  // namespace system
}  // namespace bacs
