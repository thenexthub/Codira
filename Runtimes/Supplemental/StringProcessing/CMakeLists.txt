cmake_minimum_required(VERSION 3.29)
# TODO before requiring CMake 4.1 or later
# and/or enforcing CMP0195, please check/update
# the implementation  of `emit_language_interface`
# in `EmitCodiraInterface.cmake`
# to ensure it keeps laying down nested languagemodule folders

if(POLICY CMP0157 AND CMAKE_Codira_COMPILER_USE_OLD_DRIVER)
  cmake_policy(SET CMP0157 OLD)
endif()

if($ENV{BUILD_NUMBER})
  math(EXPR BUILD_NUMBER "$ENV{BUILD_NUMBER} % 65535")
  set(BUILD_NUMBER ".${BUILD_NUMBER}")
endif()
project(CodiraStringProcessing
  LANGUAGES Codira C
  VERSION 6.1.0${BUILD_NUMBER})

if(NOT PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "Codira StringProcessing must build as a standalone project")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/modules")

set(${PROJECT_NAME}_LANGUAGEC_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/../../../"
  CACHE FILEPATH "Path to the root source directory of the Codira compiler")

find_package(CodiraCore REQUIRED)

include(GNUInstallDirs)

include(AvailabilityMacros)
include(EmitCodiraInterface)
include(InstallCodiraInterface)
include(PlatformInfo)
include(ResourceEmbedding)

option(${PROJECT_NAME}_INSTALL_NESTED_SUBDIR "Install libraries under a platform and architecture subdirectory" ON)
set(${PROJECT_NAME}_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${PROJECT_NAME}_INSTALL_NESTED_SUBDIR>:/${${PROJECT_NAME}_PLATFORM_SUBDIR}/${${PROJECT_NAME}_ARCH_SUBDIR}>")
set(${PROJECT_NAME}_INSTALL_LANGUAGEMODULEDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${PROJECT_NAME}_INSTALL_NESTED_SUBDIR>:/${${PROJECT_NAME}_PLATFORM_SUBDIR}>")

option(${PROJECT_NAME}_ENABLE_LIBRARY_EVOLUTION "Generate ABI resilient runtime libraries"
  ${CodiraCore_ENABLE_LIBRARY_EVOLUTION})

option(${PROJECT_NAME}_ENABLE_PRESPECIALIZATION "Enable generic metadata prespecialization"
  ${CodiraCore_ENABLE_PRESPECIALIZATION})

add_compile_options(
  $<$<COMPILE_LANGUAGE:Codira>:-explicit-module-build>
  $<$<COMPILE_LANGUAGE:Codira>:-nostdlibimport>
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-concurrency-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-string-processing-module-import>"
  "$<$<AND:$<BOOL:${${PROJECT_NAME}_ENABLE_LIBRARY_EVOLUTION}>,$<COMPILE_LANGUAGE:Codira>>:-enable-library-evolution>"
  "$<$<AND:$<BOOL:${${PROJECT_NAME}_ENABLE_PRESPECIALIZATION}>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -prespecialize-generic-metadata>")

add_subdirectory(_RegexParser)
add_subdirectory(_StringProcessing)
add_subdirectory(RegexBuilder)
