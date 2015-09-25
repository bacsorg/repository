#include "testlib.h"

#include <cstdio>
#include <cstdlib>

static double getEps() {
  double eps = 1e-9;
  const char *const s_eps = std::getenv("CHECKER_EPS");
  if (s_eps) std::sscanf(s_eps, "%lf", &eps);
  return eps;
}

int main(int argc, char *argv[]) {
  registerTestlibCmd(argc, argv);
  double ja = ouf.readDouble();
  double pa = ans.readDouble();
  if (!doubleCompare(ja, pa, getEps())) {
    quitf(_wa, "%.5lf != %.5lf", ja, pa);
  }
  quitf(_ok, "the answer is correct");
}
