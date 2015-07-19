#pragma once

#include <bacs/problem/single/settings.pb.h>

#include <boost/filesystem/path.hpp>

#include <sys/types.h>

namespace bacs {
namespace system {
namespace single {
namespace detail {
namespace file {

boost::filesystem::path to_path(const problem::single::settings::Path &path);
void touch(const boost::filesystem::path &path);
mode_t mode(const problem::single::settings::File::Permissions &value);
mode_t mode(const google::protobuf::RepeatedField<int> &permissions);

}  // namespace file
}  // namespace detail
}  // namespace single
}  // namespace system
}  // namespace bacs
