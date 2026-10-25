
set(CT dtl.string.tests)

dtl_test_gmock_main(${CT}
        constexpr_expressions.tests.cpp
)

target_link_libraries(${CT} PRIVATE
        dtl::dtl.string

        cmpr::add_warnings
)


