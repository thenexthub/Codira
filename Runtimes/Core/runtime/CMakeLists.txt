# TODO: clean this up so it's less Apple-specific.
# Detect B&I builds.
set(LANGUAGE_BNI_OS_BUILD FALSE)
set(LANGUAGE_BNI_XCODE_BUILD FALSE)
if(DEFINED ENV{RC_XBS})
  if((NOT DEFINED ENV{RC_XCODE} OR NOT "$ENV{RC_XCODE}") AND (NOT DEFINED ENV{RC_PLAYGROUNDS} OR NOT "$ENV{RC_PLAYGROUNDS}"))
    set(LANGUAGE_BNI_OS_BUILD TRUE)
  else()
    set(LANGUAGE_BNI_XCODE_BUILD TRUE)
  endif()
endif()

configure_file("CMakeConfig.h.in"
  "${PROJECT_BINARY_DIR}/include/language/Runtime/CMakeConfig.h"
  ESCAPE_QUOTES @ONLY)

add_library(languageRuntime OBJECT
    "${PROJECT_SOURCE_DIR}/CompatibilityOverride/CompatibilityOverride.cpp"
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

if(CodiraCore_ENABLE_RUNTIME_LEAK_CHECKER)
  target_sources(languageRuntime PRIVATE
    Leaks.mm)
endif()

# runtime/Paths.cpp uses `LANGUAGE_ARCH` and `LANGUAGE_LIB_SUBDIR` to find the
# subdirectory under `libexec` in which to find the backtracer.
# Fail the configuration if it isn't set appropriately.
# TODO: In the future, we might consider moving the backtracer up to
# `libexec/language` and drop the platform/architecture specific subdirectories
if(CodiraCore_ENABLE_FILESYSTEM_SUPPORT AND (NOT CodiraCore_ARCH_SUBDIR OR NOT CodiraCore_PLATFORM_SUBDIR))
  message(SEND_ERROR "Filesystem support requires setting `CodiraCore_ARCH_SUBDIR` and `CodiraCore_PLATFORM_SUBDIR`")
endif()

# TODO: Probably worth considering putting half of these in a RuntimeConfig.h.in
#       file rather than pushing them through macro flags.
target_compile_definitions(languageRuntime
  PRIVATE
    -DLANGUAGE_RUNTIME
    -DLANGUAGE_TARGET_LIBRARY_NAME=languageRuntimeCore
    $<$<BOOL:${BUILD_SHARED_LIBS}>:-DlanguageCore_EXPORTS>
    $<$<BOOL:${CodiraCore_ENABLE_BACKTRACING}>:-DLANGUAGE_ENABLE_BACKTRACING>
    $<$<BOOL:${CodiraCore_ENABLE_OVERRIDABLE_RETAIN_RELEASE}>:-DLANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE>
    $<$<BOOL:${CodiraCore_BACKTRACER_PATH}>:-DLANGUAGE_RUNTIME_FIXED_BACKTRACER_PATH="${CodiraCore_BACKTRACER_PATH}">
    $<$<BOOL:${CodiraCore_ENABLE_STDLIB_TRACING}>:-DLANGUAGE_STDLIB_TRACING>
    $<$<BOOL:${CodiraCore_ENABLE_SHORT_MANGLING_LOOKUPS}>:-DLANGUAGE_STDLIB_SHORT_MANGLING_LOOKUPS>
    $<$<BOOL:${CodiraCore_HAS_ASL}>:-DLANGUAGE_STDLIB_HAS_ASL>
    $<$<BOOL:${CodiraCore_ENABLE_TYPE_PRINTING}>:-DLANGUAGE_STDLIB_HAS_TYPE_PRINTING>
    $<$<BOOL:${CodiraCore_HAS_DLADDR}>:-DLANGUAGE_STDLIB_HAS_DLADDR>
    $<$<BOOL:${CodiraCore_HAS_DLSYM}>:-DLANGUAGE_STDLIB_HAS_DLSYM>
    $<$<BOOL:${CodiraCore_ENABLE_FILESYSTEM_SUPPORT}>:-DLANGUAGE_STDLIB_HAS_FILESYSTEM>
    $<$<BOOL:${CodiraCore_ENABLE_FILESYSTEM_SUPPORT}>:-DLANGUAGE_ARCH="${CodiraCore_ARCH_SUBDIR}">
    $<$<BOOL:${CodiraCore_ENABLE_FILESYSTEM_SUPPORT}>:-DLANGUAGE_LIB_SUBDIR="${CodiraCore_PLATFORM_SUBDIR}">
    $<$<BOOL:${CodiraCore_ENABLE_FATALERROR_BACKTRACE}>:-DLANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING>
  PUBLIC
    $<$<BOOL:${CodiraCore_ENABLE_VECTOR_TYPES}>:-DLANGUAGE_STDLIB_ENABLE_VECTOR_TYPES>
    $<$<BOOL:${CodiraCore_ENABLE_OS_TRACE_LAZY_INIT}>:-DLANGUAGE_USE_OS_TRACE_LAZY_INIT>)

target_include_directories(languageRuntime PRIVATE
  "${PROJECT_BINARY_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(languageRuntime PRIVATE
  $<$<PLATFORM_ID:Windows>:User32>
  $<$<PLATFORM_ID:Windows>:ShLwApi>
  languageShims
  languageDemangling)

# FIXME: Refactor so that we're not pulling sources from the compiler files
target_sources(languageRuntime PRIVATE
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Threading/ThreadSanitizer.cpp")

# FIXME: Private.h uses `Demangler.h` and `TypeLookupError.h` from the compiler
#        headers. We should split out the parts that are needed by the runtime
#        to avoid pulling in headers from the compiler.
target_include_directories(languageRuntime PRIVATE
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include")

if(CodiraCore_ENABLE_BACKTRACING)
  target_sources(languageRuntime PRIVATE
    Backtrace.cpp
    BacktraceUtils.cpp
    CrashHandlerMacOS.cpp
    CrashHandlerLinux.cpp)
endif()

target_sources(languageRuntime PRIVATE
  ErrorObject.mm
  CodiraObject.mm)

if(CodiraCore_ENABLE_OBJC_INTEROP)
  target_sources(languageRuntime PRIVATE
    CodiraValue.mm
    ReflectionMirrorObjC.mm
    ObjCRuntimeGetImageNameFromClass.mm)
endif()

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS languageRuntime
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
endif()

string(TOLOWER "${CodiraCore_OBJECT_FORMAT}x" CodiraCore_OBJECT_FORMAT)
if("${CodiraCore_OBJECT_FORMAT}" STREQUAL "elfx")
  add_library(languagert OBJECT CodiraRT-ELF-WASM.cpp)
  target_compile_definitions(languagert PRIVATE
    -DLANGUAGE_ENABLE_BACKTRACING=$<BOOL:${CodiraCore_ENABLE_BACKTRACING}>)
  target_include_directories(languagert PRIVATE
    "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include")
  target_link_libraries(languagert PRIVATE languageShims)
  # The driver requires that languagert.o is under
  # `usr/lib/(language|language_static)/<platform>/<arch>` regardless of whether the
  # other files are under the platform and architecture subdirectories:
  # https://github.com/languagelang/language-driver/blob/f66e33575150cc778289b5f573218c7a0c70bab6/Sources/CodiraDriver/Jobs/GenericUnixToolchain%2BLinkerSupport.code#L186
  install(FILES $<TARGET_OBJECTS:languagert>
    COMPONENT CodiraCore_runtime
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}"
    RENAME languagert.o)

  install(TARGETS languagert
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
elseif("${CodiraCore_OBJECT_FORMAT}" STREQUAL "coffx")
  add_library(languagertT OBJECT CodiraRT-COFF.cpp)
  target_compile_definitions(languagertT PRIVATE
    LANGUAGE_STATIC_STDLIB)
  target_link_libraries(languagertT PRIVATE languageShims)
  install(FILES $<TARGET_OBJECTS:languagertT>
    COMPONENT CodiraCore_runtime
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/language/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}"
    RENAME languagertT.obj)

  add_library(languagert OBJECT CodiraRT-COFF.cpp)
  target_link_libraries(languagert PRIVATE languageShims)
  install(FILES $<TARGET_OBJECTS:languagert>
    COMPONENT CodiraCore_runtime
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/language/${CodiraCore_PLATFORM_SUBDIR}/${CodiraCore_ARCH_SUBDIR}"
    RENAME languagert.obj)

  install(TARGETS languagertT languagert
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
elseif(NOT "${CodiraCore_OBJECT_FORMAT}" STREQUAL "x")
  message(SEND_ERROR "Unknown object format '${CodiraCore_OBJECT_FORMAT}'")
endif()
