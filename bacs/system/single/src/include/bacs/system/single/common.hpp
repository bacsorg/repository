#pragma once

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>

namespace bacs {
namespace system {
namespace single {

using file_map = std::unordered_map<std::string, boost::filesystem::path>;

}  // namespace single
}  // namespace system
}  // namespace bacs
