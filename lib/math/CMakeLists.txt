
set(CT dtl.math)

dtl_public_library(${CT})
target_sources(${CT} INTERFACE
        FILE_SET HEADERS
        TYPE HEADERS
        BASE_DIRS
            inc
            #"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>"
            #"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>"
        FILES
            "inc/dtl/lpow.hpp"
)

if (DTL_TESTS)
    add_subdirectory(tests)
endif ()
if (DTL_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()
