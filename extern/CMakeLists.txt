# Enable FetchContent.
include(FetchContent)

# Fetch boost.
if (TREESEARCHSOLVER_BUILD_TEST OR TREESEARCHSOLVER_BUILD_EXAMPLES)
  set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options dynamic_bitset)
  set(BOOST_ENABLE_CMAKE ON)
  FetchContent_Declare(
          Boost
          URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
          EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(Boost)
endif()

# Fetch googletest.
if(TREESEARCHSOLVER_BUILD_TEST)
  FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF)
  FetchContent_MakeAvailable(googletest)
endif()

# Fetch fontanf/orproblems.
if(TREESEARCHSOLVER_BUILD_EXAMPLES)
  FetchContent_Declare(
      orproblems
      GIT_REPOSITORY https://github.com/fontanf/orproblems.git
      GIT_TAG ba099d6bd0ac186e4113fe340789a1800fa72852
#      SOURCE_DIR "${PROJECT_SOURCE_DIR}/../orproblems/"
      EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(orproblems)
endif()