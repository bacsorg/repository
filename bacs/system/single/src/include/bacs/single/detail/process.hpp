#pragma once

#include "bacs/single/api/pb/resource.pb.h"
#include "bacs/single/api/pb/settings.pb.h"

#include "yandex/contest/invoker/Forward.hpp"

namespace bacs{namespace single{namespace detail{namespace process
{
    void setup(const api::pb::ResourceLimits &resource_limits,
               const yandex::contest::invoker::ProcessGroupPointer &process_group,
               const yandex::contest::invoker::ProcessPointer &process);
}}}}
