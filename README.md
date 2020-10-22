SNemoDB Client
==============

Demo of Git-based resource file tagging and management using libgit2/GitCondDB.

Quickstart
======

- Assuming you have an install of the SuperNEMO software stack, do

  ```
  $ brew snemo-shell
  ...
  snemo-shell> brew install libgit2
  ...
  snemo-shell> git clone https://github.com/drbenmorgan/GitResourceDB.git
  snemo-shell> mkdir build && cd build
  snemo-shell> cmake ../GitResourceDB && make
  snemo-shell> ./exercise-lg2 ../GitResourceDB <revspec>
  ```

  You can pass `exercise-lg2` the path to any local git repo plus a "revspec"
  to query. This can be a tag (e.g. "v0.3.0") or a branch (e.g. "master").
  The program will dump the contents of the repo at this revspec to stdout.

At present remote GitHub/GitLab repos are not supported. It's likely they
can be however, but this is not a priority.


