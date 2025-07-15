# Add default compiler and linker flags to 'target'.
#
# FIXME: this is a HACK.  All SourceKit CMake code using this function should be
# rewritten to use 'add_language_host_library' or 'add_language_target_library'.
function(add_sourcekit_default_compiler_flags target)
  # Add variant-specific flags.
  _add_host_variant_c_compile_flags(${target})
  _add_host_variant_link_flags(${target})

  # Set compilation and link flags.
  if(LANGUAGE_HOST_VARIANT_SDK STREQUAL "WINDOWS")
    language_windows_include_for_arch(${LANGUAGE_HOST_VARIANT_ARCH}
      ${LANGUAGE_HOST_VARIANT_ARCH}_INCLUDE)
    target_include_directories(${target} SYSTEM PRIVATE
      ${${LANGUAGE_HOST_VARIANT_ARCH}_INCLUDE})
  endif()
  target_compile_options(${target} PRIVATE
    -fblocks)
endfunction()

function(add_sourcekit_language_runtime_link_flags target path HAS_LANGUAGE_MODULES)
  set(RPATH_LIST)

  # If we are linking in the Codira Codira parser, we at least need host tools
  # to do it.
  set(ASKD_BOOTSTRAPPING_MODE ${BOOTSTRAPPING_MODE})
  if (NOT ASKD_BOOTSTRAPPING_MODE)
    if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
      set(ASKD_BOOTSTRAPPING_MODE HOSTTOOLS)
    endif()
  endif()

  if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)

    # Lists of rpaths that we are going to add to our executables.
    #
    # Please add each rpath separately below to the list, explaining why you are
    # adding it.
    set(sdk_dir "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_PATH}/usr/lib/language")

    # If we found a language compiler and are going to use language code in language
    # host side tools but link with clang, add the appropriate -L paths so we
    # find all of the necessary language libraries on Darwin.
    if (HAS_LANGUAGE_MODULES AND ASKD_BOOTSTRAPPING_MODE)

      if(ASKD_BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
        # Add in the toolchain directory so we can grab compatibility libraries
        get_filename_component(TOOLCHAIN_BIN_DIR ${LANGUAGE_EXEC_FOR_LANGUAGE_MODULES} DIRECTORY)
        get_filename_component(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_BIN_DIR}/../lib/language/macosx" ABSOLUTE)
        target_link_directories(${target} PUBLIC ${TOOLCHAIN_LIB_DIR})

        # Add the SDK directory for the host platform.
        target_link_directories(${target} PRIVATE "${sdk_dir}")

        # Include the abi stable system stdlib in our rpath.
        list(APPEND RPATH_LIST "/usr/lib/language")

      elseif(ASKD_BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE-WITH-HOSTLIBS")

        # Intentionally don't add the lib dir of the cross-compiled compiler, so that
        # the stdlib is not picked up from there, but from the SDK.
        # This requires to explicitly add all the needed compatibility libraries. We
        # can take them from the current build.
        target_link_libraries(${target} PUBLIC HostCompatibilityLibs)

        # Add the SDK directory for the host platform.
        target_link_directories(${target} PRIVATE "${sdk_dir}")

        # Include the abi stable system stdlib in our rpath.
        list(APPEND RPATH_LIST "/usr/lib/language")

      elseif(ASKD_BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING-WITH-HOSTLIBS")
        # Add the SDK directory for the host platform.
        target_link_directories(${target} PRIVATE "${sdk_dir}")

        # A backup in case the toolchain doesn't have one of the compatibility libraries.
        target_link_directories(${target} PRIVATE
          "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")

        # Include the abi stable system stdlib in our rpath.
        list(APPEND RPATH_LIST "/usr/lib/language")

      elseif(ASKD_BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")
        # At build time link against the built language libraries from the
        # previous bootstrapping stage.
        get_bootstrapping_language_lib_dir(bs_lib_dir "")
        target_link_directories(${target} PRIVATE ${bs_lib_dir})

        # Required to pick up the built liblanguageCompatibility<n>.a libraries
        target_link_directories(${target} PRIVATE
          "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")

        # At runtime link against the built language libraries from the current
        # bootstrapping stage.
        file(RELATIVE_PATH relative_rtlib_path "${path}" "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
        list(APPEND RPATH_LIST "@loader_path/${relative_rtlib_path}")
      else()
        message(FATAL_ERROR "Unknown ASKD_BOOTSTRAPPING_MODE '${ASKD_BOOTSTRAPPING_MODE}'")
      endif()

      # Workaround for a linker crash related to autolinking: rdar://77839981
      set_property(TARGET ${target} APPEND_STRING PROPERTY
                   LINK_FLAGS " -lobjc ")

    endif() # HAS_LANGUAGE_MODULES AND ASKD_BOOTSTRAPPING_MODE
  elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|FREEBSD|OPENBSD" AND HAS_LANGUAGE_MODULES AND ASKD_BOOTSTRAPPING_MODE)
    set(languagert "languageImageRegistrationObject${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_OBJECT_FORMAT}-${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}-${LANGUAGE_HOST_VARIANT_ARCH}")
    if(ASKD_BOOTSTRAPPING_MODE MATCHES "HOSTTOOLS|CROSSCOMPILE")
      if(ASKD_BOOTSTRAPPING_MODE MATCHES "HOSTTOOLS")
        # At build time and run time, link against the language libraries in the
        # installed host toolchain.
        get_filename_component(language_bin_dir ${LANGUAGE_EXEC_FOR_LANGUAGE_MODULES} DIRECTORY)
        get_filename_component(language_dir ${language_bin_dir} DIRECTORY)
        set(host_lib_dir "${language_dir}/lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
      else()
        set(host_lib_dir "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
      endif()
      set(host_lib_arch_dir "${host_lib_dir}/${LANGUAGE_HOST_VARIANT_ARCH}")

      set(languagert "${host_lib_arch_dir}/languagert${CMAKE_C_OUTPUT_EXTENSION}")
      target_link_libraries(${target} PRIVATE ${languagert})
      target_link_libraries(${target} PRIVATE "languageCore")

      target_link_directories(${target} PRIVATE ${host_lib_dir})
      target_link_directories(${target} PRIVATE ${host_lib_arch_dir})

      file(RELATIVE_PATH relative_rtlib_path "${path}" "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
      list(APPEND RPATH_LIST "$ORIGIN/${relative_rtlib_path}")
      # NOTE: SourceKit components are NOT executed before stdlib is built.
      # So there's no need to add RUNPATH to builder's runtime libraries.

    elseif(ASKD_BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")
      get_bootstrapping_language_lib_dir(bs_lib_dir "")
      target_link_directories(${target} PRIVATE ${bs_lib_dir})
      target_link_libraries(${target} PRIVATE ${languagert})
      target_link_libraries(${target} PRIVATE "languageCore")

      # At runtime link against the built language libraries from the current
      # bootstrapping stage.
      file(RELATIVE_PATH relative_rtlib_path "${path}" "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
      list(APPEND RPATH_LIST "$ORIGIN/${relative_rtlib_path}")

    elseif(ASKD_BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING-WITH-HOSTLIBS")
      message(FATAL_ERROR "ASKD_BOOTSTRAPPING_MODE 'BOOTSTRAPPING-WITH-HOSTLIBS' not supported on Linux")
    else()
      message(FATAL_ERROR "Unknown ASKD_BOOTSTRAPPING_MODE '${ASKD_BOOTSTRAPPING_MODE}'")
    endif()
  elseif(LANGUAGE_HOST_VARIANT_SDK STREQUAL "WINDOWS")
    if(ASKD_BOOTSTRAPPING_MODE MATCHES "HOSTTOOLS")
      set(languagert_obj
        ${LANGUAGE_PATH_TO_LANGUAGE_SDK}/usr/lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}/${LANGUAGE_HOST_VARIANT_ARCH}/languagert${CMAKE_C_OUTPUT_EXTENSION})
      target_link_libraries(${target} PRIVATE ${languagert_obj})
    endif()
  endif()

  if(LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
      # Add rpath to the host Codira libraries.
      file(RELATIVE_PATH relative_hostlib_path "${path}" "${LANGUAGELIB_DIR}/host/compiler")
      list(APPEND RPATH_LIST "@loader_path/${relative_hostlib_path}")
    elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|FREEBSD|OPENBSD")
      # Add rpath to the host Codira libraries.
      file(RELATIVE_PATH relative_hostlib_path "${path}" "${LANGUAGELIB_DIR}/host/compiler")
      list(APPEND RPATH_LIST "$ORIGIN/${relative_hostlib_path}")
    else()
      target_link_directories(${target} PRIVATE
        ${LANGUAGE_PATH_TO_LANGUAGE_SDK}/usr/lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}/${LANGUAGE_HOST_VARIANT_ARCH})
    endif()

    # For the "end step" of bootstrapping configurations on Darwin, need to be
    # able to fall back to the SDK directory for liblanguageCore et al.
    if (BOOTSTRAPPING_MODE MATCHES "BOOTSTRAPPING.*")
      if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
        target_link_directories(${target} PRIVATE "${sdk_dir}")

        # Include the abi stable system stdlib in our rpath.
        set(language_runtime_rpath "/usr/lib/language")

        # Add in the toolchain directory so we can grab compatibility libraries
        get_filename_component(TOOLCHAIN_BIN_DIR ${LANGUAGE_EXEC_FOR_LANGUAGE_MODULES} DIRECTORY)
        get_filename_component(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_BIN_DIR}/../lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" ABSOLUTE)
        target_link_directories(${target} PUBLIC ${TOOLCHAIN_LIB_DIR})
      endif()
    endif()

    if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS AND LANGUAGE_ALLOW_LINKING_LANGUAGE_CONTENT_IN_DARWIN_TOOLCHAIN)
      get_filename_component(TOOLCHAIN_BIN_DIR ${CMAKE_Codira_COMPILER} DIRECTORY)
      get_filename_component(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_BIN_DIR}/../lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" ABSOLUTE)
      target_link_directories(${target} BEFORE PUBLIC ${TOOLCHAIN_LIB_DIR})
    endif()
  endif()

  set(RPATH_LIST ${RPATH_LIST} PARENT_SCOPE)
endfunction()

# Add a new SourceKit library.
#
# Usage:
#   add_sourcekit_library(name     # Name of the library
#     [DEPENDS dep1 ...]           # Targets this library depends on
#     [TOOLCHAIN_LINK_COMPONENTS comp1 ...]  # LLVM components this library depends on
#     [INSTALL_IN_COMPONENT comp]  # The Codira installation component that this library belongs to.
#     [SHARED]
#     [HAS_LANGUAGE_MODULES]          # Whether to link CodiraCompilerModules
#     source1 [source2 source3 ...]) # Sources to add into this library
macro(add_sourcekit_library name)
  cmake_parse_arguments(SOURCEKITLIB
      "SHARED;HAS_LANGUAGE_MODULES"
      "INSTALL_IN_COMPONENT"
      "HEADERS;DEPENDS;TOOLCHAIN_LINK_COMPONENTS"
      ${ARGN})
  set(srcs ${SOURCEKITLIB_UNPARSED_ARGUMENTS})

  toolchain_process_sources(srcs ${srcs})
  if(MSVC_IDE)
    # Add public headers
    file(RELATIVE_PATH lib_path
      ${SOURCEKIT_SOURCE_DIR}/lib/
      ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(NOT lib_path MATCHES "^[.][.]")
      file(GLOB_RECURSE headers
        ${SOURCEKIT_SOURCE_DIR}/include/SourceKit/${lib_path}/*.h
        ${SOURCEKIT_SOURCE_DIR}/include/SourceKit/${lib_path}/*.def
      )
      set_source_files_properties(${headers} PROPERTIES HEADER_FILE_ONLY ON)

      file(GLOB_RECURSE tds
        ${SOURCEKIT_SOURCE_DIR}/include/SourceKit/${lib_path}/*.td
      )
      source_group("TableGen descriptions" FILES ${tds})
      set_source_files_properties(${tds} PROPERTIES HEADER_FILE_ONLY ON)

      set(srcs ${srcs} ${headers} ${tds})
    endif()
  endif()
  if (MODULE)
    set(libkind MODULE)
  elseif(SOURCEKITLIB_SHARED)
    set(libkind SHARED)
  else()
    set(libkind)
  endif()
  add_library(${name} ${libkind} ${srcs})
  if(NOT LANGUAGE_BUILT_STANDALONE AND SOURCEKIT_LANGUAGE_SWAP_COMPILER)
    add_dependencies(${name} clang)
  endif()
  toolchain_update_compile_flags(${name})

  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)

  set_output_directory(${name}
      BINARY_DIR ${SOURCEKIT_RUNTIME_OUTPUT_INTDIR}
      LIBRARY_DIR ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR})

  if(TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif(TOOLCHAIN_COMMON_DEPENDS)

  if(SOURCEKITLIB_DEPENDS)
    add_dependencies(${name} ${SOURCEKITLIB_DEPENDS})
  endif(SOURCEKITLIB_DEPENDS)

  language_common_toolchain_config(${name} ${SOURCEKITLIB_TOOLCHAIN_LINK_COMPONENTS})

  if(SOURCEKITLIB_SHARED AND EXPORTED_SYMBOL_FILE)
    add_toolchain_symbol_exports(${name} ${EXPORTED_SYMBOL_FILE})
  endif()

  # Once the new Codira parser is linked, everything has Codira modules.
  if (LANGUAGE_BUILD_LANGUAGE_SYNTAX AND SOURCEKITLIB_SHARED)
    set(SOURCEKITLIB_HAS_LANGUAGE_MODULES ON)
  endif()

  if(SOURCEKITLIB_SHARED)
    set(RPATH_LIST)
    add_sourcekit_language_runtime_link_flags(${name} ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR} ${SOURCEKITLIB_HAS_LANGUAGE_MODULES})

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
      set_target_properties(${name} PROPERTIES INSTALL_NAME_DIR "@rpath")
    endif()
    set_target_properties(${name} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(${name} PROPERTIES INSTALL_RPATH "${RPATH_LIST}")
  endif()

  if("${SOURCEKITLIB_INSTALL_IN_COMPONENT}" STREQUAL "")
    if(SOURCEKITLIB_SHARED)
      set(SOURCEKITLIB_INSTALL_IN_COMPONENT tools)
    else()
      set(SOURCEKITLIB_INSTALL_IN_COMPONENT dev)
    endif()
  endif()
  add_dependencies(${SOURCEKITLIB_INSTALL_IN_COMPONENT} ${name})
  language_install_in_component(TARGETS ${name}
    LIBRARY
      DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}"
      COMPONENT "${SOURCEKITLIB_INSTALL_IN_COMPONENT}"
    ARCHIVE
      DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}"
      COMPONENT "${SOURCEKITLIB_INSTALL_IN_COMPONENT}"
    RUNTIME
      DESTINATION "bin"
      COMPONENT "${SOURCEKITLIB_INSTALL_IN_COMPONENT}")

  language_install_in_component(FILES ${SOURCEKITLIB_HEADERS}
                             DESTINATION "include/SourceKit"
                             COMPONENT "${SOURCEKITLIB_INSTALL_IN_COMPONENT}")
  set_target_properties(${name} PROPERTIES FOLDER "SourceKit libraries")
  add_sourcekit_default_compiler_flags("${name}")

  language_is_installing_component("${SOURCEKITLIB_INSTALL_IN_COMPONENT}" is_installing)
  if(NOT is_installing)
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_BUILDTREE_EXPORTS ${name})
  else()
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_EXPORTS ${name})
  endif()
endmacro()

# Add a new SourceKit executable.
#
# Usage:
#   add_sourcekit_executable(name        # Name of the executable
#     [TOOLCHAIN_LINK_COMPONENTS comp1 ...] # LLVM components this executable
#                                        # depends on
#     source1 [source2 source3 ...])  # Sources to add into this executable
macro(add_sourcekit_executable name)
  set(SOURCEKIT_EXECUTABLE_options)
  set(SOURCEKIT_EXECUTABLE_single_parameter_options)
  set(SOURCEKIT_EXECUTABLE_multiple_parameter_options TOOLCHAIN_LINK_COMPONENTS)
  cmake_parse_arguments(SOURCEKITEXE "${SOURCEKIT_EXECUTABLE_options}"
    "${SOURCEKIT_EXECUTABLE_single_parameter_options}"
    "${SOURCEKIT_EXECUTABLE_multiple_parameter_options}" ${ARGN})

  add_executable(${name} ${SOURCEKITEXE_UNPARSED_ARGUMENTS})
  if(NOT LANGUAGE_BUILT_STANDALONE AND SOURCEKIT_LANGUAGE_SWAP_COMPILER)
    add_dependencies(${name} clang)
  endif()
  toolchain_update_compile_flags(${name})
  set_output_directory(${name}
      BINARY_DIR ${SOURCEKIT_RUNTIME_OUTPUT_INTDIR}
      LIBRARY_DIR ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR})

  # Add appropriate dependencies
  if(TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif()

  language_common_toolchain_config(${name} ${SOURCEKITEXE_TOOLCHAIN_LINK_COMPONENTS})
  target_link_libraries(${name} PRIVATE ${TOOLCHAIN_COMMON_LIBS})

  set_target_properties(${name} PROPERTIES FOLDER "SourceKit executables")
  add_sourcekit_default_compiler_flags("${name}")

  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
endmacro()

# Add a new SourceKit framework.
#
# Usage:
#   add_sourcekit_framework(name     # Name of the framework
#     [TOOLCHAIN_LINK_COMPONENTS comp1 ...]  # LLVM components this framework depends on
#     [MODULEMAP modulemap]          # Module map file for this framework
#     [INSTALL_IN_COMPONENT comp]    # The Codira installation component that this framework belongs to.
#     [HAS_LANGUAGE_MODULES]          # Whether to link CodiraCompilerModules
#     source1 [source2 source3 ...]) # Sources to add into this framework
macro(add_sourcekit_framework name)
  cmake_parse_arguments(SOURCEKITFW
    "HAS_LANGUAGE_MODULES" "MODULEMAP;INSTALL_IN_COMPONENT" "TOOLCHAIN_LINK_COMPONENTS" ${ARGN})
  set(srcs ${SOURCEKITFW_UNPARSED_ARGUMENTS})

  set(lib_dir ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR})
  set(framework_location "${lib_dir}/${name}.framework")

  # Once the new Codira parser is linked, everything has Codira modules.
  if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    set(SOURCEKITFW_HAS_LANGUAGE_MODULES ON)
  endif()

  if (NOT SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
    set(FLAT_FRAMEWORK_NAME "${name}")
    set(FLAT_FRAMEWORK_IDENTIFIER "org.code.${name}")
    set(FLAT_FRAMEWORK_SHORT_VERSION_STRING "1.0")
    set(FLAT_FRAMEWORK_BUNDLE_VERSION "${SOURCEKIT_VERSION_STRING}")
    set(FLAT_FRAMEWORK_DEPLOYMENT_TARGET "${SOURCEKIT_DEPLOYMENT_TARGET}")
    configure_file(
      "${SOURCEKIT_SOURCE_DIR}/cmake/FlatFrameworkInfo.plist.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist")
    add_custom_command(OUTPUT "${framework_location}/Info.plist"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist" "${framework_location}/Info.plist")
    list(APPEND srcs "${framework_location}/Info.plist")
  endif()

  toolchain_process_sources(srcs ${srcs})
  add_library(${name} SHARED ${srcs})
  toolchain_update_compile_flags(${name})

  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)

  set(headers)
  foreach(src ${srcs})
    get_filename_component(extension ${src} EXT)
    if(extension STREQUAL ".h")
      list(APPEND headers ${src})
    endif()
  endforeach()

  if(MSVC_IDE)
    set_source_files_properties(${headers} PROPERTIES HEADER_FILE_ONLY ON)
  endif(MSVC_IDE)

  if(TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif(TOOLCHAIN_COMMON_DEPENDS)

  language_common_toolchain_config(${name} ${SOURCEKITFW_TOOLCHAIN_LINK_COMPONENTS})

  if (EXPORTED_SYMBOL_FILE)
    add_toolchain_symbol_exports(${name} ${EXPORTED_SYMBOL_FILE})
  endif()

  if(SOURCEKITFW_MODULEMAP)
    set(modulemap "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCEKITFW_MODULEMAP}")
    set_property(TARGET ${name} APPEND PROPERTY LINK_DEPENDS "${modulemap}")
    if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
      set(modules_dir "${framework_location}/Versions/A/Modules")
      add_custom_command(TARGET ${name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${modulemap}" "${modules_dir}/module.modulemap"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Modules" "${framework_location}/Modules")
    else()
      set(modules_dir "${framework_location}/Modules")
      add_custom_command(TARGET ${name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${modulemap}" "${modules_dir}/module.modulemap")
    endif()
  endif()

  if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
    set_output_directory(${name}
        BINARY_DIR ${SOURCEKIT_RUNTIME_OUTPUT_INTDIR}
        LIBRARY_DIR ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR})
    set(RPATH_LIST)
    add_sourcekit_language_runtime_link_flags(${name} "${framework_location}/Versions/A" ${SOURCEKITFW_HAS_LANGUAGE_MODULES})
    file(RELATIVE_PATH relative_lib_path
      "${framework_location}/Versions/A" "${SOURCEKIT_LIBRARY_OUTPUT_INTDIR}")
    list(APPEND RPATH_LIST "@loader_path/${relative_lib_path}")

    file(GENERATE OUTPUT "xpc_service_name.txt" CONTENT "org.code.SourceKitService.${SOURCEKIT_VERSION_STRING}_${SOURCEKIT_TOOLCHAIN_NAME}")
    target_sources(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/xpc_service_name.txt")

    set_target_properties(${name} PROPERTIES
                          BUILD_WITH_INSTALL_RPATH TRUE
                          FOLDER "SourceKit frameworks"
                          FRAMEWORK TRUE
                          INSTALL_NAME_DIR "@rpath"
                          INSTALL_RPATH "${RPATH_LIST}"
                          MACOSX_FRAMEWORK_INFO_PLIST "${SOURCEKIT_SOURCE_DIR}/cmake/MacOSXFrameworkInfo.plist.in"
                          MACOSX_FRAMEWORK_IDENTIFIER "org.code.${name}"
                          MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
                          MACOSX_FRAMEWORK_BUNDLE_VERSION "${SOURCEKIT_VERSION_STRING}"
                          PUBLIC_HEADER "${headers}"
                          RESOURCE "${CMAKE_CURRENT_BINARY_DIR}/xpc_service_name.txt")
    add_dependencies(${SOURCEKITFW_INSTALL_IN_COMPONENT} ${name})
    language_install_in_component(TARGETS ${name}
                               FRAMEWORK
                                 DESTINATION lib${TOOLCHAIN_LIBDIR_SUFFIX}
                                 COMPONENT ${SOURCEKITFW_INSTALL_IN_COMPONENT}
                               LIBRARY
                                 DESTINATION lib${TOOLCHAIN_LIBDIR_SUFFIX}
                                 COMPONENT ${SOURCEKITFW_INSTALL_IN_COMPONENT}
                               ARCHIVE
                                 DESTINATION lib${TOOLCHAIN_LIBDIR_SUFFIX}
                                 COMPONENT ${SOURCEKITFW_INSTALL_IN_COMPONENT}
                               RUNTIME
                                 DESTINATION bin
                                 COMPONENT ${SOURCEKITFW_INSTALL_IN_COMPONENT})
  else()
    set_output_directory(${name}
        BINARY_DIR ${framework_location}
        LIBRARY_DIR ${framework_location})
    set(RPATH_LIST)
    add_sourcekit_language_runtime_link_flags(${name} "${framework_location}" SOURCEKITFW_HAS_LANGUAGE_MODULES RPATH_LIST)
    file(RELATIVE_PATH relative_lib_path
      "${framework_location}" "${SOURCEKIT_LIBRARY_OUTPUT_INTDIR}")
    list(APPEND RPATH_LIST "@loader_path/${relative_lib_path}")

    set_target_properties(${name} PROPERTIES
                          BUILD_WITH_INSTALL_RPATH TRUE
                          FOLDER "SourceKit frameworks"
                          INSTALL_NAME_DIR "@rpath/${name}.framework"
                          INSTALL_RPATH "${RPATH_LIST}"
                          PREFIX ""
                          SUFFIX "")
    language_install_in_component(DIRECTORY ${framework_location}
                               DESTINATION lib${TOOLCHAIN_LIBDIR_SUFFIX}
                               COMPONENT ${SOURCEKITFW_INSTALL_IN_COMPONENT}
                               USE_SOURCE_PERMISSIONS)

    foreach(hdr ${headers})
      get_filename_component(hdrname ${hdr} NAME)
      add_custom_command(TARGET ${name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${hdr}" "${framework_location}/Headers/${hdrname}")
    endforeach()
  endif()

  language_is_installing_component("${SOURCEKITFW_INSTALL_IN_COMPONENT}" is_installing)
  if(NOT is_installing)
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_BUILDTREE_EXPORTS ${name})
  else()
    set_property(GLOBAL APPEND PROPERTY LANGUAGE_EXPORTS ${name})
  endif()

  add_sourcekit_default_compiler_flags("${name}")
endmacro(add_sourcekit_framework)

# Add a new SourceKit XPC service to a framework.
#
# Usage:
#   add_sourcekit_xpc_service(name      # Name of the XPC service
#     [TOOLCHAIN_LINK_COMPONENTS comp1 ...]   # LLVM components this service depends on
#     [HAS_LANGUAGE_MODULES]          # Whether to link CodiraCompilerModules
#     source1 [source2 source3 ...])    # Sources to add into this service
macro(add_sourcekit_xpc_service name framework_target)
  cmake_parse_arguments(SOURCEKITXPC "HAS_LANGUAGE_MODULES" "" "TOOLCHAIN_LINK_COMPONENTS" ${ARGN})
  set(srcs ${SOURCEKITXPC_UNPARSED_ARGUMENTS})

  set(lib_dir ${SOURCEKIT_LIBRARY_OUTPUT_INTDIR})
  set(framework_location "${lib_dir}/${framework_target}.framework")
  if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
    set(xpc_bundle_dir "${framework_location}/Versions/A/XPCServices/${name}.xpc")
    set(xpc_contents_dir "${xpc_bundle_dir}/Contents")
    set(xpc_bin_dir "${xpc_contents_dir}/MacOS")
  else()
    set(xpc_bundle_dir "${framework_location}/XPCServices/${name}.xpc")
    set(xpc_contents_dir "${xpc_bundle_dir}")
    set(xpc_bin_dir "${xpc_contents_dir}")
  endif()

  set(XPCSERVICE_NAME ${name})
  set(XPCSERVICE_IDENTIFIER "org.code.${name}.${SOURCEKIT_VERSION_STRING}_${SOURCEKIT_TOOLCHAIN_NAME}")
  set(XPCSERVICE_BUNDLE_VERSION "${SOURCEKIT_VERSION_STRING}")
  set(XPCSERVICE_SHORT_VERSION_STRING "1.0")
  configure_file(
    "${SOURCEKIT_SOURCE_DIR}/cmake/XPCServiceInfo.plist.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist")
  add_custom_command(OUTPUT "${xpc_contents_dir}/Info.plist"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/${name}.Info.plist" "${xpc_contents_dir}/Info.plist")
  list(APPEND srcs "${xpc_contents_dir}/Info.plist")

  add_toolchain_executable(${name} ${srcs})
  set_target_properties(${name} PROPERTIES FOLDER "XPC Services")
  set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${xpc_bin_dir}")

  set_output_directory(${name}
    BINARY_DIR "${xpc_bin_dir}"
    LIBRARY_DIR "${xpc_bin_dir}")

  # Add appropriate dependencies
  if(TOOLCHAIN_COMMON_DEPENDS)
    add_dependencies(${name} ${TOOLCHAIN_COMMON_DEPENDS})
  endif(TOOLCHAIN_COMMON_DEPENDS)

  language_common_toolchain_config(${name} ${SOURCEKITXPC_TOOLCHAIN_LINK_COMPONENTS})
  target_link_libraries(${name} PRIVATE ${TOOLCHAIN_COMMON_LIBS})

  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)

  add_dependencies(${framework_target} ${name})

  set(RPATH_LIST)
  add_sourcekit_language_runtime_link_flags(${name} ${xpc_bin_dir} ${SOURCEKITXPC_HAS_LANGUAGE_MODULES})

  file(RELATIVE_PATH relative_lib_path "${xpc_bin_dir}" "${lib_dir}")
  list(APPEND RPATH_LIST "@loader_path/${relative_lib_path}")

  # Add rpath for sourcekitdInProc
  # lib/${framework_target}.framework/Versions/A/XPCServices/${name}.xpc/Contents/MacOS/${name}
  set_target_properties(${name} PROPERTIES
                        BUILD_WITH_INSTALL_RPATH On
                        INSTALL_RPATH "${RPATH_LIST}"
                        INSTALL_NAME_DIR "@rpath")

  if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/XPCServices" XPCServices
      WORKING_DIRECTORY ${framework_location})
  endif()

  # ASan does not play well with exported_symbol option. This should be fixed soon.
  if(NOT LANGUAGE_ASAN_BUILD)
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
      set_property(TARGET ${name} APPEND_STRING PROPERTY
                   LINK_FLAGS " -Wl,-exported_symbol,_main ")
    endif()
  endif()
  add_sourcekit_default_compiler_flags("${name}")
endmacro()
