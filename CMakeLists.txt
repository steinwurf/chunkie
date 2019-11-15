cmake_minimum_required (VERSION 3.10)
project(chunkie)

if(NOT DEFINED STEINWURF_RESOLVE)
    message(STATUS "Resolving dependencies...")
    execute_process(
        COMMAND python waf resolve
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(STEINWURF_RESOLVE "${CMAKE_CURRENT_SOURCE_DIR}/resolve_symlinks")
endif()

add_library(chunkie INTERFACE)
add_library(steinwurf::chunkie ALIAS chunkie)

add_subdirectory("${STEINWURF_RESOLVE}/bitter" bitter)
add_subdirectory("${STEINWURF_RESOLVE}/endian" endian)

target_link_libraries(chunkie
    INTERFACE steinwurf::endian
    INTERFACE steinwurf::bitter)

target_include_directories(chunkie INTERFACE src)

target_compile_features(chunkie INTERFACE cxx_std_14)

add_executable(serialize_deserialize_concatenated_buffers ./examples/serialize_deserialize_concatenated_buffers.cpp)
target_link_libraries(serialize_deserialize_concatenated_buffers steinwurf::chunkie)
