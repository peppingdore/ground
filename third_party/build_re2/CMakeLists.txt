cmake_minimum_required(VERSION 3.15)
project(build_re2)

set(ABSL_ENABLE_INSTALL ON)

add_subdirectory(../abseil-cpp abseil-cpp)
add_subdirectory(../re2 re2)
