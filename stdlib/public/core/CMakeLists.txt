#===--- CMakeLists.txt - Build the core standard library -----------------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2022 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
#===----------------------------------------------------------------------===#

function(split_embedded_sources)
  cmake_parse_arguments(SPLIT1 "" "OUT_LIST_EMBEDDED;OUT_LIST_NORMAL" "EMBEDDED;NORMAL" ${ARGN})
  string(REPLACE "EMBEDDED" "NORMAL" ARGN2 "${ARGN}")
  cmake_parse_arguments(SPLIT2 "" "OUT_LIST_EMBEDDED;OUT_LIST_NORMAL" "EMBEDDED;NORMAL" ${ARGN2})
  set(${SPLIT1_OUT_LIST_EMBEDDED} "${SPLIT1_EMBEDDED}" PARENT_SCOPE)
  set(${SPLIT1_OUT_LIST_NORMAL} "${SPLIT2_NORMAL}" PARENT_SCOPE)
endfunction()

# The complete list of sources in the core standard library. Each file is
# annotated with either "EMBEDDED" which means it contributes to both the
# embedded Codira stdlib and the regular stdlib, or "NORMAL" which means it only
# contributes to the regular stdlib. The split_embedded_sources splits this list
# into LANGUAGELIB_EMBEDDED_SOURCES and LANGUAGELIB_SOURCES accordingly.
split_embedded_sources(
  OUT_LIST_EMBEDDED LANGUAGELIB_EMBEDDED_SOURCES
  OUT_LIST_NORMAL LANGUAGELIB_SOURCES

  ### "ESSENTIAL" SOURCES
  ### -- PLEASE KEEP THIS LIST IN ALPHABETICAL ORDER ###
  # Some files can't be sorted alphabetically, see notes in the list below.
  EMBEDDED Algorithm.code
  EMBEDDED ArrayBody.code
  EMBEDDED ArrayBuffer.code
  EMBEDDED ArrayBufferProtocol.code
  EMBEDDED ArrayCast.code
  EMBEDDED Array.code
  EMBEDDED ArrayShared.code
  EMBEDDED ArraySlice.code
  EMBEDDED ArrayType.code
  EMBEDDED ASCII.code
  EMBEDDED Assert.code
  EMBEDDED AssertCommon.code
  EMBEDDED BidirectionalCollection.code
  EMBEDDED Bitset.code
  EMBEDDED Boolean.code
    NORMAL BridgeObjectiveC.code
  EMBEDDED BridgeStorage.code
    NORMAL BridgingBuffer.code
  EMBEDDED Builtin.code
  EMBEDDED BuiltinMath.code
  EMBEDDED Character.code
    NORMAL CocoaArray.code
    NORMAL Codable.code
  EMBEDDED Collection.code
  EMBEDDED CollectionAlgorithms.code
  EMBEDDED Comparable.code
  EMBEDDED CompilerProtocols.code
  EMBEDDED Sendable.code
  EMBEDDED ContiguousArray.code
  EMBEDDED ContiguouslyStored.code
  EMBEDDED ClosedRange.code
  EMBEDDED ContiguousArrayBuffer.code
  EMBEDDED CString.code
  EMBEDDED CTypes.code
    NORMAL DebuggerSupport.code
  EMBEDDED Dictionary.code
  EMBEDDED DictionaryBridging.code
  EMBEDDED DictionaryBuilder.code
  EMBEDDED DictionaryCasting.code
  EMBEDDED DictionaryStorage.code
  EMBEDDED DictionaryVariant.code
  EMBEDDED DiscontiguousSlice.code
  EMBEDDED DropWhile.code
    NORMAL Dump.code
  EMBEDDED EmptyCollection.code
  EMBEDDED EnumeratedSequence.code
  EMBEDDED Equatable.code
  EMBEDDED ErrorType.code
  EMBEDDED ExistentialCollection.code
  EMBEDDED Filter.code
  EMBEDDED FlatMap.code
  EMBEDDED Flatten.code
  EMBEDDED FloatingPoint.code
  EMBEDDED Hashable.code
  # WORKAROUND: This file name is not sorted alphabetically in the list because
  # if we do so, the compiler crashes.
  EMBEDDED AnyHashable.code
  # END WORKAROUND
  EMBEDDED Hasher.code
    NORMAL Hashing.code
  EMBEDDED HashTable.code
  EMBEDDED Identifiable.code
  EMBEDDED Indices.code
  EMBEDDED InlineArray.code
  EMBEDDED InputStream.code
  EMBEDDED IntegerParsing.code
  EMBEDDED Integers.code
  EMBEDDED Join.code
  EMBEDDED KeyPath.code
  EMBEDDED KeyValuePairs.code
  EMBEDDED LazyCollection.code
  EMBEDDED LazySequence.code
    NORMAL LegacyABI.code
  EMBEDDED LifetimeManager.code
    NORMAL Macros.code
  EMBEDDED ManagedBuffer.code
  EMBEDDED Map.code
  EMBEDDED MemoryLayout.code
  EMBEDDED UnicodeScalar.code # ORDER DEPENDENCY: Must precede Mirrors.code
    NORMAL Mirrors.code
  EMBEDDED Misc.code
  EMBEDDED MutableCollection.code
  EMBEDDED NativeDictionary.code
  EMBEDDED NativeSet.code
    NORMAL NewtypeWrapper.code
  EMBEDDED NFC.code
  EMBEDDED NFD.code
  EMBEDDED ObjectIdentifier.code
  EMBEDDED Optional.code
  EMBEDDED OptionSet.code
  EMBEDDED OutputStream.code
  EMBEDDED Pointer.code
  EMBEDDED Policy.code
  EMBEDDED PrefixWhile.code
    NORMAL Prespecialize.code
    NORMAL Print.code
  EMBEDDED PtrAuth.code
  EMBEDDED Random.code
  EMBEDDED RandomAccessCollection.code
  EMBEDDED Range.code
  EMBEDDED RangeReplaceableCollection.code
  EMBEDDED RangeSet.code
  EMBEDDED RangeSetRanges.code
    NORMAL ReflectionMirror.code
  EMBEDDED Repeat.code
    NORMAL REPL.code
  EMBEDDED Result.code
  EMBEDDED Reverse.code
  EMBEDDED Runtime.code
    NORMAL RuntimeFunctionCounters.code
  EMBEDDED SipHash.code
  EMBEDDED Sequence.code
  EMBEDDED SequenceAlgorithms.code
  EMBEDDED Set.code
  EMBEDDED SetAlgebra.code
    NORMAL SetAnyHashableExtensions.code
    NORMAL SetBridging.code
  EMBEDDED SetBuilder.code
  EMBEDDED SetCasting.code
  EMBEDDED SetStorage.code
  EMBEDDED SetVariant.code
  EMBEDDED ShadowProtocols.code
    NORMAL Shims.code
  EMBEDDED Slice.code
  EMBEDDED SmallString.code
  EMBEDDED Sort.code
  EMBEDDED Span/MutableRawSpan.code
  EMBEDDED Span/MutableSpan.code
  EMBEDDED Span/RawSpan.code
  EMBEDDED Span/Span.code
  EMBEDDED StaticString.code
  EMBEDDED StaticPrint.code
  EMBEDDED Stride.code
  EMBEDDED StringHashable.code  # ORDER DEPENDENCY: Must precede String.code
  EMBEDDED String.code
  EMBEDDED StringBreadcrumbs.code
  EMBEDDED StringBridge.code
  EMBEDDED StringCharacterView.code
  EMBEDDED StringComparable.code
  EMBEDDED StringComparison.code
  EMBEDDED StringCreate.code
  EMBEDDED StringGuts.code
  EMBEDDED StringGutsSlice.code
  EMBEDDED StringGutsRangeReplaceable.code
  EMBEDDED StringObject.code
  EMBEDDED StringProtocol.code
  EMBEDDED StringIndex.code
  EMBEDDED StringIndexConversions.code
  EMBEDDED StringIndexValidation.code
  EMBEDDED StringInterpolation.code
  EMBEDDED StringLegacy.code
  EMBEDDED StringNormalization.code
  EMBEDDED StringRangeReplaceableCollection.code
  EMBEDDED StringStorage.code
  EMBEDDED StringStorageBridge.code
  EMBEDDED StringSwitch.code
  EMBEDDED StringTesting.code
  EMBEDDED StringUnicodeScalarView.code
  EMBEDDED StringUTF16View.code
  EMBEDDED StringUTF8View.code
  EMBEDDED StringUTF8Validation.code
  EMBEDDED StringWordBreaking.code
  EMBEDDED Substring.code
  EMBEDDED CodiraNativeNSArray.code
  EMBEDDED TemporaryAllocation.code
    NORMAL ThreadLocalStorage.code
  EMBEDDED UIntBuffer.code
  EMBEDDED UnavailableStringAPIs.code
  EMBEDDED UnicodeData.code
  EMBEDDED UnicodeEncoding.code
  EMBEDDED UnicodeBreakProperty.code
  EMBEDDED UnicodeHelpers.code
  EMBEDDED UnicodeParser.code
  EMBEDDED UnicodeScalarProperties.code
  EMBEDDED CharacterProperties.code # ORDER DEPENDENCY: UnicodeScalarProperties.code
  EMBEDDED UnicodeSPI.code
  EMBEDDED Unmanaged.code
  EMBEDDED UnmanagedOpaqueString.code
  EMBEDDED UnmanagedString.code
  EMBEDDED UnsafePointer.code
  EMBEDDED UnsafeRawPointer.code
  EMBEDDED UTFEncoding.code
  EMBEDDED UTF8.code
  EMBEDDED UTF8EncodingError.code
  EMBEDDED UTF8Span.code
  EMBEDDED UTF8SpanBits.code
  EMBEDDED UTF8SpanComparisons.code
  EMBEDDED UTF8SpanFundamentals.code
  EMBEDDED UTF8SpanInternalHelpers.code
  EMBEDDED UTF8SpanIterators.code
  EMBEDDED UTF16.code
  EMBEDDED UTF32.code
  EMBEDDED Unicode.code # ORDER DEPENDENCY: must follow new unicode support
  EMBEDDED StringGraphemeBreaking.code # ORDER DEPENDENCY: Must follow UTF16.code
  EMBEDDED ValidUTF8Buffer.code
  EMBEDDED WriteBackMutableSlice.code
  EMBEDDED MigrationSupport.code

    ### "NON-ESSENTIAL" SOURCES, LAYERED ON TOP OF THE "ESSENTIAL" ONES
    ### -- PLEASE KEEP THIS LIST IN ALPHABETICAL ORDER ###
  EMBEDDED Availability.code
  EMBEDDED CollectionDifference.code
  EMBEDDED CollectionOfOne.code
  EMBEDDED Diffing.code
  EMBEDDED Duration.code
  EMBEDDED DurationProtocol.code
  EMBEDDED FloatingPointRandom.code
  EMBEDDED Instant.code
  EMBEDDED Int128.code
  EMBEDDED Mirror.code
    NORMAL PlaygroundDisplay.code
  EMBEDDED CodiraifyImport.code
    NORMAL CommandLine.code
  EMBEDDED SliceBuffer.code
  EMBEDDED StaticBigInt.code
  EMBEDDED UInt128.code
  EMBEDDED UnfoldSequence.code
  EMBEDDED UnsafeBufferPointerSlice.code
    NORMAL VarArgs.code
  EMBEDDED Zip.code
    NORMAL "${LANGUAGE_SOURCE_DIR}/stdlib/linker-support/magic-symbols-for-install-name.c"
  )

split_embedded_sources(
  OUT_LIST_EMBEDDED LANGUAGELIB_EMBEDDED_GYB_SOURCES
  OUT_LIST_NORMAL LANGUAGELIB_GYB_SOURCES

    NORMAL AtomicInt.code.gyb
  EMBEDDED FloatingPointParsing.code.gyb
  EMBEDDED FloatingPointTypes.code.gyb
  EMBEDDED IntegerTypes.code.gyb
  EMBEDDED LegacyInt128.code.gyb
  EMBEDDED UnsafeBufferPointer.code.gyb
  EMBEDDED UnsafeRawBufferPointer.code.gyb
  EMBEDDED Tuple.code.gyb
  )

if(LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES)
  split_embedded_sources(
    OUT_LIST_EMBEDDED LANGUAGELIB_EMBEDDED_VECTOR_GYB_SOURCES
    OUT_LIST_NORMAL LANGUAGELIB_VECTOR_GYB_SOURCES
    
    EMBEDDED SIMDIntegerConcreteOperations.code.gyb
    EMBEDDED SIMDFloatConcreteOperations.code.gyb
    EMBEDDED SIMDMaskConcreteOperations.code.gyb
    EMBEDDED SIMDVectorTypes.code.gyb
  )

  list(APPEND LANGUAGELIB_SOURCES SIMDVector.code)
  list(APPEND LANGUAGELIB_GYB_SOURCES ${LANGUAGELIB_VECTOR_GYB_SOURCES})

  list(APPEND LANGUAGELIB_EMBEDDED_SOURCES SIMDVector.code)
  list(APPEND LANGUAGELIB_EMBEDDED_GYB_SOURCES ${LANGUAGELIB_EMBEDDED_VECTOR_GYB_SOURCES})
endif()

# Freestanding and Linux/Android builds both have failures to resolve.
if(NOT BOOTSTRAPPING_MODE STREQUAL "OFF" AND NOT LANGUAGE_FREESTANDING_FLAVOR AND NOT LANGUAGE_HOST_VARIANT_SDK STREQUAL "LINUX" AND NOT LANGUAGE_HOST_VARIANT_SDK STREQUAL "ANDROID")
  list(APPEND LANGUAGELIB_SOURCES ObjectIdentifier+DebugDescription.code)
endif()

list(APPEND LANGUAGELIB_EMBEDDED_SOURCES
  EmbeddedRuntime.code
  EmbeddedStubs.code
  EmbeddedPrint.code
  )

set(GROUP_INFO_JSON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/GroupInfo.json)
set(language_core_link_flags "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}")
set(language_core_framework_depends)
set(language_core_private_link_libraries)
set(language_stdlib_compile_flags "${LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS}")

if(LANGUAGE_PRIMARY_VARIANT_SDK STREQUAL "CYGWIN")
  # TODO(compnerd) cache this variable to permit re-configuration
  execute_process(COMMAND "cygpath" "-u" "$ENV{SYSTEMROOT}" OUTPUT_VARIABLE ENV_SYSTEMROOT)
  list(APPEND language_core_private_link_libraries "${ENV_SYSTEMROOT}/system32/psapi.dll")
elseif(LANGUAGE_PRIMARY_VARIANT_SDK STREQUAL "FREEBSD")
  find_library(EXECINFO_LIBRARY execinfo)
  list(APPEND language_core_private_link_libraries ${EXECINFO_LIBRARY})
elseif(LANGUAGE_PRIMARY_VARIANT_SDK STREQUAL "LINUX")
  if(LANGUAGE_BUILD_STATIC_STDLIB)
    list(APPEND language_core_private_link_libraries)
  endif()
elseif(LANGUAGE_PRIMARY_VARIANT_SDK STREQUAL "WINDOWS")
  list(APPEND language_core_private_link_libraries shell32;DbgHelp;Synchronization)
endif()

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
  list(APPEND language_stdlib_compile_flags "-Xtoolchain" "-sil-inline-generics")
  list(APPEND language_stdlib_compile_flags "-Xtoolchain" "-sil-partial-specialization")
endif()
if(LANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING)
  list(APPEND language_stdlib_compile_flags "-enforce-exclusivity=checked")
endif()
if(LANGUAGE_STDLIB_ENABLE_DEBUG_PRECONDITIONS_IN_RELEASE)
  list(APPEND language_stdlib_compile_flags "-DLANGUAGE_STDLIB_ENABLE_DEBUG_PRECONDITIONS_IN_RELEASE")
endif()
set(compile_flags_for_final_build)
if(LANGUAGE_ENABLE_ARRAY_COW_CHECKS)
  list(APPEND compile_flags_for_final_build "-DCOW_CHECKS_ENABLED")
endif()

# STAGING: Temporarily avoids having to write #fileID in Codira.codeinterface.
list(APPEND language_stdlib_compile_flags "-Xfrontend" "-enable-experimental-concise-pound-file")

list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "Macros")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "FreestandingMacros")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "Extern")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "BitwiseCopyable")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "ValueGenerics")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "AddressableParameters")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "AddressableTypes")
list(APPEND language_stdlib_compile_flags "-enable-experimental-feature" "AllowUnsafeAttribute")
list(APPEND language_stdlib_compile_flags "-strict-memory-safety")

if("${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}" STREQUAL "")
  set(language_bin_dir "${CMAKE_BINARY_DIR}/bin")
  set(language_lib_dir "${CMAKE_BINARY_DIR}/lib")
else()
  set(language_bin_dir "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}")
  set(language_lib_dir "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/../lib")
endif()

# For `@DebugDescription`.
list(APPEND language_stdlib_compile_flags "-plugin-path" "${language_lib_dir}/language/host/plugins")

set(language_core_incorporate_object_libraries)
list(APPEND language_core_incorporate_object_libraries languageRuntimeCore)
list(APPEND language_core_incorporate_object_libraries languageLLVMSupport)
list(APPEND language_core_incorporate_object_libraries languageDemangling)
list(APPEND language_core_incorporate_object_libraries languageStdlibStubs)
list(APPEND language_core_incorporate_object_libraries languageThreading)
if(LANGUAGE_STDLIB_HAS_COMMANDLINE)
  list(APPEND language_core_incorporate_object_libraries languageCommandLineSupport)
endif()

# Backward deployment of Span on Apple platforms is implemented via the
# liblanguageCompatibilitySpan.dylib shim.
#
# This flag ensures we emit the correct $ld$previous$ symbols in the standard
# library. When building a binary for an older deployment target, we
# pretend certain symbols are actually in liblanguageCompatibilitySpan.dylib.
#
# Further discussion of liblanguageCompatibilitySpan.dylib appears below.
#
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND language_stdlib_compile_flags "-Xfrontend" "-previous-module-installname-map-file" "-Xfrontend" "${CMAKE_CURRENT_SOURCE_DIR}/PreviousModuleInstallName.json")
endif()

set(languageCore_common_options
                  IS_STDLIB IS_STDLIB_CORE
                    ${LANGUAGELIB_SOURCES}
                  GYB_SOURCES
                    ${LANGUAGELIB_GYB_SOURCES}
                  LINK_FLAGS
                    ${language_core_link_flags}
                  PRIVATE_LINK_LIBRARIES
                    ${language_core_private_link_libraries}
                  INCORPORATE_OBJECT_LIBRARIES
                    ${language_core_incorporate_object_libraries}
                  FRAMEWORK_DEPENDS
                    ${language_core_framework_depends}
                  LANGUAGE_COMPILE_FLAGS
                    ${language_stdlib_compile_flags} -Xcc -DlanguageCore_EXPORTS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS})

# The copy_shim_headers target dependency is required to immutable the
# build system know that there's a rule to produce the shims
# directory, but is not sufficient to cause the object file to be rebuilt
# when the shim header changes.  Therefore, we pass both the target
# and the generated directory as dependencies.
set(languageCore_common_dependencies
    copy_shim_headers "${LANGUAGELIB_DIR}/shims" ${GROUP_INFO_JSON_FILE})

if(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")

  set(b0_deps symlink-headers-bootstrapping0)
  set(b1_deps symlink-headers-bootstrapping1)

  list(FIND LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES "SHARED" index_of_shared)
  if(index_of_shared EQUAL -1)
    message(FATAL_ERROR "bootstrapping requires SHARED stdlib build type")
  endif()

  set(languageCore_common_bootstrapping_options
                    SHARED
                    ${languageCore_common_options}
                    SDK ${LANGUAGE_HOST_VARIANT_SDK}
                    ARCHITECTURE ${LANGUAGE_HOST_VARIANT_ARCH}
                    INSTALL_IN_COMPONENT
                      "never_install")

  # Bootstrapping - stage 0

  add_language_target_library_single(languageCore-bootstrapping0 languageCore
                    ${languageCore_common_bootstrapping_options}
                    FILE_DEPENDS
                      ${b0_deps} ${languageCore_common_dependencies}
                    BOOTSTRAPPING 0)

  # Bootstrapping - stage 1

  add_language_target_library_single(languageCore-bootstrapping1 languageCore
                    ${languageCore_common_bootstrapping_options}
                    FILE_DEPENDS
                      ${b1_deps} ${languageCore_common_dependencies}
                    BOOTSTRAPPING 1)
endif()

set(tooling_stdlib_deps)
if(TARGET libCodiraScan)
  list(APPEND tooling_stdlib_deps libCodiraScan)
endif()

add_language_target_library(languageCore
                  ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES}
                  ${languageCore_common_options}
                  ${compile_flags_for_final_build}
                  FILE_DEPENDS
                    ${languageCore_common_dependencies}
                  DEPENDS ${tooling_stdlib_deps}
                  INSTALL_IN_COMPONENT
                    stdlib
                  MACCATALYST_BUILD_FLAVOR
                    zippered
                 )

# When a binary built with an older deployment target is run with a new
# standard library, the symbols that were previously in the backward
# compatibility shim are now in the standard library. To ensure the
# dynamic linker continues to find these symbols, the compatibility
# shim must be replaced by a symlink to the standard library.
#
# Since we're building the "new" standard library here, we create this
# symlink in the same directory, from liblanguageCompatibilitySpan.dylib to
# liblanguageCore.dylib.
#
# The sources for the real liblanguageCompatibilitySpan.dylib are found in
# stdlib/toolchain/CompatibilitySpan. When running a binary with an old
# standard library, this dynamic library must be present at run time.
#
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  foreach(sdk ${LANGUAGE_SDKS})
    set(lib_dir "${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")
    set(lib_path "${LANGUAGELIB_DIR}/${lib_dir}")
    set(compat_lib_name "${lib_path}/liblanguageCompatibilitySpan.dylib")

    # This doesn't depend on liblanguageCore.dylib because we don't actually need
    # for it to exist to create the symlink, nor is there any need to recreate
    # the symlink if the dylib changes.
    add_custom_command_target(unused_var
                              CUSTOM_TARGET_NAME "languageCompatibilitySpan-symlink-${lib_dir}"
                              OUTPUT "${compat_lib_name}"
                              COMMAND ${CMAKE_COMMAND} "-E" "create_symlink" "${lib_path}/liblanguageCore.dylib" "${compat_lib_name}")
    foreach(ARCH ${LANGUAGE_SDK_${sdk}_ARCHITECTURES})
      add_dependencies("languageCore-${lib_dir}-${ARCH}" "languageCompatibilitySpan-symlink-${lib_dir}")
    endforeach()

    language_install_symlink_component(stdlib
      LINK_NAME liblanguageCompatibilitySpan.dylib
      TARGET liblanguageCore.dylib
      DESTINATION "lib/language/${lib_dir}")
  endforeach()
endif()

# Embedded standard library - embedded libraries are built as .codemodule only,
# i.e. there is no .o or .a file produced (no binary code is actually produced)
# and only users of a library are going to actually compile any needed code.
if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  add_custom_target(embedded-stdlib)
  add_dependencies(embedded-libraries embedded-stdlib)

  set(LANGUAGE_ENABLE_REFLECTION OFF)
  set(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT OFF)
  set(LANGUAGE_STDLIB_STABLE_ABI OFF)
  set(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP OFF)
  set(LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES OFF)

  # Under Embedded Codira, all stdlib APIs should be available always. Replace
  # all availability macros with very very old OS versions.
  set(LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS_EMBEDDED)
  foreach(def ${LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS})
    string(REGEX REPLACE ":.*" ":macOS 10.9, iOS 7.0, watchOS 2.0, tvOS 9.0, visionOS 1.0" replaced "${def}")
    list(APPEND LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS_EMBEDDED "${replaced}")
  endforeach()
  set(LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS "${LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS_EMBEDDED}")

  foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
    string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
    list(GET list 0 arch)
    list(GET list 1 mod)
    list(GET list 2 triple)
    
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_TRIPLE "${triple}")
    add_language_target_library_single(
      embedded-stdlib-${mod}
      languageCore
      ONLY_LANGUAGEMODULE
      IS_STDLIB IS_STDLIB_CORE IS_FRAGILE
      ${LANGUAGELIB_EMBEDDED_SOURCES}
      GYB_SOURCES ${LANGUAGELIB_EMBEDDED_GYB_SOURCES}
      LANGUAGE_COMPILE_FLAGS
        ${language_stdlib_compile_flags} -Xcc -ffreestanding -enable-experimental-feature Embedded
        -Xfrontend -enable-ossa-modules
      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${arch}"
      FILE_DEPENDS ${languageCore_common_dependencies}
      DEPENDS ${tooling_stdlib_deps}
      INSTALL_IN_COMPONENT stdlib
      )
    add_dependencies(embedded-stdlib embedded-stdlib-${mod})
  endforeach()
endif()
