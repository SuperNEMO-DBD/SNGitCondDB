#include <GitCondDB.h>

#include <iomanip>
#include <iostream>

// So resources == conditions, but can wrap GitCondDB if we want more
// convenience (e.g. lock to tag, don't allow return of directories)
// Basically just an interface reduction

std::string throwing_dir_converter(const GitCondDB::CondDB::dir_content&) {
  throw std::runtime_error("directory return not allowed");
}

struct ResourceDB {
  using Key = std::string;

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

void resourceByGitCondDB(const std::string& repo, const std::string& tag, const std::string& path) {
  // Create the connection
  auto conn = GitCondDB::connect(repo);
  conn.logger()->level = GitCondDB::Logger::Level::Debug;

  // With GitCondDB, always have to supply the tag. time point is optional
  auto res = conn.get({tag, path});

  // Returned content can be a directory (tree), but can make returning this an
  // error via the dir_converter type (throw an exception on a path that resolves to a dir/tree)
  auto resource = std::get<0>(res);
  std::cout << "resource <<<<<:\n" << resource << ">>>>> resource\n";

  // Must disconnect or get seg fault (Maybe need scoped_connection?)
  conn.disconnect();
}

void resourceByResourceDB(const std::string& repo, const std::string& tag,
                          const std::string& path) {
  ResourceDB conn{repo, tag};
  auto resource = conn.get(path);
  std::cout << "resource <<<<<:\n" << resource << ">>>>> resource\n";
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "wrong args\n";
    return 1;
  }

  // Assume as input:
  // - repo path
  // - a branch/tag name to "get" (eventually want the tree at this, so must be a tree!)
  std::string repoPath{argv[1]};
  std::string revspec{argv[2]};
  std::string resource{argv[3]};

  std::cout << "Using GitCondDB:\n";
  resourceByGitCondDB(repoPath, revspec, resource);
  std::cout<<std::endl;
 
  std::cout << "Using GitCondDB:\n";
  resourceByResourceDB(repoPath, revspec, resource);
  std::cout<<std::endl;
  

  return 0;
}