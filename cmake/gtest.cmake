

include(FetchContent)
FetchContent_Declare(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5 # v1.16.0
)

fetchcontent_makeavailable(gtest)
include(GoogleTest)

