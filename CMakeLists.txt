cmake_minimum_required(VERSION 3.11)
project(kson CXX)

option(KSON_BUILD_SHARED "Build shared library" OFF)
option(KSON_BUILD_TOOL_KSH2KSON "Build ksh2kson tool" ON)
option(KSON_BUILD_TOOL_KSON2KSH "Build kson2ksh tool" ON)
option(KSON_BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Get git commit hash
find_package(Git QUIET)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE KSON_GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
else()
	set(KSON_GIT_HASH "unknown")
endif()

set(KSON_GIT_HASH "${KSON_GIT_HASH}" PARENT_SCOPE)

set(KSON_VERSION "0.1.0")
if(NOT KSON_GIT_HASH STREQUAL "unknown")
	set(KSON_VERSION_FULL "${KSON_VERSION}+${KSON_GIT_HASH}")
else()
	set(KSON_VERSION_FULL "${KSON_VERSION}")
endif()

file(GLOB_RECURSE sources src/*.cpp)

if(KSON_BUILD_SHARED)
    add_library(kson SHARED ${sources})
else()
    add_library(kson STATIC ${sources})
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kson PUBLIC -fconcepts)
endif()
target_include_directories(kson PUBLIC ${PROJECT_SOURCE_DIR}/include)
if(NOT WIN32)
    find_package(Iconv REQUIRED)
    target_link_libraries(kson PRIVATE Iconv::Iconv)
endif()

if(KSON_BUILD_TOOL_KSH2KSON)
	# Generate version header for ksh2kson
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/tool/ksh2kson_version.h.in
		${CMAKE_CURRENT_BINARY_DIR}/tool/ksh2kson_version.h
		@ONLY
	)
	add_executable(ksh2kson ${PROJECT_SOURCE_DIR}/tool/ksh2kson.cpp)
	target_link_libraries(ksh2kson kson)
	target_include_directories(ksh2kson PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/tool)
endif()

if(KSON_BUILD_TOOL_KSON2KSH)
    add_executable(kson2ksh ${PROJECT_SOURCE_DIR}/tool/kson2ksh.cpp)
    target_link_libraries(kson2ksh kson)
endif()

if(KSON_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
