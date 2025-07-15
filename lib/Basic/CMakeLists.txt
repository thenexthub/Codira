
# On non-Darwin require UUID.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(UUID_INCLUDE "")
  set(UUID_LIBRARIES "")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(UUID_INCLUDE "")
  set(UUID_LIBRARIES "rpcrt4.lib")
else()
  find_package(UUID REQUIRED)
  set(UUID_INCLUDE "${UUID_INCLUDE_DIRS}")
endif()

function(generate_revision_inc revision_inc_var name dir)
  if(LANGUAGE_APPEND_VC_REV)
    # generate_vcs_version_script generates header with only `undef`s
    # inside when source directory doesn't exist.
    find_first_existing_vc_file("${dir}" ${name}_vc)
    set(dir_when_append_enabled ${dir})
  endif()

  # Create custom target to generate the VC revision include.
  set(version_inc "${CMAKE_CURRENT_BINARY_DIR}/${name}Revision.inc")

  set(generate_vcs_version_script "${TOOLCHAIN_MAIN_SRC_DIR}/cmake/modules/GenerateVersionFromVCS.cmake")

  add_custom_command(OUTPUT "${version_inc}"
    DEPENDS "${${name}_vc}" "${generate_vcs_version_script}"
    COMMAND ${CMAKE_COMMAND} "-DNAMES=$<UPPER_CASE:${name}>"
                             "-D$<UPPER_CASE:${name}>_SOURCE_DIR=${dir_when_append_enabled}"
                             "-DHEADER_FILE=${version_inc}"
                             -P "${generate_vcs_version_script}")

  # Mark the generated header as being generated.
  set_source_files_properties("${version_inc}"
    PROPERTIES GENERATED TRUE
               HEADER_FILE_ONLY TRUE)
  set(${revision_inc_var} ${version_inc} PARENT_SCOPE)
endfunction()

generate_revision_inc(toolchain_revision_inc LLVM "${TOOLCHAIN_MAIN_SRC_DIR}")
generate_revision_inc(language_revision_inc Codira "${LANGUAGE_SOURCE_DIR}")

add_language_host_library(languageBasic STATIC
  Assertions.cpp
  BasicBridging.cpp
  BasicSourceInfo.cpp
  Cache.cpp
  CASOptions.cpp
  ClusteredBitVector.cpp
  DiverseStack.cpp
  Edit.cpp
  EditorPlaceholder.cpp
  ExponentialGrowthAppendingBinaryByteStream.cpp
  Feature.cpp
  FileSystem.cpp
  FileTypes.cpp
  Fingerprint.cpp
  ParseableOutput.cpp
  JSONSerialization.cpp
  LangOptions.cpp
  LoadDynamicLibrary.cpp
  Located.cpp
  Mangler.cpp
  OutputFileMap.cpp
  Platform.cpp
  PrefixMap.cpp
  PrettyStackTrace.cpp
  PrimitiveParsing.cpp
  Program.cpp
  QuotedString.cpp
  Sandbox.cpp
  SmallBitVector.cpp
  SourceLoc.cpp
  StableHasher.cpp
  Statistic.cpp
  StringExtras.cpp
  SupportedFeatures.cpp
  TargetInfo.cpp
  TaskQueue.cpp
  ThreadSafeRefCounted.cpp
  Unicode.cpp
  UUID.cpp
  Version.cpp
  BlockList.cpp

  ${toolchain_revision_inc}
  ${clang_revision_inc}
  ${language_revision_inc}

  # Platform-specific TaskQueue implementations
  Unix/TaskQueue.inc

  # Platform-agnostic fallback TaskQueue implementation
  Default/TaskQueue.inc

  TOOLCHAIN_LINK_COMPONENTS support targetparser)
_language_gyb_target_sources(languageBasic PRIVATE
    UnicodeExtendedGraphemeClusters.cpp.gyb)

target_include_directories(languageBasic PRIVATE
  clangBasic
  ${UUID_INCLUDE})

target_link_libraries(languageBasic PUBLIC
  languageDemangling)
target_link_libraries(languageBasic PRIVATE
  ${UUID_LIBRARIES})

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CodiraVersion.cmake)
message(STATUS "Codira version: ${LANGUAGE_VERSION}")
message(STATUS "Codira vendor: ${LANGUAGE_VENDOR}")

string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)?" "\\1" LANGUAGE_VERSION_MAJOR
  ${LANGUAGE_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)?" "\\1" LANGUAGE_VERSION_MINOR
  ${LANGUAGE_VERSION})

set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
  " -DLANGUAGE_VERSION=${LANGUAGE_VERSION} -DLANGUAGE_VERSION_MAJOR=${LANGUAGE_VERSION_MAJOR} -DLANGUAGE_VERSION_MINOR=${LANGUAGE_VERSION_MINOR}")

if ("${LANGUAGE_VERSION}" MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" LANGUAGE_VERSION_PATCHLEVEL
    ${LANGUAGE_VERSION})
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DLANGUAGE_HAS_VERSION_PATCHLEVEL=1 -DLANGUAGE_VERSION_PATCHLEVEL=${LANGUAGE_VERSION_PATCHLEVEL}")
else()
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DLANGUAGE_HAS_VERSION_PATCHLEVEL=0")
endif()

if(NOT "${LANGUAGE_VENDOR}" STREQUAL "")
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
      " -DLANGUAGE_VENDOR=\"\\\"${LANGUAGE_VENDOR}\\\"\"")
endif()

message(STATUS "Codira compiler version: ${LANGUAGE_COMPILER_VERSION}")
message(STATUS "Codira toolchain version: ${LANGUAGE_TOOLCHAIN_VERSION}")
message(STATUS "Embedded clang compiler version: ${CLANG_COMPILER_VERSION}")

if(LANGUAGE_COMPILER_VERSION)
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DLANGUAGE_COMPILER_VERSION=\"\\\"${LANGUAGE_COMPILER_VERSION}\\\"\"")
endif()

if(CLANG_COMPILER_VERSION)
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DCLANG_COMPILER_VERSION=\"\\\"${CLANG_COMPILER_VERSION}\\\"\"")
endif()

if(LANGUAGE_TOOLCHAIN_VERSION)
  set_property(SOURCE Version.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DLANGUAGE_TOOLCHAIN_VERSION=\"\\\"${LANGUAGE_TOOLCHAIN_VERSION}\\\"\"")
endif()
