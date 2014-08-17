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

    static const unistd::access::Id OWNER_ID(1000, 1000);

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
            OWNER_ID,
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

        const ProcessPointer process = pimpl->solution->create(
            pimpl->process_group(),
            settings.execution().argument()
        );
        pimpl->setup(process, settings);
        process->setOwnerId(OWNER_ID);

        // note: ignore current_path from settings.execution()
        process->setCurrentPath(testing_path);

        pimpl->set_test_files(
            process,
            settings,
            test_,
            testing_path,
            OWNER_ID
        );

        pimpl->synchronized_call();
        pimpl->send_test_files(result);

        pimpl->parse_result(
            process,
            *result.mutable_execution()
        );
        pimpl->run_checker_if_ok(pimpl->checker_, result);

        return pimpl->fill_status(result);
    }
}}}
