#include "bunsan/config.hpp"

#include "bacs/single/tests.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

namespace bacs{namespace single{namespace tests
{
    void create(const std::string &test_id, const std::string &data_id,
                const boost::filesystem::path &path)
    {
        const boost::filesystem::path name = test_id + "." + data_id;
        const boost::filesystem::path src = "share/tests" / name;
        if (boost::filesystem::exists("etc/tests/convert" / name))
        {
            boost::filesystem::ifstream fin(src, std::ios::binary);
            boost::filesystem::ofstream fout(path);
            // TODO
        }
        else
        {
            boost::filesystem::copy_file(src, path);
        }
    }

    std::unordered_set<std::string> data_set()
    {
        // TODO
    }

    std::unordered_set<std::string> test_set()
    {
        // TODO
    }
}}}
