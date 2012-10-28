#include "bunsan/config.hpp"
#include "bunsan/system_error.hpp"

#include "bacs/single/tests.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "yandex/contest/serialization/unordered_set.hpp"

namespace bacs{namespace single
{
    class tests::impl
    {
    public:
        std::unordered_set<std::string> test_set, data_set;
    };

    tests::tests(): pimpl(new impl)
    {
        try
        {
            boost::filesystem::ifstream fin("etc/tests");
            if (fin.bad())
                BOOST_THROW_EXCEPTION(bunsan::system_error("open"));
            {
                boost::archive::text_iarchive ia(fin);
                ia >> pimpl->test_set >> pimpl->data_set;
            }
            if (fin.bad())
                BOOST_THROW_EXCEPTION(bunsan::system_error("read"));
            if (fin.bad())
                BOOST_THROW_EXCEPTION(bunsan::system_error("close"));
        }
        catch (...)
        {
            delete pimpl;
            throw;
        }
    }

    tests::~tests()
    {
        delete pimpl;
    }

    void tests::copy(const std::string &test_id, const std::string &data_id,
                     const boost::filesystem::path &path)
    {
        boost::filesystem::copy_file(location(test_id, data_id), path);
    }

    boost::filesystem::path tests::location(const std::string &test_id, const std::string &data_id)
    {
        return boost::filesystem::path("share/tests") / (test_id + "." + data_id);
    }

    std::unordered_set<std::string> tests::data_set()
    {
        return pimpl->data_set;
    }

    std::unordered_set<std::string> tests::test_set()
    {
        return pimpl->test_set;
    }
}}
