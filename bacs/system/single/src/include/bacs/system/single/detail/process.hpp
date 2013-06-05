#pragma once

#include "bacs/problem/single/resource.pb.h"
#include "bacs/problem/single/settings.pb.h"

#include "yandex/contest/invoker/Forward.hpp"

namespace bacs{namespace system{namespace single{namespace detail{namespace process
{
    void setup(const problem::single::ResourceLimits &resource_limits,
               const yandex::contest::invoker::ProcessGroupPointer &process_group,
               const yandex::contest::invoker::ProcessPointer &process);
}}}}}
