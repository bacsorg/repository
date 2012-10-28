#include "bacs/single/detail/file.hpp"

#include "bunsan/system_error.hpp"

#include <algorithm>

#include <boost/filesystem/fstream.hpp>

namespace bacs{namespace single{namespace detail{namespace file
{
    boost::filesystem::path to_path(const api::pb::settings::Path &path)
    {
        boost::filesystem::path p;
        if (path.has_root())
            p = path.root();
        for (const std::string &element: path.elements())
            p /= element;
        return p;
    }

    void touch(const boost::filesystem::path &path)
    {
        boost::filesystem::ofstream fout(path);
        if (fout.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("open"));
        fout.close();
        if (fout.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("close"));
    }

    mode_t mode(const api::pb::settings::File::Permissions &value)
    {
        switch (value)
        {
        case api::pb::settings::File::READ:
            return 0444;
        case api::pb::settings::File::WRITE:
            return 0222;
        case api::pb::settings::File::EXECUTE:
            return 0111;
        default:
            BOOST_ASSERT(false);
            return 0;
        }
    }

    mode_t mode(const google::protobuf::RepeatedField<int> &permissions)
    {
        mode_t m = 0;
        for (const int p: permissions)
        {
            m |= mode(static_cast<api::pb::settings::File::Permissions>(p));
        }
        return m;
    }
}}}}
