#pragma once

#include <bacs/system/callback.hpp>

#include <bacs/problem/single/intermediate.pb.h>
#include <bacs/problem/single/result.pb.h>

namespace bacs{namespace system{namespace single{namespace callback
{
    using system::callback::message_interface;

    typedef message_interface<problem::single::result::Result> result;
    typedef message_interface<problem::single::intermediate::Result> intermediate;
}}}}
