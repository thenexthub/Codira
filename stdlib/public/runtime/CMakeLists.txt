set(language_runtime_compile_flags ${LANGUAGE_RUNTIME_CORE_CXX_FLAGS})
set(language_runtime_linker_flags ${LANGUAGE_RUNTIME_CORE_LINK_FLAGS})

if(LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS)
  list(APPEND language_runtime_compile_flags
      "-DLANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS=1")
endif()

if(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT)
  list(APPEND language_runtime_compile_flags
      "-DLANGUAGE_HAVE_CRASHREPORTERCLIENT=1")
endif()

set(language_runtime_leaks_sources)
if(LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER)
  list(APPEND language_runtime_compile_flags
       "-DLANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER=1")
  set(language_runtime_leaks_sources Leaks.mm)
endif()

if(LANGUAGE_RUNTIME_FIXED_BACKTRACER_PATH)
  list(APPEND language_runtime_compile_flags
    "-DLANGUAGE_RUNTIME_FIXED_BACKTRACER_PATH=\"${LANGUAGE_RUNTIME_FIXED_BACKTRACER_PATH}\"")
endif()

set(language_runtime_objc_sources
    ErrorObject.mm
    CodiraObject.mm
    CodiraValue.mm
    ReflectionMirrorObjC.mm
    ObjCRuntimeGetImageNameFromClass.mm)

set(language_runtime_sources
    ../CompatibilityOverride/CompatibilityOverride.cpp
    AnyHashableSupport.cpp
    Array.cpp
    AutoDiffSupport.cpp
    Bincompat.cpp
    BytecodeLayouts.cpp
    Casting.cpp
    CrashReporter.cpp
    Demangle.cpp
    DynamicCast.cpp
    Enum.cpp
    EnvironmentVariables.cpp
    ErrorObjectCommon.cpp
    ErrorObjectNative.cpp
    Errors.cpp
    ErrorDefaultImpls.cpp
    Exception.cpp
    Exclusivity.cpp
    ExistentialContainer.cpp
    Float16Support.cpp
    FoundationSupport.cpp
    FunctionReplacement.cpp
    GenericMetadataBuilder.cpp
    Heap.cpp
    HeapObject.cpp
    ImageInspectionCommon.cpp
    ImageInspectionMachO.cpp
    ImageInspectionELF.cpp
    ImageInspectionCOFF.cpp
    ImageInspectionStatic.cpp
    ImageInspectionWasm.cpp
    SymbolInfo.cpp
    KeyPaths.cpp
    KnownMetadata.cpp
    LibPrespecialized.cpp
    Metadata.cpp
    MetadataLookup.cpp
    Numeric.cpp
    Once.cpp
    Paths.cpp
    Portability.cpp
    ProtocolConformance.cpp
    RefCount.cpp
    ReflectionMirror.cpp
    RuntimeInvocationsTracking.cpp
    CodiraDtoa.cpp
    CodiraTLSContext.cpp
    ThreadingError.cpp
    Tracing.cpp
    AccessibleFunction.cpp
    Win32.cpp)

# We pull this in separately here because other dylibs will need it, but only
# will have the __tsan_on_initialize called, and on Darwin, RTLD_NEXT can't be
# used to call subsequence dylibs' copies of that.
set(language_runtime_threading_sources
  ${LANGUAGE_SOURCE_DIR}/lib/Threading/ThreadSanitizer.cpp)

set(language_runtime_backtracing_sources
  Backtrace.cpp
  BacktraceUtils.cpp
  CrashHandlerMacOS.cpp
  CrashHandlerLinux.cpp)

# Acknowledge that the following sources are known.
set(TOOLCHAIN_OPTIONAL_SOURCES
    CodiraRT-COFF.cpp
    CodiraRT-ELF-WASM.cpp
    ${language_runtime_sources}
    ${language_runtime_objc_sources}
    ${language_runtime_leaks_sources}
    ${language_runtime_backtracing_sources})

set(language_enable_backtracing)
if(LANGUAGE_ENABLE_BACKTRACING)
  list(APPEND language_runtime_sources ${language_runtime_backtracing_sources})
  set(language_enable_backtracing -DLANGUAGE_ENABLE_BACKTRACING)
endif()

set(language_runtime_library_compile_flags ${language_runtime_compile_flags})
list(APPEND language_runtime_library_compile_flags -DlanguageCore_EXPORTS)
list(APPEND language_runtime_library_compile_flags -I${LANGUAGE_SOURCE_DIR}/stdlib/include/toolchain/Support -I${LANGUAGE_SOURCE_DIR}/include)

set(static_binary_lnk_file_list)

foreach(sdk ${LANGUAGE_SDKS})
  if(NOT LANGUAGE_SDK_${sdk}_STATIC_LINKING_SUPPORTED)
    continue()
  endif()

  if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
    set(lib_dir "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")
    set(static_binary_lnk_src "${LANGUAGE_SOURCE_DIR}/stdlib/public/Resources/${lib_dir}/static-executable-args.lnk")

    # Generate the static-executable-args.lnk file used for ELF systems (eg linux)
    set(linkfile "${lib_dir}/static-executable-args.lnk")
    add_custom_command_target(language_static_binary_${sdk}_args
      COMMAND
      "${CMAKE_COMMAND}" -E copy
      "${static_binary_lnk_src}"
      "${LANGUAGESTATICLIB_DIR}/${linkfile}"
      OUTPUT
      "${LANGUAGESTATICLIB_DIR}/${linkfile}"
      DEPENDS
      "${static_binary_lnk_src}")

    list(APPEND static_binary_lnk_file_list ${language_static_binary_${sdk}_args})
    language_install_in_component(FILES "${LANGUAGESTATICLIB_DIR}/${linkfile}"
      DESTINATION "lib/language_static/${lib_dir}"
      COMPONENT stdlib)
  endif()
endforeach()

# In modern CMake, we would be able to use the previous name `languageRuntime`
# without clashing with the `Runtime` module, but the current build system
# is architected in such a way that we had to rename this to `languageRuntimeCore`
# in order to avoid a clash with the new Codira module.
add_language_target_library(languageRuntimeCore OBJECT_LIBRARY
  ${language_runtime_sources}
  ${language_runtime_objc_sources}
  ${language_runtime_leaks_sources}
  ${language_runtime_threading_sources}
  C_COMPILE_FLAGS
    ${language_runtime_library_compile_flags}
    ${language_enable_backtracing}
  LINK_FLAGS ${language_runtime_linker_flags}
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT never_install)

set(ELFISH_SDKS)
set(COFF_SDKS)
set(WASM_SDKS)
foreach(sdk ${LANGUAGE_SDKS})
  if("${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "ELF")
    list(APPEND ELFISH_SDKS ${sdk})
  elseif("${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "COFF")
    list(APPEND COFF_SDKS ${sdk})
  elseif("${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "WASM")
    list(APPEND WASM_SDKS ${sdk})
  endif()
endforeach()


# Gold LTO is unsupported. To prevent tests from failing when building
# with LTO, force language runtime to compile without LTO for Linux.
add_language_target_library(languageImageRegistrationObjectELF
                  OBJECT_LIBRARY IS_STDLIB IS_STDLIB_CORE
                  CodiraRT-ELF-WASM.cpp
                  C_COMPILE_FLAGS
                    ${LANGUAGE_RUNTIME_CORE_CXX_FLAGS}
                    ${language_enable_backtracing}
                  C_COMPILE_FLAGS_LINUX -fno-lto
                  LINK_FLAGS ${LANGUAGE_RUNTIME_CORE_LINK_FLAGS}
                  TARGET_SDKS ${ELFISH_SDKS}
                  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
                  INSTALL_IN_COMPONENT none)
# FIXME(compnerd) this should be compiled twice, once for static and once for
# shared.  The static version should be used for building the standard library.
add_language_target_library(languageImageRegistrationObjectCOFF
                  OBJECT_LIBRARY IS_STDLIB IS_STDLIB_CORE
                  CodiraRT-COFF.cpp
                  C_COMPILE_FLAGS
                    ${LANGUAGE_RUNTIME_CORE_CXX_FLAGS}
                    ${language_enable_backtracing}
                  LINK_FLAGS ${LANGUAGE_RUNTIME_CORE_LINK_FLAGS}
                  TARGET_SDKS ${COFF_SDKS}
                  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
                  INSTALL_IN_COMPONENT none)

add_language_target_library(languageImageRegistrationObjectWASM
                  OBJECT_LIBRARY IS_STDLIB IS_STDLIB_CORE
                  CodiraRT-ELF-WASM.cpp
                  C_COMPILE_FLAGS
                    ${LANGUAGE_RUNTIME_CORE_CXX_FLAGS}
                    ${language_enable_backtracing}
                  LINK_FLAGS ${LANGUAGE_RUNTIME_CORE_LINK_FLAGS}
                  TARGET_SDKS ${WASM_SDKS}
                  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
                  INSTALL_IN_COMPONENT none)

foreach(sdk ${LANGUAGE_SDKS})
  foreach(arch ${LANGUAGE_SDK_${sdk}_ARCHITECTURES})
    set(arch_subdir "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}/${arch}")
    set(arch_suffix "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}-${arch}")

    if("${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "ELF" OR
       "${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "COFF" OR
       "${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "WASM")
      # TODO(compnerd) switch to the generator expression when cmake is upgraded
      # to a version which supports it.
      # set(languagertObject "$<TARGET_OBJECTS:languageImageRegistrationObject${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}-${arch_suffix}>")
      set(languagertSourceName CodiraRT-${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}.cpp)
      if("${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "ELF" OR
         "${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}" STREQUAL "WASM")
       set(languagertSourceName CodiraRT-ELF-WASM.cpp)
      endif()
      set(languagertObject ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/languageImageRegistrationObject${LANGUAGE_SDK_${sdk}_OBJECT_FORMAT}-${arch_suffix}.dir/${languagertSourceName}${CMAKE_C_OUTPUT_EXTENSION})

      if(sdk STREQUAL "WINDOWS")
        set(extension .obj)
      else()
        set(extension .o)
      endif()

      set(shared_runtime_registrar "${LANGUAGELIB_DIR}/${arch_subdir}/languagert${extension}")
      set(static_runtime_registrar "${LANGUAGESTATICLIB_DIR}/${arch_subdir}/languagert${extension}")

      add_custom_command_target(languageImageRegistration-${arch_suffix}
                                COMMAND
                                  "${CMAKE_COMMAND}" -E copy "${languagertObject}" "${shared_runtime_registrar}"
                                COMMAND
                                  "${CMAKE_COMMAND}" -E copy "${languagertObject}" "${static_runtime_registrar}"
                                OUTPUT
                                  "${shared_runtime_registrar}"
                                  "${static_runtime_registrar}"
                                DEPENDS
                                  "${languagertObject}")
      if(LANGUAGE_BUILD_DYNAMIC_STDLIB AND NOT LANGUAGE_SDK_${sdk}_STATIC_ONLY)
        language_install_in_component(FILES
                                     "${shared_runtime_registrar}"
                                   DESTINATION
                                     "lib/language/${arch_subdir}"
                                   COMPONENT
                                     stdlib)
      endif()
      if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
        language_install_in_component(FILES
                                     "${static_runtime_registrar}"
                                   DESTINATION
                                     "lib/language_static/${arch_subdir}"
                                   COMPONENT
                                     stdlib)
      endif()

      add_dependencies(language-stdlib-${arch_suffix} ${languageImageRegistration-${arch_suffix}})

      add_custom_target(languageImageRegistration-${arch_suffix}
                        ALL DEPENDS
                          ${languageImageRegistration-${arch_suffix}})
      add_dependencies(stdlib language-stdlib-${arch_suffix} languageImageRegistration-${arch_suffix})
    endif()

  endforeach()

  # Generate the static-stdlib-args.lnk file used by -static-stdlib option for
  # 'GenericUnix' (eg linux)
  if(LANGUAGE_SDK_${sdk}_OBJECT_FORMAT STREQUAL "ELF")
    set(lib_dir "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")
    set(static_stdlib_lnk_src "${LANGUAGE_SOURCE_DIR}/stdlib/public/Resources/${lib_dir}/static-stdlib-args.lnk")
    set(linkfile ${lib_dir}/static-stdlib-args.lnk)
    if(EXISTS "${static_stdlib_lnk_src}")
      add_custom_command_target(language_static_stdlib_${sdk}_args
        COMMAND
        "${CMAKE_COMMAND}" -E copy
        "${static_stdlib_lnk_src}"
        "${LANGUAGESTATICLIB_DIR}/${linkfile}"
        OUTPUT
        "${LANGUAGESTATICLIB_DIR}/${linkfile}"
        DEPENDS
        "${static_stdlib_lnk_src}")
      list(APPEND static_binary_lnk_file_list ${language_static_stdlib_${sdk}_args})
    else()
      set(libpthread -lpthread)
      set(concurrency_libs)
      set(android_libraries)
      if(sdk STREQUAL "ANDROID")
        set(android_libraries -llog)
        set(libpthread)
      elseif(LANGUAGE_CONCURRENCY_USES_DISPATCH)
        set(concurrency_libs "-ldispatch -lBlocksRuntime")
      endif()

      file(WRITE "${LANGUAGESTATICLIB_DIR}/${linkfile}" "
-ldl
${libpthread}
${android_libraries}
-llanguageCore
${concurrency_libs}
-lstdc++
-lm
-Xlinker -export-dynamic
-Xlinker --exclude-libs
-Xlinker ALL")
    endif()

    language_install_in_component(FILES "${LANGUAGESTATICLIB_DIR}/${linkfile}"
      DESTINATION "lib/language_static/${lib_dir}"
      COMPONENT stdlib)
  endif()

endforeach()

if(static_binary_lnk_file_list)
  add_dependencies(stdlib ${static_binary_lnk_file_list})
  add_custom_target(static_binary_magic ALL DEPENDS ${static_binary_lnk_file_list})
endif()
