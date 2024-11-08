cmake_minimum_required(VERSION 3.10)
project(project-name)

set(CMAKE_CXX_STANDARD 11)

enable_testing()

add_subdirectory(tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_scanner tests/test_scanner.cpp)
target_link_libraries(test_scanner gtest gtest_main)

add_test(NAME test_scanner COMMAND test_scanner)