#include <bacs/system/single/testing.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scope_exit.hpp>

#include <algorithm>
#include <functional>

#include <cstdint>

namespace bacs {
namespace system {
namespace single {

using namespace yandex::contest::invoker;
namespace unistd = yandex::contest::system::unistd;

static ContainerConfig testing_config(ContainerConfig config) {
  if (!config.lxcConfig.mount)
    config.lxcConfig.mount = yandex::contest::system::lxc::MountConfig();
  if (!config.lxcConfig.mount->entries)
    config.lxcConfig.mount->entries = std::vector<unistd::MountEntry>();

  filesystem::Directory dir;
  dir.mode = 0555;

  dir.path = testing::PROBLEM_ROOT;
  config.filesystemConfig.createFiles.push_back(filesystem::CreateFile(dir));

  const boost::filesystem::path bin = boost::filesystem::current_path() / "bin";
  dir.path = testing::PROBLEM_BIN;
  config.filesystemConfig.createFiles.push_back(filesystem::CreateFile(dir));
  if (boost::filesystem::exists(bin))
    config.lxcConfig.mount->entries->push_back(
        unistd::MountEntry::bindRO(bin, testing::PROBLEM_BIN));

  const boost::filesystem::path lib = boost::filesystem::current_path() / "lib";
  dir.path = testing::PROBLEM_LIB;
  config.filesystemConfig.createFiles.push_back(filesystem::CreateFile(dir));
  if (boost::filesystem::exists(lib))
    config.lxcConfig.mount->entries->push_back(
        unistd::MountEntry::bindRO(lib, testing::PROBLEM_LIB));

  return config;
}

testing::testing(const problem::single::task::Callbacks &callbacks)
    : m_container(Container::create(
          testing_config(ContainerConfig::fromEnvironment()))),
      m_checker(m_container),
      m_tester(m_container),
      m_result_cb(callbacks.result()) {
  if (callbacks.has_intermediate())
    m_intermediate_cb.assign(callbacks.intermediate());
  m_intermediate.set_state(problem::single::intermediate::INITIALIZED);
  send_intermediate();
}

void testing::test(const bacs::process::Buildable &solution,
                   const problem::single::testing::SolutionTesting &testing) {
  check_hash() && build(solution) && test(testing);
  send_result();
}

bool testing::check_hash() {
  // TODO
  m_result.mutable_system()->set_status(
      problem::single::result::SystemResult::OK);
  return true;
}

bool testing::build(const bacs::process::Buildable &solution) {
  m_intermediate.set_state(problem::single::intermediate::BUILDING);
  send_intermediate();
  return m_tester.build(solution, *m_result.mutable_build());
}

bool testing::test(const problem::single::testing::SolutionTesting &testing) {
  test(testing, *m_result.mutable_testing_result());
  // top-level testing is always successful
  return true;
}

void testing::send_intermediate() { m_intermediate_cb.call(m_intermediate); }

void testing::send_result() { m_result_cb.call(m_result); }

namespace {
bool satisfies_requirement(
    const problem::single::result::TestGroupResult &result,
    const problem::single::testing::Dependency::Requirement requirement) {
  using problem::single::testing::Dependency;
  using problem::single::result::TestResult;

  BOOST_ASSERT(result.has_executed());
  if (!result.executed()) return false;

  std::size_t all_number = 0, ok_number = 0, fail_number = 0;
  for (const TestResult &test : result.test()) {
    ++all_number;
    if (test.status() == TestResult::OK) {
      ++ok_number;
    } else {
      ++fail_number;
    }
  }

  switch (requirement) {
    case Dependency::ALL_OK:
      return ok_number == all_number;
    case Dependency::ALL_FAIL:
      return fail_number == all_number;
    case Dependency::AT_LEAST_ONE_OK:
      return ok_number >= 1;
    case Dependency::AT_MOST_ONE_FAIL:
      return fail_number >= 1;
    case Dependency::AT_LEAST_HALF_OK:
      return ok_number * 2 >= all_number;
    default:
      BOOST_ASSERT(false);
  }
}
}  // namespace

void testing::test(const problem::single::testing::SolutionTesting &testing,
                   problem::single::result::SolutionTestingResult &result) {
  using problem::single::testing::TestGroup;
  using problem::single::testing::Dependency;
  using problem::single::result::TestGroupResult;

  m_intermediate.set_state(problem::single::intermediate::TESTING);

  std::unordered_map<std::string, const TestGroup *> test_groups;
  std::unordered_map<std::string, TestGroupResult *> results;

  for (const TestGroup &test_group : testing.test_group()) {
    test_groups[test_group.id()] = &test_group;
    TestGroupResult &r = *result.add_test_group();
    results[test_group.id()] = &r;
    r.set_id(test_group.id());
  }

  std::unordered_set<std::string> in_test_group;
  const std::function<void(const TestGroup &)> run =
      [&](const TestGroup &test_group) {
        if (in_test_group.find(test_group.id()) != in_test_group.end())
          BOOST_THROW_EXCEPTION(
              test_group_circular_dependencies_error()
              << test_group_circular_dependencies_error::test_group(
                  test_group.id()));
        in_test_group.insert(test_group.id());
        BOOST_SCOPE_EXIT_ALL(&) { in_test_group.erase(test_group.id()); };

        TestGroupResult &result = *results.at(test_group.id());

        if (result.has_executed()) return;

        for (const Dependency &dependency : test_group.dependency()) {
          const auto iter = test_groups.find(dependency.test_group());
          if (iter == test_groups.end())
            BOOST_THROW_EXCEPTION(
                test_group_dependency_not_found_error()
                << test_group_dependency_not_found_error::test_group(
                       test_group.id())
                << test_group_dependency_not_found_error::test_group_dependency(
                       dependency.test_group()));
          const TestGroup &dep_test_group = *iter->second;

          run(dep_test_group);

          const TestGroupResult &dep_result =
              *results.at(dependency.test_group());
          if (!satisfies_requirement(dep_result, dependency.requirement())) {
            result.set_executed(false);
            return;
          }
        }
        result.set_executed(true);
        test(test_group, result);
      };

  for (const TestGroup &test_group : testing.test_group()) run(test_group);
}

bool testing::test(const problem::single::testing::TestGroup &test_group,
                   problem::single::result::TestGroupResult &result) {
  m_intermediate.set_test_group_id(test_group.id());
  result.set_id(test_group.id());
  result.set_executed(true);
  const problem::single::settings::TestGroupSettings &settings =
      test_group.settings();
  const std::unordered_set<std::string> test_set =
      m_tests.test_set(test_group.test_set());
  std::vector<std::string> test_order(test_set.begin(), test_set.end());
  std::function<bool(const std::string &, const std::string &)> less;
  switch (settings.run().order()) {
    case problem::single::settings::Run::NUMERIC:
      less = [](const std::string &left, const std::string &right) {
        return boost::lexical_cast<std::uint64_t>(left) <
               boost::lexical_cast<std::uint64_t>(right);
      };
      // strip from non-integer values
      test_order.erase(
          std::remove_if(test_order.begin(), test_order.end(),
                         [](const std::string &s) {
                           return !std::all_of(s.begin(), s.end(),
                                               boost::algorithm::is_digit());
                         }),
          test_order.end());
      break;
    case problem::single::settings::Run::LEXICOGRAPHICAL:
      less = std::less<std::string>();
      break;
  }
  std::sort(test_order.begin(), test_order.end(), less);
  bool skip = false;
  for (const std::string &test_id : test_order) {
    const bool ret = !skip
                         ? test(settings.process(), test_id, *result.add_test())
                         : skip_test(test_id, *result.add_test());
    switch (settings.run().algorithm()) {
      case problem::single::settings::Run::ALL:
        // ret does not matter
        break;
      case problem::single::settings::Run::WHILE_NOT_FAIL:
        if (!ret) skip = true;
        break;
    }
  }
  return true;  // note: empty test group is OK
}

bool testing::test(const problem::single::settings::ProcessSettings &settings,
                   const std::string &test_id,
                   problem::single::result::TestResult &result) {
  m_intermediate.set_test_id(test_id);
  send_intermediate();
  const bool ret = m_tester.test(settings, m_tests.test(test_id), result);
  result.set_id(test_id);
  return ret;
}

bool testing::skip_test(const std::string &test_id,
                        problem::single::result::TestResult &result) {
  m_intermediate.set_test_id(test_id);
  send_intermediate();
  result.set_id(test_id);
  result.set_status(problem::single::result::TestResult::SKIPPED);
  return false;
}

const boost::filesystem::path testing::PROBLEM_ROOT = "/problem_root";
const boost::filesystem::path testing::PROBLEM_BIN = PROBLEM_ROOT / "bin";
const boost::filesystem::path testing::PROBLEM_LIB = PROBLEM_ROOT / "lib";

}  // namespace single
}  // namespace system
}  // namespace bacs
