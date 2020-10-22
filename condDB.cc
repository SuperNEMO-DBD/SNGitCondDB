#include <GitCondDB.h>

#include <iomanip>
#include <iostream>

void print_time_spans() {
  // An IOV is just two time_point_t instances.
  // That's just an integer counter:
  using IOV = GitCondDB::CondDB::IOV;

  std::cout << "GitCondDB uses the Interval Of Validity (IOV) concept for conditions that change "
               "over time\n";
  std::cout << "time_point_t is just a ticker with a range of "
            << "[" << IOV::min() << ", " << IOV::max() << ")\n";
  std::cout << "Interpreted as seconds, gives a range of      " << IOV::max() / (365 * 24 * 60 * 60)
            << " years\n";
  std::cout << "Interpreted as microseconds, gives a range of "
            << IOV::max() / (365 * 24 * 60 * 60 * 1e6) << " years\n";
  std::cout << "Interpreted as nanoseconds, gives a range of  "
            << IOV::max() / (365 * 24 * 60 * 60 * 1e9) << " years\n";
  std::cout << "Interpreted as picoseconds, gives a range of  "
            << IOV::max() / (365 * 24 * 60 * 60 * 1e12) << " years\n";
}

void print_pressure(const std::string_view& path, const std::string& revspec) {
  using time_point_t = GitCondDB::CondDB::time_point_t;

  auto conn = GitCondDB::connect(path);

  std::clog << "At revspec " << revspec << ":" << std::endl;

  std::string condition{"tracker/gas/pressure"};
  std::vector<time_point_t> times{50, 200};

  for (const time_point_t& t : times) {
    auto [content, iov] = conn.get({revspec, condition, t});

    std::clog << "At time " << t << "\n"
              << "  condition " << condition << " has\n"
              << "  IOV   = [" << iov.since << "," << iov.until << ")\n"
              << "  value = " << content << std::endl;
  }

  // Given a time range (could be run start/stop), we can get a vector of times where the IOV changes
  auto bounds = conn.iov_boundaries(revspec, condition, {times.front(), times.back()});
  std::clog << "Between times [" << times.front() << ", " << times.back() << "), there are "
            << bounds.size() << " IOV boundaries at times:\n";
  for (size_t i = 0; i < bounds.size(); ++i) {
    std::clog << "  boundary = " << i << ", time = " << bounds[i] << std::endl;
  }

  // TBD: given an IOV that's run [start,stop), how to accumulate conditions over that?
  // Is this a calculation, or how data should be structured in IOVs? e.g. only have IOVs
  // on run boundaries? What about data whose IOV might be within a run, or spanning several?
}

int main(int argc, char* argv[]) {
  print_time_spans();

  // for the master branch...
  print_pressure(argv[1], "main");

  // for the old API tag
  print_pressure(argv[1], "vOld");
  return 0;
}