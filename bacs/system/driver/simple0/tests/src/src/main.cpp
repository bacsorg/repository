#include "bunsan/config.hpp"

#if 0
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/archive/text_iarchive.hpp>

    class tests::impl
    {
        std::unordered_set<std::string> test_set, data_set, text_data_set;
    };

    tests::tests():
        pimpl(new impl)
    {
        boost::filesystem::ifstream fin("etc/tests");
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("open"));
        {
            boost::text_iarchive ia(fin);
            ia >> test_set >> data_set >> text_data_set;
        }
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("read"));
        if (fin.bad())
            BOOST_THROW_EXCEPTION(bunsan::system_error("close"));
    }

    tests::~tests()
    {
        delete pimpl;
    }

    void tests::create(const std::string &test_id, const std::string &data_id,
                       const boost::filesystem::path &path)
    {
        const boost::filesystem::path name = test_id + "." + data_id;
        const boost::filesystem::path src = "share/tests" / name;
        if (pimpl->text_data_set.find(data_id) != pimpl->text-data_set.end())
        {
            //boost::filesystem::ifstream fin(src, std::ios::binary);
            //boost::filesystem::ofstream fout(path);
            // FIXME transform line endings
            boost::filesystem::copy_file(src, path);
        }
        else
        {
            boost::filesystem::copy_file(src, path);
        }
    }

    std::unordered_set<std::string> tests::data_set()
    {
        return pimpl->data_set;
    }

    std::unordered_set<std::string> tests::test_set()
    {
        return pimpl->test_set;
    }
#endif

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
    BOOST_ASSERT(argc >= 1 + 1);
    const boost::filesystem::path dst = argv[1];
    for (int i = 2; i < argc; ++i)
    {
        const boost::filesystem::path test = argv[i];
        convert(test, dst / test.filename());
    }
}
