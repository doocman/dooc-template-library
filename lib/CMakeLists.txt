
add_subdirectory(internal)
add_subdirectory(math)
add_subdirectory(string)

dtl_public_library(dtl
        HEADERS
        MODULES
            dtl.cppm
)
target_link_libraries(dtl INTERFACE ${DTL_ALL_LIBS_PREFIXED})
