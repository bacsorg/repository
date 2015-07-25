#include <bacs/system/single/checker.hpp>

#include <bacs/system/single/error.hpp>

namespace bacs {
namespace system {
namespace single {

struct none_not_implemented_error : virtual error {};

class none_checker_result_mapper : public checker::result_mapper {
 public:
  none_checker_result_mapper() = default;
  problem::single::JudgeResult::Status map(int) override {
    BOOST_THROW_EXCEPTION(none_not_implemented_error());
  }
  static result_mapper_uptr make_instance() {
    return std::make_unique<none_checker_result_mapper>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER_NESTED(
    checker, result_mapper, none_checker_result_mapper,
    none_checker_result_mapper::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
