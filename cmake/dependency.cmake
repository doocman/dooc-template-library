
function(dtl_dependency)
    set(options )
    set(oneValueArgs NAME ALIAS)
    set(multiValueArgs TARGETS CONFIGURATIONS)
    cmake_parse_arguments(PARSE_ARGV 0 arg
            "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )
endfunction()
