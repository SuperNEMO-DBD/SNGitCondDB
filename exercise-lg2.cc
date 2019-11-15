#include <git2.h>

#include <iostream>
#include <set>
#include <vector>

using fileindex_t = std::set<std::string>;

void print_fileindex(const fileindex_t& f) {
  for(const std::string& e : f) {
    std::clog << "-- " << e << std::endl;
  }
}

void git_handle_error(int err) {
  if (err < 0) {
    const git_error *e = giterr_last();
    std::cerr << "Error " << err << "/" << e->klass << ": " << e->message << std::endl;
    exit(err);
  }
}

int my_treewalk_cb(const char *root, const git_tree_entry *entry, void *payload) {
  // Now a tree entry can be either a tree or a blob, but alos a commit if
  // we have submodules! See:
  // https://matthew-brett.github.io/curious-git/git_submodules.html
  // Use cases probably won't use submodules, but should check how traversal works here.
  // For now, we just want to check files (blobs) can be accessed.
  switch (git_tree_entry_type(entry)) {
    case GIT_OBJ_COMMIT: {
      std::clog << "oh, a submodule : " << root << std::endl;
      break;
    }
    case GIT_OBJ_TREE: {
      //std::clog << "a tree: " << root << git_tree_entry_name(entry) << std::endl;
      break;
    }
    case GIT_OBJ_BLOB: {
      // Handle blob
      // ... but can't get object (blob) from tree_entry without a tree/repo!
      //     As all we're doing for now is checking we can get content, store
      //     bloba paths in the payload
      auto f = (fileindex_t*)payload;
      std::string pth{root};
      pth += git_tree_entry_name(entry);
      //std::clog << "a blob: " << pth << std::endl;
      f->insert(pth);
      break;
    }
    default: {
      std::cerr << "Tree entry is ???\n";
      break;
    }
  }
  return 0;
}

// Get tree associated with supplied object, if any
// Returned tree must be freed
int tree_from_object(git_tree** out_tree, const git_object* source) {
  switch (git_object_type(source)) {
    case GIT_OBJ_COMMIT: {
      return git_commit_tree(out_tree, (const git_commit *)source);
      break;
    }
    case GIT_OBJ_TREE: {
      return git_tree_dup(out_tree, (git_tree *)source);
      break;
    }
    case GIT_OBJ_TAG: {
      git_object *tag_target{nullptr};
      // Two level process, so decide what to do on non-zero error
      int error = git_tag_peel(&tag_target, (const git_tag *)source);

      if(error != 0) {
        return error;
      }
      
      // Peeling should lead to a commit, but need to check that's always the case
      error = git_commit_tree(out_tree, (const git_commit *)tag_target);
      git_object_free(tag_target);
      return error;
      break;
    }
    default:
      // Not clear this is the right thing to do...
      giterr_set_str(GITERR_INVALID, "cannot derive tree object");
      return GITERR_INVALID;
      break;
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "wrong args\n";
    return 1;
  }

  git_libgit2_init();

  // Assume as input:
  // - repo path
  // - a branch/tag name to "get" (eventually want the tree at this, so must be a tree!)
  std::string repoPath{argv[1]};
  std::string revisionName{argv[2]};

  // Connect
  git_repository *repo{nullptr};
  int error = git_repository_open(&repo, repoPath.c_str());
  git_handle_error(error);

  // Try and get the object associated with the revision
  git_object *revobj{nullptr};
  error = git_revparse_single(&revobj, repo, revisionName.c_str());
  git_handle_error(error);

  // What object is this?
  std::cout << "revspec " << revisionName << " is a "
            << git_object_type2string(git_object_type(revobj)) << std::endl;

  // We want something that can resolve to a tree...
  // So must be a commit, tag or tree?
  git_tree* tr{nullptr};
  error = tree_from_object(&tr, revobj);
  git_handle_error(error);
  
  // Traverse by walking because by entry does not recurse into child trees
  // Index the revspec to store a list of blob names
  fileindex_t pl;
  error = git_tree_walk(tr, GIT_TREEWALK_PRE, my_treewalk_cb, &pl);
  git_handle_error(error);

  // Iterate through names to get blobs
  // Sort of revisits tree_walk, so in interface can consider on-demand lookup
  // of paths, cacheing content (since it should be fixed over the "resource db" lifetime)
  size_t totalSize{0};
  for (const std::string& p : pl) {
    // Since we're dealing with a tree, have to use path for nested trees!
    git_tree_entry* ent{nullptr};
    error = git_tree_entry_bypath(&ent, tr, p.c_str());
    git_handle_error(error);

    // Now the object the entry points to
    git_object* fblob{nullptr};
    error = git_tree_entry_to_object(&fblob, repo, ent);

    totalSize += git_blob_rawsize((git_blob*)fblob); 

    std::clog << "- [perms] " << git_blob_rawsize((git_blob*)fblob) << " " << p;

    // The returned void* is owned by the blob, so needs copying if going to be used elsewhere
    // For now can handle directly as blob is still in scope until end of loop.
    // Makes assumption that sizeof(char) is 1 byte
    std::vector<char> byteArray{};
    byteArray.reserve(git_blob_rawsize((git_blob*)fblob));
    char* begin = (char*)git_blob_rawcontent((git_blob*)fblob);
    char* end = begin + git_blob_rawsize((git_blob*)fblob);
    byteArray.assign(begin, end);

    std::clog << " buffersize: " << byteArray.size() << std::endl;

    // ONLY for convenient printing, seems to produce right output, but likely
    // want a better interface for treating content as a stream.
    std::string x{byteArray.begin(), byteArray.end()};
    std::clog << ">>> content (" << x.size() << " chars)\n" << x <<"\n<<< content" << std::endl;

    // Clean up
    git_object_free(fblob);
    git_tree_entry_free(ent);
  }
  
  // Track total raw size to see if caching in memory is reasaonable
  std::clog << "total tree raw size: " << totalSize / 1000. << "K" << std::endl;


  // Clean up objects that are our responsibility 
  git_tree_free(tr);
  git_object_free(revobj);
  git_repository_free(repo);

  git_libgit2_shutdown();
  return 0;
}