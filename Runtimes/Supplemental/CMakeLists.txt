cmake_minimum_required(VERSION 3.29)
# TODO before requiring CMake 4.1 or later
# and/or enforcing CMP0195, please check/update
# the implementation  of `emit_language_interface`
# in `EmitCodiraInterface.cmake`
# to ensure it keeps laying down nested languagemodule folders

project(CodiraRuntime LANGUAGES Codira C CXX)

include(ExternalProject)
include(GNUInstallDirs)

set(CodiraRuntime_LANGUAGEC_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../")

foreach(lib ${Codira_ENABLE_RUNTIMES})
  string(TOLOWER ${lib} name)
  set(CodiraRuntime_ENABLE_${name} YES)
endforeach()

if(CodiraCore_DIR)
  set(CodiraCore_DIR_FLAG "-DCodiraCore_DIR=${CodiraCore_DIR}")
endif()

if(CMAKE_MAKE_PROGRAM)
  set(MAKE_PROGRAM_FLAG "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
endif()

set(COMMON_OPTIONS
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  -DCodira_SDKROOT=${Codira_SDKROOT}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
  -DCMAKE_INSTALL_NAME_DIR=${CMAKE_INSTALL_NAME_DIR}
  -DCMAKE_BUILD_WITH_INSTALL_NAME_DIR=${CMAKE_BUILD_WITH_INSTALL_NAME_DIR}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_COLOR_DIAGNOSTICS=${CMAKE_COLOR_DIAGNOSTICS}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_Codira_COMPILER=${CMAKE_Codira_COMPILER}
  -DCMAKE_C_COMPILER_TARGET=${CMAKE_C_COMPILER_TARGET}
  -DCMAKE_CXX_COMPILER_TARGET=${CMAKE_CXX_COMPILER_TARGET}
  -DCMAKE_Codira_COMPILER_TARGET=${CMAKE_Codira_COMPILER_TARGET}
  -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=${CMAKE_FIND_PACKAGE_PREFER_CONFIG}
  ${CodiraCore_DIR_FLAG}
  ${MAKE_PROGRAM_FLAG})

# StringProcessing
if(CodiraRuntime_ENABLE_stringprocessing)
  ExternalProject_Add(StringProcessing
    PREFIX "StringProcessing"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/StringProcessing"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    INSTALL_COMMAND ""
    # To ensure incremental builds work as expected
    BUILD_ALWAYS 1
    CMAKE_ARGS
      ${COMMON_OPTIONS})
endif()

# Synchronization
if(CodiraRuntime_ENABLE_synchronization)
  ExternalProject_Add(Synchronization
    PREFIX "Synchronization"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Synchronization"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    INSTALL_COMMAND ""
    # To ensure incremental builds work as expected
    BUILD_ALWAYS 1
    CMAKE_ARGS
      ${COMMON_OPTIONS})
endif()
