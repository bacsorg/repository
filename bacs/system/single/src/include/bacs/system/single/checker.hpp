#pragma once

#include <bacs/system/single/common.hpp>

#include <yandex/contest/invoker/Forward.hpp>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <memory>

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
        explicit checker(const yandex::contest::invoker::ContainerPointer &container);
        ~checker();

        result check(const file_map &test_files, const file_map &solution_files);

    private:
        class impl;

        std::unique_ptr<impl> pimpl;
    };
}}}
