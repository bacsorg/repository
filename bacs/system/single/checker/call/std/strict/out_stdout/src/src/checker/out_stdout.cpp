#include <bacs/system/single/detail/checker.hpp>

#include <bunsan/enable_error_info.hpp>
#include <bunsan/filesystem/fstream.hpp>

namespace bacs{namespace system{namespace single
{
    class checker::impl {};

    checker::checker() {}
    checker::~checker() {}

    checker::result checker::check(const file_map &test_files, const file_map &solution_files)
    {
        result result_;
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            bunsan::filesystem::ifstream hint(test_files.at("out"), std::ios::binary),
                out(solution_files.at("stdout"), std::ios::binary);
            result_.status = detail::checker::equal(out, hint);
        }
        BUNSAN_EXCEPTIONS_WRAP_END()
        return result_;
    }
}}}
