# Notes:
#
# The Demangling library uses `#if LANGUAGE_HAVE_CRASHREPORTERCLIENT` while the
# runtime library uses `#ifdef LANGUAGE_HAVE_CRASHREPORTERCLIENT` to toggle that
# functionality. When building the demangling library, the macro should be set
# to 0 or 1 to indicate the presence of the crashreporter.
# When building the runtime library, the existence of the macro indicates the
# presence of the crashreporter.
#
# Runtime library pulls sources and headers from compiler sources (ThreadSanitizer)
# Demangling library pulls sources and headers from compiler sources (all)
#
#
# gyb pulls sources from compiler sources
#
# Stubs:
#   Pulls in headers from compiler
#     - include/language/Basic
#     - include/language/Runtime
#     - include/language/Threading

# TODO:
# Platform support:
#   - Work on/Verify cross-compiling
#   - Work on/Verify Windows and Linux native builds
# Embedded
#   -- -Xfrontend -emit-empty-object-file
# Catalyst Support
#   -- Will need shadow invocations to generate languagemodules for Codira parts
# Install *.abi.json, languagedoc, and languagesourceinfo

cmake_minimum_required(VERSION 3.29)
# TODO before requiring CMake 4.1 or later
# and/or enforcing CMP0195, please check/update
# the implementation  of `emit_language_interface`
# in `EmitCodiraInterface.cmake`
# to ensure it keeps laying down nested languagemodule folders

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

set(CMAKE_POSITION_INDEPENDENT_CODE YES)

set(CodiraCore_CMAKE_MODULES_DIR "${CMAKE_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH ${CodiraCore_CMAKE_MODULES_DIR})

include(CMakeWorkarounds)
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
project(CodiraCore
  LANGUAGES C CXX Codira
  VERSION 6.1.0${BUILD_NUMBER})

# The Codira standard library is not intended for use as a sub-library as part of
# another project. It is tightly coupled with the compiler version.
if(NOT PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "Codira Core must build as a standalone project")
endif()

# FIXME: We should not need to refer back into the compiler sources. This is
#        needed by gyb, Demangling, Threading, the runtime (ThreadSanitizer.cpp)
set(CodiraCore_LANGUAGEC_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/../../"
  CACHE FILEPATH "Path to the root source directory of the Codira compiler")

# Hook point for vendor-specific extensions to the build system
# Allowed extension points:
#   - DefaultSettings.cmake
#   - Settings.cmake
set(CodiraCore_VENDOR_MODULE_DIR "${CodiraCore_CMAKE_MODULES_DIR}/vendor"
  CACHE FILEPATH "Location for private build system extension")

include(GNUInstallDirs)
include(CheckSymbolExists)
include(CheckIncludeFileCXX)
include(CompilerSettings)
include(DefaultSettings)
include(EmitCodiraInterface)
include(InstallCodiraInterface)
include(PlatformInfo)
include(gyb)
include(ResourceEmbedding)
include(CatalystSupport)
include(AvailabilityMacros)

check_symbol_exists("asl_log" "asl.h" CodiraCore_HAS_ASL)
check_symbol_exists("dladdr" "dlfcn.h" CodiraCore_HAS_DLADDR)
check_symbol_exists("dlsym" "dlfcn.h" CodiraCore_HAS_DLSYM)
check_symbol_exists("os_trace" "os/trace.h" CodiraCore_HAS_OS_TRACE)

check_include_file_cxx("clocale" CodiraCore_HAS_LOCALE)

include("${CodiraCore_VENDOR_MODULE_DIR}/Settings.cmake" OPTIONAL)

defaulted_option(CodiraCore_ENABLE_LIBRARY_EVOLUTION "Generate ABI resilient runtime libraries")

defaulted_option(CodiraCore_ENABLE_CRASH_REPORTER_CLIENT "Enable Apple CrashReporter integration")
defaulted_option(CodiraCore_ENABLE_OBJC_INTEROP "Enable runtime ObjC interop")
defaulted_option(CodiraCore_ENABLE_TYPE_PRINTING "Enable printing type names")
defaulted_option(CodiraCore_ENABLE_VECTOR_TYPES "Enable vector support")
defaulted_option(CodiraCore_ENABLE_REFLECTION "Enable runtime support for mirrors and reflection support")
defaulted_option(CodiraCore_ENABLE_RUNTIME_FUNCTION_COUNTERS "Enable runtime function counter support")
defaulted_option(CodiraCore_ENABLE_STDIN "Enable functions that use stdin support")
defaulted_option(CodiraCore_ENABLE_ENVIRONMENT "Enable environment variable support")
defaulted_option(CodiraCore_ENABLE_OVERRIDABLE_RETAIN_RELEASE "Enable override hooks for retain/release")
defaulted_option(CodiraCore_ENABLE_RUNTIME_OS_VERSIONING "Enable runtime OS version detection")
defaulted_option(CodiraCore_ENABLE_STATIC_PRINT "Disable full print")
defaulted_option(CodiraCore_ENABLE_COMPACT_ABSOLUTE_FUNCTION_POINTERS "Resolve absolute function pointer as identity")
defaulted_option(CodiraCore_ENABLE_BACKDEPLOYMENT_SUPPORT "Add symbols for runtime backdeployment")
defaulted_option(CodiraCore_ENABLE_STDLIB_TRACING "Enable tracing in the runtime. Assumes the presence of os_log(3) and the os_signpost(3) API.")
defaulted_option(CodiraCore_ENABLE_CONCURRENCY "Enable Concurrency runtime support")
defaulted_set(CodiraCore_CONCURRENCY_GLOBAL_EXECUTOR STRING "Default Concurrency global executor implementation")
option(CodiraCore_ENABLE_COMMANDLINE_SUPPORT "Enable command line argument support" ON)
option(CodiraCore_ENABLE_UNICODE_DATA "Include unicode data in Codira runtimes" ON)
option(CodiraCore_ENABLE_SHORT_MANGLING_LOOKUPS "Build with fast-path context descriptor lookups based on well-known short manglings." ON)
option(CodiraCore_ENABLE_FILESYSTEM_SUPPORT "Build for systems that have a filesystem" ON)
option(CodiraCore_ENABLE_OS_TRACE_LAZY_INIT "Use os_trace call to check if lazy init has been completed before making os_signpost calls." ${CodiraCore_HAS_OS_TRACE})
option(CodiraCore_HAS_DARWIN_LIBMALLOC "Use Darwin malloc features" ${APPLE})

defaulted_option(CodiraCore_ENABLE_BACKTRACING "Enable backtracing runtime support")
defaulted_set(CodiraCore_BACKTRACER_PATH STRING "Set a fixed path to the Codira backtracer")
defaulted_option(CodiraCore_ENABLE_FATALERROR_BACKTRACE "Build stdlib fatalError with backtrace output")
defaulted_option(CodiraCore_ENABLE_PRESPECIALIZATION "Enable generic metadata prespecialization")
defaulted_option(CodiraCore_ENABLE_STRICT_AVAILABILITY "Enable strict availability; this will cause things to break at desk or in CI if the host OS is a lower version than some `@availability` annotations in the runtime code")

option(CodiraCore_ENABLE_CLOBBER_FREED_OBJECTS "" OFF)
option(CodiraCore_ENABLE_RUNTIME_LEAK_CHECKER "" OFF)
option(CodiraCore_ENABLE_INTERNAL_CHECKS "" OFF)
set(CodiraCore_OPTIMIZATION_REMARKS OFF CACHE STRING "Set opt-remark format")

defaulted_set(CodiraCore_OBJECT_FORMAT STRING "Object format: ELF COFF")
defaulted_set(CodiraCore_THREADING_PACKAGE STRING "Threading Package: C11 Linux PThreads Win32")

option(CodiraCore_INSTALL_NESTED_SUBDIR "Install libraries under a platform and architecture subdirectory" ON)
set(CodiraCore_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${CodiraCore_INSTALL_NESTED_SUBDIR}>:/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}>")
set(CodiraCore_INSTALL_LANGUAGEMODULEDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${CodiraCore_INSTALL_NESTED_SUBDIR}>:/${CodiraCore_PLATFORM_SUBDIR}>")

option(CodiraCore_ENABLE_ONONESUPPORT "Enable Onone Support" YES)

# TODO: Most of these should be attached to specific targets rather than applied
#       globally. They can be applied as part of the library interface if they
#       affect the behavior of libraries that build against a given target.
add_compile_definitions(
  $<$<BOOL:${CodiraCore_ENABLE_OBJC_INTEROP}>:-DLANGUAGE_OBJC_INTEROP>
  $<$<BOOL:${CodiraCore_ENABLE_LIBRARY_EVOLUTION}>:-DLANGUAGE_LIBRARY_EVOLUTION>
  $<$<BOOL:${CodiraCore_ENABLE_CRASH_REPORTER_CLIENT}>:-DLANGUAGE_HAVE_CRASHREPORTERCLIENT>
  $<$<BOOL:${CodiraCore_ENABLE_REFLECTION}>:-DLANGUAGE_ENABLE_REFLECTION>
  $<$<BOOL:${CodiraCore_ENABLE_RUNTIME_FUNCTION_COUNTERS}>:-DLANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS>
  $<$<BOOL:${CodiraCore_ENABLE_STDIN}>:-DLANGUAGE_STDLIB_HAS_STDIN> # core + stubs
  $<$<BOOL:${CodiraCore_ENABLE_RUNTIME_OS_VERSIONING}>:-DLANGUAGE_RUNTIME_OS_VERSIONING> # core + stubs
  $<$<BOOL:${CodiraCore_ENABLE_INTERNAL_CHECKS}>:-DINTERNAL_CHECKS_ENABLED> # core
  $<$<BOOL:${CodiraCore_ENABLE_STATIC_PRINT}>:-DLANGUAGE_STDLIB_STATIC_PRINT> # core + concurrency
  $<$<BOOL:${CodiraCore_ENABLE_UNICODE_DATA}>:-DLANGUAGE_STDLIB_ENABLE_UNICODE_DATA> # Stubs
  $<$<BOOL:${CodiraCore_ENABLE_ENVIRONMENT}>:-DLANGUAGE_STDLIB_HAS_ENVIRON> # Concurrency, runtime, shims, platform overlay
  $<$<BOOL:${CodiraCore_ENABLE_BACKDEPLOYMENT_SUPPORT}>:-DLANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT> # Concurrency, Compatibility override, magic symbols
  $<$<BOOL:${CodiraCore_HAS_DARWIN_LIBMALLOC}>:-DLANGUAGE_STDLIB_HAS_DARWIN_LIBMALLOC> # Anything that includes include/language/Runtime/Config.h
  $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_THREADING_${CodiraCore_THREADING_PACKAGE}>
  $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER=$<BOOL:${CodiraCore_ENABLE_RUNTIME_LEAK_CHECKER}>>
  $<$<BOOL:${CodiraCore_ENABLE_CLOBBER_FREED_OBJECTS}>:-DLANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS>)

add_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
  $<$<COMPILE_LANGUAGE:CXX>:-funwind-tables>)

# LNK4049: symbol 'symbol' defined in 'filename.obj' is imported
# LNK4286: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj'
# LNK4217: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj' in function 'function'
#
# We cannot selectively filter the linker warnings as we do not use the MSVC
# frontned and `clang-cl` (and `clang`) currently do not support `/WX:nnnn`. As
# a compromise, treat all linker warnings as errors.
add_link_options($<$<PLATFORM_ID:Windows>:LINKER:/WX>)

add_compile_options(
  $<$<COMPILE_LANGUAGE:Codira>:-explicit-module-build>
  "$<$<COMPILE_LANGUAGE:Codira>:-nostdlibimport>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-library-level api>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-runtime-compatibility-version none>"
  "$<$<COMPILE_LANGUAGE:Codira>:-disable-autolinking-runtime-compatibility-dynamic-replacements>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-autolinking-runtime-compatibility-concurrency>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-strict-memory-safety>"
  "$<$<AND:$<BOOL:${CodiraCore_OPTIMIZATION_REMARKS}>,$<COMPILE_LANGUAGE:Codira>>:-save-optimization-record=${CodiraCore_OPTIMIZATION_REMARKS}>"
  "$<$<COMPILE_LANGUAGE:Codira>:-warn-implicit-overrides>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enable-lexical-lifetimes=false>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-concurrency-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-string-processing-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:-no-link-objc-runtime>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enforce-exclusivity=unchecked>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enable-ossa-modules>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -empty-abi-descriptor>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -target-min-inlining-version -Xfrontend min>"
  "$<$<AND:$<NOT:$<BOOL:${CodiraCore_ENABLE_OBJC_INTEROP}>>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -disable-objc-interop>"
  "$<$<AND:$<PLATFORM_ID:Windows>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xcc -Xclang -Xcc -fbuiltin-headers-in-system-modules>"
  "$<$<AND:$<BOOL:${CodiraCore_ENABLE_LIBRARY_EVOLUTION}>,$<COMPILE_LANGUAGE:Codira>>:-enable-library-evolution>"
  "$<$<AND:$<BOOL:${CodiraCore_ENABLE_PRESPECIALIZATION}>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -prespecialize-generic-metadata>")

include(ExperimentalFeatures)

include_directories(include)

add_subdirectory(LLVMSupport)
add_subdirectory(CodiraShims/language/shims)
add_subdirectory(Demangling)
add_subdirectory(Threading)
add_subdirectory(runtime)
add_subdirectory(stubs)
add_subdirectory(core)
if(CodiraCore_ENABLE_COMMANDLINE_SUPPORT)
  add_subdirectory(CommandLineSupport)
endif()
if(CodiraCore_ENABLE_ONONESUPPORT)
  add_subdirectory(CodiraOnoneSupport)
endif()
if(CodiraCore_ENABLE_CONCURRENCY)
  add_subdirectory(Concurrency)
endif()

# Inter-project install info
export(EXPORT CodiraCoreTargets
  FILE "cmake/CodiraCore/CodiraCoreTargets.cmake")
install(EXPORT CodiraCoreTargets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CodiraCore"
  FILE "CodiraCoreTargets.cmake"
  COMPONENT CodiraCore_cmake)
include(CMakePackageConfigHelpers)
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/interface/CodiraCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/CodiraCore/CodiraCoreConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CodiraCore")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/CodiraCore/CodiraCoreConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY ExactVersion)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/CodiraCore/CodiraCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/CodiraCore/CodiraCoreConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CodiraCore")
