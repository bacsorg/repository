#include "mono.hpp"

#include <boost/regex.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace system{namespace single{namespace builders
{
    const bool mono::factory_reg_hook = builder::register_new("mono",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new mono(arguments));
            return tmp;
        });

    static const boost::regex positional("[^=]+"), key_value("([^=]+)=(.*)");

    mono::mono(const std::vector<std::string> &arguments)
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
                if (key == "lang")
                {
                    m_lang = value;
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

    compilable::name_type mono::name(const std::string &/*source*/)
    {
        return {.source = "source.cs", .executable = "source.exe"};
    }

    ProcessPointer mono::create_process(const ProcessGroupPointer &process_group,
                                        const name_type &name)
    {
        const ProcessPointer process = process_group->createProcess(m_lang + "mcs");
        process->setArguments(process->executable(), "-out:" + name.executable.string(), name.source);
        return process;
    }

    solution_ptr mono::create_solution(const ContainerPointer &container,
                                       bunsan::tempfile &&tmpdir,
                                       const name_type &name)
    {
        solution_ptr tmp(new interpretable_solution(
            container, std::move(tmpdir), name, "mono", m_flags));
        return tmp;
    }
}}}}
