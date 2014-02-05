#pragma once

#include <bacs/problem/single/result.pb.h>

#include <yandex/contest/invoker/Process.hpp>
#include <yandex/contest/invoker/ProcessGroup.hpp>

namespace bacs{namespace system{namespace single{namespace detail{namespace result
{
    /// \return true if process has completed successfully
    bool parse(const yandex::contest::invoker::ProcessGroup::Result &process_group_result,
               const yandex::contest::invoker::Process::Result &process_result,
               problem::single::result::Execution &result);
}}}}}
