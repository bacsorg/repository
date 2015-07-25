#include <bacs/system/single/checker.hpp>

namespace bacs {
namespace system {
namespace single {

class ejudge_checker_result_mapper : public checker::result_mapper {
 public:
  ejudge_checker_result_mapper() = default;
  problem::single::JudgeResult::Status map(const int exit_status) {
    switch (exit_status) {
      case 0:
        return problem::single::JudgeResult::OK;
      case 5:
        return problem::single::JudgeResult::WRONG_ANSWER;
      case 4:
        return problem::single::JudgeResult::PRESENTATION_ERROR;
      default:
        return problem::single::JudgeResult::FAILED;
    }
  }
  static result_mapper_uptr make_instance() {
    return std::make_unique<ejudge_checker_result_mapper>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER_NESTED(
    checker, result_mapper, ejudge_checker_result_mapper,
    ejudge_checker_result_mapper::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
