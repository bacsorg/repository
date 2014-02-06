#include <bacs/system/single/checker.hpp>

namespace bacs{namespace system{namespace single
{
    class checker::impl {};

    checker::checker(const yandex::contest::invoker::ContainerPointer &/*container*/) {}
    checker::~checker() {}

    bool checker::check(
        const file_map &/*test_files*/,
        const file_map &/*solution_files*/,
        problem::single::result::Judge &result)
    {
        result.set_status(problem::single::result::Judge::OK);
        return true;
    }
}}}
