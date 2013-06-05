#include "bacs/system/single/detail/file.hpp"

#include "bunsan/enable_error_info.hpp"
#include "bunsan/filesystem/fstream.hpp"

#include <algorithm>

namespace bacs{namespace system{namespace single{namespace detail{namespace file
{
    boost::filesystem::path to_path(const problem::single::settings::Path &path)
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
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            bunsan::filesystem::ofstream fout(path);
            fout.close();
        }
        BUNSAN_EXCEPTIONS_WRAP_END()
    }

    mode_t mode(const problem::single::settings::File::Permissions &value)
    {
        switch (value)
        {
        case problem::single::settings::File::READ:
            return 0444;
        case problem::single::settings::File::WRITE:
            return 0222;
        case problem::single::settings::File::EXECUTE:
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
            m |= mode(static_cast<problem::single::settings::File::Permissions>(p));
        }
        return m;
    }
}}}}}
