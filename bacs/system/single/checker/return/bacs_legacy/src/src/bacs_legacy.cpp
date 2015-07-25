#include <bacs/system/single/checker.hpp>

namespace bacs {
namespace system {
namespace single {

class bacs_legacy_checker_result_mapper : public checker::result_mapper {
 public:
  bacs_legacy_checker_result_mapper() = default;
  problem::single::JudgeResult::Status map(const int exit_status) {
    switch (exit_status) {
      case 0:
        return problem::single::JudgeResult::OK;
      case 2:
      case 5:
        return problem::single::JudgeResult::WRONG_ANSWER;
      case 4:
        return problem::single::JudgeResult::PRESENTATION_ERROR;
      default:
        return problem::single::JudgeResult::FAILED;
    }
  }
  static result_mapper_uptr make_instance() {
    return std::make_unique<bacs_legacy_checker_result_mapper>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER_NESTED(
    checker, result_mapper, bacs_legacy_checker_result_mapper,
    bacs_legacy_checker_result_mapper::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
