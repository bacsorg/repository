#pragma once

#include "bacs/single/api/pb/result.pb.h"

#include "yandex/contest/invoker/ProcessGroup.hpp"
#include "yandex/contest/invoker/Process.hpp"

namespace bacs{namespace single{namespace detail{namespace result
{
    void parse(const yandex::contest::invoker::ProcessGroup::Result &process_group_result,
               const yandex::contest::invoker::Process::Result &process_result,
               api::pb::result::Execution &result);
}}}}
