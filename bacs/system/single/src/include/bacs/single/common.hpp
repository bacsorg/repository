#pragma once

#include <string>
#include <unordered_map>

#include <boost/filesystem/path.hpp>

namespace bacs{namespace single
{
    typedef std::unordered_map<std::string, boost::filesystem::path> file_map;
}}
