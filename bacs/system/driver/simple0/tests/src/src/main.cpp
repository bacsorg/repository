#include "bunsan/config.hpp"
#include "bunsan/system_error.hpp"

#include <unordered_set>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/assert.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "yandex/contest/serialization/unordered_set.hpp"

namespace
{
    void convert(const boost::filesystem::path &src,
                 const boost::filesystem::path &dst)
    {
        // FIXME simplified stub implementation
        boost::filesystem::copy_file(src, dst);
    }
}

int main(int argc, char *argv[])
{
    BOOST_ASSERT(argc >= 2 + 1);
    std::unordered_set<std::string> test_set, data_set, text_data_set;
    {
        boost::filesystem::ifstream fin("etc/tests");
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("open"));
        {
            boost::archive::text_iarchive ia(fin);
            ia >> test_set >> data_set >> text_data_set;
        }
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("read"));
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("close"));
    }
    {
        boost::filesystem::ofstream fout(argv[1]);
        if (fout.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("open"));
        {
            boost::archive::text_oarchive oa(fout);
            oa << test_set << data_set;
        }
        if (fout.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("write"));
        if (fout.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("close"));
    }
    const boost::filesystem::path dst_dir = argv[2];
    for (int i = 3; i < argc; ++i)
    {
        const boost::filesystem::path test = argv[i];
        const boost::filesystem::path dst = dst_dir / test.filename();
        const std::string data_id = test.extension().string();
        BOOST_ASSERT(!data_id.empty());
        if (text_data_set.find(data_id.substr(1)) != text_data_set.end())
            convert(test, dst);
        else
            boost::filesystem::copy(test, dst);
    }
}
