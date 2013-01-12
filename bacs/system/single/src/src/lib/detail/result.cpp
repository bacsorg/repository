#include "bacs/single/detail/result.hpp"

#include "bunsan/config/cast.hpp"

#include <sstream>

#include <boost/property_tree/json_parser.hpp>

namespace bacs{namespace single{namespace detail{namespace result
{
    using namespace yandex::contest::invoker;

    bool parse(const ProcessGroup::Result &process_group_result,
               const Process::Result &process_result,
               api::pb::result::Execution &result)
    {
        switch (process_group_result.completionStatus)
        {
        case ProcessGroup::Result::CompletionStatus::OK:
        case ProcessGroup::Result::CompletionStatus::ABNORMAL_EXIT:
            switch (process_result.completionStatus)
            {
            case Process::Result::CompletionStatus::OK:
                result.set_status(api::pb::result::Execution::OK);
                break;
            case Process::Result::CompletionStatus::ABNORMAL_EXIT:
                result.set_status(api::pb::result::Execution::ABNORMAL_EXIT);
                break;
            case Process::Result::CompletionStatus::MEMORY_LIMIT_EXCEEDED:
                result.set_status(api::pb::result::Execution::MEMORY_LIMIT_EXCEEDED);
                break;
            case Process::Result::CompletionStatus::USER_TIME_LIMIT_EXCEEDED:
                result.set_status(api::pb::result::Execution::TIME_LIMIT_EXCEEDED);
                break;
            case Process::Result::CompletionStatus::OUTPUT_LIMIT_EXCEEDED:
                result.set_status(api::pb::result::Execution::OUTPUT_LIMIT_EXCEEDED);
                break;
            case Process::Result::CompletionStatus::TERMINATED_BY_SYSTEM:
            case Process::Result::CompletionStatus::START_FAILED:
            case Process::Result::CompletionStatus::STOPPED:
                result.set_status(api::pb::result::Execution::FAILED);
                break;
            }
            break;
        case ProcessGroup::Result::CompletionStatus::REAL_TIME_LIMIT_EXCEEDED:
            result.set_status(api::pb::result::Execution::REAL_TIME_LIMIT_EXCEEDED);
            break;
        case ProcessGroup::Result::CompletionStatus::STOPPED:
            result.set_status(api::pb::result::Execution::FAILED);
            break;
        }
        if (process_result.exitStatus)
            result.set_exit_status(process_result.exitStatus.get());
        if (process_result.termSig)
            result.set_term_sig(process_result.termSig.get());
        {
            api::pb::ResourceUsage &resource_usage = *result.mutable_resource_usage();
            resource_usage.set_time_usage_millis(std::chrono::duration_cast<std::chrono::milliseconds>(
                process_result.resourceUsage.userTimeUsage).count());
            resource_usage.set_memory_usage_bytes(process_result.resourceUsage.memoryUsageBytes);
        }
        // full: dump all results here
        {
            boost::property_tree::ptree ptree;
            ptree.put_child("processGroupResult", bunsan::config::save<boost::property_tree::ptree>(process_group_result));
            ptree.put_child("processResult", bunsan::config::save<boost::property_tree::ptree>(process_result));
            std::ostringstream buf;
            boost::property_tree::write_json(buf, ptree);
            result.set_full(buf.str());
        }
        return result.status() == api::pb::result::Execution::OK;
    }
}}}}
