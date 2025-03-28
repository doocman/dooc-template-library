cmake_minimum_required(VERSION 3.30)
project(dtl)

set(CMAKE_CXX_STANDARD 26)

function(dtl_add_3pp FETCH TARGET)
    set(DTL_SKIP_${FETCH} OFF CACHE BOOL "Set to off if you provide ${TARGET} by other means")
    set (FETCH_FILE cmake/${FETCH}.cmake)
    if(TARGET ${TARGET})
    elseif (DTL_SKIP_${FETCH})
    else ()
        include(cmake/${FETCH}.cmake)
        if (NOT TARGET ${TARGET})
            message(SEND_ERROR "${TARGET} not found after including ${FETCH_FILE}")
        endif ()
    endif ()
endfunction()

if(PROJECT_IS_TOP_LEVEL)
    set(DTL_TESTS ON CACHE BOOL "")
    set(DTL_BENCHMARKS ON CACHE BOOL "")
else ()
    set(DTL_TESTS OFF CACHE BOOL "")
    set(DTL_BENCHMARKS OFF CACHE BOOL "")
endif ()

if(DTL_TESTS)
    enable_testing()
    add_subdirectory(cmake-presets)
    dtl_add_3pp(gtest gmock)
endif ()
if(DTL_BENCHMARKS)
    dtl_add_3pp(gbenchmark benchmark)
endif ()

add_subdirectory(lib)
