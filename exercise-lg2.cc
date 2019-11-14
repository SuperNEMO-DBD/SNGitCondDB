#include <git2.h>

#include <iostream>

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
      std::clog << "a tree: " << root << git_tree_entry_name(entry) << std::endl;
      break;
    }
    case GIT_OBJ_BLOB: {
      std::clog << "a blob: " << root << git_tree_entry_name(entry) << std::endl;
      break;
    }
    default: { std::cerr << "Tree entry is ???\n"; }
  }
  return 0;
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
  switch (git_object_type(revobj)) {
    case GIT_OBJ_COMMIT: {
      // show_commit((const git_commit *)obj);
      // get the tree, parse that...
      std::clog << "handling commit object\n";
      git_tree *tr{nullptr};
      error = git_commit_tree(&tr, (git_commit *)revobj);
      git_handle_error(error);

      // Traverse tree by entry?
      // ... but doesn't recurse!
      // int entries = git_tree_entrycount(tr);
      // std::clog << "tree holds " << entries << " entries\n";
      // for (int i=0; i < entries; ++i) {
      //  const git_tree_entry* entry = git_tree_entry_byindex(tr, i);
      //  std::cout << i << ": " << git_tree_entry_name(entry) << "\n";
      //}

      // Traverse by walking?
      error = git_tree_walk(tr, GIT_TREEWALK_PRE, my_treewalk_cb, nullptr);
      git_handle_error(error);

      // Could also just use git_tree_entry_byname and wrap/lookup calls to that
      // e.g. take path, if not found, cache it, else return prior.

      git_tree_free(tr);
      break;
    }
    case GIT_OBJ_TREE: {
      // show_tree((const git_tree *)obj);
      // it's a tree already?
      std::clog << "handling tree object\n";
      error = git_tree_walk((const git_tree *)revobj, GIT_TREEWALK_PRE, my_treewalk_cb, nullptr);
      git_handle_error(error);
      break;
    }
    case GIT_OBJ_TAG: {
      // show_tag((const git_tag *)obj);
      // peel the tag, get the tree, parse that...
      std::cout << "handling tag object\n";
      git_object *tag_target{nullptr};
      error = git_tag_peel(&tag_target, (const git_tag *)revobj);
      git_handle_error(error);

      // Should now have something that is tree like?
      std::clog << "peeled object type is: " << git_object_type2string(git_object_type(tag_target))
                << std::endl;

      // Expect a commit here, but need to check that's always the case
      git_tree *tr{nullptr};
      error = git_commit_tree(&tr, (const git_commit *)tag_target);
      git_handle_error(error);

      error = git_tree_walk(tr, GIT_TREEWALK_PRE, my_treewalk_cb, nullptr);
      git_handle_error(error);

      git_tree_free(tr);
      git_object_free(tag_target);
      break;
    }
    default:
      std::cerr << "unsupported object type\n";
      break;
  }

  git_object_free(revobj);

  // Must free non-const pointer
  git_repository_free(repo);

  git_libgit2_shutdown();
  return 0;
}