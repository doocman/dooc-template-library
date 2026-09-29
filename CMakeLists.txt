cmake_minimum_required(VERSION 3.30)
project(dtl LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 26)
set(DTL_INSTALL ${PROJECT_IS_TOP_LEVEL} CACHE BOOL "ON if you want to add install commands for dtl, off otherwise")

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

set (DTL_ALL_LIBS "" CACHE INTERNAL "")
set (DTL_ALL_LIBS_PREFIXED "" CACHE INTERNAL "")
function(dtl_public_library NAME)
    add_library(${NAME} INTERFACE)
    add_library(dtl::${NAME} ALIAS ${NAME})
    set(DTL_ALL_LIBS ${DTL_ALL_LIBS} ${NAME} CACHE INTERNAL "")
    set(DTL_ALL_LIBS_PREFIXED ${DTL_ALL_LIBS_PREFIXED} dtl::${NAME} CACHE INTERNAL "")
endfunction()
add_subdirectory(lib)
if (DTL_INSTALL)
    include(CMakePackageConfigHelpers)
    install(TARGETS
            ${DTL_ALL_LIBS} dtl
            EXPORT dtlTargets
            PUBLIC_HEADER COMPONENT Development
            CXX_MODULES_BMI COMPONENT Development
            FILE_SET HEADERS COMPONENT Development
            FILE_SET CXX_MODULES COMPONENT Development
    )
    write_basic_package_version_file(
            "${PROJECT_BINARY_DIR}/dtlConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )
    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/dtlConfig.cmake.in"
            "${PROJECT_BINARY_DIR}/dtlConfig.cmake"
            INSTALL_DESTINATION lib/cmake/dtl
    )
    install (
            EXPORT dtlTargets
            DESTINATION lib/cmake/dtl
            NAMESPACE dtl::
    )
    install(FILES
            "${PROJECT_BINARY_DIR}/dtlConfigVersion.cmake"
            "${PROJECT_BINARY_DIR}/dtlConfig.cmake"
            DESTINATION lib/cmake/dtl
    )
endif ()
