cmake_minimum_required(VERSION 3.14)
project(p101_error_tests C CXX)
enable_testing()
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(P101_TEST_COVERAGE "Instrument the code under test for coverage" OFF)
set(P101_TEST_COVERAGE_FLAGS "")
if(P101_TEST_COVERAGE)
    set(P101_TEST_COVERAGE_FLAGS --coverage)
endif()

add_library(p101_error_under_test STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/check.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/error.c"
)
target_include_directories(p101_error_under_test PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_definitions(p101_error_under_test PRIVATE _POSIX_C_SOURCE=200809L)
target_compile_options(p101_error_under_test PRIVATE ${P101_TEST_COVERAGE_FLAGS})

add_executable(test_error test_error.c)
target_link_libraries(test_error PRIVATE p101_error_under_test m Threads::Threads)
target_compile_definitions(test_error PRIVATE _POSIX_C_SOURCE=200809L)
target_compile_options(test_error PRIVATE ${P101_TEST_COVERAGE_FLAGS})
target_link_options(test_error PRIVATE ${P101_TEST_COVERAGE_FLAGS})
add_test(NAME test_error COMMAND test_error)

add_executable(test_check_cxx test_check_cxx.cpp)
target_link_libraries(test_check_cxx PRIVATE p101_error_under_test)
target_compile_options(test_check_cxx PRIVATE
        ${P101_TEST_COVERAGE_FLAGS}
        -Wall
        -Wextra
        -Werror
        -pedantic-errors
)
target_link_options(test_check_cxx PRIVATE ${P101_TEST_COVERAGE_FLAGS})
add_test(NAME test_check_cxx COMMAND test_check_cxx)
