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
project(CodiraSynchronization
  LANGUAGES Codira
  VERSION 6.1.0${BUILD_NUMBER})
# FIXME(compnerd) this is a workaround for `GNUInstallDirs` which cannot be used
# with a pure Codira project.
enable_language(C)

if(NOT PROJECT_IS_TOP_LEVEL)
  message(SEND_ERROR "Codira Synchronization must build as a standalone project")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/modules")

set(${PROJECT_NAME}_LANGUAGEC_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/../../../"
  CACHE FILEPATH "Path to the root source directory of the Codira compiler")

# Hook point for vendor-specific extensions to the build system
# Allowed extension points:
#   - DefaultSettings.cmake
#   - Settings.cmake
set(${PROJECT_NAME}_VENDOR_MODULE_DIR "${CMAKE_SOURCE_DIR}/../cmake/modules/vendor"
  CACHE FILEPATH "Location for private build system extension")

find_package(CodiraCore REQUIRED)

include(GNUInstallDirs)

include(AvailabilityMacros)
include(EmitCodiraInterface)
include(InstallCodiraInterface)
include(PlatformInfo)
include(gyb)
include(ResourceEmbedding)
include(CatalystSupport)

option(${PROJECT_NAME}_INSTALL_NESTED_SUBDIR "Install libraries under a platform and architecture subdirectory" ON)
set(${PROJECT_NAME}_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${${PROJECT_NAME}_INSTALL_NESTED_SUBDIR}>:/${${PROJECT_NAME}_PLATFORM_SUBDIR}/${${PROJECT_NAME}_ARCH_SUBDIR}>" CACHE STRING "")
set(${PROJECT_NAME}_INSTALL_LANGUAGEMODULEDIR "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>$<$<BOOL:${${PROJECT_NAME}_INSTALL_NESTED_SUBDIR}>:/${${PROJECT_NAME}_PLATFORM_SUBDIR}>" CACHE STRING "")

include("${${PROJECT_NAME}_VENDOR_MODULE_DIR}/Settings.cmake" OPTIONAL)

option(${PROJECT_NAME}_ENABLE_LIBRARY_EVOLUTION "Generate ABI resilient runtime libraries"
  ${CodiraCore_ENABLE_LIBRARY_EVOLUTION})

option(${PROJECT_NAME}_ENABLE_PRESPECIALIZATION "Enable generic metadata prespecialization"
  ${CodiraCore_ENABLE_PRESPECIALIZATION})

option(${PROJECT_NAME}_SINGLE_THREADED_MODE "Build Synchronization assuming it will be used in an environment with only a single thread" OFF)

add_compile_options(
  $<$<COMPILE_LANGUAGE:Codira>:-explicit-module-build>
  $<$<COMPILE_LANGUAGE:Codira>:-nostdlibimport>
  $<$<COMPILE_LANGUAGE:Codira>:-enable-builtin-module>
  $<$<COMPILE_LANGUAGE:Codira>:-strict-memory-safety>
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature NoncopyableGenerics2>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature SuppressedAssociatedTypes>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature SE427NoInferenceOnExtension>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature NonescapableTypes>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature LifetimeDependence>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature InoutLifetimeDependence>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature LifetimeDependenceMutableAccessors>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-upcoming-feature MemberImportVisibility>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature RawLayout>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature StaticExclusiveOnly>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature Extern>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-runtime-compatibility-version none>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-autolinking-runtime-compatibility-dynamic-replacements>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-autolinking-runtime-compatibility-concurrency>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-concurrency-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-string-processing-module-import>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enforce-exclusivity=unchecked>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -target-min-inlining-version -Xfrontend min>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enable-lexical-lifetimes=false>"
  "$<$<AND:$<BOOL:${${PROJECT_NAME}_ENABLE_LIBRARY_EVOLUTION}>,$<COMPILE_LANGUAGE:Codira>>:-enable-library-evolution>"
  "$<$<AND:$<BOOL:${${PROJECT_NAME}_ENABLE_PRESPECIALIZATION}>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -prespecialize-generic-metadata>")

gyb_expand(Atomics/AtomicIntegers.code.gyb Atomics/AtomicIntegers.code)
gyb_expand(Atomics/AtomicStorage.code.gyb Atomics/AtomicStorage.code)

add_library(languageSynchronization
  Atomics/Atomic.code
  Atomics/AtomicBool.code
  Atomics/AtomicFloats.code
  Atomics/AtomicLazyReference.code
  Atomics/AtomicMemoryOrderings.code
  Atomics/AtomicOptional.code
  Atomics/AtomicPointers.code
  Atomics/AtomicRepresentable.code
  Atomics/WordPair.code
  Atomics/AtomicStorage.code
  Atomics/AtomicIntegers.code
  Cell.code)

# Determine Mutex definition
if(${PROJECT_NAME}_SINGLE_THREADED_MODE)
  target_sources(languageSynchronization PRIVATE Mutex/MutexUnavailable.code)
else()
  target_sources(languageSynchronization PRIVATE
    Mutex/Mutex.code
    $<$<PLATFORM_ID:Darwin>:Mutex/DarwinImpl.code>
    $<$<PLATFORM_ID:Android,Linux>:Mutex/LinuxImpl.code>
    $<$<PLATFORM_ID:Android,WASI>:Mutex/SpinLoopHint.code>
    $<$<PLATFORM_ID:Windows>:Mutex/WindowsImpl.code>)
endif()

set_target_properties(languageSynchronization PROPERTIES
  Codira_MODULE_NAME Synchronization)

target_link_libraries(languageSynchronization PRIVATE
  languageCore
  $<$<PLATFORM_ID:Darwin>:languageDarwin>)

install(TARGETS languageSynchronization
  EXPORT CodiraSynchronizationTargets
  COMPONENT ${PROJECT_NAME}_runtime
  ARCHIVE DESTINATION "${${PROJECT_NAME}_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${${PROJECT_NAME}_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageSynchronization)
install_language_interface(languageSynchronization)

# Configure plist creation for Darwin platforms.
generate_plist("${CMAKE_PROJECT_NAME}" "${CMAKE_PROJECT_VERSION}" languageSynchronization)
embed_manifest(languageSynchronization)

include("${${PROJECT_NAME}_VENDOR_MODULE_DIR}/languageSynchronization.cmake" OPTIONAL)
