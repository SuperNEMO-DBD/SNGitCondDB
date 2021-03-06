Directory-Based Tagging in SuperNEMO
======
NB: Since this was written, Falaise has moved to a flattened directory
structure in preparation for storing resources in either a Git or SQL DB.
This presents the arguments for reducing director-based tags to pure Git tags.

Current resource files use a tagging system based on directories. These
tags may represent either changes in Time (e.g. parameter change) or API
(e.g. new/removed parameters). Relationship of latter to source code
yet to be fully determined.

A typical filesystem layout of resource files that configure SuperNEMO
software is usually organised as follows:

```
resources/
├── foo
    ├── 1.0
    |   ├── FooSetup.conf
    |   └── models
    |       └── FooModel.conf
    └── 2.0
        └── FooSetup.conf
```

The above shows the smallest possible example of a resource layout in the "Directory-Based Tagging" (DBT) design,
with the tags being the `X.Y` directory names. As such, it provides a simple
"content/NoSQL" database using the filesystem as the keying mechanism. The problem with
this system is less this mechanism than:

1. How new tags are proposed
2. Reuse of files between tags
3. Confusion between these *directory* tags, and those of the *Version
Control System* used to maintain and develop the files

These lead to several issues in reliably tracking changes between tags,
and consequently the validation of their behaviour. To illustrate, we'll
walk through a simple example, which you can also try from your terminal.


Practical Use of Directory-Based Tagging
=====
Starting the Project
-----
As SuperNEMO uses Git for version control, we'll use that here, but the same
principles apply for other tools like SVN.

We start by creating the empty project:

```
$ mkdir resources
$ cd resources
$ git init
Initialized empty Git repository in /path/to/resources/.git
$
```

Creating the first Directory-Based Tag
-----
Now we create the `foo` directory. In SuperNEMO nested directories
also form part of the overall "key", marking different "categories" of
configuration, e.g. `detector`.

```
$ mkdir -p foo
```

So now we want to add configuration for the `foo` category, and
in Directory-Based Tagging we need to create the directory for the tag.
Like Git Tags, the directory name is totally arbitrary and is just a shorthand marker for
"the content of the project at this point in development". Some naming convention
is usually adopted to provide meaning, such as "Compatibility" or "Date" (and others).
SuperNEMO uses "X.Y" names though the meaning is not clearly defined
("X.Y" names are *conventionally* taken to indicate compatibility level).

We'll follow that convention, so create the directory `1.0` for this initial work/tag

```
$ mkdir -p foo/1.0
```

For the purposes of this example, we populate the configuration files using
a common pattern in SuperNEMO: a top level file that includes another from
a lower level. Create the files as follows:

```
$ ls -a
.    ..   .git foo
$ cd foo/1.0
$ mkdir models
$ touch FooSetup.conf
$ touch models/FooModel.conf
```

and edit them as you prefer to set their contents to:

```
# foo/1.0/FooSetup.conf
fooValue = 3.14
fooModels = "@resources:foo/1.0/models/FooModel.conf"
```

and

```
# foo/1.0/models/FooModel.conf
models = "One" "Two" "Three"
```

In `FooSetup.conf`, `@resources` is just a shorthand understood in SuperNEMO software
for the full path to the `resources/` directory.

Though we have "tagged" we still want (and in fact need) to track changes to
the files using git and the familiar `add/commit` combination:

```
$ ls
FooSetup.conf models
$ cd ../..
$ git status
On branch master

No commits yet

Untracked files:
  (use "git add <file>..." to include in what will be committed)

	foo/

nothing added to commit but untracked files present (use "git add" to track)
$ git add foo
$ git status
On branch master

No commits yet

Changes to be committed:
  (use "git rm --cached <file>..." to unstage)

	new file:   foo/1.0/FooSetup.conf
	new file:   foo/1.0/models/FooModel.conf
$ git commit -m "Created foo resources, tag 1.0"
[master (root-commit) 457699e] Created foo resources, tag 1.0
 2 files changed, 5 insertions(+)
 create mode 100644 foo/1.0/FooSetup.conf
 create mode 100644 foo/1.0/models/FooModel.conf
$
```


Directory-Based Tags vs Git Tags
-----

In the preceding section, we created the "foo/1.0" Directory-Based Tag and
committed it in the Git repository being used to track its development. Though
we have the Directory Tag, we can also create a standard Git Tag. To distinguish
between the Directory and Git Tags, we'll name the later using single capital
letters preceded by `v`, with each new tag being named for the next letter of the alphabet.
Thus out first Git Tag is `vA` and we create it thus:

```
$ git tag -a vA -m "foo git tag, vA"
$ git tag
vA
$ git log --oneline vA
457699e (HEAD -> master, tag: vA) Created foo resources, tag 1.0
```

At this point, our Directory-Based Tag `1.0` has exactly the same content as the `vA` tag.
This illustrates an immediate issue with Directory-Based Tags in that they *reproduce an
existing and well-known functionality of Git*.

Git's hashes provide a guarantee of content, Directory Tags don't as it's possible to
edit the files without changing the directory tag:

```
$ edit foo/1.0/FooSetup.conf
... change param, no directory change ...
$ git add
$ git commit
$ git tag -a vB "foo git tag, vB"
... Now have two git tags which are different, but DBT is the same, despite content having changed ....
```

Normally, this shouldn't happen if the developer is careful, but in more complex hierarchies
with multiple DBTs mistakes can creep in and be difficult to spot. This brings us to another issue with DBTs: how
to make new DBTs.

Making a New DBT
-----
First, the new directory has to be created, and semantics of naming are important to establish the
meaning (we don't have a git commit to record this info). A related issue is that this has no marker
of "in progress" like the `HEAD` of a git branch

Here, we consider one simple case of just modding a parameter. We begin by *copying the files* from the old
DBT to the new:

```
resources/
├── foo
    ├── 1.0
    |   ├── FooSetup.conf
    |   └── models
    |       └── FooModel.conf
    └── 2.0
        └── FooSetup.conf
        └── models
            └── FooModel.conf
```

Now we mod the `fooValue` parameter, and we should also update the path to the copied `FooModel.conf`.

```
# foo/2.0/FooSetup.conf
fooValue = 4.13
fooModels = "@resources:foo/2.0/models/FooModel.conf"
```

As the content of `FooModel.conf` hasn't changed, the typical pattern was to _not_ copy this

```
resources/
├── foo
    ├── 1.0
    |   ├── FooSetup.conf
    |   └── models
    |       └── FooModel.conf
    └── 2.0
        └── FooSetup.conf
```

and then point `FooSetup.conf` *back* to `1.0` model file

```
# foo/2.0/FooSetup.conf
fooValue = 4.13
fooModels = "@resources:foo/1.0/models/FooModel.conf"
```

After git add/commit we have the new DBT, *but what actually changed*? This is critical in
keeping track of data provenance, i.e. what data processed with what parameters?

If we use `git diff` we don't get a straightforward answer. We see that a new file
has been created with the *actual change being all its entire contents*. What is not obvious
from that is the change to the value of `fooValue`. The creation of the new file hides,
or at least requires further digging to check, this change. If we'd just editted the file,
the diff would only show something like:

```
-fooValue = 3.14
+fooValue = 4.13
```

a clear and unamibiguous change. Furthemore, whilst we can see the reuse of the models
file, to see exactly how the overall configuration was constructed, we have to trace it back
through the filesystem. In this simple case it's not too bad, but as the number of files and DBTs
expands, one can end up in a maze of reuse. This type of issue can be seen particularly in [this Pull Request](https://github.com/SuperNEMO-DBD/Falaise/pull/113/files)

Now imagine we'd changed the models file as well. This would also have been copied
and so we would never see what had changed, just the addition of the files without a
guide to how the content had changed, if at all.


Why Are Directory-Based Tags Used?
=====

So far, no clear and succint explanation has been provided, but some suppositions can be made.
Through the lifetime of an experiment, certain configurations will change over time,
for example a cabling map. Let's say the first quarter of the runs used configuration "A"
and the remainder configuration "B". Processing/Analysis of the data could use runs
from either side of this boundary, so will need both configurations present concurrently,
the appropriate one chosen from the run number being processed.

By "present concurrently" we mean "as files on disk in two separate locations", e.g.:

```
resources/
- A/
   config.conf
- B/
   config.conf
```

which yields the Directory-Based Tag structure. It's really an artifact of trying to
manage a two-fold tagging system in a single git repo.


Replacing Directory-based tags with Git Tags
=====

As noted above, DBTs appear to arise as an artifact of a misunderstanding of Git tagging
and repository structures. Instead, it's possible to reduce the problem to *just Git tags*
provided we can:

1. Isolate the sets of resource files into separate git repositories, tagging those.
2. Provide an API that can return the content of a file at a given tag

The first requirement is simple, just move the required resource directory into its
own Git repository. The diectory tags can be removed, and replaced by git tags. Thus:

```
resources/
├── foo
    ├── 1.0
    |   ├── FooSetup.conf
    |   └── models
    |       └── FooModel.conf
    └── 2.0
        └── FooSetup.conf
```

becomes:

```
resources/
├── foo
    ├── FooSetup.conf
        └── models
            └── FooModel.conf
```

with git tags `v1.0` and `v2.0`. To see the difference, we can use the git command line
to get the contents of a given file at a given tag, e.g.

```
$ git show v1.0:resources/foo/FooSetup.conf
# I'm version 1.0!
$ git show v2.0:resources/foo/FooSetup.conf
# I'm version 2.0!
```

or their diff:

```
git diff v1.0:resources/foo/FooSetup.conf v2.0:resources/foo/FooSetup.conf
-# I'm version 1.0!
+# I'm version 2.0!

Thus git provides a way to manage tags and retrieve file contents at that tag. To use in
software, we need a C/C++ API instead of the Git command line, so the [libgit2](https://libgit2.org) and [GitCondDB](https://gitlab.cern.ch/lhcb/GitCondDB)
projects can help.

Examples of these are provided in the main repository.
