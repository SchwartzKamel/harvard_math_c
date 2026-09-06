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
option(HM_BUILD_BENCHMARKS "Build harvard_math benchmarks" OFF)
option(HM_BENCHMARK_GSL "Compare benchmarks with GNU Scientific Library" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(HM_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/harvard_math")

add_library(harvard_math src/stats.c src/regression.c)
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

if(BUILD_SHARED_LIBS)
    target_compile_definitions(harvard_math PUBLIC HM_BUILD_SHARED PRIVATE HM_BUILDING_LIBRARY)
endif()

install(
    TARGETS harvard_math
    EXPORT harvard_mathTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
    EXPORT harvard_mathTargets
    FILE harvard_mathTargets.cmake
    NAMESPACE harvard_math::
    DESTINATION ${HM_INSTALL_CMAKEDIR}
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/harvard_mathConfig.cmake.in
    ${PROJECT_BINARY_DIR}/harvard_mathConfig.cmake
    INSTALL_DESTINATION ${HM_INSTALL_CMAKEDIR}
)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/harvard_mathConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES
        ${PROJECT_BINARY_DIR}/harvard_mathConfig.cmake
        ${PROJECT_BINARY_DIR}/harvard_mathConfigVersion.cmake
    DESTINATION ${HM_INSTALL_CMAKEDIR}
)

set(HM_PKGCONFIG_MATH_LIB "")
if(NOT WIN32)
    set(HM_PKGCONFIG_MATH_LIB "-lm")
endif()
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/harvard_math.pc.in
    ${PROJECT_BINARY_DIR}/harvard_math.pc
    @ONLY
)
install(
    FILES ${PROJECT_BINARY_DIR}/harvard_math.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

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

if(HM_BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif()
