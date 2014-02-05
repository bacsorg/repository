#define BOOST_TEST_MODULE out_stdout
#include <boost/test/unit_test.hpp>

#include <bacs/system/single/detail/checker.hpp>

using namespace bacs::system::single;

checker::result::status_type test_equal(const std::string &out, const std::string &hint)
{
    std::stringstream out_(out), hint_(hint);
    return detail::checker::equal(out_, hint_);
}

BOOST_AUTO_TEST_CASE(ln)
{
    std::stringstream ss("\n\n");
    BOOST_CHECK(detail::checker::seek_eof(ss) == checker::result::OK);
    ss.clear();
    ss.str("");
    BOOST_CHECK(detail::checker::seek_eof(ss) == checker::result::OK);
}

BOOST_AUTO_TEST_CASE(equal)
{
    BOOST_CHECK(test_equal("", "") == checker::result::OK);
    BOOST_CHECK(test_equal("\n\n", "\n") == checker::result::OK);
    BOOST_CHECK(test_equal("n\n", "n") == checker::result::OK);
    BOOST_CHECK(test_equal("n", "n\n\n") == checker::result::OK);
    BOOST_CHECK(test_equal("n\nn", "n\nn\n") == checker::result::OK);
    BOOST_CHECK(test_equal("123", "1234") == checker::result::WRONG_ANSWER);
    BOOST_CHECK(test_equal("123\n", "1234") == checker::result::WRONG_ANSWER);
    BOOST_CHECK(test_equal("123\n", "123") == checker::result::OK);
}

// note: CR should not treated different
BOOST_AUTO_TEST_CASE(CR)
{
    BOOST_CHECK(test_equal("\r\n", "\r") == checker::result::OK);
    BOOST_CHECK(test_equal("123", "\r123\r\n\n") == checker::result::OK);
    BOOST_CHECK(test_equal("\r\r123\n\n\n\n\r\n", "123\n\n") == checker::result::OK);
    BOOST_CHECK(test_equal("123\n", "123\r\n") == checker::result::OK);
    BOOST_CHECK(test_equal("1\r2\r3", "123\n") == checker::result::OK);
    BOOST_CHECK(test_equal("123\r\n321", "123\n321") == checker::result::OK);
}
