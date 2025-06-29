cmake_minimum_required(VERSION 3.15.0)

project(TreeSearchSolver LANGUAGES CXX)

# Build options.
option(TREESEARCHSOLVER_BUILD_TEST "Build the unit tests" ON)
option(TREESEARCHSOLVER_BUILD_EXAMPLES "Build examples" ON)

# Avoid FetchContent warning.
cmake_policy(SET CMP0135 NEW)

# Require C++11.
set(CMAKE_CXX_STANDARD 11)

# Enable output of compile commands during generation.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add sub-directories.
add_subdirectory(extern)
add_subdirectory(src)
if(TREESEARCHSOLVER_BUILD_TEST)
  add_subdirectory(test)
endif()
if(TREESEARCHSOLVER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
