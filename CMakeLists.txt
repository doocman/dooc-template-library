cmake_minimum_required(VERSION 3.30)
project(dtl LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 26)
set(DTL_INSTALL ${PROJECT_IS_TOP_LEVEL} CACHE BOOL "ON if you want to add install commands for dtl, off otherwise")

function(dtl_add_3pp FETCH TARGET)
    set(DTL_SKIP_${FETCH} OFF CACHE BOOL "Set to off if you provide ${TARGET} by other means")
    set(FETCH_FILE cmake/${FETCH}.cmake)
    if (TARGET ${TARGET})
    elseif (DTL_SKIP_${FETCH})
    else ()
        include(cmake/${FETCH}.cmake)
        if (NOT TARGET ${TARGET})
            message(SEND_ERROR "${TARGET} not found after including ${FETCH_FILE}")
        endif ()
    endif ()
endfunction()

if (PROJECT_IS_TOP_LEVEL)
    set(DTL_TESTS ON CACHE BOOL "")
    set(DTL_BENCHMARKS ON CACHE BOOL "")
else ()
    set(DTL_TESTS OFF CACHE BOOL "")
    set(DTL_BENCHMARKS OFF CACHE BOOL "")
endif ()

if (DTL_TESTS)
    enable_testing()
    add_subdirectory(cmake-presets)
    dtl_add_3pp(gtest gmock)
endif ()
if (DTL_BENCHMARKS)
    dtl_add_3pp(gbenchmark benchmark)
endif ()

set(DTL_ALL_LIBS "" CACHE INTERNAL "")
set(DTL_ALL_LIBS_PREFIXED "" CACHE INTERNAL "")
set(DTL_MODULES ON CACHE BOOL "Enable to build c++-20 modules")
set(DTL_STD_MODULE OFF CACHE BOOL "Enable to change all standard includes to import std")
function(dtl_public_library NAME)
    set(options COMPAT)
    set(one_value_args)
    set(multi_value_args HEADERS MODULES)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "${multi_value_args}")
    set(LIB_TYPE INTERFACE)
    set(LIB_PUB INTERFACE)
    set(INC_MODULES OFF)
    if (DTL_MODULES AND NOT "${ARG_MODULES}" STREQUAL "")
        set(LIB_TYPE)
        set(LIB_PUB PUBLIC)
        set(INC_MODULES ON)
    endif ()
    set(LIB_PUB ${LIB_PUB} PARENT_SCOPE)
    add_library(${NAME} ${LIB_TYPE})
    add_library(dtl::${NAME} ALIAS ${NAME})
    set(DTL_ALL_LIBS ${DTL_ALL_LIBS} ${NAME} CACHE INTERNAL "")
    set(DTL_ALL_LIBS_PREFIXED ${DTL_ALL_LIBS_PREFIXED} dtl::${NAME} CACHE INTERNAL "")
    message(
            "
            ${NAME} ${LIB_PUB}
            FILE_SET HEADERS
            BASE_DIRS inc
            FILES ${ARG_HEADERS}
            "
    )
    target_sources(${NAME} ${LIB_PUB}
            FILE_SET HEADERS
            BASE_DIRS inc
            FILES ${ARG_HEADERS}
    )
    if (INC_MODULES)
        target_sources(${NAME} ${LIB_PUB}
                FILE_SET CXX_MODULES
                FILES ${ARG_MODULES}
        )
    endif ()
    if (NOT ${ARG_COMPAT})
        target_link_libraries(${NAME} ${LIB_PUB}
                dtl::dtl.internal
        )
    endif ()
endfunction()

function(dtl_test_gmock_main NAME)
    add_executable(${NAME} ${ARGN})
    if (DTL_MODULES)
        target_compile_definitions(${NAME} PRIVATE
                DTL_MODULES
        )
    endif ()
    target_link_libraries(${NAME} PRIVATE
            GTest::gmock_main
    )
endfunction()

add_subdirectory(lib)
if (DTL_INSTALL)
    include(CMakePackageConfigHelpers)
    install(TARGETS
            ${DTL_ALL_LIBS}
            EXPORT dtlTargets
            PUBLIC_HEADER COMPONENT Development
            CXX_MODULES_BMI COMPONENT Development DESTINATION lib/modules
            FILE_SET HEADERS COMPONENT Development
            FILE_SET CXX_MODULES COMPONENT Development DESTINATION lib/modules
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
    install(
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
