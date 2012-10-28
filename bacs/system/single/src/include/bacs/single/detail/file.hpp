#pragma once

#include "bacs/single/api/pb/settings.pb.h"

#include <boost/filesystem/path.hpp>

#include <sys/types.h>

namespace bacs{namespace single{namespace detail{namespace file
{
    boost::filesystem::path to_path(const api::pb::settings::Path &path);
    void touch(const boost::filesystem::path &path);
    mode_t mode(const api::pb::settings::File::Permissions &value);
    mode_t mode(const google::protobuf::RepeatedField<int> &permissions);
}}}}
