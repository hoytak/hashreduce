project(hashreduce)

cmake_minimum_required(VERSION 2.4)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
endif()

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "")
endif()

if(NOT CMAKE_LIBRARY_PATH)
  set(CMAKE_LIBRARY_PATH "")
endif()

# guard against bad build-type strings
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)

if(    NOT CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug"
   AND NOT CMAKE_BUILD_TYPE_TOLOWER STREQUAL "release")
  message(FATAL_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug and Release")
endif()

include(CheckCCompilerFlag)

################################################################################
# Check for compiler options

set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -UNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g -O3 -DNDEBUG")

if(CMAKE_COMPILER_IS_GNUCXX)
  message("Detected compuler is GNU C.")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -mfpmath=sse -ffast-math -fno-strict-aliasing -fno-common")

  if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "release")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
  endif()
    
endif()

add_subdirectory(src)

install(
  DIRECTORY "src/"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/include/hashreduce"
  FILES_MATCHING PATTERN "*.h" 
)

message("")
message("Configured HashReduce.")
message("")

string(TOLOWER "${CMAKE_GENERATOR}" cmake_generator_tolower)
if(cmake_generator_tolower MATCHES "makefile")
  message("Some things you can do now:")
  message("--------------+----------------------------------------------------------------")
  message("Command       |   Description")
  message("--------------+----------------------------------------------------------------")
  message("make install  | Install to ${CMAKE_INSTALL_PREFIX}. To change that:")
  message("              |     cmake . -DCMAKE_INSTALL_PREFIX=yourpath")
  message("--------------+----------------------------------------------------------------")
endif()

message("")
