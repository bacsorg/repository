#pragma once

#include "bacs/single/callback.hpp"
#include "bacs/single/tests.hpp"
#include "bacs/single/checker.hpp"
#include "bacs/single/validator.hpp"

#include "bacs/single/api/pb/settings.pb.h"
#include "bacs/single/api/pb/testing.pb.h"
#include "bacs/single/api/pb/result.pb.h"

#include "yandex/contest/invoker/Forward.hpp"

namespace bacs{namespace single
{
    class testing
    {
    public:
        explicit testing(const api::pb::task::Callbacks &callbacks);

        void test(const api::pb::task::Solution &solution,
                  const api::pb::testing::SolutionTesting &testing);

    private:
        bool check_hash();

        bool build(const api::pb::task::Solution &solution);

        bool test(const api::pb::testing::SolutionTesting &testing);

    private:
        void send_intermediate();
        void send_result();

    private:
        void test(const api::pb::testing::SolutionTesting &testing,
                  api::pb::result::SolutionTestingResult &result);

        bool test(const api::pb::testing::TestGroup &test_group,
                  api::pb::result::TestGroupResult &result);

        bool test(const api::pb::settings::TestGroupSettings &settings,
                  const std::string &test_id,
                  api::pb::result::TestResult &result);

    private:
        tests m_tests;
        checker m_checker;
        validator m_validator;
        callback::result m_result_cb;
        callback::intermediate m_intermediate_cb;
        api::pb::intermediate::Result m_intermediate;
        api::pb::result::Result m_result;
        yandex::contest::invoker::ContainerPointer m_container;
    };
}}
