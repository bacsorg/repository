#include <bacs/system/single/error.hpp>
#include <bacs/system/single/testing.hpp>

#include <bacs/problem/single/task.pb.h>

#include <yandex/contest/system/Trace.hpp>
#include <yandex/contest/TypeInfo.hpp>

#include <boost/assert.hpp>

#include <iostream>

using namespace bacs::system::single;

int main(int argc, char *argv[]) {
  try {
    yandex::contest::system::Trace::handle(SIGABRT);
    BOOST_ASSERT(argc == 1);
    (void)argv;
    bacs::problem::single::task::Task task;
    if (!task.ParseFromIstream(&std::cin))
      BOOST_THROW_EXCEPTION(
          bacs::system::single::error()
          << bacs::system::single::error::message("Unable to parse task."));
    testing testing_(task.callbacks());
    testing_.test(task.solution(), task.testing());
  } catch (std::exception &e) {
    std::cerr << "Program terminated due to exception of type \""
              << yandex::contest::typeinfo::name(e) << "\"." << std::endl;
    std::cerr << "what() returns the following message:" << std::endl
              << e.what() << std::endl;
    return 1;
  }
}
