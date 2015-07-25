#include <bacs/system/single/tester.hpp>

#include <bacs/system/single/error.hpp>

namespace bacs {
namespace system {
namespace single {

struct none_not_implemented_error : virtual error {};

class none_tester_result_mapper : public tester::result_mapper {
 public:
  none_tester_result_mapper() = default;
  problem::single::JudgeResult::Status map(int) override {
    BOOST_THROW_EXCEPTION(none_not_implemented_error());
  }
  static result_mapper_uptr make_instance() {
    return std::make_unique<none_tester_result_mapper>();
  }
};

BUNSAN_PLUGIN_AUTO_REGISTER_NESTED(
    tester, result_mapper, none_tester_result_mapper,
    none_tester_result_mapper::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
