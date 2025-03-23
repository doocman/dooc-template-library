
set(CT dtl.math.tests)

add_executable(${CT}
        lpow.tests.cpp
)

target_link_libraries(${CT} PRIVATE
        GTest::gmock_main

        dtl::dtl.math
)

