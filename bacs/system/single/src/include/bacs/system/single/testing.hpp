#pragma once

#include "bacs/system/single/callback.hpp"
#include "bacs/system/single/tests.hpp"
#include "bacs/system/single/checker.hpp"
#include "bacs/system/single/builder.hpp"

#include "bacs/problem/single/settings.pb.h"
#include "bacs/problem/single/testing.pb.h"
#include "bacs/problem/single/result.pb.h"

#include "yandex/contest/invoker/Forward.hpp"

namespace bacs{namespace system{namespace single
{
    class testing
    {
    public:
        explicit testing(const problem::single::task::Callbacks &callbacks);

        void test(const problem::single::task::Solution &solution,
                  const problem::single::testing::SolutionTesting &testing);

    private:
        bool check_hash();

        bool build(const problem::single::task::Solution &solution);

        bool test(const problem::single::testing::SolutionTesting &testing);

    private:
        void send_intermediate();
        void send_result();

    private:
        /// Test submit.
        void test(const problem::single::testing::SolutionTesting &testing,
                  problem::single::result::SolutionTestingResult &result);

        /// Test single test group.
        bool test(const problem::single::testing::TestGroup &test_group,
                  problem::single::result::TestGroupResult &result);

        /// Test single test.
        bool test(const problem::single::settings::ProcessSettings &settings,
                  const std::string &test_id,
                  problem::single::result::TestResult &result);

    private:
        tests m_tests;
        checker m_checker;
        builder_ptr m_builder;
        solution_ptr m_solution;
        callback::result m_result_cb;
        callback::intermediate m_intermediate_cb;
        problem::single::intermediate::Result m_intermediate;
        problem::single::result::Result m_result;
        yandex::contest::invoker::ContainerPointer m_container;
    };
}}}
