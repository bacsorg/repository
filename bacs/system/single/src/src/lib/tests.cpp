#include <bacs/system/single/tests.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/regex.hpp>

#include <memory>

#include <fnmatch.h>

namespace bacs{namespace system{namespace single
{
    namespace
    {
        class matcher
        {
        public:
            template <typename T>
            explicit matcher(const T &query):
                m_impl(impl_(query)) {}

            matcher(const matcher &)=default;
            matcher &operator=(const matcher &)=default;

            bool operator()(const std::string &test_id) const
            {
                BOOST_ASSERT(m_impl);
                return m_impl->match(test_id);
            }

        private:
            class impl
            {
            public:
                virtual ~impl() {}

                virtual bool match(const std::string &test_id) const=0;
            };

            class id: public impl
            {
            public:
                explicit id(const std::string &test_id): m_test_id(test_id) {}

                bool match(const std::string &test_id) const override
                {
                    return test_id == m_test_id;
                }

            private:
                const std::string m_test_id;
            };

            class wildcard: public impl
            {
            public:
                explicit wildcard(
                    const problem::single::testing::WildcardQuery &query):
                        m_wildcard(query.value()), m_flags(flags(query)) {}

                bool match(const std::string &test_id) const override
                {
                    const int m = ::fnmatch(
                        m_wildcard.c_str(),
                        test_id.c_str(),
                        m_flags
                    );
                    if (m && m != FNM_NOMATCH)
                        BOOST_THROW_EXCEPTION(
                            error() <<
                            error::message("fnmatch() has failed"));
                    return !m;
                }

            private:
                int flags(const problem::single::testing::WildcardQuery &query)
                {
                    int flags_ = 0;
                    for (const int flag: query.flag())
                    {
                        switch (static_cast<
                            problem::single::testing::WildcardQuery::Flag>(flag))
                        {
                        case problem::single::testing::WildcardQuery::IGNORE_CASE:
                            flags_ |= FNM_CASEFOLD;
                            break;
                        }
                    }
                    return flags_;
                }

            private:
                const std::string m_wildcard;
                int m_flags;
            };

            class regex: public impl
            {
            public:
                explicit regex(const problem::single::testing::RegexQuery &query):
                    m_regex(query.value(), flags(query)) {}

                bool match(const std::string &test_id) const override
                {
                    return boost::regex_match(test_id, m_regex);
                }

            private:
                boost::regex_constants::syntax_option_type flags(
                    const problem::single::testing::RegexQuery &query)
                {
                    boost::regex_constants::syntax_option_type flags_ =
                        boost::regex_constants::normal;
                    for (const int flag: query.flag())
                    {
                        switch (static_cast<
                            problem::single::testing::RegexQuery::Flag>(flag))
                        {
                        case problem::single::testing::RegexQuery::IGNORE_CASE:
                            flags_ |= boost::regex_constants::icase;
                            break;
                        }
                    }
                    return flags_;
                }

            private:
                const boost::regex m_regex;
            };

            class any_of: public impl, public std::vector<std::unique_ptr<impl>>
            {
            public:
                bool match(const std::string &test_id) const override
                {
                    for (const std::unique_ptr<impl> &imp: *this)
                        if (imp->match(test_id))
                            return true;
                    return false;
                }
            };

        private:
            static std::unique_ptr<impl> impl_(
                const google::protobuf::RepeatedPtrField<
                    problem::single::testing::TestQuery> &test_query)
            {
                std::unique_ptr<any_of> tmp(new any_of);
                for (const problem::single::testing::TestQuery &query: test_query)
                    tmp->push_back(impl_(query));
                return std::move(tmp);
            }

            static std::unique_ptr<impl> impl_(
                const problem::single::testing::TestQuery &query)
            {
                std::unique_ptr<impl> tmp;
                if (query.has_id())
                    tmp.reset(new id(query.id()));
                else if (query.has_wildcard())
                    tmp.reset(new wildcard(query.wildcard()));
                else if (query.has_regex())
                    tmp.reset(new regex(query.regex()));
                else
                    BOOST_THROW_EXCEPTION(invalid_test_query_error());
                return std::move(tmp);
            }

        private:
            std::shared_ptr<impl> m_impl;
        };
    }

    std::unordered_set<std::string> tests::test_set(
        const google::protobuf::RepeatedPtrField<
            problem::single::testing::TestQuery> &test_query)
    {
        const matcher m(test_query);
        const std::unordered_set<std::string> full_set = test_set();
        return std::unordered_set<std::string>{
            boost::make_filter_iterator(m, full_set.begin()),
            boost::make_filter_iterator(m, full_set.end())
        };
    }

    single::test tests::test(const std::string &test_id)
    {
        return single::test(*this, test_id);
    }

    test::test(tests &tests_, const std::string &test_id):
        m_tests(&tests_),
        m_test_id(test_id)
    {}

    void test::copy(
        const std::string &data_id,
        const boost::filesystem::path &path) const
    {
        m_tests->copy(m_test_id, data_id, path);
    }

    boost::filesystem::path test::location(const std::string &data_id) const
    {
        return m_tests->location(m_test_id, data_id);
    }

    std::unordered_set<std::string> test::data_set() const
    {
        return m_tests->data_set();
    }
}}}
