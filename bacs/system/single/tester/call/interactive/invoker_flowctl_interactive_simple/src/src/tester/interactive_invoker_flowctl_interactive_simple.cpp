#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/file.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/detail/file.hpp>
#include <bacs/system/single/testing.hpp>

#include <bacs/system/process.hpp>

#include <yandex/contest/invoker/All.hpp>
#include <yandex/contest/invoker/flowctl/interactive/SimpleBroker.hpp>

#include <bunsan/filesystem/fstream.hpp>
#include <bunsan/tempfile.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

namespace bacs{namespace system{namespace single
{
    using namespace yandex::contest::invoker;
    namespace unistd = yandex::contest::system::unistd;
    typedef flowctl::interactive::SimpleBroker Broker;

    static const boost::filesystem::path testing_path = "/testing";
    static const boost::filesystem::path logging_path = "/logging";

    static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
    static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000);

    constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;

    class tester::impl
    {
    public:
        explicit impl(const ContainerPointer &container_):
            container(container_),
            checker_(container_) {}

        ContainerPointer container;
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
            pimpl->container,
            SOLUTION_OWNER_ID,
            solution.source(),
            solution.build_settings().resource_limits(),
            result);
        return static_cast<bool>(pimpl->solution);
    }

    bool tester::test(const problem::single::settings::ProcessSettings &settings,
                      const single::test &test_,
                      problem::single::result::TestResult &result)
    {
        // preinitialization
        const boost::filesystem::path container_testing_path =
            pimpl->container->filesystem().keepInRoot(testing_path);
        boost::filesystem::create_directories(container_testing_path);

        const boost::filesystem::path container_logging_path =
            pimpl->container->filesystem().keepInRoot(logging_path);
        boost::filesystem::create_directories(container_logging_path);

        // initialize working directory
        const bunsan::tempfile tmpdir =
            bunsan::tempfile::directory_in_directory(container_testing_path);
        const boost::filesystem::path current_path =
            testing_path / tmpdir.path().filename();
        pimpl->container->filesystem().setOwnerId(
            current_path,
            INTERACTOR_OWNER_ID
        );
        pimpl->container->filesystem().setMode(current_path, 0500);

        const bunsan::tempfile broker_log =
            bunsan::tempfile::regular_file_in_directory(container_logging_path);
        const boost::filesystem::path broker_log_path =
            logging_path / broker_log.path().filename();

        const bunsan::tempfile interactor_log =
            bunsan::tempfile::regular_file_in_directory(container_logging_path);
        const boost::filesystem::path interactor_log_path =
            logging_path / interactor_log.path().filename();

        // initialize process
        const ProcessGroupPointer process_group =
            pimpl->container->createProcessGroup();

        const ProcessPointer solution = pimpl->solution->create(
            process_group, settings.execution().argument());
        solution->setName("solution");
        system::process::setup(process_group, solution, settings.resource_limits());
        solution->setTerminateGroupOnCrash(false);

        const ProcessPointer interactor =
            process_group->createProcess(testing::PROBLEM_BIN / "interactor");
        interactor->setName("interactor");
        interactor->setTerminateGroupOnCrash(false);

        const ProcessPointer broker =
            process_group->createProcess(
                "yandex_contest_invoker_flowctl_interactive_simple_broker");
        broker->setName("broker");
        broker->setArguments(
            broker->executable(),
            "--notifier", "3",
            "--interactor-source", "4",
            "--interactor-sink", "5",
            "--solution-source", "6",
            "--solution-sink", "7",
            "--output-limit-bytes", "67108864", // FIXME
            "--termination-real-time-limit-millis", "4000" // FIXME
        );

        ProcessEnvironment env = interactor->environment();

        // files
        file_map test_files, solution_files;
        for (const std::string &data_id: test_.data_set())
            test_files[data_id] = test_.location(data_id);

        // notifier
        {
            const Pipe pipe = process_group->createPipe();
            process_group->addNotifier(pipe.writeEnd());
            broker->setStream(3, pipe.readEnd());
        }

        // connect interactor and broker
        {
            const Pipe source = process_group->createPipe();
            const Pipe sink = process_group->createPipe();

            broker->setStream(5, sink.writeEnd());
            interactor->setStream(0, sink.readEnd());

            interactor->setStream(1, source.writeEnd());
            broker->setStream(4, source.readEnd());
        }

        // connect solution and broker
        {
            const Pipe source = process_group->createPipe();
            const Pipe sink = process_group->createPipe();

            broker->setStream(7, sink.writeEnd());
            solution->setStream(0, sink.readEnd());

            solution->setStream(1, source.writeEnd());
            broker->setStream(6, source.readEnd());
        }

        // logging
        {
            broker->setStream(2, File(broker_log_path, AccessMode::WRITE_ONLY));
            interactor->setStream(2, File(interactor_log_path, AccessMode::WRITE_ONLY));
        }

        // interactor files
        const auto data_set = test_.data_set();

        BOOST_ASSERT(data_set.find("in") != data_set.end());
        const boost::filesystem::path test_in = current_path / "in";
        env["JUDGE_TEST_IN"] = test_in.string();
        test_.copy("in", pimpl->container->filesystem().keepInRoot(test_in));
        pimpl->container->filesystem().setOwnerId(test_in, INTERACTOR_OWNER_ID);
        pimpl->container->filesystem().setMode(test_in, 0400);

        if (data_set.find("out") != data_set.end())
        {
            const boost::filesystem::path test_in = current_path / "hint";
            env["JUDGE_TEST_HINT"] = test_in.string();
            test_.copy("in", pimpl->container->filesystem().keepInRoot(test_in));
            pimpl->container->filesystem().setOwnerId(test_in, INTERACTOR_OWNER_ID);
            pimpl->container->filesystem().setMode(test_in, 0600);
        }

        const boost::filesystem::path interactor_output = current_path / "output";
        env["JUDGE_INTERACTOR_OUTPUT"] = interactor_output.string();
        detail::file::touch(pimpl->container->filesystem().keepInRoot(interactor_output));
        pimpl->container->filesystem().setOwnerId(interactor_output, INTERACTOR_OWNER_ID);
        pimpl->container->filesystem().setMode(interactor_output, 0600);

        const boost::filesystem::path interactor_custom_message = current_path / "custom_message";
        env["JUDGE_INTERACTOR_CUSTOM_MESSAGE"] = interactor_custom_message.string();
        detail::file::touch(pimpl->container->filesystem().keepInRoot(interactor_custom_message));
        pimpl->container->filesystem().setOwnerId(interactor_custom_message, INTERACTOR_OWNER_ID);
        pimpl->container->filesystem().setMode(interactor_custom_message, 0600);

        interactor->setEnvironment(env);

        // execution
        solution->setOwnerId(SOLUTION_OWNER_ID);
        interactor->setOwnerId(INTERACTOR_OWNER_ID);
        // note: ignore current_path from settings.execution()
        // note: current path is not used
        // note: arguments is already set

        // execute
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
        const Process::Result solution_result = solution->result();
        const Process::Result interactor_result = interactor->result();
        const Process::Result broker_result = broker->result();

        // fill result
        problem::single::result::Judge &judge = *result.mutable_judge();

        const bool execution_success = process::parse_result(
            process_group_result,
            solution_result,
            *result.mutable_execution()
        );

        // utilities
        const bool interactor_execution_success = process::parse_result(
            process_group_result,
            interactor_result,
            *judge.
                mutable_utilities()->
                mutable_interactor()->
                mutable_execution()
        );
        const bacs::process::ExecutionResult &interactor_execution =
            result.judge().utilities().interactor().execution();
        judge.mutable_utilities()->mutable_interactor()->set_output(
            bacs::system::file::read_first(interactor_log.path(), MAX_MESSAGE_SIZE));

        problem::single::result::Judge::AuxiliaryUtility &broker_utility =
            *judge.mutable_utilities()->add_auxiliary();
        broker_utility.set_name("broker");
        const bool broker_execution_success = process::parse_result(
            process_group_result,
            broker_result,
            *broker_utility.mutable_utility()->mutable_execution()
        );
        const bacs::process::ExecutionResult &broker_execution =
            broker_utility.utility().execution();
        broker_utility.mutable_utility()->set_output(
            bacs::system::file::read_first(broker_log.path(), MAX_MESSAGE_SIZE));

        // analyze
        if (result.execution().status() ==
            bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED)
        {
            judge.set_status(problem::single::result::Judge::SKIPPED);
            goto return_;
        }
        if (!broker_execution.has_exit_status())
            goto failed_;
        switch (broker_execution.status())
        {
        case bacs::process::ExecutionResult::OK:
        case bacs::process::ExecutionResult::ABNORMAL_EXIT:
            switch (broker_execution.exit_status())
            {
            case Broker::SOLUTION_TERMINATION_REAL_TIME_LIMIT_EXCEEDED:
                if (!interactor_execution.has_exit_status())
                {
                    judge.set_status(
                        problem::single::result::Judge::TERMINATION_REAL_TIME_LIMIT_EXCEEDED);
                }
                else if (interactor_execution.exit_status() == 0)
                {
                    if (result.execution().status() == bacs::process::ExecutionResult::OK)
                        judge.set_status(
                            problem::single::result::Judge::TERMINATION_REAL_TIME_LIMIT_EXCEEDED);
                    else
                        judge.set_status(problem::single::result::Judge::SKIPPED);
                }
                else
                {
                    goto broker_ok_;
                }
                break;
            case Broker::OK:
            broker_ok_:
                if (!interactor_execution.has_exit_status())
                    goto failed_;
                switch (interactor_execution.exit_status())
                {
                case 0:
                    judge.set_status(problem::single::result::Judge::OK);
                    break;
                case 1:
                    judge.set_status(problem::single::result::Judge::WRONG_ANSWER);
                    break;
                case 2:
                    judge.set_status(problem::single::result::Judge::PRESENTATION_ERROR);
                    break;
                case 3:
                    judge.set_status(problem::single::result::Judge::QUERIES_LIMIT_EXCEEDED);
                    break;
                case 4:
                    judge.set_status(problem::single::result::Judge::INCORRECT_REQUEST);
                    break;
                case 5:
                    if (result.execution().status() == bacs::process::ExecutionResult::OK)
                        judge.set_status(problem::single::result::Judge::INSUFFICIENT_DATA);
                    else
                        judge.set_status(problem::single::result::Judge::SKIPPED);
                    break;
                case 100:
                    judge.set_status(problem::single::result::Judge::CUSTOM_FAILURE);
                    judge.set_message(
                        bacs::system::file::read_first(
                            pimpl->container->filesystem().keepInRoot(
                                interactor_custom_message
                            ),
                            MAX_MESSAGE_SIZE
                        )
                    );
                    break;
                case 200:
                    judge.set_status(problem::single::result::Judge::FAIL_TEST);
                    break;
                default:
                    judge.set_status(problem::single::result::Judge::FAILED);
                }
                break;
            case Broker::SOLUTION_OUTPUT_LIMIT_EXCEEDED:
                judge.set_status(problem::single::result::Judge::OUTPUT_LIMIT_EXCEEDED);
                break;
            case Broker::SOLUTION_EXCESS_DATA:
                judge.set_status(problem::single::result::Judge::EXCESS_DATA);
                break;
            case Broker::FAILED:
            case Broker::INTERACTOR_OUTPUT_LIMIT_EXCEEDED:
            case Broker::INTERACTOR_TERMINATION_REAL_TIME_LIMIT_EXCEEDED:
            case Broker::INTERACTOR_EXCESS_DATA:
            default:
                judge.set_status(problem::single::result::Judge::FAILED);
            }
            break;
        case bacs::process::ExecutionResult::MEMORY_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::TIME_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::OUTPUT_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::REAL_TIME_LIMIT_EXCEEDED:
        case bacs::process::ExecutionResult::TERMINATED_BY_SYSTEM:
        case bacs::process::ExecutionResult::FAILED:
            judge.set_status(problem::single::result::Judge::FAILED);
            break;
        }

        if (result.execution().status() == bacs::process::ExecutionResult::OK &&
            result.judge().status() == problem::single::result::Judge::OK)
        {
            solution_files["stdout"] = pimpl->container->filesystem().keepInRoot(interactor_output);
            pimpl->checker_.check(test_files, solution_files, *result.mutable_judge());
        }

        goto return_;

    failed_:
        judge.set_status(problem::single::result::Judge::FAILED);

    return_:
        result.set_status(
            result.execution().status() == bacs::process::ExecutionResult::OK &&
            result.judge().status() == problem::single::result::Judge::OK ?
                problem::single::result::TestResult::OK :
                problem::single::result::TestResult::FAILED
        );
        return result.status() == problem::single::result::TestResult::OK;
    }
}}}
