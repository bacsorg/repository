#pragma once

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>

namespace bacs{namespace system{namespace single
{
    typedef std::unordered_map<std::string, boost::filesystem::path> file_map;
}}}
