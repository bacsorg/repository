#include "fpc.hpp"

#include <boost/regex.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace single{namespace builders
{
    const bool fpc::factory_reg_hook = builder::register_new("fpc",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new fpc(arguments));
            return tmp;
        });

    static const boost::regex positional("^[^=]+$"), key_value("^([^=]+)=(.*)$");

    fpc::fpc(const std::vector<std::string> &arguments)
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
                BOOST_ASSERT(match.size() == 2);
                const std::string key = match[1].str(), value = match[2].str();
                if (key == "optimize")
                {
                    m_flags.push_back("-O" + value);
                }
                else if (key == "lang")
                {
                    m_flags.push_back("-x");
                    m_flags.push_back(value);
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

    ProcessPointer fpc::create_process(const ProcessGroupPointer &process_group,
                                       const name_type &name)
    {
        const ProcessPointer process = process_group->createProcess("fpc");
        process->setArguments(process->executable(), m_flags, name.source, "-o", name.executable);
        return process;
    }
}}}
