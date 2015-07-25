#include <bacs/system/single/checker.hpp>

namespace bacs {
namespace system {
namespace single {

class ok_checker : public checker {
 public:
  ok_checker() = default;
  bool check(const file_map & /*test_files*/,
             const file_map & /*solution_files*/,
             problem::single::JudgeResult &result) override {
    result.set_status(problem::single::JudgeResult::OK);
    return true;
  }
  static checker_uptr make_instance(
      const yandex::contest::invoker::ContainerPointer & /*container*/,
      result_mapper_uptr /*mapper*/) {
    return std::make_unique<ok_checker>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER(checker, ok_checker, ok_checker::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
