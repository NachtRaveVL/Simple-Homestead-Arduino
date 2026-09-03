cmake_minimum_required(VERSION 3.16)
project(TerraduinoHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
endif()

find_package(Python3 COMPONENTS Interpreter)

set(TERRADUINO_SRC
    ../src/TerraActivation.cpp
    ../src/TerraActuators.cpp
    ../src/TerraAttachments.cpp
    ../src/TerraBalancers.cpp
    ../src/TerraData.cpp
    ../src/TerraDatas.cpp
    ../src/TerraDrivers.cpp
    ../src/TerraFactory.cpp
    ../src/TerraLogger.cpp
    ../src/TerraMeasurements.cpp
    ../src/TerraModules.cpp
    ../src/TerraObject.cpp
    ../src/TerraPins.cpp
    ../src/TerraPublisher.cpp
    ../src/TerraRails.cpp
    ../src/TerraReservoirs.cpp
    ../src/TerraScheduler.cpp
    ../src/TerraSensors.cpp
    ../src/TerraStreams.cpp
    ../src/TerraStrings.cpp
    ../src/TerraTriggers.cpp
    ../src/TerraUtils.cpp
    ../src/Terraduino.cpp
    ../src/shared/TerraUIData.cpp
)

add_library(terraduino_host STATIC ${TERRADUINO_SRC})
target_include_directories(terraduino_host PUBLIC ../src)

add_executable(test_automation host/test_automation.cpp)
target_link_libraries(test_automation PRIVATE terraduino_host)

add_executable(test_hardening host/test_hardening.cpp)
target_link_libraries(test_hardening PRIVATE terraduino_host)

add_executable(test_hardware host/test_hardware.cpp)
target_link_libraries(test_hardware PRIVATE terraduino_host)

add_executable(test_infrastructure host/test_infrastructure.cpp)
target_link_libraries(test_infrastructure PRIVATE terraduino_host)

add_executable(test_serialization host/test_serialization.cpp)
target_link_libraries(test_serialization PRIVATE terraduino_host)

add_executable(test_terraduino host/test_terraduino.cpp)
target_link_libraries(test_terraduino PRIVATE terraduino_host)

enable_testing()
add_test(NAME automation COMMAND test_automation)
add_test(NAME hardening COMMAND test_hardening)
add_test(NAME hardware COMMAND test_hardware)
add_test(NAME infrastructure COMMAND test_infrastructure)
add_test(NAME serialization COMMAND test_serialization)
add_test(NAME terraduino COMMAND test_terraduino)
