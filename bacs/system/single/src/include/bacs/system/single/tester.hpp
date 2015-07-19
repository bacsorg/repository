#pragma once

#include <bacs/system/single/common.hpp>
#include <bacs/system/single/tests.hpp>

#include <bacs/problem/single/settings.pb.h>
#include <bacs/problem/single/result.pb.h>
#include <bacs/problem/single/task.pb.h>

#include <bacs/process.pb.h>

#include <yandex/contest/invoker/Forward.hpp>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <memory>

namespace bacs {
namespace system {
namespace single {

/// \note must be implemented in problem
class tester : private boost::noncopyable {
 public:
  static problem::single::result::Judge::Status return_cast(int exit_status);

 public:
  explicit tester(const yandex::contest::invoker::ContainerPointer &container);
  ~tester();

  bool build(const bacs::process::Buildable &solution,
             bacs::process::BuildResult &result);

  bool test(const problem::single::settings::ProcessSettings &settings,
            const single::test &test_,
            problem::single::result::TestResult &result);

 private:
  class impl;

  std::unique_ptr<impl> pimpl;
};

}  // namespace single
}  // namespace system
}  // namespace bacs
