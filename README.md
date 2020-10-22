# SNGitCondDB Prototype

Demo of libgit2/GitCondDB based management of resource and conditions data for
SuperNEMO

# Quickstart

Requirements:

- CMake 3.16 or newer
- C++17 capable compiler
- `git` and `libgit2`
- `fmt` library
- Network connection for accessing Git databases

These can be set up as follows for macOS/Linux systems:

- macOS (10.15/Catalina)
  - Use Homebrew (not SuperNEMO brew!) with `brew install cmake libgit2 fmt`
- CentOS7
  - Ensure CVMFS is installed and that you can do `ls /cvmfs/sft.cern.ch`
  - Run `source ./scripts/setup-centos7-cvmfs.sh` from the source directory before building
- Ubuntu 20.04
  - Run `apt install cmake fmt-dev g++ libgit2-dev`

To build, CMake can be used in the standard way:

```
$ pwd
/pathto/GitResourceDB.git
$ cmake -S. -B build
$ cmake --build build -jN
$ cmake --build build --target test -jN
```

where `N` can be set to the number of parallel builds (e.g. 4 on a quad-core machine).

# Exploring Functionality
Two minimal demonstrator applications are built to demonstrate two different
cases of "database" like functionality in SuperNEMO.

These are both built on the [LHCb GitCondDB Library](https://gitlab.cern.ch/lhcb/GitCondDB),
and you should consult that repo and the [main documentation](http://lhcb-core-doc.web.cern.ch/lhcb-core-doc/GitCondDB.html#) for details on implementation and design. The three core concepts to
understand here are:

1. Paths are used to identify conditions (e.g. `tracker/gas_system/pressure`)
2. Git tags are used to track _changes in implementation_ (e.g. how a calibration constant is calculated)
3. _IOVs_ track how a condition _changes in time_ (e.g. a calibration constant is valid between two points in time)

A good overview of these is provided in [GitCondDB's main doc](http://lhcb-core-doc.web.cern.ch/lhcb-core-doc/GitCondDB.html#).

TODO: There is also a [GUI frontend](https://twiki.cern.ch/twiki/bin/view/LHCb/CondDBBrowser)
but this is not tested with SuperNEMO prototypes yet.

TODO: At present direct connection to remote GitHub/GitLab repos ala MySQL is not implemented
but should not be required (as downloading/sharing the repo/DB locaaly is trivial).

## resourceDB
Implemented in [resourceDB.cc](resourceDB.cc), this demonstrates the use of GitCondDB to read
"resource files" in SuperNEMO. These are text files used to describe things like the geometry
and event generators. They don't generally change over time (those that do are strictly _conditions_), but do change
in implementation (new features, optimizations etc), so they only use the _path_ and _tag_ axes offered
by GitCondDB. In Falaise, a typical way to organise resources files was the following directory structure:

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

Whilst `resources` might itself be a Git repository with tags, the `1.0, 2.0` directories are also _tags_
in the sense of marking implementation changes. However they are not managed through `git` but manually
by a developer. A full comparison of GitCondDB vs these "Directory based" tags is provided in [docs/ResourceImplementations.md](docs/ResourceImplementations.md), and it shown that this structure can be
reduced to simple standard Git tags without the traps and pitfalls associated with hand-managing changes.

The `resourceDB` application shows a simple use of GitCondDB with this structure. A simple git repo with
resource files has been created at: https://github.com/SuperNEMO-DBD/SNemoResourceDB-proto. This just stores
one configuration file, a list of generators. The list of tags is browsable at: https://github.com/SNemoResourceDB-proto/tags. The build of `resourceDB` clones this repo locally, so it is also accessible under `SNemoResourceDB.git` in the build directory. To run `resourceDB` with this repo, change into the build directory and run as:

```
$ ./resourceDB ./SNemoResourceDB.git
```

The application itself reads the generator configuration and prints it to screen, once for the `v1.0.0` tag and
once for the `main` (primary) branch. You should see output similar to:

```
connecting to ./SNemoResourceDB.git@v1.0.0
debug  : get Git object v1.0.0:genbb/generators.conf
debug  : found blob object
data@v1.0.0 <<<<<
Se82

>>>>> data
debug  : disconnect from Git repository
connecting to ./SNemoResourceDB.git@main
debug  : get Git object main:genbb/generators.conf
debug  : found blob object
data@main <<<<<
Se82
Mo100
Nd150

>>>>> data
debug  : disconnect from Git repository
```

The C++ code itself essentially reduces to:

```c++
// Like any other DB connection
auto conn = GitCondDB::connect("/path/to/repo");
// A query gives back a tuple of the result and the IOV
auto result = conn.get(tagValue, pathToResource);
// The first column is the value, second is IOV (which is infinite in this case so we ignore)
std::string resource = std::get<0>(result);
// do as you will with resource
```

The application shows that this can be simplified/eased by wrapping in a frontend (which can
also hide the exact implementation). Results are always returned as strings, so it's up to
the caller to interpret these in a suitable way (e.g. JSON binding, convert to binary). Here, it's simple
text file content so fine to just dump to screen.

## conditionsDB
Implemented in [condDB.cc](condDB.cc), this demonstrates the use of GitCondDB to read
an example detector "condition", in this case gas pressure.

A simple git repo with files for this condition has been created at: https://github.com/SuperNEMO-DBD/SNemoConditionsDB-proto. The time axis makes things a little more complex in that repo, but the
code in [condDB.cc](condDB.cc) is still very simple, the only addition being a time parameter to
the DB query. The build of `condDB` clones this repo locally, so it is also accessible under `SNemoConditionsDB.git` in the build directory. To run `condDB` with this repo, change into the build directory and run as:

```
$ ./condDB ./SNemoConditionsDB.git
```

This just prints some information about possible time spans offered by IOVs. These use a large
unsigned integer type to measure time without units, so the zero point and unit may be chosen
according to need.

The remainder of the info shows use of both the _IOV_ and _tag_ axes to get the gas pressure "value".
In GitCondDB, all values are files, and here the content is just dumped to screen. It can be seen
how additional frontend interfaces could be added to present actual numbers to the user.

This only demonstrates a very simple use case, and we would need to think how to structure
conditions into different repositories (Configuration, Slow, Calibration), and how to group
data into IOVs. For example, we might have IOV bounds on run start/stop and have one file
with all gas pressures recorded over a run.

Best practices and patterns of use are something we can ask the LHCb experts about, along with tools
to integrate and automate publication of commits/tags etc.