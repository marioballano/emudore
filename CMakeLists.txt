cmake_minimum_required (VERSION 2.8)
project(emudore)
set(SRC_FILES "src/c64.cpp"
              "src/cpu.cpp"
              "src/memory.cpp"
              "src/cia1.cpp"
              "src/cia2.cpp"
              "src/vic.cpp"
              "src/io.cpp"
              "src/loader.cpp"
              "src/main.cpp")
# GCC 
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
# C++11
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
# r2 debugging support, for now only available on Linux and
# enabled on Debug builds, if you want it enabled on release 
# mode just remove the second part of the expression
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUGGER_SUPPORT)
  set(SRC_FILES ${SRC_FILES} "src/debugger.cpp")
endif()         
# MSVC, using pre-built binaries downloaded from:
# https://www.libsdl.org/download-2.0.php
if (MSVC)
  set(SDL2_PATH "C:\\SDL2")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  add_executable (emudore ${SRC_FILES} res/emudore.rc)
  set_target_properties(emudore PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
  set_target_properties(emudore PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
else()
  add_executable (emudore ${SRC_FILES})
endif()
# JavaScript build with emscripten
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -s USE_SDL=2 --preload-file assets/")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()
# SDL2 
# Cmake 2.8 does not provide scripts to find SDL2, adding 
# them to the cmake module path and here for convenience
if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIR})
endif()
# link
target_link_libraries(emudore ${SDL2_LIBRARY})
# copy assets to build directory 
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/assets/ ${CMAKE_BINARY_DIR}/assets/)
