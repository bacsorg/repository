#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/file.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/detail/file.hpp>
#include <bacs/system/single/detail/tester.hpp>
#include <bacs/system/single/testing.hpp>

#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <bunsan/filesystem/fstream.hpp>
#include <bunsan/tempfile.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;

    static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
    static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000); // FIXME

    constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

    class tester::impl: public detail::tester
    {
    public:
        explicit impl(const ContainerPointer &container):
            tester(container, "/testing"),
            checker_(container) {}

        builder_ptr builder;
        executable_ptr solution;
        checker checker_;
    };

    tester::tester(const yandex::contest::invoker::ContainerPointer &container):
        pimpl(new impl(container)) {}

    tester::~tester() { /* implicit destructor */ }

    bool tester::build(
        const bacs::process::Buildable &solution,
        bacs::process::BuildResult &result)
    {
        pimpl->builder = builder::instance(solution.build_settings().builder());
        pimpl->solution = pimpl->builder->build(
            pimpl->container(),
            SOLUTION_OWNER_ID,
            solution.source(),
            solution.build_settings().resource_limits(),
            result
        );
        return static_cast<bool>(pimpl->solution);
    }

    bool tester::test(const problem::single::settings::ProcessSettings &settings,
                      const single::test &test_,
                      problem::single::result::TestResult &result)
    {
        pimpl->reset();

        const boost::filesystem::path testing_path =
            pimpl->create_directory("testing", unistd::access::Id{0, 0}, 0555);
        const boost::filesystem::path logging_path =
            pimpl->create_directory("logging", unistd::access::Id{0, 0}, 0555);

        const ProcessPointer process = pimpl->solution->create(
            pimpl->process_group(),
            settings.execution().argument()
        );
        process->setName("solution");
        pimpl->setup(process, settings);
        process->setOwnerId(SOLUTION_OWNER_ID);
        process->setTerminateGroupOnCrash(false);
        process->setGroupWaitsForTermination(false);

        const ProcessPointer interactor =
            pimpl->create_process(testing::PROBLEM_BIN / "interactor");
        interactor->setName("interactor");
        interactor->setOwnerId(INTERACTOR_OWNER_ID);
        interactor->setTerminateGroupOnCrash(true);

        interactor->setStream(
            0,
            pimpl->add_notifier(NotificationStream::Protocol::PLAIN_TEXT)
        );

        // note: ignore current_path from settings.execution()
        process->setCurrentPath(testing_path);

        pimpl->set_test_files(
            process,
            settings,
            test_,
            testing_path,
            SOLUTION_OWNER_ID
        );

        const auto data_set = test_.data_set();
        if (data_set.find("in") == data_set.end())
            BOOST_THROW_EXCEPTION(
                error() <<
                error::message("Test should contain \"in\" data"));

        const boost::filesystem::path test_in = testing_path / "in";
        pimpl->copy_test_file(
            test_,
            "in",
            test_in,
            INTERACTOR_OWNER_ID,
            0400
        );
        interactor->setEnvironment(
            "JUDGE_TEST_IN",
            test_in.string()
        );

        if (data_set.find("out") != data_set.end())
        {
            const boost::filesystem::path test_hint = testing_path / "hint";
            pimpl->copy_test_file(
                test_,
                "out",
                test_hint,
                INTERACTOR_OWNER_ID,
                0400
            );
            interactor->setEnvironment(
                "JUDGE_TEST_HINT",
                test_hint.string()
            );
        }

        const boost::filesystem::path interactor_output =
            testing_path / "output";
        pimpl->touch_test_file(
            interactor_output,
            INTERACTOR_OWNER_ID,
            0600
        );
        interactor->setEnvironment(
            "JUDGE_INTERACTOR_OUTPUT",
            interactor_output.string()
        );

        const boost::filesystem::path interactor_custom_message =
            testing_path / "custom_message";
        pimpl->touch_test_file(
            interactor_custom_message,
            INTERACTOR_OWNER_ID,
            0600
        );
        interactor->setEnvironment(
            "JUDGE_INTERACTOR_CUSTOM_MESSAGE",
            interactor_custom_message.string()
        );

        const boost::filesystem::path interactor_log_path =
            testing_path / "interactor_log";
        pimpl->touch_test_file(
            interactor_log_path,
            INTERACTOR_OWNER_ID,
            0600
        );
        interactor->setStream(
            2,
            File(interactor_log_path, AccessMode::WRITE_ONLY)
        );

        pimpl->synchronized_call();
        pimpl->send_test_files(result);

        problem::single::result::Judge &judge = *result.mutable_judge();
        const bool execution_success = pimpl->parse_result(
            process,
            *result.mutable_execution()
        );
        const bool interactor_execution_success = pimpl->parse_result(
            interactor,
            *result.
                mutable_judge()->
                mutable_utilities()->
                mutable_interactor()->
                mutable_execution()
        );
        const bacs::process::ExecutionResult &interactor_execution =
            judge.utilities().interactor().execution();
        judge.mutable_utilities()->mutable_interactor()->set_output(
            pimpl->read_first(
                interactor_log_path,
                MAX_MESSAGE_SIZE
            )
        );

        // analyze
        if (result.execution().status() ==
            bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED)
        {
            judge.set_status(problem::single::result::Judge::SKIPPED);
            goto return_;
        }
        if (!interactor_execution.has_exit_status())
            goto failed_;
        judge.set_status(return_cast(interactor_execution.exit_status()));
        switch (judge.status())
        {
        case problem::single::result::Judge::INSUFFICIENT_DATA:
            if (result.execution().status() != bacs::process::ExecutionResult::OK)
                judge.set_status(problem::single::result::Judge::SKIPPED);
            break;
        case problem::single::result::Judge::CUSTOM_FAILURE:
            judge.set_message(pimpl->read_first(
                interactor_custom_message,
                MAX_MESSAGE_SIZE
            ));
            break;
        }

        pimpl->use_solution_file("stdout", interactor_output);
        pimpl->run_checker_if_ok(pimpl->checker_, result);

        goto return_;

    failed_:
        judge.set_status(problem::single::result::Judge::FAILED);

    return_:
        return pimpl->fill_status(result);
    }
}}}
