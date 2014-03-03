#include "java.hpp"

#include <boost/assert.hpp>
#include <boost/regex.hpp>
#include <boost/scope_exit.hpp>

namespace bacs{namespace system{namespace single{namespace builders
{
    const bool java::factory_reg_hook = builder::register_new("java",
        [](const std::vector<std::string> &arguments)
        {
            builder_ptr tmp(new java(arguments));
            return tmp;
        });

    static const boost::regex positional("[^=]+");
    static const boost::regex key_value("([^=]+)=(.*)");
    static const boost::regex filename_error(
        ".*class (\\S+) is public, should be declared "
        "in a file named \\1.java.*"
    );

    java::java(const std::vector<std::string> &arguments, bool parse_name)
    {
        if (parse_name)
            m_java.reset(new java(arguments, false));
        for (const std::string &arg: arguments)
        {
            boost::smatch match;
            if (boost::regex_match(arg, match, positional))
            {
                BOOST_ASSERT(match.size() == 1);
                m_flags.push_back("-" + arg);
            }
            /*else if (boost::regex_match(arg, match, key_value))
            {
                BOOST_ASSERT(match.size() == 3);
                const std::string key = match[1].str(), value = match[2].str();
                if (false)
                {
                }
                else
                {
                    BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                          invalid_argument_error::argument(arg));
                }
            }*/
            else
            {
                BOOST_THROW_EXCEPTION(invalid_argument_error() <<
                                      invalid_argument_error::argument(arg));
            }
        }
    }

    solution_ptr java::build(const ContainerPointer &container,
                             const unistd::access::Id &owner_id,
                             const std::string &source,
                             const problem::single::ResourceLimits &resource_limits,
                             problem::single::result::BuildResult &result)
    {
        if (m_java)
        {
            const solution_ptr solution = m_java->build(
                container,
                owner_id,
                source,
                resource_limits,
                result
            );
            if (solution)
            {
                return solution;
            }
            else
            {
                boost::smatch match;
                if (boost::regex_match(result.output(), match, filename_error))
                {
                    BOOST_ASSERT(match.size() == 2);
                    m_java->m_class = match[1];
                    return m_java->build(
                        container,
                        owner_id,
                        source,
                        resource_limits,
                        result
                    );
                }
                else
                {
                    return solution; // null
                }
            }
        }
        else
        {
            // nested object
            return compilable::build(
                container,
                owner_id,
                source,
                resource_limits,
                result
            );
        }
    }

    compilable::name_type java::name(const std::string &/*source*/)
    {
        return {.source = m_class + ".java", .executable = m_class};
    }

    ProcessPointer java::create_process(const ProcessGroupPointer &process_group,
                                        const name_type &name)
    {
        const ProcessPointer process = process_group->createProcess("javac");
        process->setArguments(process->executable(), name.source);
        return process;
    }

    solution_ptr java::create_solution(const ContainerPointer &container,
                                       bunsan::tempfile &&tmpdir,
                                       const name_type &name)
    {
        solution_ptr tmp(new java_solution(
            container, std::move(tmpdir), name, "java", m_flags));
        return tmp;
    }

    std::vector<std::string> java_solution::arguments() const
    {
        std::vector<std::string> arguments_ = flags();
        arguments_.push_back("-classpath");
        arguments_.push_back(dir().string());
        arguments_.push_back(name().executable.string());
        return arguments_;
    }
}}}}
