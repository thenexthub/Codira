
cmake_minimum_required(VERSION 3.26...3.29)
# TODO before requiring CMake 4.1 or later
# and/or enforcing CMP0195, please check/update
# the implementation  of `emit_language_interface`
# in `EmitCodiraInterface.cmake`
# to ensure it keeps laying down nested languagemodule folders

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

if(POLICY CMP0157 AND CMAKE_Codira_COMPILER_USE_OLD_DRIVER)
  cmake_policy(SET CMP0157 OLD)
endif()

# NOTE: always use the 3-component style as the expansion as
# `${PROJECT_VERSION}` will not extend this to the complete form and this can
# change the behaviour for comparison with non-SemVer compliant parsing. If
# possible, use the 4-version component as that is used to differentiate the
# builds of the runtime for Windows.
if($ENV{BUILD_NUMBER})
  # NOTE: SxS modules have a limit on each component being [0-65535].
  # https://learn.microsoft.com/en-us/windows/win32/sbscs/assembly-versions
  math(EXPR BUILD_NUMBER "$ENV{BUILD_NUMBER} % 65535")
  set(BUILD_NUMBER ".${BUILD_NUMBER}")
endif()
project(CodiraOverlay
  LANGUAGES C CXX Codira
  VERSION 6.1.0${BUILD_NUMBER})

find_package(CodiraCore)

# FIXME: We should not need to refer back into the compiler sources. This is
# needed by gyb and AvailabilityMacros
set(CodiraOverlay_LANGUAGEC_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/../../"
  CACHE FILEPATH "Path to the root source directory of the Codira compiler")

include(GNUInstallDirs)

include(gyb)
include(AvailabilityMacros)
include(DefaultSettings)
include(EmitCodiraInterface)
include(InstallCodiraInterface)
include(PlatformInfo)
include(ResourceEmbedding)

defaulted_option(CodiraOverlay_ENABLE_REFLECTION "Enable runtime support for mirrors and reflection support")

option(CodiraOverlay_INSTALL_NESTED_SUBDIR "Install libraries under a platform and architecture subdirectory" ON)
set(CodiraOverlay_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${CodiraOverlay_INSTALL_NESTED_SUBDIR}>:/${CodiraOverlay_PLATFORM_SUBDIR}/${CodiraOverlay_ARCH_SUBDIR}>")
set(CodiraOverlay_INSTALL_LANGUAGEMODULEDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${CodiraOverlay_INSTALL_NESTED_SUBDIR}>:/${CodiraOverlay_PLATFORM_SUBDIR}>")

add_compile_options(
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-concurrency-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-string-processing-module-import>")

add_subdirectory(clang)
if(ANDROID)
  add_subdirectory(Android)
endif()
if(WIN32)
  add_subdirectory(Windows)
endif()
