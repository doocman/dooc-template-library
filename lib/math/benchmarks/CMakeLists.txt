
set(CT dtl.math.bb)
add_executable(${CT} lpow.bb.cpp)

target_link_libraries(${CT} PRIVATE
        dtl::dtl.math

        benchmark::benchmark_main
)
