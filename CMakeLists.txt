cmake_minimum_required(VERSION 3.8)
project(kson CXX)
file(GLOB_RECURSE sources src/*.cpp)
add_library(kson STATIC ${sources})
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
        target_compile_options(kson PRIVATE /MTd /W4)
        message("Build type: Debug")
    else()
        target_compile_options(kson PRIVATE /MT /Ox /W4 /DNDEBUG)
        message("Build type: Release")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kson PRIVATE -O2 -Wall -fPIC -fconcepts)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(kson PRIVATE -O2 -Wall -fPIC)
else()
    target_compile_options(kson PRIVATE -O2 -Wall -fPIC)
    message("Warning: Unsupported compiler")
endif()
target_compile_features(kson PRIVATE cxx_std_20)
target_include_directories(kson PUBLIC ${PROJECT_SOURCE_DIR}/include)