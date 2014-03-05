#include <bacs/system/single/tester.hpp>

#include <bacs/system/builder.hpp>
#include <bacs/system/single/error.hpp>
#include <bacs/system/single/detail/file.hpp>
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

    static const boost::filesystem::path testing_path = "/testing";

    static const unistd::access::Id SOLUTION_OWNER_ID(1000, 1000);
    static const unistd::access::Id INTERACTOR_OWNER_ID(1000, 1000);

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
        // initialize working directory
        const bunsan::tempfile tmpdir =
            bunsan::tempfile::directory_in_directory(container_testing_path);
        const boost::filesystem::path current_path = testing_path / tmpdir.path().filename();
        pimpl->container->filesystem().setOwnerId(current_path, INTERACTOR_OWNER_ID);
        pimpl->container->filesystem().setMode(current_path, 0500);
        // initialize process
        const ProcessGroupPointer process_group = pimpl->container->createProcessGroup();

        const ProcessPointer process = pimpl->solution->create(
            process_group, settings.execution().argument());
        system::process::setup(process_group, process, settings.resource_limits());
        process->setTerminateGroupOnCrash(false);

        const ProcessPointer interactor =
            process_group->createProcess(testing::PROBLEM_BIN / "interactor");
        interactor->setTerminateGroupOnCrash(false);

        const ProcessPointer pipectl =
            process_group->createProcess("yandex_contest_invoker_flowctl_pipectl");
        pipectl->setTerminateGroupOnCrash(false);

        ProcessEnvironment env = interactor->environment();
        // files
        file_map test_files, solution_files;
        for (const std::string &data_id: test_.data_set())
            test_files[data_id] = test_.location(data_id);

        // connect interactor and solution
        {
            const Pipe pipe01 = process_group->createPipe();
            const Pipe pipe02 = process_group->createPipe();
            process->setStream(0, pipe01.readEnd());
            pipectl->setStream(1, pipe01.writeEnd());
            pipectl->setStream(0, pipe02.readEnd());
            interactor->setStream(1, pipe02.writeEnd());

            const Pipe pipe1 = process_group->createPipe();
            interactor->setStream(0, pipe1.readEnd());
            process->setStream(1, pipe1.writeEnd());
        }

        {
            const auto data_set = test_.data_set();

            BOOST_ASSERT(data_set.find("in") != data_set.end());
            const boost::filesystem::path test_in = current_path / "in";
            env["JUDGE_TEST_IN"] = test_in.string();
            test_.copy("in", pimpl->container->filesystem().keepInRoot(test_in));
            pimpl->container->filesystem().setOwnerId(test_in, INTERACTOR_OWNER_ID);
            pimpl->container->filesystem().setMode(test_in, 0500);

            if (data_set.find("out") != data_set.end())
            {
                const boost::filesystem::path test_in = current_path / "hint";
                env["JUDGE_TEST_HINT"] = test_in.string();
                test_.copy("in", pimpl->container->filesystem().keepInRoot(test_in));
                pimpl->container->filesystem().setOwnerId(test_in, INTERACTOR_OWNER_ID);
                pimpl->container->filesystem().setMode(test_in, 0500);
            }

            const boost::filesystem::path interactor_output = current_path / "output";
            env["JUDGE_INTERACTOR_OUTPUT"] = interactor_output.string();
            detail::file::touch(pimpl->container->filesystem().keepInRoot(interactor_output));
            pimpl->container->filesystem().setOwnerId(interactor_output, INTERACTOR_OWNER_ID);
            pimpl->container->filesystem().setMode(interactor_output, 0500);

            const boost::filesystem::path interactor_custom_message = current_path / "custom_message";
            env["JUDGE_INTERACTOR_CUSTOM_MESSAGE"] = interactor_custom_message.string();
            detail::file::touch(pimpl->container->filesystem().keepInRoot(interactor_custom_message));
            pimpl->container->filesystem().setOwnerId(interactor_custom_message, INTERACTOR_OWNER_ID);
            pimpl->container->filesystem().setMode(interactor_custom_message, 0500);
        }

        interactor->setEnvironment(env);

        // execution
        process->setOwnerId(SOLUTION_OWNER_ID);
        interactor->setOwnerId(INTERACTOR_OWNER_ID);
        // note: ignore current_path from settings.execution()
        // note: current path is not used
        // note: arguments is already set
        // execute
        const ProcessGroup::Result process_group_result = process_group->synchronizedCall();
        const Process::Result process_result = process->result();
        const Process::Result interactor_result = interactor->result();
        const Process::Result pipectl_result = pipectl->result();
        // fill result
        const bool execution_success = process::parse_result(
            process_group_result, process_result, *result.mutable_execution());

        const bool interactor_execution_success = process::parse_result(
            process_group_result,
            interactor_result,
            *result.mutable_judge()->mutable_utilities()->mutable_interactor()->mutable_execution()
        );
        // TODO interactor's output

        {
            problem::single::result::Judge::AuxiliaryUtility &utility =
                *result.mutable_judge()->mutable_utilities()->add_auxiliary();
            utility.set_name("pipectl [interactor -> solution]");
            const bool pipectl_execution_success = process::parse_result(
                process_group_result,
                pipectl_result,
                *utility.mutable_utility()->mutable_execution()
            );
        }

        // analyze
        {
            const bacs::process::ExecutionResult &interactor_execution =
                result.judge().utilities().interactor().execution();

            problem::single::result::Judge &judge = *result.mutable_judge();

            switch (interactor_execution.status())
            {
            case bacs::process::ExecutionResult::OK:
                judge.set_status(problem::single::result::Judge::OK);
                break;
            case bacs::process::ExecutionResult::ABNORMAL_EXIT:
                if (interactor_execution.has_exit_status())
                {
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
                        judge.set_status(problem::single::result::Judge::INSUFFICIENT_DATA);
                        break;
                    case 100:
                        judge.set_status(problem::single::result::Judge::CUSTOM_FAILURE);
                        // TODO load custom message
                        judge.set_message("TODO (custom failure)");
                        break;
                    case 200:
                        judge.set_status(problem::single::result::Judge::FAIL_TEST);
                        break;
                    default:
                        judge.set_status(problem::single::result::Judge::FAILED);
                    }
                }
                else
                {
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
        }
        // TODO checker
        return result.execution().status() == bacs::process::ExecutionResult::OK &&
            result.judge().status() == problem::single::result::Judge::OK;
    }
}}}
