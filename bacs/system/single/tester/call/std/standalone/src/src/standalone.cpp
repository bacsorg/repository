#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/tester_util.hpp>

#include <boost/assert.hpp>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;

static const unistd::access::Id OWNER_ID(1000, 1000);

class standalone_tester : public tester {
 public:
  standalone_tester(const yandex::contest::invoker::ContainerPointer &container,
                    result_mapper_uptr mapper, checker_uptr checker)
      : m_util(container, "/testing"),
        m_mapper(std::move(mapper)),
        m_checker(std::move(checker)) {
    BOOST_ASSERT(m_mapper);
    BOOST_ASSERT(m_checker);
  }

  bool build(const bacs::process::Buildable &solution,
             bacs::process::BuildResult &result) override {
    m_builder = builder::instance(solution.build_settings().config());
    m_solution =
        m_builder->build(m_util.container(), OWNER_ID, solution.source(),
                         solution.build_settings().resource_limits(), result);
    return static_cast<bool>(m_solution);
  }

  bool test(const problem::single::process::Settings &settings,
            const test::storage::test &test,
            problem::single::TestResult &result) override {
    m_util.reset();

    const boost::filesystem::path testing_path =
        m_util.create_directory("testing", unistd::access::Id{0, 0}, 0555);

    const ProcessPointer process = m_solution->create(
        m_util.process_group(), settings.execution().argument());
    m_util.setup(process, settings);
    process->setOwnerId(OWNER_ID);

    // note: ignore current_path from settings.execution()
    process->setCurrentPath(testing_path);

    m_util.set_test_files(process, settings, test, testing_path, OWNER_ID);

    m_util.synchronized_call();
    m_util.send_test_files(result);

    m_util.parse_result(process, *result.mutable_execution());
    m_util.run_checker_if_ok(*m_checker, result);

    return m_util.fill_status(result);
  }

  static tester_uptr make_instance(
      const yandex::contest::invoker::ContainerPointer &container,
      result_mapper_uptr mapper, checker_uptr checker) {
    return std::make_unique<standalone_tester>(container, std::move(mapper),
                                               std::move(checker));
  }

 private:
  tester_util m_util;
  const result_mapper_uptr m_mapper;
  const checker_uptr m_checker;
  builder_ptr m_builder;
  executable_ptr m_solution;
};

BUNSAN_PLUGIN_AUTO_REGISTER(tester, standalone_tester,
                            standalone_tester::make_instance)

}  // namespace single
}  // namespace system
}  // namespace bacs
