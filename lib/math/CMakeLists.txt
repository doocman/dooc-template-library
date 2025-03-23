
set(CT dtl.math)

add_library(${CT} INTERFACE)
add_library(dtl::${CT} ALIAS ${CT})
target_include_directories(${CT} INTERFACE inc)

if (DTL_TESTS)
    add_subdirectory(tests)
endif ()
if (DTL_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()
