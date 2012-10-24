#pragma once

#include "bacs/single/common.hpp"

#include "bacs/single/api/pb/result.hpp"

#include <boost/optional.hpp>

namespace bacs{namespace single{namespace checker
{
    struct result
    {
        typedef api::pb::result::Checking::Status status_type;

        status_type status;
        boost::optional<std::string> message;
        boost::optional<double> score;
        boost::optional<double> max_score;
    };

    /// \note must be implemented in problem
    result check(const file_map &test_files, const file_map &solution_files);
}}}
