#include "bacs/system/single/detail/process.hpp"

#include "yandex/contest/invoker/All.hpp"

namespace bacs{namespace system{namespace single{namespace detail{namespace process
{
    using namespace yandex::contest::invoker;

    void setup(const problem::single::ResourceLimits &resource_limits,
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
            RLIM_UPDATE(time_limit_millis, timeLimit, std::chrono::milliseconds);
            RLIM_UPDATE(memory_limit_bytes, memoryLimitBytes, );
            RLIM_UPDATE(output_limit_bytes, outputLimitBytes, );
            RLIM_UPDATE(number_of_processes, numberOfProcesses, );
            process->setResourceLimits(rlimit);
        }
#undef RLIM_UPDATE
    }
}}}}}
