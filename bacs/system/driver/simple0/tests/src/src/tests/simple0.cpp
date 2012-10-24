#include "bunsan/config.hpp"

#include "bacs/single/tests.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

namespace bacs{namespace single
{
    class tests::impl
    {
        std::unordered_set<std::string> test_set, data_set, text_data_set;
    };

    tests::tests():
        pimpl(new impl)
    {
        boost::property_tree::ptree tests_info;
        // TODO load internal data
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
            boost::filesystem::ifstream fin(src, std::ios::binary);
            boost::filesystem::ofstream fout(path);
            // TODO
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
}}
