#include <bacs/system/single/detail/checker.hpp>

namespace bacs{namespace system{namespace single{namespace detail{namespace checker
{
    problem::single::result::Judge::Status equal(std::istream &out, std::istream &hint)
    {
        // read all symbols and compare
        char o, c;
        do
        {
            while (out.get(o) && o == '\r');
            while (hint.get(c) && c == '\r');
            if (out && hint && o != c)
                return problem::single::result::Judge::WRONG_ANSWER;
        }
        while (out && hint);
        if (out.eof() && hint.eof())
            return problem::single::result::Judge::OK;
        else
        {
            if (out.eof() && hint)
            {
                hint.putback(c);
                return seek_eof(hint);
            }
            else if (out && hint.eof())
            {
                out.putback(o);
                return seek_eof(out);
            }
            else
                return problem::single::result::Judge::WRONG_ANSWER;
        }
    }

    problem::single::result::Judge::Status seek_eof(std::istream &in)
    {
        char c;
        while (in.get(c))
        {
            if (c != '\n' && c != '\r')
                return problem::single::result::Judge::WRONG_ANSWER;
        }
        return problem::single::result::Judge::OK;
    }
}}}}}
