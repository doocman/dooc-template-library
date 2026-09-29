



include(FetchContent)
FetchContent_Declare(
        gbenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG c58e6d0710581e3a08d65c349664128a8d9a2461 # v1.9.1
        EXCLUDE_FROM_ALL SYSTEM
)

fetchcontent_makeavailable(gbenchmark)

