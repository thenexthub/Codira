cmake_minimum_required(VERSION 3.28)

project(language-inspect
  LANGUAGES C CXX Codira)

include(FetchContent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

FetchContent_Declare(ArgumentParser
  GIT_REPOSITORY  https://github.com/apple/language-argument-parser
  GIT_TAG         1.5.0
  GIT_SHALLOW     TRUE
  EXCLUDE_FROM_ALL
  FIND_PACKAGE_ARGS CONFIG)

block(SCOPE_FOR VARIABLES)
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build as a shared library")
  set(BUILD_TESTING OFF CACHE INTERNAL "Skip building ArgumentParser tests")
  set(BUILD_EXAMPLES OFF CACHE INTERNAL "Skip building ArgumentParser examples")
  FetchContent_MakeAvailable(ArgumentParser)
endblock()

add_library(SymbolicationShims INTERFACE)
target_include_directories(SymbolicationShims INTERFACE
  Sources/SymbolicationShims)

if(WIN32)
  add_library(CodiraInspectClientInterface INTERFACE)
  target_include_directories(CodiraInspectClientInterface INTERFACE
    Sources/CodiraInspectClientInterface)

  add_library(CodiraInspectClient SHARED
    Sources/CodiraInspectClient/CodiraInspectClient.cpp)
  target_link_libraries(CodiraInspectClient PRIVATE
    CodiraInspectClientInterface)
endif()

if (ANDROID)
  add_library(AndroidCLib STATIC
    Sources/AndroidCLib/heap.c)
  target_include_directories(AndroidCLib PUBLIC
    Sources/AndroidCLib/include)
  set_property(TARGET AndroidCLib PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(ANDROID OR LINUX)
  add_library(LinuxSystemHeaders INTERFACE)
  target_include_directories(LinuxSystemHeaders INTERFACE
    Sources/CodiraInspectLinux/SystemHeaders)

  add_library(CodiraInspectLinux STATIC
    Sources/CodiraInspectLinux/ElfFile.code
    Sources/CodiraInspectLinux/LinkMap.code
    Sources/CodiraInspectLinux/MemoryMap.code
    Sources/CodiraInspectLinux/Process.code
    Sources/CodiraInspectLinux/ProcFS.code
    Sources/CodiraInspectLinux/PTrace.code
    Sources/CodiraInspectLinux/RegisterSet.code
    Sources/CodiraInspectLinux/SymbolCache.code)
  target_compile_options(CodiraInspectLinux PRIVATE
    -Xcc -D_GNU_SOURCE)
  target_link_libraries(CodiraInspectLinux PUBLIC
    LinuxSystemHeaders)
endif()

add_executable(language-inspect
  Sources/language-inspect/Operations/DumpArray.code
  Sources/language-inspect/Operations/DumpCacheNodes.code
  Sources/language-inspect/Operations/DumpConcurrency.code
  Sources/language-inspect/Operations/DumpConformanceCache.code
  Sources/language-inspect/Operations/DumpGenericMetadata.code
  Sources/language-inspect/Operations/DumpRawMetadata.code
  Sources/language-inspect/AndroidRemoteProcess.code
  Sources/language-inspect/Backtrace.code
  Sources/language-inspect/DarwinRemoteProcess.code
  Sources/language-inspect/LinuxRemoteProcess.code
  Sources/language-inspect/main.code
  Sources/language-inspect/Process.code
  Sources/language-inspect/RemoteMirror+Extensions.code
  Sources/language-inspect/RemoteProcess.code
  Sources/language-inspect/String+Extensions.code
  Sources/language-inspect/Symbolication+Extensions.code
  Sources/language-inspect/WindowsRemoteProcess.code
  Sources/language-inspect/WinSDK+Extentions.code)
target_compile_options(language-inspect PRIVATE
  -parse-as-library)
target_link_libraries(language-inspect PRIVATE
  ArgumentParser
  languageRemoteMirror)
if(WIN32)
  target_link_libraries(language-inspect PRIVATE
    CodiraInspectClientInterface)
endif()
if(ANDROID)
  target_link_libraries(language-inspect PRIVATE
    AndroidCLib)
endif()
if(ANDROID OR LINUX)
  target_link_libraries(language-inspect PRIVATE
    CodiraInspectLinux)
endif()

install(TARGETS language-inspect
  DESTINATION bin)
if(WIN32)
  install(TARGETS CodiraInspectClient
    RUNTIME DESTINATION bin)
endif()
