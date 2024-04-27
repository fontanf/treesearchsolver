# Enable FetchContent.
include(FetchContent)

# Fetch fontanf/orproblems.
FetchContent_Declare(
    orproblems
    GIT_REPOSITORY https://github.com/fontanf/orproblems.git
    GIT_TAG 538e605f7348b461710665fa0c62ec9d8ed8b3c4
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../orproblems/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(orproblems)
