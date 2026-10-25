
dtl_public_library(dtl.internal
        COMPAT
        HEADERS
            inc/dtl/internal/std/inplace_vector.hpp
            inc/dtl/internal/export.hpp
            inc/dtl/internal/std.hpp
)

if (DTL_TESTS)
    add_subdirectory(tests)
endif ()
