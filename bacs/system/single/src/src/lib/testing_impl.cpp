#include "bacs/single/testing.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single
{
    using namespace yandex::contest::invoker;

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

    bool testing::test(const api::pb::settings::TestGroupSettings &settings,
                       const std::string &test_id,
                       api::pb::result::TestResult &result)
    {
        m_intermediate.set_test_id(test_id);
        send_intermediate();
        const ProcessGroupPointer processGroup = m_container->createProcessGroup();
        //const ProcessPointer process = processGroup->createProcess();
        // TODO
    }
}}
