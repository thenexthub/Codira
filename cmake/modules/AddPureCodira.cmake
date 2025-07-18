include(macCatalystUtils)

# Workaround a cmake bug, see the corresponding function in language-syntax
function(force_add_dependencies TARGET)
  foreach(DEPENDENCY ${ARGN})
    string(REGEX REPLACE [<>:\"/\\|?*] _ sanitized ${DEPENDENCY})
    set(depfile "${CMAKE_CURRENT_BINARY_DIR}/forced-${sanitized}-dep.code")
    add_custom_command(OUTPUT ${depfile}
      COMMAND ${CMAKE_COMMAND} -E touch ${depfile}
      DEPENDS ${DEPENDENCY}
    )
    target_sources(${TARGET} PRIVATE ${depfile})
  endforeach()
endfunction()

function(force_target_link_libraries TARGET)
  target_link_libraries(${TARGET} ${ARGN})

  cmake_parse_arguments(ARGS "PUBLIC;PRIVATE;INTERFACE" "" "" ${ARGN})
  force_add_dependencies(${TARGET} ${ARGS_UNPARSED_ARGUMENTS})
endfunction()

# Add compile options shared between libraries and executables.
function(_add_host_language_compile_options name)
  # Avoid introducing an implicit dependency on the string-processing library.
  if(LANGUAGE_SUPPORTS_DISABLE_IMPLICIT_STRING_PROCESSING_MODULE_IMPORT)
    target_compile_options(${name} PRIVATE
      "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-implicit-string-processing-module-import>")
  endif()

  # Emitting module seprately doesn't give us any benefit.
  target_compile_options(${name} PRIVATE
    "$<$<COMPILE_LANGUAGE:Codira>:-no-emit-module-separately-wmo>")

  if(LANGUAGE_ANALYZE_CODE_COVERAGE)
     set(_cov_flags $<$<COMPILE_LANGUAGE:Codira>:-profile-generate -profile-coverage-mapping>)
     target_compile_options(${name} PRIVATE ${_cov_flags})
     target_link_options(${name} PRIVATE ${_cov_flags})
  endif()

  if("${BRIDGING_MODE}" STREQUAL "PURE")
    target_compile_options(${name} PRIVATE
      "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -DPURE_BRIDGING_MODE>")
  endif()

  # The compat56 library is not available in current toolchains. The stage-0
  # compiler will build fine since the builder compiler is not aware of the 56
  # compat library, but the stage-1 and subsequent stage compilers will fail as
  # the stage-0 compiler is aware and will attempt to include the appropriate
  # compatibility library. We should turn this back on once we are building the
  # compiler correctly.
  # Note: This is safe at the moment because the 5.6 compat library only
  #       contains concurrency runtime fixes, and the compiler frontend does not
  #       use concurrency at the moment.
  target_compile_options(${name} PRIVATE
    $<$<COMPILE_LANGUAGE:Codira>:-runtime-compatibility-version>
    $<$<COMPILE_LANGUAGE:Codira>:none>)

  target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:Codira>:-target;${LANGUAGE_HOST_TRIPLE}>)
  if(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
    add_dependencies(${name} language-stdlib-${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}-${LANGUAGE_HOST_VARIANT_ARCH})
    target_compile_options(${name} PRIVATE
      $<$<COMPILE_LANGUAGE:Codira>:-sdk;${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_PATH};>
      $<$<COMPILE_LANGUAGE:Codira>:-resource-dir;${LANGUAGELIB_DIR};>)
    if(LANGUAGE_HOST_VARIANT_SDK STREQUAL "ANDROID" AND NOT "${LANGUAGE_ANDROID_NDK_PATH}" STREQUAL "")
      language_android_tools_path(${LANGUAGE_HOST_VARIANT_ARCH} tools_path)
      target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:Codira>:-tools-directory;${tools_path};>)
    endif()
  endif()

  target_compile_options(${name} PRIVATE
    $<$<COMPILE_LANGUAGE:Codira>:-color-diagnostics>
  )

  if(TOOLCHAIN_ENABLE_ASSERTIONS)
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -UNDEBUG>")
  else()
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -DNDEBUG>")
  endif()
endfunction()

# Set compile options for C/C++ interop
function(_set_language_cxx_interop_options name)
  target_compile_options(${name} PRIVATE
    "SHELL: -Xcc -std=c++17 -Xcc -DCOMPILED_WITH_LANGUAGE"

    # FIXME: Needed to work around an availability issue with CxxStdlib
    "SHELL: -Xfrontend -disable-target-os-checking"

    # Necessary to avoid treating IBOutlet and IBAction as keywords
    "SHELL:-Xcc -UIBOutlet -Xcc -UIBAction -Xcc -UIBInspectable"
  )

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_options(${name} PRIVATE
      # Make 'offsetof()' a const value.
      "SHELL:-Xcc -D_CRT_USE_BUILTIN_OFFSETOF"
      # Workaround for https://github.com/languagelang/toolchain-project/issues/7172
      "SHELL:-Xcc -Xclang -Xcc -fmodule-format=raw"
    )
  endif()

  # Prior to 5.9, we have to use the experimental flag for C++ interop.
  if (CMAKE_Codira_COMPILER_VERSION VERSION_LESS 5.9)
    target_compile_options(${name} PRIVATE
      "SHELL:-Xfrontend -enable-experimental-cxx-interop"
    )
  else()
    target_compile_options(${name} PRIVATE
      "-cxx-interoperability-mode=default"
    )
  endif()
endfunction()

function(_set_pure_language_link_flags name relpath_to_lib_dir)
  if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    # Don't add builder's stdlib RPATH automatically.
    target_compile_options(${name} PRIVATE
      $<$<COMPILE_LANGUAGE:Codira>:-no-toolchain-stdlib-rpath>
    )

    set_property(TARGET ${name}
      APPEND PROPERTY INSTALL_RPATH
        # At runtime, use languageCore in the current just-built toolchain.
        # NOTE: This relies on the ABI being the same as the builder.
        "$ORIGIN/${relpath_to_lib_dir}language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}"
    )
    # NOTE: At this point we don't have any pure language executables/shared
    # libraries required for building runtime/stdlib. So we don't need to add
    # RPATH to the builder's runtime.
  endif()
endfunction()

function(_set_pure_language_profile_flags target_name)
  # This replicates the code existing in LLVM toolchain/cmake/modules/HandleLLVMOptions.cmake
  # The second part of the clause replicates the LINKER_IS_LLD_LINK of the
  # original.
  if(TOOLCHAIN_BUILD_INSTRUMENTED AND NOT (LANGUAGE_COMPILER_IS_MSVC_LIKE AND LANGUAGE_USE_LINKER STREQUAL "lld"))
    string(TOUPPER "${TOOLCHAIN_BUILD_INSTRUMENTED}" uppercase_TOOLCHAIN_BUILD_INSTRUMENTED)
    if(TOOLCHAIN_ENABLE_IR_PGO OR uppercase_TOOLCHAIN_BUILD_INSTRUMENTED STREQUAL "IR")
      target_link_options(${target_name} PRIVATE
        "SHELL:-Xclang-linker -fprofile-generate=\"${TOOLCHAIN_PROFILE_DATA_DIR}\"")
    elseif(uppercase_TOOLCHAIN_BUILD_INSTRUMENTED STREQUAL "CSIR")
      target_link_options(${target_name} PRIVATE
        "SHELL:-Xclang-linker -fcs-profile-generate=\"${TOOLCHAIN_CSPROFILE_DATA_DIR}\"")
    else()
      target_link_options(${target_name} PRIVATE
        "SHELL:-Xclang-linker -fprofile-instr-generate=\"${TOOLCHAIN_PROFILE_FILE_PATTERN}\"")
    endif()
  endif()
endfunction()

function(_set_pure_language_package_options target_name package_name)
  if(NOT package_name OR NOT Codira_COMPILER_PACKAGE_CMO_SUPPORT)
    return()
  endif()

  # Enable package CMO if possible.
  # NOTE: '-enable-library-evolution' is required for package CMO even when we
  # don't need '.codeinterface'. E.g. executables.
  if(Codira_COMPILER_PACKAGE_CMO_SUPPORT STREQUAL "IMPLEMENTED")
    target_compile_options("${target_name}" PRIVATE
      "-enable-library-evolution"
      "SHELL:-package-name ${package_name}"
      "SHELL:-Xfrontend -package-cmo"
      "SHELL:-Xfrontend -allow-non-resilient-access"
    )
  elseif(Codira_COMPILER_PACKAGE_CMO_SUPPORT STREQUAL "EXPERIMENTAL")
    target_compile_options("${target_name}" PRIVATE
      "-enable-library-evolution"
      "SHELL:-package-name ${package_name}"
      "SHELL:-Xfrontend -experimental-package-cmo"
      "SHELL:-Xfrontend -experimental-allow-non-resilient-access"
      "SHELL:-Xfrontend -experimental-package-bypass-resilience"
    )
  endif()
endfunction()

# Add a new "pure" Codira host library.
#
# "Pure" Codira host libraries can only contain Codira code, and will be built
# with the host compiler. They are always expected to be part of the built
# compiler, without bootstrapping.
#
# All of these libraries depend on the language-syntax stack, since they are
# meant to be part of the compiler.
#
# Usage:
#   add_pure_language_host_library(name
#     [SHARED]
#     [STATIC]
#     [TOOLCHAIN_LINK_COMPONENTS comp1 ...]
#     source1 [source2 source3 ...])
#
# name
#   Name of the library (e.g., languageParse).
#
# SHARED
#   Build a shared library.
#
# STATIC
#   Build a static library.
#
# CXX_INTEROP
#   Use C++ interop.
#
# EMIT_MODULE
#   Emit '.codemodule' to
#
# PACKAGE_NAME
#   Name of the Codira package this library belongs to.
#
# DEPENDENCIES
#   Target names to pass target_link_library
#
# LANGUAGE_DEPENDENCIES
#   Target names to pass force_target_link_library.
#   TODO: Remove this and use DEPENDENCIES when CMake is fixed
#
# source1 ...
#   Sources to add into this library.
function(add_pure_language_host_library name)
  if (NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    message(STATUS "Not building ${name} because language-syntax is not available")
    return()
  endif()

  # Option handling
  set(options
        SHARED
        STATIC
        CXX_INTEROP
        EMIT_MODULE)
  set(single_parameter_options
        PACKAGE_NAME)
  set(multiple_parameter_options
        DEPENDENCIES
        LANGUAGE_DEPENDENCIES)

  cmake_parse_arguments(APSHL
                        "${options}"
                        "${single_parameter_options}"
                        "${multiple_parameter_options}"
                        ${ARGN})
  set(APSHL_SOURCES ${APSHL_UNPARSED_ARGUMENTS})

  translate_flags(APSHL "${options}")

  # Determine what kind of library we're building.
  if(APSHL_SHARED)
    set(libkind SHARED)
  elseif(APSHL_STATIC)
    set(libkind STATIC)
  endif()

  # Create the library.
  add_library(${name} ${libkind} ${APSHL_SOURCES})
  _add_host_language_compile_options(${name})
  _set_pure_language_package_options(${name} "${APSHL_PACKAGE_NAME}")
  if(APSHL_CXX_INTEROP)
    _set_language_cxx_interop_options(${name})
  endif()

  set_property(TARGET ${name}
    PROPERTY BUILD_WITH_INSTALL_RPATH YES)

  if(APSHL_SHARED AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Allow install_name_tool to update paths (for rdar://109473564)
    set_property(TARGET ${name} APPEND_STRING PROPERTY
                 LINK_FLAGS " -Xlinker -headerpad_max_install_names")
  endif()

  # Respect TOOLCHAIN_COMMON_DEPENDS if it is set.
  #
  # TOOLCHAIN_COMMON_DEPENDS if a global variable set in ./lib that provides targets
  # such as language-syntax or tblgen that all LLVM/Codira based tools depend on. If
  # we don't have it defined, then do not add the dependency since some parts of
  # language host tools do not interact with LLVM/Codira tools and do not define
  # TOOLCHAIN_COMMON_DEPENDS.
  if (TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif()

  # Depends on all '*.h' files in 'include/module.modulemap'.
  force_add_dependencies(${name} importedHeaderDependencies)

  # Link against dependencies.
  target_link_libraries(${name} PUBLIC
    ${APSHL_DEPENDENCIES}
  )
  # TODO: Change to target_link_libraries when cmake is fixed
  force_target_link_libraries(${name} PUBLIC
    ${APSHL_LANGUAGE_DEPENDENCIES}
  )

  if(APSHL_EMIT_MODULE)
    set(module_triple "${LANGUAGE_HOST_MODULE_TRIPLE}")
    set(module_dir "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
    set(module_base "${module_dir}/${name}.codemodule")
    set(module_file "${module_base}/${module_triple}.codemodule")
    set(module_interface_file "${module_base}/${module_triple}.codeinterface")
    set(module_private_interface_file "${module_base}/${module_triple}.private.codeinterface")
    set(module_sourceinfo_file "${module_base}/${module_triple}.codesourceinfo")

    # Create the module directory.
    add_custom_command(
        TARGET ${name}
        PRE_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${module_base}
        COMMENT "Generating module directory for ${name}")

    # Configure the emission of the Codira module files.
    target_compile_options("${name}" PRIVATE
        $<$<COMPILE_LANGUAGE:Codira>:
        -module-name;$<TARGET_PROPERTY:${name},Codira_MODULE_NAME>;
        -enable-library-evolution;
        -emit-module-path;${module_file};
        -emit-module-source-info-path;${module_sourceinfo_file};
        -emit-module-interface-path;${module_interface_file};
        -emit-private-module-interface-path;${module_private_interface_file}
        >)
  else()
    # Emit a languagemodule in the current directory.
    set(module_dir "${CMAKE_CURRENT_BINARY_DIR}/modules")
    set(module_file "${module_dir}/${name}.codemodule")
  endif()

  set_target_properties(${name} PROPERTIES
    # Set the default module name to the target name.
    Codira_MODULE_NAME ${name}
    # Install the Codira module into the appropriate location.
    Codira_MODULE_DIRECTORY ${module_dir}
    # NOTE: workaround for CMake not setting up include flags.
    INTERFACE_INCLUDE_DIRECTORIES ${module_dir})

  # Workaround to touch the library and its objects so that we don't
  # continually rebuild (again, see corresponding change in language-syntax).
  add_custom_command(
      TARGET ${name}
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E touch_nocreate $<TARGET_FILE:${name}> $<TARGET_OBJECTS:${name}> "${module_file}"
      COMMAND_EXPAND_LISTS
      COMMENT "Update mtime of library outputs workaround")

  # Downstream linking should include the languagemodule in debug builds to allow lldb to
  # work correctly. Only do this on Darwin since neither gold (currently used by default
  # on Linux), nor the default Windows linker 'link' support '-add_ast_path'.
  is_build_type_with_debuginfo("${CMAKE_BUILD_TYPE}" debuginfo)
  if(debuginfo AND LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    target_link_options(${name} PUBLIC "SHELL:-Xlinker -add_ast_path -Xlinker ${module_file}")
  endif()

  if(TOOLCHAIN_USE_LINKER)
    target_link_options(${name} PRIVATE
      "-use-ld=${TOOLCHAIN_USE_LINKER}"
    )
  endif()

  _set_pure_language_profile_flags(${name})

  # Enable build IDs
  if(LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_USE_BUILD_ID)
    target_link_options(${name} PRIVATE
      "SHELL:-Xlinker --build-id=sha1")
  endif()

  # Export this target.
  set_property(GLOBAL APPEND PROPERTY LANGUAGE_EXPORTS ${name})
endfunction()

# Add a new "pure" Codira host tool.
#
# "Pure" Codira host tools can only contain Codira code, and will be built
# with the host compiler.
#
# Usage:
#   add_pure_language_host_tool(name
#     [DEPENDENCIES dep1 ...]
#     [LANGUAGE_DEPENDENCIES languagedep1 ...]
#     source1 [source2 source3 ...])
#
# name
#   Name of the tool (e.g., language-frontend).
#
# PACKAGE_NAME
#   Name of the Codira package this executable belongs to.
#
# DEPENDENCIES
#   Target names to pass target_link_library
#
# LANGUAGE_DEPENDENCIES
#   Target names to pass force_target_link_library.
#   TODO: Remove this and use DEPENDENCIES when CMake is fixed
#
# source1 ...
#   Sources to add into this tool.
function(add_pure_language_host_tool name)
  if (NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    message(STATUS "Not building ${name} because language-syntax is not available")
    return()
  endif()

  # Option handling
  set(options)
  set(single_parameter_options
    LANGUAGE_COMPONENT
    PACKAGE_NAME)
  set(multiple_parameter_options
        DEPENDENCIES
        LANGUAGE_DEPENDENCIES)

  cmake_parse_arguments(APSHT
                        "${options}"
                        "${single_parameter_options}"
                        "${multiple_parameter_options}"
                        ${ARGN})
  set(APSHT_SOURCES ${APSHT_UNPARSED_ARGUMENTS})

  # Create the library.
  add_executable(${name} ${APSHT_SOURCES})
  _add_host_language_compile_options(${name})
  _set_pure_language_link_flags(${name} "../lib/")
  _set_pure_language_package_options(${name} "${APSHT_PACKAGE_NAME}")

  if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set_property(TARGET ${name}
      APPEND PROPERTY INSTALL_RPATH
        "@executable_path/../lib/language/host")
  elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    set_property(TARGET ${name}
      APPEND PROPERTY INSTALL_RPATH
        "$ORIGIN/../lib/language/host")
  endif()

  set_property(TARGET ${name}
    PROPERTY BUILD_WITH_INSTALL_RPATH YES)

  # Respect TOOLCHAIN_COMMON_DEPENDS if it is set.
  #
  # TOOLCHAIN_COMMON_DEPENDS if a global variable set in ./lib that provides targets
  # such as language-syntax or tblgen that all LLVM/Codira based tools depend on. If
  # we don't have it defined, then do not add the dependency since some parts of
  # language host tools do not interact with LLVM/Codira tools and do not define
  # TOOLCHAIN_COMMON_DEPENDS.
  if (TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif()

  # Depends on all '*.h' files in 'include/module.modulemap'.
  force_add_dependencies(${name} importedHeaderDependencies)

  # Link against dependencies.
  target_link_libraries(${name} PUBLIC
    ${APSHT_DEPENDENCIES}
  )
  # TODO: Change to target_link_libraries when cmake is fixed
  force_target_link_libraries(${name} PUBLIC
    ${APSHT_LANGUAGE_DEPENDENCIES}
  )

  # Make sure we can use the host libraries.
  target_include_directories(${name} PUBLIC
    "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
  target_link_directories(${name} PUBLIC
    "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")

  if(TOOLCHAIN_USE_LINKER)
    target_link_options(${name} PRIVATE
      "-use-ld=${TOOLCHAIN_USE_LINKER}"
    )
  endif()

  # Enable build IDs
  if(LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_USE_BUILD_ID)
    target_link_options(${name} PRIVATE
      "SHELL:-Xlinker --build-id=sha1")
  endif()

  # Workaround to touch the library and its objects so that we don't
  # continually rebuild (again, see corresponding change in language-syntax).
  add_custom_command(
      TARGET ${name}
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E touch_nocreate $<TARGET_FILE:${name}> $<TARGET_OBJECTS:${name}>
      COMMAND_EXPAND_LISTS
      COMMENT "Update mtime of executable outputs workaround")

  # Even worse hack - ${name}.codemodule is added as an output, even though
  # this is an executable target. Just touch it all the time to avoid having
  # to rebuild it every time.
  add_custom_command(
      TARGET ${name}
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/${name}.codemodule"
      COMMAND_EXPAND_LISTS
      COMMENT "Update mtime of executable outputs workaround")

  if(NOT APSHT_LANGUAGE_COMPONENT STREQUAL no_component)
    add_dependencies(${APSHT_LANGUAGE_COMPONENT} ${name})
    language_install_in_component(TARGETS ${name}
      COMPONENT ${APSHT_LANGUAGE_COMPONENT}
      RUNTIME DESTINATION bin)
    language_is_installing_component(${APSHT_LANGUAGE_COMPONENT} is_installing)
  endif()

  if(NOT is_installing)
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_BUILDTREE_EXPORTS ${name})
  else()
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_EXPORTS ${name})
  endif()

  _set_pure_language_profile_flags(${name})
endfunction()
