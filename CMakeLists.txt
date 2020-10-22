cmake_minimum_required(VERSION 3.16...3.18)
project(GitResourceService VERSION 0.1.0)

include(CTest)
include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig)
pkg_check_modules(git2 libgit2 REQUIRED IMPORTED_TARGET)

add_executable(exercise-lg2 exercises/exercise-lg2.cc)
target_link_libraries(exercise-lg2 PRIVATE PkgConfig::git2)

# Try GitCondDB, if available
find_package(GitCondDB QUIET)
if(NOT GitCondDB_FOUND)
  FetchContent_Declare(GitCondDB
    GIT_REPOSITORY https://gitlab.cern.ch/LHCb/GitCondDB.git
    GIT_TAG        master)
  FetchContent_MakeAvailable(GitCondDB)
endif()

foreach(_dbapp resourceDB condDB)
  add_executable(${_dbapp} ${_dbapp}.cc)
  target_link_libraries(${_dbapp} PRIVATE GitCondDB::GitCondDB)
endforeach()

# Fetch example DBs
FetchContent_Declare(snemoresourcedb
  GIT_REPOSITORY https://github.com/SuperNEMO-DBD/SNemoResourceDB-proto.git
  GIT_TAG        main
  SOURCE_DIR     SNemoResourceDB.git)
FetchContent_GetProperties(snemoresourcedb)
if(NOT snemoresourcedb_POPULATED)
  FetchContent_Populate(snemoresourcedb)
endif()

FetchContent_Declare(snemoconditionsdb
  GIT_REPOSITORY https://github.com/SuperNEMO-DBD/SNemoConditionsDB-proto.git
  GIT_TAG        main
  SOURCE_DIR     SNemoConditionsDB.git)
FetchContent_GetProperties(snemoconditionsdb)
if(NOT snemoconditionsdb_POPULATED)
  FetchContent_Populate(snemoconditionsdb)
endif()

# - Run examples as tests to confirm expected behaviour
add_test(NAME test_resourceDB COMMAND resourceDB "${snemoresourcedb_SOURCE_DIR}")
add_test(NAME test_condDB COMMAND condDB "${snemoconditionsdb_SOURCE_DIR}")