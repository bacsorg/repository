#include <bacs/system/single/testing.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <functional>

#include <cstdint>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    static ContainerConfig testing_config(ContainerConfig config)
    {
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

    testing::testing(const problem::single::task::Callbacks &callbacks):
        m_container(Container::create(testing_config(ContainerConfig::fromEnvironment()))),
        m_checker(m_container),
        m_tester(m_container),
        m_result_cb(callbacks.result())
    {
        if (callbacks.has_intermediate())
            m_intermediate_cb.assign(callbacks.intermediate());
        m_intermediate.set_state(problem::single::intermediate::INITIALIZED);
        send_intermediate();
    }

    void testing::test(const problem::single::task::Solution &solution,
                       const problem::single::testing::SolutionTesting &testing)
    {
        check_hash() && build(solution) && test(testing);
        send_result();
    }

    bool testing::check_hash()
    {
        // TODO
        m_result.mutable_system()->set_status(problem::single::result::SystemResult::OK);
        return true;
    }

    bool testing::build(const problem::single::task::Solution &solution)
    {
        m_intermediate.set_state(problem::single::intermediate::BUILDING);
        send_intermediate();
        return m_tester.build(solution, *m_result.mutable_build());
    }

    bool testing::test(const problem::single::testing::SolutionTesting &testing)
    {
        test(testing, *m_result.mutable_testing_result());
        // top-level testing is always successful
        return true;
    }

    void testing::send_intermediate()
    {
        m_intermediate_cb.call(m_intermediate);
    }

    void testing::send_result()
    {
        m_result_cb.call(m_result);
    }

    void testing::test(const problem::single::testing::SolutionTesting &testing,
                       problem::single::result::SolutionTestingResult &result)
    {
        m_intermediate.set_state(problem::single::intermediate::TESTING);
        // TODO test group dependencies
        for (const problem::single::testing::TestGroup &test_group: testing.test_group())
            test(test_group, *result.add_test_group());
    }

    bool testing::test(const problem::single::testing::TestGroup &test_group,
                       problem::single::result::TestGroupResult &result)
    {
        m_intermediate.set_test_group_id(test_group.id());
        result.set_id(test_group.id());
        const problem::single::settings::TestGroupSettings &settings = test_group.settings();
        const std::unordered_set<std::string> test_set = m_tests.test_set(test_group.test_set());
        std::vector<std::string> test_order(test_set.begin(), test_set.end());
        std::function<bool (const std::string &, const std::string &)> less;
        switch (settings.run().order())
        {
        case problem::single::settings::Run::NUMERIC:
            less =
                [](const std::string &left, const std::string &right)
                {
                    return boost::lexical_cast<std::uint64_t>(left) <
                           boost::lexical_cast<std::uint64_t>(right);
                };
            // strip from non-integer values
            test_order.erase(std::remove_if(test_order.begin(), test_order.end(),
                [](const std::string &s)
                {
                    return !std::all_of(s.begin(), s.end(), boost::algorithm::is_digit());
                }), test_order.end());
            break;
        case problem::single::settings::Run::LEXICOGRAPHICAL:
            less = std::less<std::string>();
            break;
        }
        std::sort(test_order.begin(), test_order.end(), less);
        for (const std::string &test_id: test_order)
        {
            const bool ret = test(settings.process(), test_id, *result.add_test());
            switch (settings.run().algorithm())
            {
            case problem::single::settings::Run::ALL:
                // ret does not matter
                break;
            case problem::single::settings::Run::WHILE_NOT_FAIL:
                if (!ret)
                    return false;
                break;
            }
        }
        return true; // note: empty test group is OK
    }

    bool testing::test(const problem::single::settings::ProcessSettings &settings,
                       const std::string &test_id,
                       problem::single::result::TestResult &result)
    {
        m_intermediate.set_test_id(test_id);
        send_intermediate();
        const bool ret = m_tester.test(settings, m_tests.test(test_id), result);
        result.set_id(test_id);
        return ret;
    }

    const boost::filesystem::path testing::PROBLEM_ROOT = "/problem_root";
    const boost::filesystem::path testing::PROBLEM_BIN = testing::PROBLEM_ROOT / "bin";
    const boost::filesystem::path testing::PROBLEM_LIB = testing::PROBLEM_ROOT / "lib";
}}}
