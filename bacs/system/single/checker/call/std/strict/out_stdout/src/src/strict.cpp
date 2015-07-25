#include <bacs/system/single/checker.hpp>

#include <bacs/system/single/check.hpp>

#include <bunsan/enable_error_info.hpp>
#include <bunsan/filesystem/fstream.hpp>

namespace bacs {
namespace system {
namespace single {

class strict_checker : public checker {
 public:
  strict_checker() = default;
  bool check(const file_map &test_files, const file_map &solution_files,
             problem::single::JudgeResult &result) override {
    BUNSAN_EXCEPTIONS_WRAP_BEGIN() {
      bunsan::filesystem::ifstream hint(test_files.at("out"), std::ios::binary),
          out(solution_files.at("stdout"), std::ios::binary);
      result.set_status(check::equal(out, hint));
    } BUNSAN_EXCEPTIONS_WRAP_END()
    return result.status() == problem::single::JudgeResult::OK;
  }
  static checker_uptr make_instance(
      const yandex::contest::invoker::ContainerPointer & /*container*/,
      result_mapper_uptr /*mapper*/) {
    return std::make_unique<strict_checker>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER(checker, strict_checker,
                            strict_checker::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
