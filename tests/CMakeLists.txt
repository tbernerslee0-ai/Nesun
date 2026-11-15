cmake_minimum_required(VERSION 3.16)

# Enable CTest if not already enabled in root
if(NOT CTEST_ENABLED)
    enable_testing()
endif()

# Test Executable for CPU & Mapper Unit Tests
add_executable(nesun_unit_tests
    cpu_tests.cpp
    mapper_tests.cpp
)

target_include_directories(nesun_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(nesun_unit_tests PRIVATE
    nesun_core
)

# Register test with CTest runner
add_test(NAME NesunCoreUnitTests COMMAND nesun_unit_tests)
