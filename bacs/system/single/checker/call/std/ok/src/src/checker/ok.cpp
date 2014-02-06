#include <bacs/system/single/checker.hpp>

namespace bacs{namespace system{namespace single
{
    class checker::impl {};

    checker::checker(const yandex::contest::invoker::ContainerPointer &/*container*/) {}
    checker::~checker() {}

    checker::result checker::check(const file_map &/*test_files*/, const file_map &/*solution_files*/)
    {
        return result();
    }
}}}
