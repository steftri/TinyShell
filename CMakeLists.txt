cmake_minimum_required(VERSION 3.14)

project(TinyShell LANGUAGES CXX)

add_library(TinyShell STATIC
  src/TinyShell.cpp
)

# Set C++ standard for this target
target_compile_features(TinyShell PUBLIC cxx_std_17)

# Add compiler options
target_compile_options(TinyShell PRIVATE -Wall -Werror -pedantic)

# Include directories
target_include_directories(TinyShell PUBLIC src)

install(DIRECTORY src/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)
