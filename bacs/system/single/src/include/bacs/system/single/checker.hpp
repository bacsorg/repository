#pragma once

#include "bacs/system/single/common.hpp"

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace bacs{namespace system{namespace single
{
    /// \note must be implemented in problem
    class checker: private boost::noncopyable
    {
    public:
        struct result
        {
            enum status_type
            {
                OK,
                WRONG_ANSWER,
                PRESENTATION_ERROR,
                FAIL_TEST,
                FAILED
            };

            status_type status = status_type::OK;
            boost::optional<std::string> message;
            boost::optional<double> score;
            boost::optional<double> max_score;
        };

    public:
        checker();
        ~checker();

        result check(const file_map &test_files, const file_map &solution_files);

    private:
        class impl;

        impl *pimpl;
    };
}}}
