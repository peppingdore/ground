cmake_minimum_required(VERSION 3.15)
project(re2_build)

# We should not have to enable this, but we can't link absl to re2 without it.
set(ABSL_ENABLE_INSTALL ON)
set(ABSL_CONSUME_DLL ON)

add_subdirectory(../abseil-cpp abseil-cpp)
add_subdirectory(../re2 re2)
