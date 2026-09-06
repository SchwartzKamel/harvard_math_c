cmake_minimum_required(VERSION 3.16)

project(harvard_math_consumer LANGUAGES C)

find_package(harvard_math CONFIG REQUIRED)

add_executable(harvard_math_consumer main.c)
target_compile_features(harvard_math_consumer PRIVATE c_std_99)
target_link_libraries(harvard_math_consumer PRIVATE harvard_math::harvard_math)

enable_testing()
add_test(NAME harvard_math_consumer_runs COMMAND harvard_math_consumer)
