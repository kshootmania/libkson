cmake_minimum_required(VERSION 3.12)
project(kson_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(Catch2)

# Test executable
file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(kson_test
    ${TEST_SOURCES}
)

# Include directories
target_include_directories(kson_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

if(KSON_WITHOUT_JSON_DEPENDENCY)
    target_compile_definitions(kson_test
        PUBLIC KSON_WITHOUT_JSON_DEPENDENCY
    )
endif()

# Link libraries
target_link_libraries(kson_test 
    PRIVATE 
    kson
    Catch2::Catch2
)

# Add platform-specific libraries
if(NOT WIN32)
    # Link iconv on non-Windows platforms
    find_package(Iconv REQUIRED)
    target_link_libraries(kson_test PRIVATE Iconv::Iconv)
endif()

# Copy assets directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Enable CTest
enable_testing()

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(kson_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)