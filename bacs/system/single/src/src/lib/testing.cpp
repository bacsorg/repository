#include "bacs/single/testing.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single
{
    using namespace yandex::contest::invoker;

    testing::testing(const api::pb::task::Callbacks &callbacks):
        m_result_cb(callbacks.result()),
        m_container(Container::create(ContainerConfig::fromEnvironment()))
    {
        if (callbacks.has_intermediate())
            m_intermediate_cb.assign(callbacks.intermediate());
        m_intermediate.set_state(api::pb::intermediate::INITIALIZED);
        send_intermediate();
    }

    void testing::test(const api::pb::task::Solution &solution,
                       const api::pb::testing::SolutionTesting &testing)
    {
        check_hash() && build(solution) && test(testing);
        send_result();
    }

    bool testing::check_hash()
    {
        // TODO
        m_result.mutable_system()->set_status(api::pb::result::SystemResult::OK);
        return true;
    }

    bool testing::test(const api::pb::testing::SolutionTesting &testing)
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

    void testing::test(const api::pb::testing::SolutionTesting &testing,
                       api::pb::result::SolutionTestingResult &result)
    {
        m_intermediate.set_state(api::pb::intermediate::TESTING);
        // TODO test group dependencies
        for (const api::pb::testing::TestGroup &test_group: testing.test_groups())
            test(test_group, *result.add_test_groups());
    }

    bool testing::test(const api::pb::testing::TestGroup &test_group,
                       api::pb::result::TestGroupResult &result)
    {
        m_intermediate.set_test_group_id(test_group.id());
        const api::pb::settings::TestGroupSettings &settings = test_group.settings();
        const std::unordered_set<std::string> test_set = m_tests.test_set(test_group.test_set());
        std::vector<std::string> test_order(test_set.begin(), test_set.end());
        // TODO sort settings.run().order()
        for (const std::string &test_id: test_order)
        {
            const bool ret = test(settings, test_id, *result.add_tests());
            switch (settings.run().algorithm())
            {
            case api::pb::settings::Run::ALL:
                // ret does not matter
                break;
            case api::pb::settings::Run::WHILE_NOT_FAIL:
                if (!ret)
                    return false;
                break;
            }
        }
        return true; // note: empty test group is OK
    }
}}
