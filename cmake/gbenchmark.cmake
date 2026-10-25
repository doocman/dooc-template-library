



include(FetchContent)
FetchContent_Declare(
        gbenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG 192ef10025eb2c4cdd392bc502f0c852196baa48 # v1.9.5
        EXCLUDE_FROM_ALL SYSTEM
)

fetchcontent_makeavailable(gbenchmark)

