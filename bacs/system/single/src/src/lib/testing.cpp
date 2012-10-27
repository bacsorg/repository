#include "bacs/single/testing.hpp"

namespace bacs{namespace single
{
    testing::testing(const api::pb::task::Callbacks &callbacks):
        m_result_cb(callbacks.result())
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

    bool testing::build(const api::pb::task::Solution &solution)
    {
        m_intermediate.set_state(api::pb::intermediate::BUILDING);
        send_intermediate();
        // TODO
        api::pb::result::BuildResult &build = *m_result.mutable_build();
        build.set_output("TODO");
        build.mutable_execution()->set_status(api::pb::result::Execution::OK);
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
        // TODO
    }

    void testing::test(const api::pb::testing::TestGroup &test_group,
                       api::pb::result::TestGroupResult &result)
    {
        m_intermediate.set_test_group_id(test_group.id());
        // TODO
    }

    void testing::test(const api::pb::settings::TestGroupSettings &settings,
                       const std::string &test_id,
                       api::pb::result::TestResult &result)
    {
        m_intermediate.set_test_id(test_id);
        send_intermediate();
        // TODO
    }
}}
