# Enable FetchContent.
include(FetchContent)

# Fetch fontanf/orproblems.
FetchContent_Declare(
    orproblems
    GIT_REPOSITORY https://github.com/fontanf/orproblems.git
    GIT_TAG e9a58e010f7dc0ca7b5eea2358ed11d386f8622a)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../orproblems/")
FetchContent_MakeAvailable(orproblems)
