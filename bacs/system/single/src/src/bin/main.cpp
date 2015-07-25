#include <bacs/system/single/main.hpp>

#include <boost/assert.hpp>

#include <iostream>

using namespace bacs::system::single;

int main(int argc, char *argv[]) {
  BOOST_ASSERT(argc == 1);
  (void)argv;
  return bacs::system::single::main(std::cin, std::cout);
}
