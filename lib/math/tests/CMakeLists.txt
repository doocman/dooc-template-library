
set(CT dtl.math.tests)

add_executable(${CT} powl.cpp)

target_link_libraries(${CT} PRIVATE
        GTest::gmock_main
)

