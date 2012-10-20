project(hashreduce)

cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
endif()

option(ENABLE_CHECKS "Enable internal consistency checks for debugging (very slow)." "No")
option(BUILD_IBD "Enable building of ibd graph related features." "Yes")
option(BUILD_EXAMPLES "Enable building of example code." "Yes")
option(BUILD_C_IBD_COMPARE "Enable building of ibd graph C-only library related features (rather than python version)." "Yes")
option(PYTHON_LINK_STATIC "Link against the python libraries statically." "No")

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "")
endif()

if(NOT CMAKE_LIBRARY_PATH)
  set(CMAKE_LIBRARY_PATH "")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# guard against bad build-type strings
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)

if(    NOT CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug"
   AND NOT CMAKE_BUILD_TYPE_TOLOWER STREQUAL "release")
  message(FATAL_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug and Release")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CheckCCompilerFlag)

################################################################################
# Check for compiler options

set(CMAKE_C_FLAGS_DEBUG "-g3 -O1 -UNDEBUG -Wall -Wno-address")
set(CMAKE_C_FLAGS_RELEASE "-g -O3 -DNDEBUG -Wno-address")

if(ENABLE_CHECKS)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DENABLE_CONSISTENCY_CHECKS")
  message("Enabling (slow) internal consistency checks.")
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
  message("Detected compuler is GNU C.")

  check_c_compiler_flag("-march=native" march_native)

  if(march_native)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
    message("Enabling -march=native flag for release.")
  else()
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mtune=i686")
  endif()

  check_c_compiler_flag("-ffast-math" ffast_math)

  if(march_native)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math")
    message("Enabling -ffast-math flag for release.")
  endif()
  
  if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "release")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
  endif()
endif()

############################################################
# Setting things up for the intel 

if(${CMAKE_C_COMPILER} MATCHES "icc.*$" OR ${CMAKE_C_COMPILER} MATCHES "Intel")
  set(CMAKE_COMPILER_IS_INTEL Yes)
endif()

if(CMAKE_COMPILER_IS_INTEL)
  message("Setting flags for intel compiler.")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -traceback")

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -no-ipo -O0 -UNDEBUG")
  set(LINKER_FLAGS_DEBUG "-g -no-ipo -O0")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -O3 -ipo -xHOST -no-prec-div -DNDEBUG")
  set(LINKER_FLAGS_RELEASE "-O3 -ipo -xHost")

  if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    set(LINKER_FLAGS "${LINKER_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "release")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    set(LINKER_FLAGS "${LINKER_FLAGS_RELEASE}")
  endif()

endif(CMAKE_COMPILER_IS_INTEL)

################################################################################
# Check on type stuff 

# 128 bit integers
include(CheckInt128)

Check_Int128("__uint128"           UINT128_DEF "USE__uint128")
Check_Int128("uint128"             UINT128_DEF "USEuint128")
Check_Int128("__uint128_t"         UINT128_DEF "USE__uint128_t")
Check_Int128("uint128_t"           UINT128_DEF "USEuint128_t")
Check_Int128("unsigned __int128_t" UINT128_DEF "USEunsigned__int128_t")
Check_Int128("unsigned int128_t"   UINT128_DEF "USEunsignedint128_t")
Check_Int128("unsigned __int128"   UINT128_DEF "USEunsignedint128")
Check_Int128("unsigned int128"     UINT128_DEF "USEunsignedint128")

if(INT128_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${UINT128_DEF}")
else()
  message("Compiler/platform support for 128 bit integers not found, falling back to aligned 64bit ints.")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

########################################
# Restrict stuff

include(CheckRestrict)
Check_Restrict("restrict"      RESTRICT_DEF "RESTRICT_USE_restrict")
Check_Restrict("__restrict__"  RESTRICT_DEF "RESTRICT_USE___restrict__")
Check_Restrict("__restrict"  RESTRICT_DEF "RESTRICT_USE___restrict")
Check_Restrict("_restrict"  RESTRICT_DEF "RESTRICT_USE__restrict")

if(RESTRICT_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${RESTRICT_DEF}")
  message("Compiler support for restrict keyword enabled.")
else()
  message("Compiler support for restrict keyword disabled.")
endif()

if(BUILD_IBD)
endif()

########################################
# Existance of __builtin_ctzl
set(CMAKE_REQUIRED_FLAGS "${CMAKE_C_FLAGS}")
CHECK_C_SOURCE_COMPILES("int main(int argc, char** argv){return __builtin_ctz(1);}" HAVE__builtin_ctz)

if(HAVE__builtin_ctz)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE__builtin_ctz")
  message("Builtin function __builtin_ctz found.")
else()
  message("Builtin function __builtin_ctz not found; using fallback.")
endif()

CHECK_C_SOURCE_COMPILES("int main(int argc, char** argv){return __builtin_ctzl(1);}" HAVE__builtin_ctzl)

if(HAVE__builtin_ctzl)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE__builtin_ctzl")
  message("Builtin function __builtin_ctzl found.")
else()
  message("Builtin function __builtin_ctzl not found; using fallback.")
endif()

CHECK_C_SOURCE_COMPILES("int main(int argc, char** argv){return __builtin_ctzll(1);}" HAVE__builtin_ctzll)
if(HAVE__builtin_ctzl)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE__builtin_ctzll")
  message("Builtin function __builtin_ctzll found.")
else()
  message("Builtin function __builtin_ctzll not found; using fallback.")
endif()

CHECK_C_SOURCE_COMPILES("int main(int argc, char** argv){return __builtin_clzl(1);}" HAVE__builtin_clzl)

if(HAVE__builtin_clzl)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE__builtin_clzl")
  message("Builtin function __builtin_clzl found.")
else()
  message("Builtin function __builtin_clzl not found; using fallback.")
endif()

################################################################################

if(BUILD_IBD)
  if(BUILD_C_IBD_COMPARE)

    add_executable(ibd_compare src/ibd_compare_c.c)
    target_link_libraries(ibd_compare m)

  else()
    find_package(PythonLibs)
    if(NOT PYTHONLIBS_FOUND)
      error("Python development headers not found; necessary for compiling ibd graph connection program.")
    else()
      if(NOT PYTHON_INCLUDE_DIRS)
	set(PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_PATH}")
      endif()

      # # need to use static linkage if possible
      # get_filename_component(_lib "${PYTHON_LIBRARIES}" NAME)
      # string(REPLACE "lib" "" _lib "${_lib}")

      # a HACK
      string(REPLACE ".so" ".a" PYTHON_STATIC_LIBRARY "${PYTHON_LIBRARIES}")

      if(PYTHON_LINK_STATIC) 
	if(EXISTS "${PYTHON_STATIC_LIBRARY}")
	  message("Found static python library, linking against that.")
	  SET(PYTHON_LIBRARY, PYTHON_STATIC_LIBRARY)
	else()
	  message("Did not find static python library, linking against dynamic.  Executable is less portable.")
	  SET(PYTHON_LIBRARY, PYTHON_LIBRARIES)
	endif()
      endif()
      # get_filename_component(PYTHON_LIBRARY_PATH "${PYTHON_LIBRARIES}" PATH)

      message("Python headers found in ${PYTHON_INCLUDE_DIRS} ")
    endif()

    include_directories(${PYTHON_INCLUDE_DIRS})
    add_executable(ibd_compare src/ibd_compare.c)
    target_link_libraries(ibd_compare "${PYTHON_LIBRARY}" m)
    install(TARGETS ibd_compare DESTINATION bin)
  endif()
endif()

if(BUILD_EXAMPLES)
    include_directories(src/)
    add_executable(population_example examples/populations.c)
    target_link_libraries(ibd_compare m)
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
