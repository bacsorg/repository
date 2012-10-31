#include "python.hpp"

#include <boost/regex.hpp>
#include <boost/assert.hpp>

namespace bacs{namespace single{namespace builders
{
    const bool python::factory_reg_hook = builder::register_new("python",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new python(arguments));
            return tmp;
        });

    static const boost::regex positional("^[^=]+$"), key_value("^([^=]+)=(.*)$");

    python::python(const std::vector<std::string> &arguments)
    {
        for (const std::string &arg: arguments)
        {
            boost::smatch match;
            /*if (boost::regex_match(arg, match, positional))
            {
                BOOST_ASSERT(match.size() == 1);
            }
            else */if (boost::regex_match(arg, match, key_value))
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

    ProcessPointer python::create_process(const ProcessGroupPointer &process_group,
                                          const name_type &name)
    {
        const ProcessPointer process = process_group->createProcess("python" + m_lang);
        process->setArguments(process->executable(), "-c", R"EOF(
import sys
import py_compile

if __name__=='__main__':
    src = sys.argv[1]
    try:
        py_compile.compile(src, doraise=True)
    except py_compile.PyCompileError as e:
        print(e.msg, file=sys.stderr)
        sys.exit(1)

        )EOF", name.source);
        return process;
    }

    solution_ptr python::create_solution(const ContainerPointer &container,
                                         bunsan::tempfile &&tmpdir,
                                         const name_type &name)
    {
        std::vector<std::string> flags(m_flags);
        flags.push_back(name.source.string());
        const solution_ptr tmp(new interpretable_solution(
            container, std::move(tmpdir), name, "python" + m_lang, flags));
        return tmp;
    }
}}}
