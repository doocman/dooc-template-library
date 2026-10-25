
set(CT dtl.math)

dtl_public_library(${CT}
        HEADERS
            inc/dtl/powi.hpp

)

if (DTL_TESTS)
    add_subdirectory(tests)
endif ()
if (DTL_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()
