# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli


# Following function are needed as a workaround until it's possible to compile
# language code with cmake's builtin language support.

# Add a language compiler module
#
# Creates a target to compile a language module.
# Adds the module name to the global property "language_compiler_modules".
#
function(add_language_compiler_module module)
  cmake_parse_arguments(ALSM
                        ""
                        ""
                        "DEPENDS;SOURCES"
                        ${ARGN})
  set(raw_sources ${ALSM_SOURCES} ${ALSM_UNPARSED_ARGUMENTS})

  set(target_name "CodiraModule${module}")

  # Add a target which depends on the actual compilation target, which
  # will be created in add_language_compiler_modules_library.
  # This target is mainly used to add properties, like the list of source files.
  add_custom_target(
      ${target_name}
      COMMENT "language compiler module ${module}")

  language_compiler_sources(${module} ${raw_sources})

  set_property(TARGET ${target_name} PROPERTY module_name ${module})
  set_property(TARGET ${target_name} PROPERTY module_depends ${ALSM_DEPENDS})

  get_property(modules GLOBAL PROPERTY language_compiler_modules)
  set_property(GLOBAL PROPERTY language_compiler_modules ${modules} ${module})
endfunction()

# Add source files to a language compiler module.
#
function(language_compiler_sources module)
  cmake_parse_arguments(LSS
                        ""
                        ""
                        ""
                        ${ARGN})
  set(raw_sources ${LSS_UNPARSED_ARGUMENTS})
  set(sources)
  foreach(raw_source ${raw_sources})
    get_filename_component(
      raw_source "${raw_source}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND sources "${raw_source}")
  endforeach()

  set(target_name "CodiraModule${module}")
  set_property(TARGET "CodiraModule${module}" APPEND PROPERTY SOURCES ${sources})
endfunction()

# Allow the override of the flags used to define the SDK used to compile the
# Codira compiler sources from the CMake configuration (command line or cache
# files). This allows supporting complicated sysroots and some cross-compilation
# scenarios.
set(LANGUAGE_COMPILER_SOURCES_SDK_FLAGS_default)
if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
  set(sdk_path "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_PATH}")
  list(APPEND LANGUAGE_COMPILER_SOURCES_SDK_FLAGS_default "-sdk" "${sdk_path}")
  if(NOT EXISTS "${sdk_path}/usr/include/c++")
    # Darwin SDKs in Xcode 12 or older do not include libc++, which prevents clang from finding libc++ when invoked
    # from ClangImporter. This results in build errors. To workaround this, let's explicitly pass the path to libc++
    # to clang.
    message(WARNING "Building with an outdated Darwin SDK: libc++ missing from the ${LANGUAGE_HOST_VARIANT_SDK} SDK. Will use libc++ from the toolchain.")
    get_filename_component(absolute_libcxx_path "${CMAKE_C_COMPILER}/../../include/c++/v1" REALPATH)
    if (EXISTS "${absolute_libcxx_path}")
      list(APPEND LANGUAGE_COMPILER_SOURCES_SDK_FLAGS_default "-Xcc" "-isystem" "-Xcc" "${absolute_libcxx_path}")
    else()
      message(ERROR "libc++ not found in the toolchain.")
    endif()
  endif()
elseif(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
  list(APPEND LANGUAGE_COMPILER_SOURCES_SDK_FLAGS_default "-sdk" "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_PATH}")
endif()
set(LANGUAGE_COMPILER_SOURCES_SDK_FLAGS ${LANGUAGE_COMPILER_SOURCES_SDK_FLAGS_default}
  CACHE STRING "Codira flags used to compiler the Codira compiler sources")

# Add a library target for the language compiler modules.
#
# Adds targets to compile all language compiler modules and a target for the
# library itself.
#
function(add_language_compiler_modules_library name)
  cmake_parse_arguments(ALS
                        ""
                        "BOOTSTRAPPING;LANGUAGE_EXEC"
                        "DEPENDS"
                        ${ARGN})

  set(language_compile_options
      "-color-diagnostics"
      "-Xfrontend" "-validate-tbd-against-ir=none"
      "-cxx-interoperability-mode=default"
      "-Xfrontend" "-disable-target-os-checking"
      "-Xcc" "-std=c++17"
      "-Xcc" "-DCOMPILED_WITH_LANGUAGE" "-Xcc" "-DLANGUAGE_TARGET"
      "-Xcc" "-UIBOutlet" "-Xcc" "-UIBAction" "-Xcc" "-UIBInspectable")

  if (NOT BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
    if(LANGUAGE_MIN_RUNTIME_VERSION)
      list(APPEND language_compile_options
        "-Xfrontend" "-min-runtime-version"
        "-Xfrontend" "${LANGUAGE_MIN_RUNTIME_VERSION}")
    endif()
    list(APPEND language_compile_options "-Xfrontend" "-disable-implicit-string-processing-module-import")

    # We cannot use Unsafe*Pointer when importing C++ move-only types until the
    # host libraries are updated to Codira 6.0, because that importing strategy
    # requires _Pointer have its Pointee: ~Copyable. (rdar://128013193)
    list(APPEND language_compile_options "-Xfrontend" "-cxx-interop-use-opaque-pointer-for-moveonly")
  endif()

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND language_compile_options "-g")
  else()
    list(APPEND language_compile_options "-O" "-cross-module-optimization")
  endif()

  if(TOOLCHAIN_ENABLE_ASSERTIONS)
    list(APPEND language_compile_options "-Xcc" "-UNDEBUG")
  else()
    list(APPEND language_compile_options "-Xcc" "-DNDEBUG")
  endif()

  if("${BRIDGING_MODE}" STREQUAL "PURE")
    list(APPEND language_compile_options "-Xcc" "-DPURE_BRIDGING_MODE")
  endif()

  if(NOT LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
    list(APPEND language_compile_options "-Xfrontend" "-disable-legacy-type-info")
  endif()

  get_bootstrapping_path(build_dir ${CMAKE_CURRENT_BINARY_DIR} "${ALS_BOOTSTRAPPING}")

  set(sdk_option ${LANGUAGE_COMPILER_SOURCES_SDK_FLAGS})

  if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set(deployment_version "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_DEPLOYMENT_VERSION}")
    if(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE-WITH-HOSTLIBS")
      # Let the cross-compiled compile don't pick up the compiled stdlib by providing
      # an (almost) empty resource dir.
      # The compiler will instead pick up the stdlib from the SDK.
      get_filename_component(language_exec_bin_dir ${ALS_LANGUAGE_EXEC} DIRECTORY)
      list(APPEND sdk_option "-resource-dir" "${language_exec_bin_dir}/../bootstrapping0/lib/language")
    endif()
  elseif(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
    # NOTE: prepending allows LANGUAGE_COMPILER_SOURCES_SDK_FLAGS to override the
    # resource directory if needed.
    list(PREPEND sdk_option "-resource-dir" "${LANGUAGELIB_DIR}")
  endif()
  get_versioned_target_triple(target ${LANGUAGE_HOST_VARIANT_SDK}
      ${LANGUAGE_HOST_VARIANT_ARCH} "${deployment_version}")

  # Let Codira discover CodiraShims headers which are included by some headers
  # under `include/language`. These are either located next to the compiler (in case of open source toolchains) or
  # in the SDK (in case a Codira compiler from Xcode)
  get_filename_component(language_exec_bin_dir ${ALS_LANGUAGE_EXEC} DIRECTORY)

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND language_compile_options "-static")
    list(APPEND sdk_option "-sdk" "${LANGUAGE_PATH_TO_LANGUAGE_SDK}")
    # For "language/shims/*.h".
    list(APPEND sdk_option "-I" "${LANGUAGE_PATH_TO_LANGUAGE_SDK}/usr/lib")

    # Workaround a crash in the LoadableByAddress pass
    # https://github.com/apple/language/issues/73254
    list(APPEND language_compile_options "-Xtoolchain" "-sil-disable-pass=loadable-address")

    # MSVC 14.40 (VS 17.10, MSVC_VERSION 1940) added a requirement for Clang 17 or higher.
    # Codira 6.0 is the first version to include Clang 17.
    # MSVC 14.43 (VS 17.13, MSVC_VERSION 1943) added a requirement for Clang 18 or higher.
    # Codira 6.1 is the first version to include Clang 18.
    # These requirements can be found in `include/yvals_core.h` in the MSVC headers.
    # Bootstrapping generally uses toolchains with older versions of Clang, so if we are on Windows
    # and using an earlier toolchain, we need to relax this requirement by setting
    # `_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH`.
    if((CMAKE_Codira_COMPILER_VERSION VERSION_LESS 6.0 AND MSVC_VERSION VERSION_GREATER_EQUAL 1940) OR
       (CMAKE_Codira_COMPILER_VERSION VERSION_LESS 6.1 AND MSVC_VERSION VERSION_GREATER_EQUAL 1943))
      list(APPEND language_compile_options "-Xcc" "-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")
    endif()

    # Make 'offsetof()' a const value.
    list(APPEND language_compile_options "-Xcc" "-D_CRT_USE_BUILTIN_OFFSETOF")

    # Workaround for https://github.com/languagelang/toolchain-project/issues/7172
    list(APPEND language_compile_options "-Xcc" "-Xclang" "-Xcc" "-fmodule-format=raw")
  else()
    list(APPEND sdk_option "-I" "${language_exec_bin_dir}/../lib" "-I" "${sdk_path}/usr/lib")
  endif()

  set(all_obj_files)
  set(all_module_targets)
  set(all_module_files)
  get_property(modules GLOBAL PROPERTY "language_compiler_modules")
  foreach(module ${modules})

    set(module_target "CodiraModule${module}")
    get_target_property(module ${module_target} "module_name")
    get_target_property(sources ${module_target} SOURCES)
    get_target_property(dependencies ${module_target} "module_depends")
    set(deps, "")
    if (dependencies)
      foreach(dep_module ${dependencies})
        if (DEFINED "${dep_module}_dep_target")
          # We have to add the module target for the ordering dependency
          # and the output file for the file dependency (otherwise the dependent
          # module wouldn't be rebuilt if the current module changes)
          list(APPEND deps "${${dep_module}_dep_target}" "${build_dir}/${dep_module}.o")
        else()
          message(FATAL_ERROR "module dependency ${module} -> ${dep_module} not found. Make sure to add modules in dependency order")
        endif()
      endforeach()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
      # language-driver when non-optimizing in not Mach-O  platforms adds an extra
      # job wrapmodule that uses the {module-name}.o as output, which clashes
      # with the name chosen here. Use a different name in those cases.
      set(module_obj_file "${build_dir}/${module}.object.o")
      set(output_obj_files "${module_obj_file}" "${build_dir}/${module}.o")
    else()
      set(module_obj_file "${build_dir}/${module}.o")
      set(output_obj_files "${module_obj_file}")
    endif()
    set(module_file "${build_dir}/${module}.codemodule")
    list(APPEND all_module_files ${module_file})

    list(APPEND all_obj_files ${output_obj_files})
    set(c_include_paths
      # LLVM modules and headers.
      "${TOOLCHAIN_MAIN_INCLUDE_DIR}"
      # Generated LLVM headers.
      "${TOOLCHAIN_INCLUDE_DIR}"
      # Clang modules and headers.
      ${CLANG_INCLUDE_DIRS}
      # Bridging modules and headers.
      "${LANGUAGE_MAIN_INCLUDE_DIR}"
      # Generated C headers.
      "${CMAKE_CURRENT_BINARY_DIR}/../include")
    set(c_include_paths_args)
    foreach(c_include_path ${c_include_paths})
      list(APPEND c_include_paths_args "-Xcc" "-I" "-Xcc" "${c_include_path}")
    endforeach()

    # Compile the module into an object file
    add_custom_command_target(dep_target
      COMMAND ${ALS_LANGUAGE_EXEC} "-c" "-o" ${module_obj_file}
        ${sdk_option}
        "-target" ${target}
        "-module-name" ${module} "-emit-module"
        "-emit-module-path" "${build_dir}/${module}.codemodule"
        "-parse-as-library" ${sources}
        "-wmo" ${language_compile_options}
        ${c_include_paths_args}
        # Generated language modules.
        "-I" "${build_dir}"
      OUTPUT ${output_obj_files}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${sources} ${deps} ${ALS_DEPENDS}
        importedHeaderDependencies
      COMMENT "Building language module ${module}")

    if(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
      add_dependencies(${dep_target} language-stdlib-${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}-${LANGUAGE_HOST_VARIANT_ARCH})
    endif()
    set("${module}_dep_target" ${dep_target})
    set(all_module_targets ${all_module_targets} ${dep_target})
  endforeach()

  # Create a static library containing all module object files.
  add_library(${name} STATIC ${all_obj_files})
  add_dependencies(${name} ${all_module_targets})
  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${name} PUBLIC languageCodiraOnoneSupport)
  endif()

  # Downstream linking should include the languagemodule in debug builds to allow lldb to
  # work correctly. Only do this on Darwin since neither gold (currently used by default
  # on Linux), nor the default Windows linker 'link' support '-add_ast_path'.
  is_build_type_with_debuginfo("${CMAKE_BUILD_TYPE}" debuginfo)
  if(debuginfo AND LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set(public_link_flags)
    foreach(module_file ${all_module_files})
      list(APPEND public_link_flags "SHELL:-Xlinker -add_ast_path -Xlinker ${module_file}")
    endforeach()
    target_link_options(${name} PUBLIC ${public_link_flags})
  endif()

  set_property(GLOBAL APPEND PROPERTY LANGUAGE_BUILDTREE_EXPORTS ${name})
endfunction()


# A dummy library if language in the compiler is disabled
add_language_host_library(languageCompilerStub OBJECT stubs.cpp)

if (NOT BOOTSTRAPPING_MODE)

  add_library(languageCompilerModules ALIAS languageCompilerStub)

else()
  # Note: "Codira" is not added intentionally here, because it would break
  # the bootstrapping build in case no language toolchain is installed on the host.
  project(CodiraInTheCompiler LANGUAGES C CXX)

  add_subdirectory(Sources)


  if(BOOTSTRAPPING_MODE MATCHES "HOSTTOOLS|CROSSCOMPILE")

    if (NOT LANGUAGE_EXEC_FOR_LANGUAGE_MODULES)
      message(FATAL_ERROR "Need a language toolchain building language compiler sources")
    endif()

    if(BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
      if(NOT LANGUAGE_EXEC_FOR_LANGUAGE_MODULES STREQUAL CMAKE_Codira_COMPILER)
        message(FATAL_ERROR "The Codira compiler (${CMAKE_Codira_COMPILER}) differs from the Codira compiler in LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH (${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/languagec).")
      endif()

      set(min_supported_language_version 5.9)
      if(CMAKE_Codira_COMPILER_VERSION VERSION_LESS "${min_supported_language_version}")
        message(FATAL_ERROR
            "Outdated Codira compiler: building with host tools requires Codira ${min_supported_language_version} or newer. "
            "Please update your Codira toolchain or switch BOOTSTRAPPING_MODE to BOOTSTRAPPING(-WITH-HOSTLIBS)? or OFF.")
      endif()
    endif()

    add_language_compiler_modules_library(languageCompilerModules
      LANGUAGE_EXEC "${LANGUAGE_EXEC_FOR_LANGUAGE_MODULES}")

  elseif(BOOTSTRAPPING_MODE MATCHES "BOOTSTRAPPING.*")

    set(b0_deps language-frontend-bootstrapping0 symlink-headers-bootstrapping0 copy-legacy-layouts)
    set(b1_deps language-frontend-bootstrapping1 symlink-headers-bootstrapping1)
    if(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")
      list(APPEND b0_deps languageCore-bootstrapping0)
      list(APPEND b1_deps languageCore-bootstrapping1)
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND b0_deps languageCodiraOnoneSupport-bootstrapping0)
        list(APPEND b1_deps languageCodiraOnoneSupport-bootstrapping1)
      endif()
      if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
        list(APPEND b0_deps languageDarwin-bootstrapping0)
        list(APPEND b1_deps languageDarwin-bootstrapping1)
      endif()
      if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_LIBSTDCXX_PLATFORMS)
        list(APPEND b0_deps copy-libstdcxx-modulemap-bootstrapping0 copy-libstdcxx-header-bootstrapping0)
        list(APPEND b1_deps copy-libstdcxx-modulemap-bootstrapping1 copy-libstdcxx-header-bootstrapping1)
      endif()
    endif()
    if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS AND LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
      # We cannot specify directly HostCompatibilityLibs
      # because ultimately is used to specify a dependency for a
      # custom target and, unlike `target_link_libraries`, such dependency
      # would be lost at the generation of the build system.
      get_property(compatibility_libs
        TARGET HostCompatibilityLibs
        PROPERTY INTERFACE_LINK_LIBRARIES)
      list(APPEND b0_deps ${compatibility_libs})
      list(APPEND b1_deps ${compatibility_libs})
    endif()


    # Bootstrapping - stage 1, using the compiler from level 0

    add_language_compiler_modules_library(languageCompilerModules-bootstrapping1
      LANGUAGE_EXEC $<TARGET_FILE_DIR:language-frontend-bootstrapping0>/languagec${CMAKE_EXECUTABLE_SUFFIX}
      DEPENDS ${b0_deps}
      BOOTSTRAPPING 1)

    # The final build, using the compiler from stage 1

    add_language_compiler_modules_library(languageCompilerModules
        LANGUAGE_EXEC $<TARGET_FILE_DIR:language-frontend-bootstrapping1>/languagec${CMAKE_EXECUTABLE_SUFFIX}
        DEPENDS ${b1_deps})

    if(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING-WITH-HOSTLIBS")
      file(GLOB module_dirs "${CMAKE_BINARY_DIR}/bootstrapping*/lib/language/macosx/*.codemodule")
      foreach(module_dir ${module_dirs})
        message(WARNING "${module_dir} found from a previous 'bootstrapping' build: removing")
        file(REMOVE_RECURSE "${module_dir}")
      endforeach()
    endif()
  else()
    message(FATAL_ERROR "Unknown BOOTSTRAPPING_MODE '${BOOTSTRAPPING_MODE}'")
  endif()

endif()
