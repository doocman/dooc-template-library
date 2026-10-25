
add_subdirectory(internal)
add_subdirectory(math)
add_subdirectory(string)

set(_DTL_ALL_LINKS ${DTL_ALL_LIBS_PREFIXED})

dtl_public_library(dtl
        HEADERS
        MODULES
            dtl.cppm
)
target_link_libraries(dtl ${LIB_PUB} ${_DTL_ALL_LINKS})
