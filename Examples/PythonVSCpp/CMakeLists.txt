cmake_minimum_required(VERSION 3.18)
project(cplex_examples CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release with single-configuration generators (Makefile, Ninja)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# CPLEX lookup
#
# Search order:
#   1. -DCPLEX_DIR=/path/to/CPLEX_Studio2211   (command line)
#   2. CPLEX_DIR environment variable
#   3. CPLEX_STUDIO_DIR<version> environment variables (set by the Windows installer)
#   4. default install locations (the most recent version is used)
# ---------------------------------------------------------------------------
set(CPLEX_DIR "" CACHE PATH "CPLEX Studio installation directory (contains the 'cplex' folder)")

set(_cplex_candidates "")
if(CPLEX_DIR)
  list(APPEND _cplex_candidates "${CPLEX_DIR}")
endif()
if(DEFINED ENV{CPLEX_DIR})
  file(TO_CMAKE_PATH "$ENV{CPLEX_DIR}" _dir)
  list(APPEND _cplex_candidates "${_dir}")
endif()

# CPLEX_STUDIO_DIR<version>: most recent first
execute_process(COMMAND ${CMAKE_COMMAND} -E environment OUTPUT_VARIABLE _env)
string(REGEX MATCHALL "CPLEX_STUDIO_DIR[0-9]+=[^\r\n]*" _env_dirs "${_env}")
list(SORT _env_dirs COMPARE NATURAL ORDER DESCENDING)
foreach(_entry ${_env_dirs})
  string(REGEX REPLACE "^CPLEX_STUDIO_DIR[0-9]+=" "" _dir "${_entry}")
  file(TO_CMAKE_PATH "${_dir}" _dir)
  list(APPEND _cplex_candidates "${_dir}")
endforeach()

# Default IBM installer locations
if(WIN32)
  set(_cplex_default_globs
    "C:/Program Files/IBM/ILOG/CPLEX_Studio*"
    "C:/Program Files (x86)/IBM/ILOG/CPLEX_Studio*")
elseif(APPLE)
  set(_cplex_default_globs
    "/Applications/CPLEX_Studio*"
    "/Applications/IBM/ILOG/CPLEX_Studio*"
    "$ENV{HOME}/Applications/CPLEX_Studio*"
    "$ENV{HOME}/Applications/IBM/ILOG/CPLEX_Studio*")
else()
  set(_cplex_default_globs
    "/opt/ibm/ILOG/CPLEX_Studio*"
    "/opt/IBM/ILOG/CPLEX_Studio*"
    "/opt/CPLEX_Studio*"
    "$ENV{HOME}/ibm/ILOG/CPLEX_Studio*"
    "$ENV{HOME}/CPLEX_Studio*")
endif()
foreach(_glob ${_cplex_default_globs})
  file(GLOB _dirs LIST_DIRECTORIES true "${_glob}")
  list(SORT _dirs COMPARE NATURAL ORDER DESCENDING)
  list(APPEND _cplex_candidates ${_dirs})
endforeach()

set(_cplex_root "")
foreach(_dir ${_cplex_candidates})
  if(EXISTS "${_dir}/cplex/include/ilcplex/cplex.h")
    set(_cplex_root "${_dir}")
    break()
  endif()
endforeach()
if(NOT _cplex_root)
  message(FATAL_ERROR
    "CPLEX not found. Paths tried:\n  ${_cplex_candidates}\n"
    "Specify the installation directory with  -DCPLEX_DIR=/path/to/CPLEX_StudioXXXX")
endif()
set(CPLEX_DIR "${_cplex_root}" CACHE PATH "CPLEX Studio installation directory" FORCE)

# Machine architecture, used to pick the right library
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _arch)
if(_arch MATCHES "^(x86_64|amd64)$")
  set(_arch "x86-64")
elseif(_arch MATCHES "^(arm64|aarch64)$")
  set(_arch "arm64")
endif()
set(_cplex_osx_arch "")
if(WIN32)
  # On Windows CPLEX only supports 64-bit MSVC; stat_mda = /MD runtime
  if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "On Windows CPLEX requires the 64-bit MSVC compiler (Visual Studio, x64).")
  endif()
  file(GLOB CPLEX_LIBRARY "${CPLEX_DIR}/cplex/lib/x64_windows_*/stat_mda/cplex[0-9]*.lib")
  file(GLOB CPLEX_DLL "${CPLEX_DIR}/cplex/bin/x64_win64/cplex[0-9]*.dll")
  list(SORT CPLEX_LIBRARY COMPARE NATURAL ORDER DESCENDING)
  list(GET CPLEX_LIBRARY 0 CPLEX_LIBRARY)
  # the library is built with /MD: use the same runtime in Debug too
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
else()
  # Linux: x86-64_linux, arm64_linux, ppc64le_linux, s390x_linux
  # macOS: x86-64_osx, arm64_osx
  file(GLOB _libs "${CPLEX_DIR}/cplex/lib/*/static_pic/libcplex.a")
  set(CPLEX_LIBRARY "")
  foreach(_lib ${_libs})
    if(_lib MATCHES "/${_arch}_(linux|osx)/")
      set(CPLEX_LIBRARY "${_lib}")
    endif()
  endforeach()
  if(NOT CPLEX_LIBRARY AND _libs)
    # e.g. Apple Silicon Mac with Intel-only CPLEX: build for x86_64 (runs under Rosetta)
    list(GET _libs 0 CPLEX_LIBRARY)
    if(APPLE AND CPLEX_LIBRARY MATCHES "/x86-64_osx/")
      set(_cplex_osx_arch "x86_64")
      message(WARNING "CPLEX available for Intel only: executables will be x86_64 (Rosetta).")
    else()
      message(WARNING "No CPLEX library for '${_arch}', using ${CPLEX_LIBRARY}")
    endif()
  endif()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

if(NOT CPLEX_LIBRARY OR NOT EXISTS "${CPLEX_LIBRARY}")
  message(FATAL_ERROR "CPLEX library not found in ${CPLEX_DIR}/cplex/lib")
endif()

# macOS: CPLEX requires some system frameworks (-framework ...). They are read from the
# example Makefile shipped with CPLEX (CLNFLAGS line), so they always match the
# installed version; if the Makefile is missing, CoreFoundation and IOKit are used.
set(CPLEX_FRAMEWORKS "")
if(APPLE)
  string(REGEX REPLACE "/lib/([^/]+)/static_pic/libcplex\\.a$" "/examples/\\1/static_pic/Makefile"
         _cplex_makefile "${CPLEX_LIBRARY}")
  if(EXISTS "${_cplex_makefile}")
    file(STRINGS "${_cplex_makefile}" _clnflags REGEX "^CLNFLAGS")
    string(REGEX MATCHALL "-framework[ \t]+[A-Za-z0-9_]+" _fw_flags "${_clnflags}")
    foreach(_fw ${_fw_flags})
      string(REGEX REPLACE "-framework[ \t]+" "" _fw "${_fw}")
      list(APPEND CPLEX_FRAMEWORKS "-framework ${_fw}")
    endforeach()
    list(REMOVE_DUPLICATES CPLEX_FRAMEWORKS)
  else()
    set(CPLEX_FRAMEWORKS "-framework CoreFoundation" "-framework IOKit")
  endif()
  message(STATUS "CPLEX frameworks: ${CPLEX_FRAMEWORKS}")
endif()

message(STATUS "CPLEX: ${CPLEX_DIR}")
message(STATUS "CPLEX library: ${CPLEX_LIBRARY}")

# ---------------------------------------------------------------------------
# File to build: NAME.cpp -> executable NAME
# Change the value below, or pass it on the command line:  -DNAME=other_file
# ---------------------------------------------------------------------------
set(NAME_DEFAULT "transport_timing")

if(NOT NAME)
  set(NAME "${NAME_DEFAULT}")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp")
  message(FATAL_ERROR "File '${NAME}.cpp' not found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
message(STATUS "Compiling: ${NAME}.cpp")

add_executable(${NAME} "${NAME}.cpp")
target_include_directories(${NAME} PRIVATE "${CPLEX_DIR}/cplex/include")
target_link_libraries(${NAME} PRIVATE "${CPLEX_LIBRARY}")
if(WIN32)
  target_compile_definitions(${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
  # copy cplexXXXX.dll next to the executable, so it runs even if the DLL is not in PATH
  if(CPLEX_DLL)
    add_custom_command(TARGET ${NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CPLEX_DLL} $<TARGET_FILE_DIR:${NAME}>)
  endif()
else()
  target_link_libraries(${NAME} PRIVATE Threads::Threads m ${CMAKE_DL_LIBS} ${CPLEX_FRAMEWORKS})
  if(_cplex_osx_arch)
    set_target_properties(${NAME} PROPERTIES OSX_ARCHITECTURES ${_cplex_osx_arch})
  endif()
endif()