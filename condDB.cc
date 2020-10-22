#include <GitCondDB.h>

#include <iomanip>
#include <iostream>

// A CondDB uses the three dimensions of GitCondDB
// - condition id (path in repo)
// - tag (git tag, e.g. mark for example versions of calibration algorithm employed)
// - IOV (interval in which the above condition is valid)
//

void print_time_spans() {
  // An IOV is just two time_point_t instances.
  // That's just an integer counter:
  using IOV = GitCondDB::CondDB::IOV;

  std::cout << "GitCondDB uses the Interval Of Validity (IOV) concept for conditions that change over time\n";
  std::cout << "time_point_t is just a ticker with a range of " << "[" << IOV::min() << ", " << IOV::max() << ")\n";
  std::cout << "Interpreted as seconds, gives a range of      " << IOV::max() / (365*24*60*60) << " years\n";
  std::cout << "Interpreted as microseconds, gives a range of " << IOV::max() / (365*24*60*60*1e6) << " years\n";
  std::cout << "Interpreted as nanoseconds, gives a range of  " << IOV::max() / (365*24*60*60*1e9) << " years\n";
  std::cout << "Interpreted as picoseconds, gives a range of  " << IOV::max() / (365*24*60*60*1e12) << " years\n";
}

int main(int argc, char* argv[]) {
  print_time_spans();
  return 0;
}