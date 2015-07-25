#include <bacs/system/single/tester.hpp>

namespace bacs {
namespace system {
namespace single {

class bacs_tester_result_mapper : public tester::result_mapper {
 public:
  bacs_tester_result_mapper() = default;
  problem::single::JudgeResult::Status map(int exit_status) override;
  static result_mapper_uptr make_instance() {
    return std::make_unique<bacs_tester_result_mapper>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER_NESTED(tester, result_mapper,
                                   bacs_tester_result_mapper,
                                   bacs_tester_result_mapper::make_instance)

problem::single::JudgeResult::Status bacs_tester_result_mapper::map(
    const int exit_status) {
  switch (exit_status) {
    case 0:
      return problem::single::JudgeResult::OK;
    case 1:
      return problem::single::JudgeResult::WRONG_ANSWER;
    case 2:
      return problem::single::JudgeResult::PRESENTATION_ERROR;
    case 3:
      return problem::single::JudgeResult::QUERIES_LIMIT_EXCEEDED;
    case 4:
      return problem::single::JudgeResult::INCORRECT_REQUEST;
    case 5:
      return problem::single::JudgeResult::INSUFFICIENT_DATA;
    case 100:
      return problem::single::JudgeResult::CUSTOM_FAILURE;
    case 200:
      return problem::single::JudgeResult::FAIL_TEST;
    case 250:
      return problem::single::JudgeResult::SKIPPED;
    default:
      return problem::single::JudgeResult::FAILED;
  }
}

}  // namespace single
}  // namespace system
}  // namespace bacs
