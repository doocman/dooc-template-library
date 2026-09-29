

add_subdirectory(math)
add_subdirectory(string)

add_library(dtl INTERFACE)
add_library(dtl::dtl ALIAS dtl)
target_link_libraries(dtl INTERFACE ${DTL_ALL_LIBS_PREFIXED})
