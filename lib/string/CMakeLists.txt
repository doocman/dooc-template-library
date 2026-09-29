
set(CT dtl.string)

dtl_public_library(${CT})
target_sources(${CT} INTERFACE
        FILE_SET HEADERS
        BASE_DIRS inc
        FILES
            inc/dtl/format_extend.hpp
)

