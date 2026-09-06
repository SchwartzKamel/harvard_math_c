cmake_minimum_required(VERSION 3.16)

project(harvard_math
    VERSION 0.1.0
    DESCRIPTION "Small descriptive-statistics library for C"
    LANGUAGES C
)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HM_BUILD_TESTS "Build harvard_math tests" ON)
option(HM_BUILD_EXAMPLES "Build harvard_math examples" OFF)

add_library(harvard_math src/stats.c)
add_library(harvard_math::harvard_math ALIAS harvard_math)

target_compile_features(harvard_math PUBLIC c_std_99)
target_include_directories(harvard_math
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(NOT WIN32)
    target_link_libraries(harvard_math PUBLIC m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(harvard_math PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(harvard_math PRIVATE /W4)
endif()

include(CTest)
if(HM_BUILD_TESTS AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(HM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
