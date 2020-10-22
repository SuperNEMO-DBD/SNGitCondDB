#include <GitCondDB.h>

#include <iomanip>
#include <iostream>

// Wrap GitCondDB instance as we want more convenience (e.g. lock to tag, don't allow
// return of directories)
// Basically just an interface reduction. We don't expect to use time_points for
// pure configuration
std::string throwing_dir_converter(const GitCondDB::CondDB::dir_content&) {
  throw std::runtime_error("directory return not allowed");
}

struct ResourceDB {
  using Key = std::string;
  using IOV = GitCondDB::CondDB::IOV;

  ResourceDB(std::string_view repository, std::string tag,
             std::shared_ptr<GitCondDB::Logger> logger = nullptr)
      : tag_{tag}, conn_{GitCondDB::connect(repository, logger)} {
    conn_.set_dir_converter(throwing_dir_converter);
    conn_.logger()->level = GitCondDB::Logger::Level::Debug;
  }

  ~ResourceDB() { conn_.disconnect(); }

  std::string get(const Key& key) {
    auto res = conn_.get({tag_, key});
    return std::get<0>(res);
  }

 private:
  std::string tag_;
  GitCondDB::CondDB conn_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: resourceDB <pathtorepo>\n";
    return 1;
  }

  // Assume as input:
  // - repo path
  // - a branch/tag name to "get" (eventually want the tree at this, so must be a tree!)
  std::string repo{argv[1]};

  // Make some queries
  // Given a tag...
  {
    std::string revspec{"v1.0.0"};
    std::clog << "connecting to " << repo << "@" << revspec << std::endl;

    // ... Connect the DB ...
    ResourceDB conn{repo, revspec};

    // ... Extract data into a string "blob" ...
    // NB: throws an exception if the resource cannot be found
    std::string data = conn.get("genbb/generators.conf");
    std::clog << "data@" << revspec << " <<<<<\n" << data << "\n>>>>> data\n";
  }

  // Given a branch...
  {
    // Annoyingly GitHub changed the default branch new repos get...
    std::string revspec{"main"};
    std::clog << "connecting to " << repo << "@" << revspec << std::endl;

    // ... Connect the DB ...
    ResourceDB conn{repo, revspec};

    // ... Extract a file ...
    std::string data = conn.get("genbb/generators.conf");
    std::clog << "data@" << revspec << " <<<<<\n" << data << "\n>>>>> data\n";
  }

  return 0;
}