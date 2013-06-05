#include "bunsan/config.hpp"
#include "bunsan/enable_error_info.hpp"
#include "bunsan/stream_enum.hpp"
#include "bunsan/filesystem/fstream.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/assert.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "bunsan/serialization/unordered_set.hpp"

#include <unordered_set>

namespace
{
    struct newline_error: virtual bunsan::error {};
    struct not_cr_eoln_in_cr_file: virtual newline_error {};
    struct not_lf_eoln_in_lf_file: virtual newline_error {};
    struct not_crlf_eoln_in_crlf_file: virtual newline_error {};


    BUNSAN_STREAM_ENUM(eoln,
    (
        CR_,  ///< CR, may be CRLF
        CR,   ///< definitely CR
        LF,   ///< definitely LF
        CRLF, ///< definitely CRLF
        NA    ///< not available
    ))

    bool is_ordinary(char c)
    {
        return c != '\n' && c != '\r';
    }

    eoln transform(const boost::filesystem::path &src, const boost::filesystem::path &dst)
    {
        BUNSAN_EXCEPTIONS_WRAP_BEGIN()
        {
            bunsan::filesystem::ifstream fin(src, std::ios_base::binary);
            bunsan::filesystem::ofstream fout(dst, std::ios_base::binary);
            eoln state = NA;
            char c, p;
            while (fin.get(c) && fout)
            {
                bool ord = is_ordinary(c);
                switch (state)
                {
                case CR_:
                    if (ord || c == '\r')
                    {
                        state = CR;
                        fout.put('\n');
                        if (c == '\r')
                            fout.put('\n');
                        else
                            fout.put(c);
                    }
                    else
                    {
                        BOOST_ASSERT(c == '\n');
                        state = CRLF;
                        fout.put('\n');
                    }
                    break;
                case CR:
                    if (ord)
                        fout.put(c);
                    else if (c == '\r')
                        fout.put('\n');
                    else
                        BOOST_THROW_EXCEPTION(not_cr_eoln_in_cr_file());
                    break;
                case LF:
                    if (ord || c == '\n')
                        fout.put(c);
                    else
                        BOOST_THROW_EXCEPTION(not_lf_eoln_in_lf_file());
                    break;
                case CRLF:
                    if (ord)
                        fout.put(c);
                    else if (c == '\n')
                    {
                        if (p != '\r')
                            BOOST_THROW_EXCEPTION(not_crlf_eoln_in_crlf_file());
                        fout.put('\n');
                    }
                    // we will skip \r
                    break;
                case NA:
                    if (ord)
                        fout.put(c);
                    else
                        switch (c)
                        {
                        case '\r':
                            state = CR_;
                            break;
                        case '\n':
                            state = LF;
                            fout.put('\n');
                            break;
                        }
                    break;
                }
                p = c;
            }
            fin.close();
            fout.close();
            return state;
        }
        BUNSAN_EXCEPTIONS_WRAP_END()
    }
}

int main(int argc, char *argv[])
{
    BOOST_ASSERT(argc >= 2 + 1);
    std::unordered_set<std::string> test_set, data_set, text_data_set;
    BUNSAN_EXCEPTIONS_WRAP_BEGIN()
    {
        bunsan::filesystem::ifstream fin("etc/tests");
        {
            boost::archive::text_iarchive ia(fin);
            ia >> test_set >> data_set >> text_data_set;
        }
        fin.close();
    }
    BUNSAN_EXCEPTIONS_WRAP_END()
    BUNSAN_EXCEPTIONS_WRAP_BEGIN()
    {
        bunsan::filesystem::ofstream fout(argv[1]);
        {
            boost::archive::text_oarchive oa(fout);
            oa << test_set << data_set;
        }
        fout.close();
    }
    BUNSAN_EXCEPTIONS_WRAP_END()
    const boost::filesystem::path dst_dir = argv[2];
    for (int i = 3; i < argc; ++i)
    {
        const boost::filesystem::path test = argv[i];
        const boost::filesystem::path dst = dst_dir / test.filename();
        const std::string data_id = test.extension().string();
        BOOST_ASSERT(!data_id.empty());
        std::cerr << test.filename() << ": ";
        if (text_data_set.find(data_id.substr(1)) != text_data_set.end())
        {
            const eoln eoln_ = transform(test, dst);
            std::cerr << eoln_;
        }
        else
        {
            boost::filesystem::copy(test, dst);
            std::cerr << "binary";
        }
        std::cerr << std::endl;
    }
}
