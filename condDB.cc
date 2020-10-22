#include <GitCondDB.h>

#include <iomanip>
#include <iostream>

// A CondDB uses the three dimensions of GitCondDB
// - condition id (path in repo)
// - tag (git tag, e.g. mark for example versions of calibration algorithm employed)
// - IOV (interval in which the above condition is valid)
//
// This example assumes the following layout


void conditionByGitCondDB(const std::string& repo, const std::string& tag, const std::string& path, const GitCondDB::CondDB::time_point_t& t) {
  // Create the connection
  auto conn = GitCondDB::connect(repo);
  conn.logger()->level = GitCondDB::Logger::Level::Debug;

  // With GitCondDB, always have to supply the tag. time point is optional
  auto [value, iov] = conn.get({tag, path, t});

  // Returned content can be a directory (tree), but can make returning this an
  // error via the dir_converter type (throw an exception on a path that resolves to a dir/tree)
  std::cout << "condition <<<<<:\n" << value << ">>>>> condition\n";
  std::cout << "iov: [" << iov.since << ", " << iov.until << ")\n";

  // Must disconnect or get seg fault (Maybe need scoped_connection?)
  conn.disconnect();
}


int main(int argc, char* argv[]) {
  // An IOV is just two time_point_t instances.
  // That's just an integer counter:
  using IOV = GitCondDB::CondDB::IOV;

  std::cout << "GitCondDB uses the Interval Of Validity (IOV) concept for conditions that change over time\n";
  std::cout << "time_point_t is just a ticker with a range of " << "[" << IOV::min() << ", " << IOV::max() << ")\n";
  std::cout << "Interpreted as seconds, gives a range of      " << IOV::max() / (365*24*60*60) << " years\n";
  std::cout << "Interpreted as microseconds, gives a range of " << IOV::max() / (365*24*60*60*1e6) << " years\n";
  std::cout << "Interpreted as nanoseconds, gives a range of  " << IOV::max() / (365*24*60*60*1e9) << " years\n";
  std::cout << "Interpreted as picoseconds, gives a range of  " << IOV::max() / (365*24*60*60*1e12) << " years\n";

  if (argc != 5) {
    std::cerr << "wrong args\n";
    return 1;
  }

  // Assume as input:
  // - repo path
  // - a branch/tag name to "get" (eventually want the tree at this, so must be a tree!)
  std::string repoPath{argv[1]};
  std::string revspec{argv[2]};
  std::string condition{argv[3]};
  GitCondDB::CondDB::time_point_t timespec{static_cast<GitCondDB::CondDB::time_point_t>(std::atoi(argv[4]))};

  conditionByGitCondDB(repoPath, revspec, condition, timespec);


  return 0;
}