set(HM_BENCHMARK_SOURCES benchmark.c reference_stats.c)
if(HM_BENCHMARK_GSL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GSL REQUIRED IMPORTED_TARGET gsl)
    list(APPEND HM_BENCHMARK_SOURCES gsl_adapter.c)
endif()

add_executable(harvard_math_benchmark ${HM_BENCHMARK_SOURCES})
target_compile_features(harvard_math_benchmark PRIVATE c_std_99)
target_compile_definitions(
    harvard_math_benchmark
    PRIVATE HM_BENCHMARK_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)
target_link_libraries(harvard_math_benchmark PRIVATE harvard_math::harvard_math)
if(HM_BENCHMARK_GSL)
    target_compile_definitions(harvard_math_benchmark PRIVATE HM_HAVE_GSL)
    target_link_libraries(harvard_math_benchmark PRIVATE PkgConfig::GSL)
endif()
