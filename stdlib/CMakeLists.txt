
# TODO(compnerd) once we have a newer CMake we should be able to use the new
# `Codira` LANGUAGE support in CMake to simplify the build.  For now, just add
# the project so that it is possible to start working on extracting the Codira
# standard library from the Codira compiler build.
project(language-stdlib LANGUAGES C CXX)

# CMake passes `-external:I` to clang-cl which results in the search order being
# altered, and this impacts the definitions of the intrinsics. When building
# with a MSVC toolset 19.29.30036.3 or newer, this will prevent the runtime from
# being built on Windows.  Since we know that we only support `clang-cl` as the
# compiler for the runtime due to the use of the Codira calling convention, we
# simply override the CMake behaviour unconditionally.
set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-Isystem")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-Isystem")

# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if(LANGUAGE_BUILD_SDK_OVERLAY OR LANGUAGE_INCLUDE_TESTS)
  set(LANGUAGE_BUILD_TEST_SUPPORT_MODULES_default TRUE)
else()
  set(LANGUAGE_BUILD_TEST_SUPPORT_MODULES_default FALSE)
endif()

# TODO(compnerd) use a target to avoid re-creating this file all the time
function(generate_windows_vfs_overlay)
  file(TO_CMAKE_PATH ${VCToolsInstallDir} VCToolsInstallDir)
  file(TO_CMAKE_PATH ${UniversalCRTSdkDir} UniversalCRTSdkDir)
  configure_file("${PROJECT_SOURCE_DIR}/cmake/WindowsVFS.yaml.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/windows-vfs-overlay.yaml"
                 @ONLY)
endfunction()
if(WINDOWS IN_LIST LANGUAGE_SDKS)
  generate_windows_vfs_overlay()
  file(TO_CMAKE_PATH "${CMAKE_CURRENT_BINARY_DIR}/windows-vfs-overlay.yaml"
       LANGUAGE_WINDOWS_VFS_OVERLAY)
endif()

#
# User-configurable options for the standard library.
#

# New options should be added to stdlib/cmake/modules/StdlibOptions.cmake,
# so that they are considered in configurations using StandaloneOverlay.cmake

# NOTE: Some of these variables are also initialized in StandaloneOverlay.cmake
# so that interfaces are emitted when overlays are separately built.

# TODO: migrate this section to StdlibOptions.cmake to reduce duplication

option(LANGUAGE_BUILD_TEST_SUPPORT_MODULES
       "Whether to build StdlibUnittest and other test support modules. Defaults to On when LANGUAGE_BUILD_SDK_OVERLAY is On, or when LANGUAGE_INCLUDE_TESTS is On."
       "${LANGUAGE_BUILD_TEST_SUPPORT_MODULES_default}")

# New options should be added to stdlib/cmake/modules/StdlibOptions.cmake,
# so that they are considered in configurations using StandaloneOverlay.cmake
include(StdlibOptions)

#
# End of user-configurable options.
#

set(LANGUAGE_STDLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Remove toolchain-project/toolchain/include directory from -I search part when building
# the stdlib. We have our own fork of LLVM includes (Support, ADT) in
# stdlib/include/toolchain and we don't want to silently use headers from LLVM.
get_property(stdlib_include_directories DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
list(REMOVE_ITEM stdlib_include_directories "${TOOLCHAIN_MAIN_INCLUDE_DIR}")
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES ${stdlib_include_directories})

include(AddCodiraStdlib)

# Create convenience targets for the Codira standard library.

# NOTE(compnerd) this will pass the *build* configuration to the *host*
# libraries.  Explicitly indicate to CMake that it should **NOT** track the
# implicit language runtimes.  This can go away once we migrate to an external
# project with its own configure with the CMAKE_SYSTEM_NAME set rather than
# using the custom cross-compilation solution
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

if(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER)
  if(NOT "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Building the language runtime is not supported with ${CMAKE_C_COMPILER_ID}. Use the just-built clang instead.")
  else()
    message(WARNING "Building the language runtime using the host compiler, and not the just-built clang.")
  endif()
else()
  # If we use Clang-cl or MSVC, CMake provides default compiler and linker flags that are incompatible
  # with the frontend of Clang or Clang++.
  if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
    set(CMAKE_CXX_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang-cl")
    set(CMAKE_C_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang-cl")
  else()
    set(CMAKE_CXX_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang++")
    set(CMAKE_C_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang")
  endif()

  if(CMAKE_C_COMPILER_LAUNCHER MATCHES ".*distcc")
    set(CMAKE_C_COMPILER_LAUNCHER "")
  endif()
  if(CMAKE_CXX_COMPILER_LAUNCHER MATCHES ".*distcc")
    set(CMAKE_CXX_COMPILER_LAUNCHER "")
  endif()

  # The sanitizers require using the same version of the compiler for
  # everything and there are various places where we link runtime code with
  # code built by the host compiler. Disable sanitizers for the runtime for
  # now.
  add_compile_options(-fno-sanitize=all)
endif()

# Do not enforce checks for LLVM's ABI-breaking build settings.
# The Codira runtime uses some header-only code from LLVM's ADT classes,
# but we do not want to link libSupport into the runtime. These checks rely
# on the presence of symbols in libSupport to identify how the code was
# built and cause link failures for mismatches. Without linking that library,
# we get link failures regardless, so instead, this just disables the checks.
add_compile_definitions($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:TOOLCHAIN_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1>)

set(LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES)
if(LANGUAGE_BUILD_DYNAMIC_STDLIB)
  list(APPEND LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES SHARED)
endif()
if(LANGUAGE_BUILD_STATIC_STDLIB)
  list_intersect("${LANGUAGE_DARWIN_PLATFORMS}" "${LANGUAGE_SDKS}" building_darwin_sdks)
  if(building_darwin_sdks)
    message(SEND_ERROR "cannot build static standard library for Darwin SDKs")
  else()
    list(APPEND LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES STATIC)
  endif()
endif()

function(language_create_stdlib_targets name variant define_all_alias)
  if(NOT variant STREQUAL "")
    set(variant "-${variant}")
  endif()

  if(define_all_alias)
    add_custom_target(${name}${variant}-all)
    set_target_properties(${name}${variant}-all
      PROPERTIES
      FOLDER "Codira libraries/Aggregate")
  endif()

  foreach(sdk ${LANGUAGE_SDKS})
    add_custom_target(${name}-${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}${variant})
    set_target_properties(${name}-${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}${variant}
      PROPERTIES
      FOLDER "Codira libraries/Aggregate")

    foreach(arch ${LANGUAGE_SDK_${sdk}_ARCHITECTURES})
      set(target_variant -${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}-${arch})

      add_custom_target(${name}${target_variant}${variant})
      set_target_properties(${name}${target_variant}${variant}
        PROPERTIES
        FOLDER "Codira libraries/Aggregate")
      if(define_all_alias)
        add_dependencies(${name}${variant}-all
          ${name}${target_variant}${variant})
      endif()
      add_dependencies(${name}-${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}${variant}
        ${name}${target_variant}${variant})
    endforeach()
  endforeach()

  if(NOT define_all_alias)
    set(ALL_keyword ALL)
  endif()

  # When cross-compiling host tools for multiple architectures, targeting a
  # different SDK, the primary variant is not one of the variants being built,
  # so it can't be added as a target here. build-script will invoke the
  # more-specific target, so just skip creating this target and warn in case
  # someone is using the CMake more directly.
  if(LANGUAGE_PRIMARY_VARIANT_SDK IN_LIST LANGUAGE_SDKS)
    add_custom_target(${name}${variant}
      ${ALL_keyword}
      DEPENDS
      ${name}${LANGUAGE_PRIMARY_VARIANT_SUFFIX}${variant})
    set_target_properties(${name}${variant}
      PROPERTIES
      FOLDER "Codira libraries/Aggregate")
  else()
    message(WARNING "Primary variant ${LANGUAGE_PRIMARY_VARIANT_SDK} is not being built, not creating ${name}${variant} alias target for it.")
  endif()
endfunction()

if("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "singlethreaded"
   AND NOT LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY)
  message(SEND_ERROR "Cannot enable the single-threaded global executor without enabling LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY")
endif()

if(LANGUAGE_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY)
  if(LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY)
    message(SEND_ERROR "Cannot use the single-threaded concurrency with task-to-thread concurrency model")
  endif()
  if(NOT "${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "none")
    message(SEND_ERROR "Task-to-thread concurrency model requires no global executor")
  endif()
else()
  if("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "none")
    message(SEND_ERROR "Only task-to-thread concurrency model is usable with no global executor")
  endif()
endif()

language_create_stdlib_targets("language-stdlib" "" TRUE)
if(LANGUAGE_STDLIB_ENABLE_SIB_TARGETS)
  language_create_stdlib_targets("language-stdlib" "sib" TRUE)
  language_create_stdlib_targets("language-stdlib" "sibopt" TRUE)
  language_create_stdlib_targets("language-stdlib" "sibgen" TRUE)
endif()
language_create_stdlib_targets("language-test-stdlib" "" FALSE)

# Create the libexec targets
language_create_stdlib_targets("language-libexec" "" TRUE)
language_create_stdlib_targets("language-test-libexec" "" FALSE)

# FIXME: Include the toolchain directory before the public directory. Otherwise
# the clang resource directory symlink stops installing correctly.
add_subdirectory(toolchain)
add_subdirectory(public)
if(LANGUAGE_STDLIB_BUILD_PRIVATE)
  add_subdirectory(private)
endif()
add_subdirectory(tools)
