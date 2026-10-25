
set(CT dtl.string)

dtl_public_library(${CT}
        HEADERS
            inc/dtl/format_extend.hpp
        MODULES
            mod/string.cppm
)
target_sources(${CT} ${LIB_PUB}
        FILE_SET HEADERS
        BASE_DIRS inc
        FILES
            inc/dtl/format_extend.hpp
)

if (DTL_TESTS)
    add_subdirectory(tests)
endif ()

