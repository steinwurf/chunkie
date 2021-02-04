cmake_minimum_required(VERSION 3.10)
project(chunkie)

# Use waf to resolve dependencies
if(NOT DEFINED STEINWURF_RESOLVE)
  message(STATUS "Resolving dependencies...")
  execute_process(COMMAND python waf resolve
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set(STEINWURF_RESOLVE "${CMAKE_CURRENT_SOURCE_DIR}/resolve_symlinks")
endif()

# endian dependency
if(NOT TARGET steinwurf::endian)
  add_subdirectory("${STEINWURF_RESOLVE}/endian" endian)
endif()

# bitter dependency
if(NOT TARGET steinwurf::bitter)
  add_subdirectory("${STEINWURF_RESOLVE}/bitter" bitter)
endif()

# Define library
add_library(chunkie INTERFACE)
target_compile_features(chunkie INTERFACE cxx_std_14)
target_include_directories(chunkie INTERFACE src/)
add_library(steinwurf::chunkie ALIAS chunkie)

# Link dependencies
target_link_libraries(chunkie INTERFACE steinwurf::endian)
target_link_libraries(chunkie INTERFACE steinwurf::bitter)

# Install headers
install(
  DIRECTORY ./src/chunkie
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  FILES_MATCHING
  PATTERN *.hpp)

# Is top level project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
  # Build executables
  add_executable(serialize_deserialize_concatenated_buffers
                 examples/serialize_deserialize_concatenated_buffers.cpp)
  target_link_libraries(serialize_deserialize_concatenated_buffers chunkie)
  add_executable(serialize_deserialize_unequal_buffers
                 examples/serialize_deserialize_unequal_buffers.cpp)
  target_link_libraries(serialize_deserialize_unequal_buffers chunkie)
  add_executable(serialize_deserialize_zeropadded_buffers
                 examples/serialize_deserialize_zeropadded_buffers.cpp)
  target_link_libraries(serialize_deserialize_zeropadded_buffers chunkie)
endif()
