#include "bacs/single/callback.hpp"
#include "bacs/single/checker.hpp"

#include "yandex/contest/TypeInfo.hpp"
#include "yandex/contest/system/Trace.hpp"

#include <iostream>

#include <boost/assert.hpp>

using namespace bacs::single;

int main(int argc, char *argv[])
{
    try
    {
        yandex::contest::system::Trace::handle(SIGABRT);
        BOOST_ASSERT(argc == 1);
        (void) argv;
        api::pb::task::Task task;
        task.ParseFromIstream(&std::cin);
        api::pb::result::Result result;
        // TODO check hash
        result.mutable_system()->set_status(api::pb::result::SystemResult::OK);
        // TODO init callbacks
        bacs::single::callback::result result_cb(task.callbacks().result());
        bacs::single::callback::intermediate intermediate_cb;
        if (task.callbacks().has_intermediate())
            intermediate_cb.assign(task.callbacks().intermediate());
        // TODO build solution
        // TODO test all
    }
    catch (std::exception &e)
    {
        std::cerr << "Program terminated due to exception of type \"" <<
                     yandex::contest::typeinfo::name(e) << "\"." << std::endl;
        std::cerr << "what() returns the following message:" << std::endl <<
                     e.what() << std::endl;
        return 1;
    }
}
