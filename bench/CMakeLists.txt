add_executable(
    harvard_math_benchmark
    benchmark.c
    reference_stats.c
)
target_compile_features(harvard_math_benchmark PRIVATE c_std_99)
target_compile_definitions(
    harvard_math_benchmark
    PRIVATE HM_BENCHMARK_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)
target_link_libraries(harvard_math_benchmark PRIVATE harvard_math::harvard_math)
