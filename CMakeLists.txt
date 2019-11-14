cmake_minimum_required(VERSION 3.8)
project(GitResourceService VERSION 0.1.0)

find_package(PkgConfig)
pkg_check_modules(git2 libgit2 REQUIRED IMPORTED_TARGET)

add_executable(exercise-lg2 exercise-lg2.cc)
target_link_libraries(exercise-lg2 PRIVATE PkgConfig::git2)