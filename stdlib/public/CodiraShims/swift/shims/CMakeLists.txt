set(sources
  AssertionReporting.h
  CoreFoundationShims.h
  EmbeddedShims.h
  FoundationShims.h
  GlobalObjects.h
  HeapObject.h
  KeyPath.h
  LibcOverlayShims.h
  LibcShims.h
  MetadataSections.h
  ObjCShims.h
  Random.h
  RefCount.h
  Reflection.h
  RuntimeShims.h
  RuntimeStubs.h
  CodiraStdbool.h
  CodiraStddef.h
  CodiraStdint.h
  System.h
  Target.h
  ThreadLocalStorage.h
  UnicodeData.h
  Visibility.h
  _CodiraConcurrency.h
  _CodiraDistributed.h
  _SynchronizationShims.h

  module.modulemap
  )
set(output_dir "${LANGUAGELIB_DIR}/shims")
set(output_dir_static "${LANGUAGESTATICLIB_DIR}/shims")

add_custom_command(
    OUTPUT "${output_dir}"
    COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${output_dir}")

add_custom_command(
    OUTPUT "${output_dir_static}"
    COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${output_dir_static}")

set(outputs)
foreach(input ${sources})
  add_custom_command(
      OUTPUT "${output_dir}/${input}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
      COMMAND
        "${CMAKE_COMMAND}" "-E" "copy_if_different"
        "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
        "${output_dir}/${input}"
      COMMENT "Copying ${input} to ${output_dir}")
  list(APPEND outputs "${output_dir}/${input}")

  add_custom_command(
    OUTPUT "${output_dir_static}/${input}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
    COMMAND
      "${CMAKE_COMMAND}" "-E" "copy_if_different"
      "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
      "${output_dir_static}/${input}"
    COMMENT "Copying ${input} to ${output_dir_static}")
  list(APPEND outputs "${output_dir_static}/${input}")
endforeach()
if (LANGUAGE_STDLIB_INSTALL_PARENT_MODULE_FOR_SHIMS)
  # Copy the module map into the parent directory. Using "parent.modulemap"
  # instead of putting "module.modulemap" into the parent module to avoid
  # module redefinition conflicts with the toolchain.
  add_custom_command(
      OUTPUT "${output_dir}/../module.modulemap"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parent.modulemap"
      COMMAND
        "${CMAKE_COMMAND}" "-E" "copy_if_different"
        "${CMAKE_CURRENT_SOURCE_DIR}/parent.modulemap"
        "${output_dir}/../module.modulemap"
      COMMENT "Copying parent.modulemap to ${output_dir}")
  list(APPEND outputs "${output_dir}/../module.modulemap")
  if(LANGUAGE_BUILD_STATIC_STDLIB)
    add_custom_command(
        OUTPUT "${output_dir_static}/../module.modulemap"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parent.modulemap"
        COMMAND
          "${CMAKE_COMMAND}" "-E" "copy_if_different"
          "${CMAKE_CURRENT_SOURCE_DIR}/parent.modulemap"
          "${output_dir_static}/../module.modulemap"
        COMMENT "Copying parent.modulemap to ${output_dir_static}")
    list(APPEND outputs "${output_dir_static}/../module.modulemap")
  endif()
endif()
# Put the output dir itself last so that it isn't considered the primary output.
list(APPEND outputs "${output_dir}")

add_custom_target("copy_shim_headers" ALL
    DEPENDS "${outputs}"
    COMMENT "Copying CodiraShims module to ${output_dir}")

if (NOT CLANG_VERSION_MAJOR)
  if (NOT TOOLCHAIN_VERSION_MAJOR)
    message(FATAL_ERROR
            "CLANG_VERSION_MAJOR or TOOLCHAIN_VERSION_MAJOR must be set \
             in order to infer the path to clang headers")
  else()
    message(WARNING
            "CLANG_VERSION_MAJOR is not defined, falling back to \
             TOOLCHAIN_VERSION_MAJOR to infer the path to clang headers")
    set(CLANG_VERSION_MAJOR "${TOOLCHAIN_VERSION_MAJOR}")
  endif()
endif()

# Symlink in the Clang headers from either the provided clang...
if(NOT LANGUAGE_INCLUDE_TOOLS AND
   (LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER OR LANGUAGE_PREBUILT_CLANG))
  if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
    execute_process(COMMAND ${CMAKE_C_COMPILER} /clang:-print-resource-dir
      OUTPUT_VARIABLE clang_headers_location
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
  else()
    execute_process(COMMAND ${CMAKE_C_COMPILER} -print-resource-dir
      OUTPUT_VARIABLE clang_headers_location
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
  endif()
  message(STATUS "Using clang Resource Directory: ${clang_headers_location}")
else()
  # ... or the one we just built
  set(clang_headers_location "${TOOLCHAIN_LIBRARY_OUTPUT_INTDIR}/clang/${CLANG_VERSION_MAJOR}")
endif()

add_custom_command_target(unused_var
    COMMAND
      "${CMAKE_COMMAND}" "-E" "make_directory" "${LANGUAGELIB_DIR}"
    COMMAND
      "${CMAKE_COMMAND}" "-E" "${LANGUAGE_COPY_OR_SYMLINK_DIR}"
      "${clang_headers_location}"
      "${LANGUAGELIB_DIR}/clang"

    CUSTOM_TARGET_NAME "symlink_clang_headers"
    OUTPUT "${LANGUAGELIB_DIR}/clang"
    COMMENT "Symlinking Clang resource headers into ${LANGUAGELIB_DIR}/clang")

add_dependencies(copy_shim_headers symlink_clang_headers)

add_custom_command_target(unused_var
  COMMAND
    "${CMAKE_COMMAND}" "-E" "make_directory" "${LANGUAGESTATICLIB_DIR}"
  COMMAND
    "${CMAKE_COMMAND}" "-E" "${LANGUAGE_COPY_OR_SYMLINK_DIR}"
    "${clang_headers_location}"
    "${LANGUAGESTATICLIB_DIR}/clang"

  CUSTOM_TARGET_NAME "symlink_clang_headers_static"
  OUTPUT "${LANGUAGESTATICLIB_DIR}/clang"
  COMMENT "Symlinking Clang resource headers into ${LANGUAGESTATICLIB_DIR}/clang")

add_dependencies(copy_shim_headers symlink_clang_headers_static)

# Add dependency to ensure that the static headers are installed. This is needed because no target
# depends on the static headers, so they would not be installed otherwise.
add_dependencies(symlink_clang_headers symlink_clang_headers_static)

if(NOT LANGUAGE_BUILT_STANDALONE)
  if(TARGET clang-resource-headers) # LLVM > 8
    set(clang_resource_headers clang-resource-headers)
  elseif(TARGET clang-headers) # LLVM <= 8
    set(clang_resource_headers clang-headers)
  else()
    message(SEND_ERROR
      "Unable to determine clang resource headers target in unified build")
  endif()

  foreach(target
      symlink_clang_headers
      clang-builtin-headers
      clang-resource-dir-symlink
      clang-builtin-headers-in-clang-resource-dir)
    add_dependencies(${target} ${clang_resource_headers})
  endforeach()
endif()

if(BOOTSTRAPPING_MODE MATCHES "BOOTSTRAPPING.*")
  foreach(bootstrapping "0" "1")
    get_bootstrapping_path(outdir ${LANGUAGELIB_DIR} ${bootstrapping})
    set(target_name "symlink-headers-bootstrapping${bootstrapping}")

    add_custom_command_target(unused_var
        COMMAND
          "${CMAKE_COMMAND}" "-E" "make_directory" "${outdir}"
        COMMAND
          "${CMAKE_COMMAND}" "-E" ${LANGUAGE_COPY_OR_SYMLINK_DIR}
          ${output_dir} "${outdir}/shims"
        COMMAND
          "${CMAKE_COMMAND}" "-E" "${LANGUAGE_COPY_OR_SYMLINK_DIR}"
          "${clang_headers_location}"
          "${outdir}/clang"

        CUSTOM_TARGET_NAME ${target_name}
        OUTPUT "${outdir}/shims" "${outdir}/clang"
        COMMENT "Symlinking headers for bootstrapping0")

    add_dependencies(${target_name} copy_shim_headers)

    if (LANGUAGE_STDLIB_INSTALL_PARENT_MODULE_FOR_SHIMS)
      set(modulemap_target_name "symlink-modulemap-bootstrapping${bootstrapping}")
      add_custom_command_target(unused_var
          COMMAND
            "${CMAKE_COMMAND}" "-E" "make_directory" "${outdir}"
          COMMAND
            "${CMAKE_COMMAND}" "-E" ${LANGUAGE_COPY_OR_SYMLINK_DIR}
            "${output_dir}/../module.modulemap" "${outdir}/module.modulemap"

          CUSTOM_TARGET_NAME ${modulemap_target_name}
          OUTPUT "${outdir}/module.modulemap"
          COMMENT "Symlinking shims modulemap for bootstrapping0")
      add_dependencies(${modulemap_target_name} copy_shim_headers)
      add_dependencies(${target_name} ${modulemap_target_name})
    endif()
  endforeach()
endif()

language_install_in_component(FILES ${sources}
                           DESTINATION "lib/language/shims"
                           COMPONENT stdlib)
if (LANGUAGE_STDLIB_INSTALL_PARENT_MODULE_FOR_SHIMS)
  language_install_in_component(FILES "${output_dir}/../module.modulemap"
                             DESTINATION "lib/language"
                             COMPONENT stdlib)
endif()

if (LANGUAGE_BUILD_STATIC_STDLIB)
language_install_in_component(FILES ${sources}
                           DESTINATION "lib/language_static/shims"
                           COMPONENT stdlib)
if (LANGUAGE_STDLIB_INSTALL_PARENT_MODULE_FOR_SHIMS)
  language_install_in_component(FILES "${output_dir_static}/../module.modulemap"
                             DESTINATION "lib/language_static"
                             COMPONENT stdlib)
endif()
endif()

# Install Clang headers under the Codira library so that an installed Codira's
# module importer can find the compiler headers corresponding to its Clang.
language_install_in_component(DIRECTORY "${clang_headers_location}/"
                           DESTINATION "lib/language/clang"
                           COMPONENT clang-builtin-headers
                           REGEX "\.(h|modulemap)$")

language_install_in_component(DIRECTORY "${clang_headers_location}/"
  DESTINATION "lib/language_static/clang"
  COMPONENT clang-builtin-headers
  REGEX "\.(h|modulemap)$")

if(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER OR LANGUAGE_PREBUILT_CLANG)
  # This will still link against the Codira-forked clang headers if the Codira
  # toolchain was built with LANGUAGE_INCLUDE_TOOLS.
  set(LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET_default ${clang_headers_location})
else()
  set(LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET_default "../clang/${CLANG_VERSION_MAJOR}")
endif()

set(LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET
  ${LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET_default} CACHE STRING "The target of the installed symlink at lib/language/clang")

language_install_symlink_component(clang-resource-dir-symlink
  LINK_NAME clang
  TARGET ${LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET}
  DESTINATION "lib/language")

language_install_symlink_component(clang-resource-dir-symlink
  LINK_NAME clang
  TARGET ${LANGUAGE_CLANG_RESOURCE_DIR_SYMLINK_INSTALL_TARGET}
  DESTINATION "lib/language_static")

# Possibly install Clang headers under Clang's resource directory in case we
# need to use a different version of the headers than the installed Clang. This
# should be used in conjunction with clang-resource-dir-symlink.
file(TO_CMAKE_PATH "${TOOLCHAIN_LIBRARY_OUTPUT_INTDIR}"
  _LANGUAGE_SHIMS_PATH_TO_CLANG_LIB_BUILD)
language_install_in_component(DIRECTORY "${_LANGUAGE_SHIMS_PATH_TO_CLANG_LIB_BUILD}/clang"
                           DESTINATION "lib"
                           COMPONENT clang-builtin-headers-in-clang-resource-dir
                           REGEX "\.(h|modulemap)$")
