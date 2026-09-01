cmake_minimum_required(VERSION 3.10)
project(TerraduinoHostTests CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wfloat-equal -Werror)
endif()

enable_testing()

add_executable(terraduino_hardening_tests
    host/test_hardening.cpp
)
target_include_directories(terraduino_hardening_tests PRIVATE ../src)

add_test(NAME terraduino_hardening_tests COMMAND terraduino_hardening_tests)
