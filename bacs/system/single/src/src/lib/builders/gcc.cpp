#include "gcc.hpp"

#include <boost/assert.hpp>
#include <boost/regex.hpp>

#include <unordered_map>

namespace bacs{namespace system{namespace single{namespace builders
{
    namespace
    {
        struct invalid_lang_error: virtual invalid_argument_error {};
    }

    const bool gcc::factory_reg_hook = builder::register_new("gcc",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new gcc(arguments));
            return tmp;
        });

    static const boost::regex positional("[^=]+"), key_value("([^=]+)=(.*)");

    gcc::gcc(const std::vector<std::string> &arguments):
        m_executable("gcc")
    {
        for (const std::string &arg: arguments)
        {
            boost::smatch match;
            if (boost::regex_match(arg, match, positional))
            {
                BOOST_ASSERT(match.size() == 1);
                m_flags.push_back("-" + arg);
            }
            else if (boost::regex_match(arg, match, key_value))
            {
                BOOST_ASSERT(match.size() == 3);
                const std::string key = match[1].str(), value = match[2].str();
                if (key == "optimize")
                {
                    m_flags.push_back("-O" + value);
                }
                else if (key == "lang")
                {
                    static const std::unordered_map<std::string, std::string> langs = {
                        {"c", "gcc"},
                        {"c++", "g++"},
                        {"objective-c", "gcc"}, // TODO unchecked
                        {"objective-c++", "g++"}, // TODO unchecked
                        {"f77", "gfortran"},
                        {"f95", "gfortran"},
                        {"go", "gccgo"}
                    };
                    const auto iter = langs.find(value);
                    if (iter == langs.end())
                        BOOST_THROW_EXCEPTION(invalid_lang_error() << invalid_lang_error::argument(value));
                    m_executable = iter->second;
                    m_flags.push_back("-x");
                    m_flags.push_back(value);
                }
                else if (key == "std")
                {
                    m_flags.push_back("-std=" + value);
                }
                else
                {
                    BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                          invalid_argument_error::argument(arg));
                }
            }
            else
            {
                BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                      invalid_argument_error::argument(arg));
            }
        }
    }

    ProcessPointer gcc::create_process(const ProcessGroupPointer &process_group,
                                       const name_type &name)
    {
        const ProcessPointer process = process_group->createProcess(m_executable);
        process->setArguments(process->executable(), m_flags, name.source, "-o", name.executable);
        return process;
    }
}}}}
