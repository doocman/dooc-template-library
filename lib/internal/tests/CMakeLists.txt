
set(CT dtl.internal.tests)
dtl_test_gmock_main(${CT} inplace_vector.cpp)
target_link_libraries(${CT} PRIVATE
        dtl::dtl.internal
)
