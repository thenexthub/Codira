include(macCatalystUtils)
include(CodiraUtils)

function(_compute_lto_language_flag option out_var)
  string(TOLOWER "${option}" lowercase_option)
  if (lowercase_option STREQUAL "full")
    set(${out_var} "-lto=toolchain-full" PARENT_SCOPE)
  elseif (lowercase_option STREQUAL "thin")
    set(${out_var} "-lto=toolchain-thin" PARENT_SCOPE)
  endif()
endfunction()

# Compute the library subdirectory to use for the given sdk and
# architecture, placing the result in 'result_var_name'.
function(compute_library_subdir result_var_name sdk arch)
  set("${result_var_name}" "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}" PARENT_SCOPE)
endfunction()

# Return a languagec flag (e.g. -O or -Onone) to control optimization level based
# on the build type.
function(language_optimize_flag_for_build_type build_type result_var_name)
  if("${build_type}" STREQUAL "Debug")
    set("${result_var_name}" "-Onone" PARENT_SCOPE)
  elseif("${build_type}" STREQUAL "RelWithDebInfo" OR
         "${build_type}" STREQUAL "Release")
    set("${result_var_name}" "-O" PARENT_SCOPE)
  elseif("${build_type}" STREQUAL "MinSizeRel")
    set("${result_var_name}" "-Osize" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unknown build type: ${build_type}")
  endif()
endfunction()

# Process the sources within the given variable, pulling out any Codira
# sources to be compiled with 'language' directly. This updates
# ${sourcesvar} in place with the resulting list and ${externalvar} with the
# list of externally-build sources.
#
# Usage:
#   handle_language_sources(sourcesvar externalvar)
#
# MACCATALYST_BUILD_FLAVOR
#   Possible values are 'ios-like', 'macos-like', 'zippered', 'unzippered-twin'
function(handle_language_sources
    dependency_target_out_var_name
    dependency_module_target_out_var_name
    dependency_sib_target_out_var_name
    dependency_sibopt_target_out_var_name
    dependency_sibgen_target_out_var_name
    sourcesvar externalvar name)
  cmake_parse_arguments(LANGUAGESOURCES
      "IS_MAIN;IS_STDLIB;IS_STDLIB_CORE;IS_SDK_OVERLAY;EMBED_BITCODE;STATIC;NO_LINK_NAME;IS_FRAGILE;ONLY_LANGUAGEMODULE;NO_LANGUAGEMODULE"
      "SDK;ARCHITECTURE;ARCHITECTURE_SUBDIR_NAME;INSTALL_IN_COMPONENT;DEPLOYMENT_VERSION_OSX;DEPLOYMENT_VERSION_IOS;DEPLOYMENT_VERSION_TVOS;DEPLOYMENT_VERSION_WATCHOS;MACCATALYST_BUILD_FLAVOR;BOOTSTRAPPING;INSTALL_BINARY_LANGUAGEMODULE"
      "DEPENDS;COMPILE_FLAGS;MODULE_NAME;MODULE_DIR;ENABLE_LTO"
      ${ARGN})
  translate_flag(${LANGUAGESOURCES_IS_MAIN} "IS_MAIN" IS_MAIN_arg)
  translate_flag(${LANGUAGESOURCES_IS_STDLIB} "IS_STDLIB" IS_STDLIB_arg)
  translate_flag(${LANGUAGESOURCES_IS_STDLIB_CORE} "IS_STDLIB_CORE"
                 IS_STDLIB_CORE_arg)
  translate_flag(${LANGUAGESOURCES_IS_SDK_OVERLAY} "IS_SDK_OVERLAY"
                 IS_SDK_OVERLAY_arg)
  translate_flag(${LANGUAGESOURCES_EMBED_BITCODE} "EMBED_BITCODE"
                 EMBED_BITCODE_arg)
  translate_flag(${LANGUAGESOURCES_STATIC} "STATIC"
                 STATIC_arg)
  translate_flag(${LANGUAGESOURCES_NO_LINK_NAME} "NO_LINK_NAME" NO_LINK_NAME_arg)
  translate_flag(${LANGUAGESOURCES_IS_FRAGILE} "IS_FRAGILE" IS_FRAGILE_arg)
  translate_flag(${LANGUAGESOURCES_ONLY_LANGUAGEMODULE} "ONLY_LANGUAGEMODULE" ONLY_LANGUAGEMODULE_arg)
  translate_flag(${LANGUAGESOURCES_NO_LANGUAGEMODULE} "NO_LANGUAGEMODULE" NO_LANGUAGEMODULE_arg)
  if(DEFINED LANGUAGESOURCES_BOOTSTRAPPING)
    set(BOOTSTRAPPING_arg "BOOTSTRAPPING" ${LANGUAGESOURCES_BOOTSTRAPPING})
  endif()

  if(LANGUAGESOURCES_IS_MAIN)
    set(LANGUAGESOURCES_INSTALL_IN_COMPONENT never_install)
  endif()

  if(NOT DEFINED LANGUAGESOURCES_INSTALL_BINARY_LANGUAGEMODULE)
    set(LANGUAGESOURCES_INSTALL_BINARY_LANGUAGEMODULE TRUE)
  endif()

  # Check arguments.
  precondition(LANGUAGESOURCES_SDK "Should specify an SDK")
  precondition(LANGUAGESOURCES_ARCHITECTURE "Should specify an architecture")
  precondition(LANGUAGESOURCES_INSTALL_IN_COMPONENT "INSTALL_IN_COMPONENT is required")
  if (NOT LANGUAGESOURCES_ARCHITECTURE_SUBDIR_NAME)
    set(LANGUAGESOURCES_ARCHITECTURE_SUBDIR_NAME "${LANGUAGESOURCES_ARCHITECTURE}")
  endif()

  # Clear the result variable.
  set("${dependency_target_out_var_name}" "" PARENT_SCOPE)
  set("${dependency_module_target_out_var_name}" "" PARENT_SCOPE)
  set("${dependency_sib_target_out_var_name}" "" PARENT_SCOPE)
  set("${dependency_sibopt_target_out_var_name}" "" PARENT_SCOPE)
  set("${dependency_sibgen_target_out_var_name}" "" PARENT_SCOPE)

  set(result)
  set(language_sources)
  foreach(src ${${sourcesvar}})
    get_filename_component(extension ${src} EXT)
    if(extension STREQUAL ".code")
      list(APPEND language_sources ${src})
    else()
      list(APPEND result ${src})
    endif()
  endforeach()

  set(language_compile_flags ${LANGUAGESOURCES_COMPILE_FLAGS})
  if (NOT LANGUAGESOURCES_IS_MAIN AND NOT LANGUAGESOURCES_NO_LINK_NAME)
    list(APPEND language_compile_flags "-module-link-name" "${name}")
  endif()

  if(language_sources)
    set(objsubdir "/${LANGUAGESOURCES_SDK}/${LANGUAGESOURCES_ARCHITECTURE_SUBDIR_NAME}")

    get_maccatalyst_build_flavor(maccatalyst_build_flavor
      "${LANGUAGESOURCES_SDK}" "${LANGUAGESOURCES_MACCATALYST_BUILD_FLAVOR}")
    if(maccatalyst_build_flavor STREQUAL "ios-like")
      set(objsubdir "/MACCATALYST/${LANGUAGESOURCES_ARCHITECTURE_SUBDIR_NAME}")
    endif()

    get_bootstrapping_path(lib_dir
        ${LANGUAGELIB_DIR} "${LANGUAGESOURCES_BOOTSTRAPPING}")
    get_bootstrapping_path(obj_dir
        "${CMAKE_CURRENT_BINARY_DIR}/${objsubdir}" "${LANGUAGESOURCES_BOOTSTRAPPING}")

    file(MAKE_DIRECTORY ${obj_dir})

    set(language_obj
        "${obj_dir}/${LANGUAGESOURCES_MODULE_NAME}${CMAKE_C_OUTPUT_EXTENSION}")

    # FIXME: We shouldn't /have/ to build things in a single process.
    # <rdar://problem/15972329>
    list(APPEND language_compile_flags "-whole-module-optimization")
    if(sdk IN_LIST LANGUAGE_DARWIN_PLATFORMS OR sdk STREQUAL "MACCATALYST")
      list(APPEND language_compile_flags "-save-optimization-record=bitstream")
    endif()
    if (LANGUAGESOURCES_ENABLE_LTO)
      _compute_lto_language_flag("${LANGUAGESOURCES_ENABLE_LTO}" _lto_flag_out)
      if (_lto_flag_out)
        list(APPEND language_compile_flags "${_lto_flag_out}")
      endif()
    endif()
    list(APPEND language_compile_flags "-color-diagnostics")
    _compile_language_files(
        dependency_target
        module_dependency_target
        sib_dependency_target
        sibopt_dependency_target
        sibgen_dependency_target
        OUTPUT ${language_obj}
        SOURCES ${language_sources}
        DEPENDS ${LANGUAGESOURCES_DEPENDS}
        FLAGS ${language_compile_flags}
        SDK ${LANGUAGESOURCES_SDK}
        ARCHITECTURE ${LANGUAGESOURCES_ARCHITECTURE}
        MODULE_NAME ${LANGUAGESOURCES_MODULE_NAME}
        MODULE_DIR ${LANGUAGESOURCES_MODULE_DIR}
        ${IS_MAIN_arg}
        ${IS_STDLIB_arg}
        ${IS_STDLIB_CORE_arg}
        ${IS_SDK_OVERLAY_arg}
        ${EMBED_BITCODE_arg}
        ${STATIC_arg}
        ${BOOTSTRAPPING_arg}
        ${IS_FRAGILE_arg}
        ${ONLY_LANGUAGEMODULE_arg}
        ${NO_LANGUAGEMODULE_arg}
        INSTALL_BINARY_LANGUAGEMODULE ${LANGUAGESOURCES_INSTALL_BINARY_LANGUAGEMODULE}
        INSTALL_IN_COMPONENT "${LANGUAGESOURCES_INSTALL_IN_COMPONENT}"
        DEPLOYMENT_VERSION_OSX ${LANGUAGESOURCES_DEPLOYMENT_VERSION_OSX}
        DEPLOYMENT_VERSION_IOS ${LANGUAGESOURCES_DEPLOYMENT_VERSION_IOS}
        DEPLOYMENT_VERSION_TVOS ${LANGUAGESOURCES_DEPLOYMENT_VERSION_TVOS}
        DEPLOYMENT_VERSION_WATCHOS ${LANGUAGESOURCES_DEPLOYMENT_VERSION_WATCHOS}
        MACCATALYST_BUILD_FLAVOR "${LANGUAGESOURCES_MACCATALYST_BUILD_FLAVOR}")
    set("${dependency_target_out_var_name}" "${dependency_target}" PARENT_SCOPE)
    set("${dependency_module_target_out_var_name}" "${module_dependency_target}" PARENT_SCOPE)
    set("${dependency_sib_target_out_var_name}" "${sib_dependency_target}" PARENT_SCOPE)
    set("${dependency_sibopt_target_out_var_name}" "${sibopt_dependency_target}" PARENT_SCOPE)
    set("${dependency_sibgen_target_out_var_name}" "${sibgen_dependency_target}" PARENT_SCOPE)

    list(APPEND result ${language_obj})
  endif()

  toolchain_process_sources(result ${result})
  set(${sourcesvar} "${result}" PARENT_SCOPE)
  set(${externalvar} ${language_sources} PARENT_SCOPE)
endfunction()

# Add Codira source files to the (Xcode) project.
#
# Usage:
#   add_language_source_group(sources)
function(add_language_source_group sources)
  source_group("Codira Sources" FILES ${sources})
  # Mark the source files as HEADER_FILE_ONLY, so that Xcode doesn't try
  # to build them itself.
  set_source_files_properties(${sources}
    PROPERTIES
    HEADER_FILE_ONLY true)
endfunction()

# Look up extra flags for a module that matches a regexp.
function(_add_extra_language_flags_for_module module_name result_var_name)
  set(result_list)
  list(LENGTH LANGUAGE_EXPERIMENTAL_EXTRA_REGEXP_FLAGS listlen)
  if (${listlen} GREATER 0)
    math(EXPR listlen "${listlen}-1")
    foreach(i RANGE 0 ${listlen} 2)
      list(GET LANGUAGE_EXPERIMENTAL_EXTRA_REGEXP_FLAGS ${i} regex)
      if (module_name MATCHES "${regex}")
        math(EXPR ip1 "${i}+1")
        list(GET LANGUAGE_EXPERIMENTAL_EXTRA_REGEXP_FLAGS ${ip1} flags)
        list(APPEND result_list ${flags})
        message(STATUS "Matched '${regex}' to module '${module_name}'. Compiling ${module_name} with special flags: ${flags}")
      endif()
    endforeach()
  endif()
  list(LENGTH LANGUAGE_EXPERIMENTAL_EXTRA_NEGATIVE_REGEXP_FLAGS listlen)
  if (${listlen} GREATER 0)
    math(EXPR listlen "${listlen}-1")
    foreach(i RANGE 0 ${listlen} 2)
      list(GET LANGUAGE_EXPERIMENTAL_EXTRA_NEGATIVE_REGEXP_FLAGS ${i} regex)
      if (NOT module_name MATCHES "${regex}")
        math(EXPR ip1 "${i}+1")
        list(GET LANGUAGE_EXPERIMENTAL_EXTRA_NEGATIVE_REGEXP_FLAGS ${ip1} flags)
        list(APPEND result_list ${flags})
        message(STATUS "Matched NEGATIVE '${regex}' to module '${module_name}'. Compiling ${module_name} with special flags: ${flags}")
      endif()
    endforeach()
  endif()
  set("${result_var_name}" ${result_list} PARENT_SCOPE)
endfunction()

function(_add_target_variant_language_compile_flags
    sdk arch build_type enable_assertions result_var_name)
  set(result ${${result_var_name}})

  cmake_parse_arguments(
    VARIANT             # prefix
    ""                  # options
    "MACCATALYST_BUILD_FLAVOR;DEPLOYMENT_VERSION_OSX;DEPLOYMENT_VERSION_IOS;DEPLOYMENT_VERSION_TVOS;DEPLOYMENT_VERSION_WATCHOS"  # single-value args
    ""                  # multi-value args
    ${ARGN})

  # On Windows, we don't set LANGUAGE_SDK_WINDOWS_PATH_ARCH_{ARCH}_PATH, so don't include it.
  if ((NOT "${sdk}" STREQUAL "WINDOWS") AND NOT ("${LANGUAGE_SDK_${sdk}_ARCH_${arch}_PATH}" STREQUAL ""))
    list(APPEND result "-sdk" "${LANGUAGE_SDK_${sdk}_ARCH_${arch}_PATH}")
  endif()

  if("${sdk}" IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set(sdk_deployment_version "${LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION}")
    if("${sdk}" STREQUAL "OSX" AND DEFINED VARIANT_DEPLOYMENT_VERSION_OSX)
      set(sdk_deployment_version ${VARIANT_DEPLOYMENT_VERSION_OSX})
    endif()
    if(("${sdk}" STREQUAL "IOS" OR "${sdk}" STREQUAL "IOS_SIMULATOR") AND DEFINED VARIANT_DEPLOYMENT_VERSION_IOS)
      set(sdk_deployment_version ${VARIANT_DEPLOYMENT_VERSION_IOS})
    endif()
    if(("${sdk}" STREQUAL "TVOS" OR "${sdk}" STREQUAL "TVOS_SIMULATOR") AND DEFINED VARIANT_DEPLOYMENT_VERSION_TVOS)
      set(sdk_deployment_version ${VARIANT_DEPLOYMENT_VERSION_TVOS})
    endif()
    if(("${sdk}" STREQUAL "WATCHOS" OR "${sdk}" STREQUAL "WATCHOS_SIMULATOR") AND DEFINED VARIANT_DEPLOYMENT_VERSION_WATCHOS)
      set(sdk_deployment_version ${VARIANT_DEPLOYMENT_VERSION_WATCHOS})
    endif()

    get_target_triple(target target_variant "${sdk}" "${arch}"
    MACCATALYST_BUILD_FLAVOR "${VARIANT_MACCATALYST_BUILD_FLAVOR}"
    DEPLOYMENT_VERSION "${sdk_deployment_version}")

    list(APPEND result "-target" "${target}")
    if(target_variant)
      list(APPEND result "-target-variant" "${target_variant}")
    endif()
  else()
    list(APPEND result
        "-target" "${LANGUAGE_SDK_${sdk}_ARCH_${arch}_TRIPLE}")
  endif()

  if(NOT BUILD_STANDALONE)
    list(APPEND result "-resource-dir" "${LANGUAGELIB_DIR}")
  endif()

  if("${sdk}" IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    # We collate -F with the framework path to avoid unwanted deduplication
    # of options by target_compile_options -- this way no undesired
    # side effects are introduced should a new search path be added.
    list(APPEND result
      "-F${LANGUAGE_SDK_${sdk}_ARCH_${arch}_PATH}/../../../Developer/Library/Frameworks")
  endif()

  language_optimize_flag_for_build_type("${build_type}" optimize_flag)
  list(APPEND result "${optimize_flag}")

  is_build_type_with_debuginfo("${build_type}" debuginfo)
  if(debuginfo)
    list(APPEND result "-g")
  elseif("${build_type}" STREQUAL "MinSizeRel")
    # MinSizeRel builds of stdlib (but not the compiler) should get debug info
    list(APPEND result "-g")
  endif()

  if(enable_assertions)
    list(APPEND result "-D" "INTERNAL_CHECKS_ENABLED")
  endif()

  if(LANGUAGE_STDLIB_COMPACT_ABSOLUTE_FUNCTION_POINTER)
    list(APPEND result "-D" "LANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
    list(APPEND result "-D" "LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED)
    list(APPEND result "-D" "LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED")
  endif()

  if(LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS)
    list(APPEND result "-D" "LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING)
    list(APPEND result "-D" "LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
    list(APPEND result "-D" "LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING")
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION)
    list(APPEND result "-D" "LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION")
  endif()

  if(LANGUAGE_ENABLE_SYNCHRONIZATION)
    list(APPEND result "-D" "LANGUAGE_ENABLE_SYNCHRONIZATION")
  endif()

  if(LANGUAGE_ENABLE_VOLATILE)
    list(APPEND result "-D" "LANGUAGE_ENABLE_VOLATILE")
  endif()

  if(LANGUAGE_STDLIB_OS_VERSIONING)
    list(APPEND result "-D" "LANGUAGE_RUNTIME_OS_VERSIONING")
  endif()
  
  if(LANGUAGE_STDLIB_STATIC_PRINT)
    list(APPEND result "-D" "LANGUAGE_STDLIB_STATIC_PRINT")
  endif()
  
  if(LANGUAGE_STDLIB_ENABLE_UNICODE_DATA)
    list(APPEND result "-D" "LANGUAGE_STDLIB_ENABLE_UNICODE_DATA")
  endif()
  
  if(LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES)
    list(APPEND result "-D" "LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES")
  endif()

  if(LANGUAGE_STDLIB_HAS_COMMANDLINE)
    list(APPEND result "-D" "LANGUAGE_STDLIB_HAS_COMMANDLINE")
  endif()

  if(LANGUAGE_STDLIB_HAS_STDIN)
    list(APPEND result "-D" "LANGUAGE_STDLIB_HAS_STDIN")
  endif()

  if(LANGUAGE_STDLIB_HAS_ENVIRON)
    list(APPEND result "-D" "LANGUAGE_STDLIB_HAS_ENVIRON")
    list(APPEND result "-Xcc" "-DLANGUAGE_STDLIB_HAS_ENVIRON")
  endif()

  if(LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY)
    list(APPEND result "-D" "LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY")
  endif()

  if(LANGUAGE_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY)
    list(APPEND result "-D" "LANGUAGE_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY")
  endif()

  if (LANGUAGE_CONCURRENCY_USES_DISPATCH)
    list(APPEND result "-D" "LANGUAGE_CONCURRENCY_USES_DISPATCH")
  endif()

  if(LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE)
    list(APPEND result "-D" "LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE")
  endif()

  string(TOUPPER "${LANGUAGE_SDK_${sdk}_THREADING_PACKAGE}" _threading_package)
  list(APPEND result "-D" "LANGUAGE_THREADING_${_threading_package}")

  set("${result_var_name}" "${result}" PARENT_SCOPE)
endfunction()

# Compile a language file into an object file (as a library).
#
# Usage:
#   _compile_language_files(
#     dependency_target_out_var_name
#     dependency_module_target_out_var_name
#     dependency_sib_target_out_var_name    # -Onone sib target
#     dependency_sibopt_target_out_var_name # -O sib target
#     dependency_sibgen_target_out_var_name # -sibgen target
#     OUTPUT objfile                    # Name of the resulting object file
#     SOURCES language_src [language_src...]  # Codira source files to compile
#     FLAGS -module-name foo            # Flags to add to the compilation
#     MACCATALYST_BUILD_FLAVOR flavor        # macCatalyst flavor
#     [SDK sdk]                         # SDK to build for
#     [ARCHITECTURE architecture]       # Architecture to build for
#     [DEPENDS cmake_target...]         # CMake targets on which the object
#                                       # file depends.
#     [IS_MAIN]                         # This is an executable, not a library
#     [IS_STDLIB]
#     [IS_STDLIB_CORE]                  # This is the core standard library
#     [ONLY_LANGUAGEMODULE]                # Emit languagemodule only, no binary
#     [NO_LANGUAGEMODULE]                  # Emit binary only, no languagemodule
#     [OPT_FLAGS]                       # Optimization flags (overrides LANGUAGE_OPTIMIZE)
#     [MODULE_DIR]                      # Put .codemodule, .codedoc., and .o
#                                       # into this directory.
#     [MODULE_NAME]                     # The module name.
#     [INSTALL_IN_COMPONENT]            # Install produced files.
#     [EMBED_BITCODE]                   # Embed LLVM bitcode into the .o files
#     [STATIC]                          # Also write .codemodule etc. to static
#                                       # resource folder
#     )
function(_compile_language_files
    dependency_target_out_var_name dependency_module_target_out_var_name
    dependency_sib_target_out_var_name dependency_sibopt_target_out_var_name
    dependency_sibgen_target_out_var_name)
  cmake_parse_arguments(LANGUAGEFILE
    "IS_MAIN;IS_STDLIB;IS_STDLIB_CORE;IS_SDK_OVERLAY;EMBED_BITCODE;STATIC;IS_FRAGILE;ONLY_LANGUAGEMODULE;NO_LANGUAGEMODULE"
    "OUTPUT;MODULE_NAME;INSTALL_IN_COMPONENT;DEPLOYMENT_VERSION_OSX;DEPLOYMENT_VERSION_IOS;DEPLOYMENT_VERSION_TVOS;DEPLOYMENT_VERSION_WATCHOS;MACCATALYST_BUILD_FLAVOR;BOOTSTRAPPING;INSTALL_BINARY_LANGUAGEMODULE"
    "SOURCES;FLAGS;DEPENDS;SDK;ARCHITECTURE;OPT_FLAGS;MODULE_DIR"
    ${ARGN})

  if(NOT DEFINED LANGUAGEFILE_INSTALL_BINARY_LANGUAGEMODULE)
    set(LANGUAGEFILE_INSTALL_BINARY_LANGUAGEMODULE TRUE)
  endif()

  # Check arguments.
  list(LENGTH LANGUAGEFILE_OUTPUT num_outputs)
  list(GET LANGUAGEFILE_OUTPUT 0 first_output)

  precondition(num_outputs MESSAGE "OUTPUT must not be empty")

  foreach(output ${LANGUAGEFILE_OUTPUT})
    if (NOT IS_ABSOLUTE "${output}")
      message(FATAL_ERROR "OUTPUT should be an absolute path")
    endif()
  endforeach()

  if(LANGUAGEFILE_IS_MAIN AND LANGUAGEFILE_IS_STDLIB)
    message(FATAL_ERROR "Cannot set both IS_MAIN and IS_STDLIB")
  endif()

  precondition(LANGUAGEFILE_SDK MESSAGE "Should specify an SDK")
  precondition(LANGUAGEFILE_ARCHITECTURE MESSAGE "Should specify an architecture")
  precondition(LANGUAGEFILE_INSTALL_IN_COMPONENT MESSAGE "INSTALL_IN_COMPONENT is required")

  # Determine if/what macCatalyst build variant we are
  get_maccatalyst_build_flavor(maccatalyst_build_flavor
    "${LANGUAGEFILE_SDK}" "${LANGUAGEFILE_MACCATALYST_BUILD_FLAVOR}")

  # Determine target triples
  get_target_triple(target_triple ignored_target_variant_triple
    "${LANGUAGEFILE_SDK}"
    "${LANGUAGEFILE_ARCHITECTURE}"
    DEPLOYMENT_VERSION "${LANGUAGE_SDK_${LANGUAGEFILE_SDK}_DEPLOYMENT_VERSION}")

  get_target_triple(maccatalyst_target_triple ignored_target_variant_triple
    "${LANGUAGEFILE_SDK}"
    "${LANGUAGEFILE_ARCHITECTURE}"
    DEPLOYMENT_VERSION "${LANGUAGE_SDK_${LANGUAGEFILE_SDK}_DEPLOYMENT_VERSION}"
    MACCATALYST_BUILD_FLAVOR "${maccatalyst_build_flavor}")

  # macCatalyst ios-like target triple
  if(maccatalyst_build_flavor STREQUAL "ios-like")
    set(target_triple "${maccatalyst_target_triple}")
  endif()

  if ("${LANGUAGEFILE_MODULE_NAME}" STREQUAL "")
    get_filename_component(LANGUAGEFILE_MODULE_NAME "${first_output}" NAME_WE)
    message(SEND_ERROR
      "No module name specified; did you mean to use MODULE_NAME "
      "${LANGUAGEFILE_MODULE_NAME}?")
  endif()

  set(source_files)
  foreach(file ${LANGUAGEFILE_SOURCES})
    # Determine where this file is.
    get_filename_component(file_path ${file} PATH)
    if(IS_ABSOLUTE "${file_path}")
      list(APPEND source_files "${file}")
    else()
      list(APPEND source_files "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    endif()
  endforeach()

  # Compute flags for the Codira compiler.
  set(language_flags)
  set(language_module_flags)

  _add_target_variant_language_compile_flags(
      "${LANGUAGEFILE_SDK}"
      "${LANGUAGEFILE_ARCHITECTURE}"
      "${LANGUAGE_STDLIB_BUILD_TYPE}"
      "${LANGUAGE_STDLIB_ASSERTIONS}"
      language_flags
      DEPLOYMENT_VERSION_OSX ${LANGUAGEFILE_DEPLOYMENT_VERSION_OSX}
      DEPLOYMENT_VERSION_IOS ${LANGUAGEFILE_DEPLOYMENT_VERSION_IOS}
      DEPLOYMENT_VERSION_TVOS ${LANGUAGEFILE_DEPLOYMENT_VERSION_TVOS}
      DEPLOYMENT_VERSION_WATCHOS ${LANGUAGEFILE_DEPLOYMENT_VERSION_WATCHOS}
      MACCATALYST_BUILD_FLAVOR "${maccatalyst_build_flavor}"
      )

  if(LANGUAGEFILE_STATIC)
    list(APPEND language_flags -static)
  endif()

  # Determine the subdirectory where the binary should be placed.
  set(library_subdir_sdk "${LANGUAGEFILE_SDK}")
  if(maccatalyst_build_flavor STREQUAL "ios-like")
    set(library_subdir_sdk "MACCATALYST")
  endif()

  compute_library_subdir(library_subdir
    "${library_subdir_sdk}" "${LANGUAGEFILE_ARCHITECTURE}")

  if(NOT "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}" STREQUAL "")
    list(APPEND language_flags "-tools-directory" "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}")
  endif()

  # If we have a custom module cache path, use it.
  if (LANGUAGE_MODULE_CACHE_PATH)
    list(APPEND language_flags "-module-cache-path" "${LANGUAGE_MODULE_CACHE_PATH}")
  endif()

  # Don't include libarclite in any build products by default.
  list(APPEND language_flags "-no-link-objc-runtime")

  if(LANGUAGE_SIL_VERIFY_ALL)
    list(APPEND language_flags "-Xfrontend" "-sil-verify-all")
  endif()

  if(LANGUAGE_SIL_VERIFY_ALL_MACOS_ONLY)
    # Only add if we have a macOS build triple
    if ("${LANGUAGEFILE_SDK}" STREQUAL "OSX" AND
        "${LANGUAGEFILE_ARCHITECTURE}" STREQUAL "x86_64")
      list(APPEND language_flags "-Xfrontend" "-sil-verify-all")
    endif()
  endif()

  # The standard library and overlays are built resiliently when LANGUAGE_STDLIB_STABLE_ABI=On.
  if(LANGUAGEFILE_IS_STDLIB AND NOT LANGUAGEFILE_IS_FRAGILE AND LANGUAGE_STDLIB_STABLE_ABI)
    list(APPEND language_flags "-enable-library-evolution")
    list(APPEND language_flags "-library-level" "api")
    list(APPEND language_flags "-Xfrontend" "-require-explicit-availability=ignore")
  endif()

  if("${LANGUAGE_SDK_${LANGUAGEFILE_SDK}_THREADING_PACKAGE}" STREQUAL "none")
    list(APPEND language_flags "-Xfrontend" "-assume-single-threaded")
  endif()

  if(NOT LANGUAGE_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING AND LANGUAGEFILE_IS_STDLIB)
    list(APPEND language_flags "-Xfrontend" "-enforce-exclusivity=unchecked")
  endif()

  if(LANGUAGE_EMIT_SORTED_SIL_OUTPUT)
    list(APPEND language_flags "-Xfrontend" "-emit-sorted-sil")
  endif()

  if(LANGUAGE_ENABLE_REFLECTION)
    list(APPEND language_flags "-D" "LANGUAGE_ENABLE_REFLECTION")
  endif()

  if("${LANGUAGE_STDLIB_REFLECTION_METADATA}" STREQUAL "enabled")
    # do nothing, emitting reflection metadata is the default in languagec
  elseif("${LANGUAGE_STDLIB_REFLECTION_METADATA}" STREQUAL "debugger-only")
    list(APPEND language_flags "-Xfrontend" "-reflection-metadata-for-debugger-only")
  elseif("${LANGUAGE_STDLIB_REFLECTION_METADATA}" STREQUAL "disabled")
    list(APPEND language_flags "-Xfrontend" "-disable-reflection-metadata")
  else()
    message(FATAL_ERROR "Invalid LANGUAGE_STDLIB_REFLECTION_METADATA value: ${LANGUAGE_STDLIB_REFLECTION_METADATA}")
  endif()

  if(NOT "${LANGUAGE_STDLIB_TRAP_FUNCTION}" STREQUAL "")
    list(APPEND language_flags "-Xfrontend" "-trap-function" "-Xfrontend" "${LANGUAGE_STDLIB_TRAP_FUNCTION}")
  endif()

  # FIXME: Cleaner way to do this?
  if(LANGUAGEFILE_IS_STDLIB_CORE)
    list(APPEND language_flags
        "-nostdimport" "-parse-stdlib" "-module-name" "Codira")
    list(APPEND language_flags "-Xfrontend" "-group-info-path"
                            "-Xfrontend" "${GROUP_INFO_JSON_FILE}")
  else()
    list(APPEND language_flags "-module-name" "${LANGUAGEFILE_MODULE_NAME}")
  endif()

  # Force language 5 mode for Standard Library and overlays.
  if (LANGUAGEFILE_IS_STDLIB)
    list(APPEND language_flags "-language-version" "5")
  endif()
  if (LANGUAGEFILE_IS_SDK_OVERLAY)
    list(APPEND language_flags "-language-version" "5")
  endif()

  # Avoiding emitting ABI descriptor files while building stdlib.
  if (LANGUAGEFILE_IS_STDLIB)
    list(APPEND language_flags "-Xfrontend" "-empty-abi-descriptor")
  endif()

  if(LANGUAGEFILE_IS_SDK_OVERLAY)
    list(APPEND language_flags "-autolink-force-load")
  endif()

  # Don't need to link runtime compatibility libraries for older runtimes 
  # into the new runtime.
  if (LANGUAGEFILE_IS_STDLIB OR LANGUAGEFILE_IS_SDK_OVERLAY)
    list(APPEND language_flags "-runtime-compatibility-version" "none")
    list(APPEND language_flags "-disable-autolinking-runtime-compatibility-dynamic-replacements")
    list(APPEND language_flags "-Xfrontend" "-disable-autolinking-runtime-compatibility-concurrency")
  endif()

  if(NOT LANGUAGE_STDLIB_SHORT_MANGLING_LOOKUPS)
    list(APPEND language_flags "-Xfrontend" "-disable-standard-substitutions-in-reflection-mangling")
  endif()

  if(NOT LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP)
    list(APPEND language_flags "-Xfrontend" "-disable-objc-interop")
  endif()

  if(LANGUAGE_STDLIB_EXPERIMENTAL_HERMETIC_SEAL_AT_LINK)
    list(APPEND language_flags "-experimental-hermetic-seal-at-link")
  endif()

  list(APPEND language_flags "-enable-experimental-feature" "NoncopyableGenerics2")
  list(APPEND language_flags "-enable-experimental-feature" "SuppressedAssociatedTypes")
  list(APPEND language_flags "-enable-experimental-feature" "SE427NoInferenceOnExtension")

  list(APPEND language_flags "-enable-experimental-feature" "NonescapableTypes")
  list(APPEND language_flags "-enable-experimental-feature" "LifetimeDependence")
  list(APPEND language_flags "-enable-experimental-feature" "InoutLifetimeDependence")
  list(APPEND language_flags "-enable-experimental-feature" "LifetimeDependenceMutableAccessors")

  list(APPEND language_flags "-enable-upcoming-feature" "MemberImportVisibility")

  if (LANGUAGE_STDLIB_ENABLE_STRICT_CONCURRENCY_COMPLETE)
    list(APPEND language_flags "-strict-concurrency=complete")
  endif()

  if (LANGUAGE_STDLIB_USE_RELATIVE_PROTOCOL_WITNESS_TABLES)
    list(APPEND language_flags "-Xfrontend" "-enable-relative-protocol-witness-tables")
    list(APPEND language_flags "-Xfrontend" "-language-async-frame-pointer=never")
  endif()

  if (LANGUAGE_STDLIB_USE_FRAGILE_RESILIENT_PROTOCOL_WITNESS_TABLES)
    list(APPEND language_flags "-Xfrontend" "-enable-fragile-relative-protocol-tables")
  endif()

  if(LANGUAGE_STDLIB_DISABLE_INSTANTIATION_CACHES)
    list(APPEND language_flags "-Xfrontend" "-disable-preallocated-instantiation-caches")
  endif()

  list(APPEND language_flags ${LANGUAGE_STDLIB_EXTRA_LANGUAGE_COMPILE_FLAGS})

  list(APPEND language_flags ${LANGUAGE_EXPERIMENTAL_EXTRA_FLAGS})

  if(LANGUAGEFILE_OPT_FLAGS)
    list(APPEND language_flags ${LANGUAGEFILE_OPT_FLAGS})
  endif()

  list(APPEND language_flags ${LANGUAGEFILE_FLAGS})

  set(dirs_to_create)
  foreach(output ${LANGUAGEFILE_OUTPUT})
    get_filename_component(objdir "${output}" PATH)
    list(APPEND dirs_to_create "${objdir}")
  endforeach()

  set(module_file)
  set(module_file_static)
  set(module_doc_file)
  set(module_doc_file_static)
  set(interface_file)
  set(interface_file_static)

  get_bootstrapping_path(lib_dir ${LANGUAGELIB_DIR} "${LANGUAGEFILE_BOOTSTRAPPING}")

  if(NOT LANGUAGEFILE_IS_MAIN)
    list(APPEND language_flags "-parse-as-library")
  endif()

  if(NOT LANGUAGEFILE_IS_MAIN AND NOT LANGUAGEFILE_NO_LANGUAGEMODULE)
    # Determine the directory where the module file should be placed.
    if(LANGUAGEFILE_MODULE_DIR)
      set(module_dir "${LANGUAGEFILE_MODULE_DIR}")
    elseif(LANGUAGEFILE_IS_STDLIB)
      set(module_dir "${lib_dir}/${library_subdir}")
    else()
      message(FATAL_ERROR "Don't know where to put the module files")
    endif()

    set(module_base "${module_dir}/${LANGUAGEFILE_MODULE_NAME}")

    set(module_dir_static "${LANGUAGESTATICLIB_DIR}/${library_subdir}")
    set(module_base_static "${module_dir_static}/${LANGUAGEFILE_MODULE_NAME}")

    set(module_triple ${LANGUAGE_SDK_${library_subdir_sdk}_ARCH_${LANGUAGEFILE_ARCHITECTURE}_MODULE})
    set(specific_module_dir "${module_base}.codemodule")
    set(module_base "${module_base}.codemodule/${module_triple}")
    set(specific_module_dir_static "${module_base_static}.codemodule")
    set(module_base_static "${module_base_static}.codemodule/${module_triple}")
    set(module_file "${module_base}.codemodule")
    set(module_doc_file "${module_base}.codedoc")

    set(module_file_static "${module_base_static}.codemodule")
    set(module_doc_file_static "${module_base_static}.codedoc")

    # FIXME: These don't really belong inside the languagemodule, but there's not
    # an obvious alternate place to put them.
    set(sib_file "${module_base}.Onone.sib")
    set(sibopt_file "${module_base}.O.sib")
    set(sibgen_file "${module_base}.sibgen")

    if(LANGUAGE_ENABLE_MODULE_INTERFACES AND NOT LANGUAGEFILE_IS_FRAGILE)
      set(interface_file "${module_base}.codeinterface")
      set(interface_file_static "${module_base_static}.codeinterface")
      set(private_interface_file "${module_base}.private.codeinterface")
      set(private_interface_file_static "${module_base_static}.private.codeinterface")
      list(APPEND language_module_flags
           "-emit-module-interface-path" "${interface_file}"
           "-emit-private-module-interface-path" "${private_interface_file}")
    endif()

    if(LANGUAGE_STDLIB_EMIT_API_DESCRIPTORS AND NOT LANGUAGEFILE_IS_FRAGILE)
      set(api_descriptor_file "${module_base}.api.json")
      list(APPEND language_module_flags
            "-emit-api-descriptor-path" "${api_descriptor_file}")
    endif()

    if (NOT LANGUAGEFILE_IS_STDLIB_CORE)
      list(APPEND language_module_flags
           "-Xfrontend" "-experimental-skip-non-inlinable-function-bodies")
    endif()

    set(module_outputs "${module_file}" "${module_doc_file}")

    if(interface_file)
      list(APPEND module_outputs "${interface_file}" "${private_interface_file}")
    endif()

    set(exclude_binary_languagemodule_installation_args "")
    if(NOT LANGUAGEFILE_INSTALL_BINARY_LANGUAGEMODULE OR
       (LANGUAGEFILE_INSTALL_BINARY_LANGUAGEMODULE STREQUAL "NON_DARWIN_ONLY" AND
        LANGUAGEFILE_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS))
        list(APPEND
          exclude_binary_languagemodule_installation_args
          "REGEX" "${LANGUAGEFILE_MODULE_NAME}.codemodule/[^/]*\\.codemodule$" EXCLUDE)
    endif()

    # macCatalyst zippered module setup
    if(maccatalyst_build_flavor STREQUAL "zippered")
      compute_library_subdir(maccatalyst_library_subdir
        "MACCATALYST" "${LANGUAGEFILE_ARCHITECTURE}")

      if(LANGUAGEFILE_MODULE_DIR)
        set(maccatalyst_module_dir "${LANGUAGEFILE_MODULE_DIR}")
      elseif(LANGUAGEFILE_IS_STDLIB)
        set(maccatalyst_module_dir "${lib_dir}/${maccatalyst_library_subdir}")
      else()
        message(FATAL_ERROR "Don't know where to put the module files")
      endif()

      set(maccatalyst_module_base "${maccatalyst_module_dir}/${LANGUAGEFILE_MODULE_NAME}")

      set(maccatalyst_module_dir_static "${LANGUAGESTATICLIB_DIR}/${maccatalyst_library_subdir}")
      set(maccatalyst_module_base_static "${maccatalyst_module_dir_static}/${LANGUAGEFILE_MODULE_NAME}")

      set(maccatalyst_module_triple ${LANGUAGE_SDK_MACCATALYST_ARCH_${LANGUAGEFILE_ARCHITECTURE}_MODULE})
      set(maccatalyst_specific_module_dir "${maccatalyst_module_base}.codemodule")
      set(maccatalyst_module_base "${maccatalyst_module_base}.codemodule/${maccatalyst_module_triple}")
      set(maccatalyst_specific_module_dir_static "${maccatalyst_module_base_static}.codemodule")
      set(maccatalyst_module_base_static "${maccatalyst_module_base_static}.codemodule/${maccatalyst_module_triple}")
      set(maccatalyst_module_file "${maccatalyst_module_base}.codemodule")
      set(maccatalyst_module_doc_file "${maccatalyst_module_base}.codedoc")

      set(maccatalyst_module_file_static "${maccatalyst_module_base_static}.codemodule")
      set(maccatalyst_module_doc_file_static "${maccatalyst_module_base_static}.codedoc")

      set(maccatalyst_module_outputs "${maccatalyst_module_file}" "${maccatalyst_module_doc_file}")
      set(maccatalyst_module_outputs_static "${maccatalyst_module_file_static}" "${maccatalyst_module_doc_file_static}")

      if(LANGUAGE_ENABLE_MODULE_INTERFACES AND NOT LANGUAGEFILE_IS_FRAGILE)
        set(maccatalyst_interface_file "${maccatalyst_module_base}.codeinterface")
        set(maccatalyst_interface_file_static "${maccatalyst_module_base_static}.codeinterface")
        set(maccatalyst_private_interface_file "${maccatalyst_module_base}.private.codeinterface")
        set(maccatalyst_private_interface_file_static "${maccatalyst_module_base_static}.private.codeinterface")
        list(APPEND maccatalyst_module_outputs "${maccatalyst_interface_file}" "${maccatalyst_private_interface_file}")
        list(APPEND maccatalyst_module_outputs_static "${maccatalyst_interface_file_static}" "${maccatalyst_private_interface_file_static}")
      else()
        set(maccatalyst_interface_file)
        set(maccatalyst_private_interface_file)
        set(maccatalyst_interface_file_static)
        set(maccatalyst_private_interface_file_static)
      endif()

      if(LANGUAGE_STDLIB_EMIT_API_DESCRIPTORS AND NOT LANGUAGEFILE_IS_FRAGILE)
        set(maccatalyst_api_descriptor_file "${maccatalyst_module_base}.api.json")
        list(APPEND maccatalyst_module_outputs "${maccatalyst_api_descriptor_file}")
      else()
        set(maccatalyst_api_descriptor_file)
      endif()

      language_install_in_component(DIRECTORY ${maccatalyst_specific_module_dir}
                                 DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${maccatalyst_library_subdir}"
                                 COMPONENT "${LANGUAGEFILE_INSTALL_IN_COMPONENT}"
                                 OPTIONAL
                                 PATTERN "Project" EXCLUDE
                                 ${exclude_binary_languagemodule_installation_args})

      if(LANGUAGEFILE_STATIC)
        language_install_in_component(DIRECTORY ${maccatalyst_specific_module_dir_static}
                                   DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language_static/${maccatalyst_library_subdir}"
                                   COMPONENT "${LANGUAGEFILE_INSTALL_IN_COMPONENT}"
                                   OPTIONAL
                                   PATTERN "Project" EXCLUDE
                                   ${exclude_binary_languagemodule_installation_args})
      endif()
    endif()

    # If we have extra regexp flags, check if we match any of the regexps. If so
    # add the relevant flags to our language_flags.
    if (LANGUAGE_EXPERIMENTAL_EXTRA_REGEXP_FLAGS OR LANGUAGE_EXPERIMENTAL_EXTRA_NEGATIVE_REGEXP_FLAGS)
      set(extra_language_flags_for_module)
      _add_extra_language_flags_for_module("${LANGUAGEFILE_MODULE_NAME}" extra_language_flags_for_module)
      if (extra_language_flags_for_module)
        list(APPEND language_flags ${extra_language_flags_for_module})
      endif()
    endif()
  endif()

  set(module_outputs "${module_file}" "${module_doc_file}")
  set(module_outputs_static "${module_file_static}" "${module_doc_file_static}")
  if(interface_file)
    list(APPEND module_outputs "${interface_file}" "${private_interface_file}")
    list(APPEND module_outputs_static "${interface_file_static}" "${private_interface_file_static}")
  endif()
  if(api_descriptor_file)
      list(APPEND module_outputs "${api_descriptor_file}")
  endif()

  language_install_in_component(DIRECTORY "${specific_module_dir}"
                             DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${library_subdir}"
                             COMPONENT "${LANGUAGEFILE_INSTALL_IN_COMPONENT}"
                             OPTIONAL
                             PATTERN "Project" EXCLUDE
                             ${exclude_binary_languagemodule_installation_args})

  if(LANGUAGEFILE_STATIC)
    language_install_in_component(DIRECTORY "${specific_module_dir_static}"
                               DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language_static/${library_subdir}"
                               COMPONENT "${LANGUAGEFILE_INSTALL_IN_COMPONENT}"
                               OPTIONAL
                               PATTERN "Project" EXCLUDE
                               ${exclude_binary_languagemodule_installation_args})
  endif()

  set(line_directive_tool "${LANGUAGE_SOURCE_DIR}/utils/line-directive")

  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(HOST_EXECUTABLE_SUFFIX .exe)
  endif()
  if(NOT LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
    # This is only for bootstrapping purposes. The just-built Codira is very
    # limited and only built for the builder to build the next stages with
    # hosttools.
    set(language_compiler_tool "${Codira_BINARY_DIR}/bin/languagec")
  elseif(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER)
    if(LANGUAGE_PREBUILT_LANGUAGE)
      set(language_compiler_tool "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/languagec${HOST_EXECUTABLE_SUFFIX}")
    elseif(CMAKE_Codira_COMPILER)
      set(language_compiler_tool "${CMAKE_Codira_COMPILER}")
    else()
      message(ERROR "Must pass in prebuilt tools using LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH or set CMAKE_Codira_COMPILER")
    endif()
  else()
    set(language_compiler_tool "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/languagec${HOST_EXECUTABLE_SUFFIX}")

    get_bootstrapping_path(language_compiler_tool
        "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/languagec${HOST_EXECUTABLE_SUFFIX}"
        "${LANGUAGEFILE_BOOTSTRAPPING}")

    if(NOT "${LANGUAGEFILE_BOOTSTRAPPING}" STREQUAL "")
      set(target_suffix "-bootstrapping${LANGUAGEFILE_BOOTSTRAPPING}")
    endif()

    if(NOT BUILD_STANDALONE)
      list(APPEND language_flags "-resource-dir" "${lib_dir}")
    endif()
  endif()

  set(language_compiler_tool_dep)
  if(LANGUAGE_INCLUDE_TOOLS AND NOT BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
    # Depend on the binary itself, in addition to the symlink, unless
    # cross-compiling the compiler.
    list(APPEND language_compiler_tool_dep "language-frontend${target_suffix}")

    if(LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
      # If we aren't cross compiling, also depend on CodiraMacros.
      list(APPEND language_compiler_tool_dep CodiraMacros)
    endif()
  endif()

  # If there are more than one output files, we assume that they are specified
  # otherwise e.g. with an output file map.
  set(output_option)
  if (${num_outputs} EQUAL 1)
    set(output_option "-o" ${first_output})
  endif()

  set(embed_bitcode_option)
  if (LANGUAGEFILE_EMBED_BITCODE)
    set(embed_bitcode_option "-embed-bitcode")
  endif()

  set(main_command "-c")
  if (LANGUAGE_CHECK_INCREMENTAL_COMPILATION)
    set(language_compiler_tool "${LANGUAGE_SOURCE_DIR}/utils/check-incremental" "${language_compiler_tool}")
  endif()

  set(custom_env "PYTHONIOENCODING=UTF8")
  if(LANGUAGE_INCLUDE_TOOLS AND (LANGUAGEFILE_IS_STDLIB OR
     # Linux "hosttools" build require builder's runtime before building the runtime.
     (BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS" AND LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD"))
  )
    get_bootstrapping_language_lib_dir(bs_lib_dir "${LANGUAGEFILE_BOOTSTRAPPING}")
    if(bs_lib_dir)
      # When building the stdlib with bootstrapping, the compiler needs
      # to pick up the stdlib from the previous bootstrapping stage, because the
      # stdlib in the current stage is not built yet.
      if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_APPLE_PLATFORMS)
        list(APPEND custom_env "DYLD_LIBRARY_PATH=${bs_lib_dir}")
      elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
        list(APPEND custom_env "LD_LIBRARY_PATH=${bs_lib_dir}")
      endif()
    endif()
  endif()
  set(set_environment_args "${CMAKE_COMMAND}" "-E" "env" "${custom_env}")

  if (LANGUAGE_REPORT_STATISTICS)
    list(GET dirs_to_create 0 first_obj_dir)
    list(APPEND language_flags "-stats-output-dir" ${first_obj_dir})
  endif()

  set(standard_outputs ${LANGUAGEFILE_OUTPUT})
  set(sib_outputs "${sib_file}")
  set(sibopt_outputs "${sibopt_file}")
  set(sibgen_outputs "${sibgen_file}")

  # macCatalyst zippered languagemodule
  if(maccatalyst_build_flavor STREQUAL "zippered")
    set(maccatalyst_language_flags "${language_flags}")
    list(APPEND maccatalyst_language_flags
      "-I" "${lib_dir}/${maccatalyst_library_subdir}")
    set(maccatalyst_language_module_flags ${language_module_flags})

    # Remove original interface file
    list(FIND maccatalyst_language_module_flags "${interface_file}" interface_file_index)
    if(NOT interface_file_index EQUAL -1)
      list(INSERT maccatalyst_language_module_flags ${interface_file_index} "${maccatalyst_interface_file}")
      math(EXPR old_interface_file_index "${interface_file_index} + 1")
      list(REMOVE_AT maccatalyst_language_module_flags ${old_interface_file_index})
    endif()

    # Remove original private interface
    list(FIND maccatalyst_language_module_flags "${private_interface_file}" private_interface_file_index)
    if(NOT private_interface_file_index EQUAL -1)
      list(INSERT maccatalyst_language_module_flags ${private_interface_file_index} "${maccatalyst_private_interface_file}")
      math(EXPR old_interface_file_index "${private_interface_file_index} + 1")
      list(REMOVE_AT maccatalyst_language_module_flags ${old_interface_file_index})
    endif()

    # Remove original api descriptor
    list(FIND maccatalyst_language_module_flags "${api_descriptor_file}" api_descriptor_file_index)
    if(NOT api_descriptor_file_index EQUAL -1)
      list(INSERT maccatalyst_language_module_flags ${api_descriptor_file_index} "${maccatalyst_api_descriptor_file}")
      math(EXPR old_api_descriptor_file_index "${api_descriptor_file_index} + 1")
      list(REMOVE_AT maccatalyst_language_module_flags ${old_api_descriptor_file_index})
    endif()

    # We still need to change the main language flags
    # so we can use the correct modules
    # when building for macOS
    list(APPEND language_flags
      "-I" "${lib_dir}/${library_subdir}")
  elseif(maccatalyst_build_flavor STREQUAL "ios-like")
    compute_library_subdir(maccatalyst_library_subdir
      "MACCATALYST" "${LANGUAGEFILE_ARCHITECTURE}")
    list(APPEND language_flags
      "-I" "${lib_dir}/${maccatalyst_library_subdir}")
  else()
    # Allow import of other Codira modules we just built.
    list(APPEND language_flags
      "-I" "${lib_dir}/${library_subdir}")

    # FIXME: should we use '-resource-dir' here?  Seems like it has no advantage
    # over '-I' in this case.
  endif()

  list(REMOVE_DUPLICATES dirs_to_create)

  # Then we can compile both the object files and the languagemodule files
  # in parallel in this target for the object file, and ...

  # Windows doesn't support long command line paths, of 8191 chars or over. We
  # need to work around this by avoiding long command line arguments. This can
  # be achieved by writing the list of file paths to a file, then reading that
  # list in the Python script.
  string(REPLACE ";" "\n" source_files_quoted "${source_files}")
  string(SHA1 file_name "${source_files_quoted}")
  set(file_path_target "filelist-${file_name}")
  set(file_path "${CMAKE_CURRENT_BINARY_DIR}/${file_name}.txt")

  if (NOT TARGET ${file_path_target})
    file(WRITE "${file_path}.tmp" "${source_files_quoted}")
    add_custom_command_target(unused_var
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file_path}.tmp" "${file_path}"
      CUSTOM_TARGET_NAME ${file_path_target}
      OUTPUT "${file_path}"
      DEPENDS "${file_path}.tmp")
  endif()

  # If this platform/architecture combo supports backward deployment to old
  # Objective-C runtimes, we need to copy a YAML file with legacy type layout
  # information to the build directory so that the compiler can find it.
  #
  # See stdlib/CMakeLists.txt and TypeConverter::TypeConverter() in
  # lib/IRGen/GenType.cpp.
  if(LANGUAGEFILE_IS_STDLIB_CORE AND LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
    set(LANGUAGEFILE_PLATFORM "${LANGUAGE_SDK_${LANGUAGEFILE_SDK}_LIB_SUBDIR}")
    set(copy_legacy_layouts_dep
        "copy-legacy-layouts-${LANGUAGEFILE_PLATFORM}-${LANGUAGEFILE_ARCHITECTURE}${target_suffix}")
  else()
    set(copy_legacy_layouts_dep)
  endif()

  if(NOT LANGUAGEFILE_ONLY_LANGUAGEMODULE)
  add_custom_command_target(
      dependency_target
      COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
      COMMAND
        ${set_environment_args}
        "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
        "${language_compiler_tool}" "${main_command}" ${language_flags}
        ${output_option} ${embed_bitcode_option} "@${file_path}"
      ${command_touch_standard_outputs}
      OUTPUT ${standard_outputs}
      DEPENDS
        "${line_directive_tool}"
        "${file_path_target}"
        ${language_compiler_tool_dep}
        ${source_files} ${LANGUAGEFILE_DEPENDS}
        ${language_ide_test_dependency}
        ${copy_legacy_layouts_dep}
      COMMENT "Compiling ${first_output}")
  set("${dependency_target_out_var_name}" "${dependency_target}" PARENT_SCOPE)
  endif()

  # This is the target to generate:
  #
  # 1. *.codemodule
  # 2. *.codedoc
  # 3. *.codeinterface
  # 4. *.Onone.sib
  # 5. *.O.sib
  # 6. *.sibgen
  #
  # Only 1,2,3 are built by default. 4,5,6 are utility targets for use by
  # engineers and thus even though the targets are generated, the targets are
  # not built by default.
  #
  # We only build these when we are not producing a main file. We could do this
  # with sib/sibgen, but it is useful for looking at the stdlib.
  if (NOT LANGUAGEFILE_IS_MAIN AND NOT LANGUAGEFILE_NO_LANGUAGEMODULE)
    add_custom_command_target(
        module_dependency_target
        COMMAND
          "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "remove" "-f" ${module_outputs}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "make_directory" ${module_dir}
          ${specific_module_dir}
        COMMAND
          ${set_environment_args}
          "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
          "${language_compiler_tool}" "-emit-module" "-o" "${module_file}"
          "-avoid-emit-module-source-info"
          ${language_flags} ${language_module_flags} "@${file_path}"
        ${command_touch_module_outputs}
        OUTPUT ${module_outputs}
        DEPENDS
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files} ${LANGUAGEFILE_DEPENDS}
          ${language_ide_test_dependency}
          ${copy_legacy_layouts_dep}
        COMMENT "Generating ${module_file}")

    if(LANGUAGEFILE_STATIC)
      set(command_copy_interface_file)
      if(interface_file)
        set(command_copy_interface_file
          COMMAND "${CMAKE_COMMAND}" "-E" "copy" ${interface_file} ${interface_file_static}
          COMMAND "${CMAKE_COMMAND}" "-E" "copy" ${private_interface_file} ${private_interface_file_static})
      endif()
      add_custom_command_target(
        module_dependency_target_static
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "make_directory" ${module_dir_static}
          ${specific_module_dir_static}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "copy" ${module_file} ${module_file_static}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "copy" ${module_doc_file} ${module_doc_file_static}
        ${command_copy_interface_file}
        OUTPUT ${module_outputs_static}
        DEPENDS
          "${module_dependency_target}"
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files} ${LANGUAGEFILE_DEPENDS}
          ${language_ide_test_dependency}
          ${copy_legacy_layouts_dep}
        COMMENT "Generating ${module_file}")
      set("${dependency_module_target_out_var_name}" "${module_dependency_target_static}" PARENT_SCOPE)
    else()
      set("${dependency_module_target_out_var_name}" "${module_dependency_target}" PARENT_SCOPE)
    endif()

    # macCatalyst zippered languagemodule
    if(maccatalyst_build_flavor STREQUAL "zippered")
      get_target_triple(ios_like_target_triple ignored_target_variant
        "${LANGUAGEFILE_SDK}"
        "${LANGUAGEFILE_ARCHITECTURE}"
        MACCATALYST_BUILD_FLAVOR "ios-like")

      # Remove previous -target <triple> and -target-variant flags from
      # the zippered Codira flags and add an ios-like target.
      remove_given_flag(maccatalyst_language_flags "target")
      remove_given_flag(maccatalyst_language_flags "target-variant")
      list(APPEND maccatalyst_language_flags
        "-target" "${ios_like_target_triple}")

      add_custom_command_target(
        maccatalyst_module_dependency_target
        COMMAND
          "${CMAKE_COMMAND}" "-E" "remove" "-f" ${maccatalyst_module_outputs}
        COMMAND
          "${CMAKE_COMMAND}" "-E" "make_directory" ${maccatalyst_module_dir} ${maccatalyst_specific_module_dir}
        COMMAND
          ${set_environment_args}
          "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
          "${language_compiler_tool}" "-emit-module" "-o" "${maccatalyst_module_file}"
          ${maccatalyst_language_flags} ${maccatalyst_language_module_flags} "@${file_path}"
        ${command_touch_maccatalyst_module_outputs}
        OUTPUT
          ${maccatalyst_module_outputs}
        DEPENDS
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files}
          ${LANGUAGEFILE_DEPENDS}
          ${language_ide_test_dependency}
          ${obj_dirs_dependency_target}
          ${copy_legacy_layouts_dep}
        COMMENT
          "Generating ${maccatalyst_module_file}")

      if(LANGUAGEFILE_STATIC)
        set(maccatalyst_command_copy_interface_file)
        if(maccatalyst_interface_file)
          set(maccatalyst_command_copy_interface_file
            COMMAND "${CMAKE_COMMAND}" "-E" "copy" ${maccatalyst_interface_file} ${maccatalyst_interface_file_static}
            COMMAND "${CMAKE_COMMAND}" "-E" "copy" ${maccatalyst_private_interface_file} ${maccatalyst_private_interface_file_static})
        endif()
        add_custom_command_target(
          maccatalyst_module_dependency_target_static
          COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
          COMMAND
            "${CMAKE_COMMAND}" "-E" "make_directory" ${maccatalyst_module_dir_static}
            ${maccatalyst_specific_module_dir_static}
          COMMAND
            "${CMAKE_COMMAND}" "-E" "copy" ${maccatalyst_module_file} ${maccatalyst_module_file_static}
          COMMAND
            "${CMAKE_COMMAND}" "-E" "copy" ${maccatalyst_module_doc_file} ${maccatalyst_module_doc_file_static}
          ${maccatalyst_command_copy_interface_file}
          OUTPUT ${maccatalyst_module_outputs_static}
          DEPENDS
            "${maccatalyst_module_dependency_target}"
            "${line_directive_tool}"
            "${file_path_target}"
            ${language_compiler_tool_dep}
            ${source_files} ${LANGUAGEFILE_DEPENDS}
            ${language_ide_test_dependency}
            ${copy_legacy_layouts_dep}
          COMMENT "Generating ${maccatalyst_module_file_static}")
        set("${dependency_module_target_out_var_name}" "${module_dependency_target_static}" "${maccatalyst_module_dependency_target_static}" PARENT_SCOPE)
      else()
        set("${dependency_module_target_out_var_name}" "${module_dependency_target}" "${maccatalyst_module_dependency_target}" PARENT_SCOPE)
      endif()
    endif()

    # This is the target to generate the .sib files. It is not built by default.
    add_custom_command_target(
        sib_dependency_target
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
        COMMAND
          ${set_environment_args}
          "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
          "${language_compiler_tool}" "-emit-sib" "-o" "${sib_file}" ${language_flags} -Onone
          "@${file_path}"
        ${command_touch_sib_outputs}
        OUTPUT ${sib_outputs}
        DEPENDS
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files} ${LANGUAGEFILE_DEPENDS}
          ${copy_legacy_layouts_dep}
        COMMENT "Generating ${sib_file}"
        EXCLUDE_FROM_ALL)
    set("${dependency_sib_target_out_var_name}" "${sib_dependency_target}" PARENT_SCOPE)

    add_custom_command_target(
        sibopt_dependency_target
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
        COMMAND
          ${set_environment_args}
          "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
          "${language_compiler_tool}" "-emit-sib" "-o" "${sibopt_file}" ${language_flags} -O
          "@${file_path}"
        ${command_touch_sibopt_outputs}
        OUTPUT ${sibopt_outputs}
        DEPENDS
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files} ${LANGUAGEFILE_DEPENDS}
          ${copy_legacy_layouts_dep}
        COMMENT "Generating ${sibopt_file}"
        EXCLUDE_FROM_ALL)
    set("${dependency_sibopt_target_out_var_name}" "${sibopt_dependency_target}" PARENT_SCOPE)

    # This is the target to generate the .sibgen files. It is not built by default.
    add_custom_command_target(
        sibgen_dependency_target
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${dirs_to_create}
        COMMAND
          ${set_environment_args}
          "$<TARGET_FILE:Python3::Interpreter>" "${line_directive_tool}" "@${file_path}" --
          "${language_compiler_tool}" "-emit-sibgen" "-o" "${sibgen_file}" ${language_flags}
          "@${file_path}"
        ${command_touch_sibgen_outputs}
        OUTPUT ${sibgen_outputs}
        DEPENDS
          "${line_directive_tool}"
          "${file_path_target}"
          ${language_compiler_tool_dep}
          ${source_files} ${LANGUAGEFILE_DEPENDS}
          ${copy_legacy_layouts_dep}
          COMMENT "Generating ${sibgen_file}"
          EXCLUDE_FROM_ALL)
    set("${dependency_sibgen_target_out_var_name}" "${sibgen_dependency_target}" PARENT_SCOPE)
  endif()


  # Make sure the build system knows the file is a generated object file.
  set_source_files_properties(${LANGUAGEFILE_OUTPUT}
      PROPERTIES
      GENERATED true
      EXTERNAL_OBJECT true
      LANGUAGE C
      OBJECT_DEPENDS "${source_files}")
endfunction()
