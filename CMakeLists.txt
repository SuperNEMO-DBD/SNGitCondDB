cmake_minimum_required(VERSION 3.18)
project(GitResourceService VERSION 0.1.0)

# For GitCondDB
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
  include(FetchContent)
  FetchContent_Declare(GitCondDB
    GIT_REPOSITORY https://gitlab.cern.ch/bmorgan/GitCondDB.git
    GIT_TAG        master)
  FetchContent_MakeAvailable(GitCondDB)
endif()

foreach(_dbapp resourceDB condDB)
  add_executable(${_dbapp} ${_dbapp}.cc)
  target_link_libraries(${_dbapp} PRIVATE GitCondDB::GitCondDB)
endforeach()