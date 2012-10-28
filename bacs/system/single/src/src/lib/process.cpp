#include "bacs/single/process.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace single{namespace process
{
    using namespace yandex::contest::invoker;

    void setup(const api::pb::ResourceLimits &resource_limits,
               const yandex::contest::invoker::ProcessGroupPointer &process_group,
               const yandex::contest::invoker::ProcessPointer &process)
    {
#define RLIM_UPDATE(SRC, DST, TR) \
        if (resource_limits.has_##SRC()) \
            rlimit.DST = TR(resource_limits.SRC())
        {
            ProcessGroup::ResourceLimits rlimit = process_group->resourceLimits();
            RLIM_UPDATE(real_time_limit_millis, realTimeLimit, std::chrono::milliseconds);
            process_group->setResourceLimits(rlimit);
        }
        {
            Process::ResourceLimits rlimit = process->resourceLimits();
            RLIM_UPDATE(time_limit_millis, userTimeLimit, std::chrono::milliseconds);
            RLIM_UPDATE(memory_limit_bytes, memoryLimitBytes, );
            RLIM_UPDATE(output_limit_bytes, outputLimitBytes, );
            RLIM_UPDATE(number_of_processes, numberOfProcesses, );
            process->setResourceLimits(rlimit);
        }
#undef RLIM_UPDATE
    }
}}}
