# TODO: Hook up the CommandLine support sources

gyb_expand(AtomicInt.code.gyb AtomicInt.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(FloatingPointParsing.code.gyb FloatingPointParsing.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(FloatingPointTypes.code.gyb FloatingPointTypes.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(IntegerTypes.code.gyb IntegerTypes.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(LegacyInt128.code.gyb LegacyInt128.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(UnsafeBufferPointer.code.gyb UnsafeBufferPointer.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(UnsafeRawBufferPointer.code.gyb UnsafeRawBufferPointer.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

gyb_expand(Tuple.code.gyb Tuple.code
  FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

# These sources are not strictly sorted alphabetically because the compiler
# crashes if they are.
add_library(languageCore
  Algorithm.code
  ArrayBody.code
  ArrayBuffer.code
  ArrayBufferProtocol.code
  ArrayCast.code
  Array.code
  ArrayShared.code
  ArraySlice.code
  ArrayType.code
  ASCII.code
  Assert.code
  AssertCommon.code
  BidirectionalCollection.code
  Bitset.code
  Bool.code
  BridgeObjectiveC.code
  BridgeStorage.code
  BridgingBuffer.code
  Builtin.code
  BuiltinMath.code
  Character.code
  CocoaArray.code
  Codable.code
  Collection.code
  CollectionAlgorithms.code
  Comparable.code
  CompilerProtocols.code
  Sendable.code
  ContiguousArray.code
  ContiguouslyStored.code
  ClosedRange.code
  ContiguousArrayBuffer.code
  CString.code
  CTypes.code
  DebuggerSupport.code
  Dictionary.code
  DictionaryBridging.code
  DictionaryBuilder.code
  DictionaryCasting.code
  DictionaryStorage.code
  DictionaryVariant.code
  DiscontiguousSlice.code
  DropWhile.code
  Dump.code
  EmptyCollection.code
  EnumeratedSequence.code
  Equatable.code
  ErrorType.code
  ExistentialCollection.code
  Filter.code
  FlatMap.code
  Flatten.code
  FloatingPoint.code
  Hashable.code
  AnyHashable.code # ORDER DEPENDENCY
  Hasher.code
  Hashing.code
  HashTable.code
  Identifiable.code
  Indices.code
  InlineArray.code
  InputStream.code
  IntegerParsing.code
  Integers.code
  Join.code
  KeyPath.code
  KeyValuePairs.code
  LazyCollection.code
  LazySequence.code
  LegacyABI.code
  LifetimeManager.code
  Macros.code
  ManagedBuffer.code
  Map.code
  MemoryLayout.code
  UnicodeScalar.code # ORDER DEPENDENCY: Must precede Mirrors.code
  Mirrors.code
  Misc.code
  MutableCollection.code
  NativeDictionary.code
  NativeSet.code
  NewtypeWrapper.code
  NFC.code
  NFD.code
  ObjectIdentifier.code
  Optional.code
  OptionSet.code
  OutputStream.code
  Pointer.code
  Policy.code
  PrefixWhile.code
  Prespecialize.code
  Print.code
  PtrAuth.code
  Random.code
  RandomAccessCollection.code
  Range.code
  RangeReplaceableCollection.code
  RangeSet.code
  RangeSetRanges.code
  ReflectionMirror.code
  Repeat.code
  REPL.code
  Result.code
  Reverse.code
  Runtime.code
  RuntimeFunctionCounters.code
  SipHash.code
  Sequence.code
  SequenceAlgorithms.code
  Set.code
  SetAlgebra.code
  SetAnyHashableExtensions.code
  SetBridging.code
  SetBuilder.code
  SetCasting.code
  SetStorage.code
  SetVariant.code
  ShadowProtocols.code
  Shims.code
  Slice.code
  SmallString.code
  Sort.code
  Span/Span.code
  Span/RawSpan.code
  Span/MutableSpan.code
  Span/MutableRawSpan.code
  StaticString.code
  StaticPrint.code
  Stride.code
  StringHashable.code  # ORDER DEPENDENCY: Must precede String.code
  String.code
  StringBreadcrumbs.code
  StringBridge.code
  StringCharacterView.code
  StringComparable.code
  StringComparison.code
  StringCreate.code
  StringGuts.code
  StringGutsSlice.code
  StringGutsRangeReplaceable.code
  StringObject.code
  StringProtocol.code
  StringIndex.code
  StringIndexConversions.code
  StringIndexValidation.code
  StringInterpolation.code
  StringLegacy.code
  StringNormalization.code
  StringRangeReplaceableCollection.code
  StringStorage.code
  StringStorageBridge.code
  StringSwitch.code
  StringTesting.code
  StringUnicodeScalarView.code
  StringUTF16View.code
  StringUTF8View.code
  StringUTF8Validation.code
  StringWordBreaking.code
  Substring.code
  CodiraNativeNSArray.code
  TemporaryAllocation.code
  ThreadLocalStorage.code
  UIntBuffer.code
  UnavailableStringAPIs.code
  UnicodeData.code
  UnicodeEncoding.code
  UnicodeBreakProperty.code
  UnicodeHelpers.code
  UnicodeParser.code
  UnicodeScalarProperties.code
  CharacterProperties.code # ORDER DEPENDENCY: UnicodeScalarProperties.code
  UnicodeSPI.code
  Unmanaged.code
  UnmanagedOpaqueString.code
  UnmanagedString.code
  UnsafePointer.code
  UnsafeRawPointer.code
  UTFEncoding.code
  UTF8.code
  UTF8EncodingError.code
  UTF8Span.code
  UTF8SpanBits.code
  UTF8SpanComparisons.code
  UTF8SpanFundamentals.code
  UTF8SpanInternalHelpers.code
  UTF8SpanIterators.code
  UTF16.code
  UTF32.code
  Unicode.code # ORDER DEPENDENCY: must follow new unicode support
  StringGraphemeBreaking.code # ORDER DEPENDENCY: Must follow UTF16.code
  ValidUTF8Buffer.code
  WriteBackMutableSlice.code
  MigrationSupport.code

  Availability.code
  CollectionDifference.code
  CollectionOfOne.code
  Diffing.code
  Duration.code
  DurationProtocol.code
  FloatingPointRandom.code
  Instant.code
  Int128.code
  Mirror.code
  PlaygroundDisplay.code
  CodiraifyImport.code
  CommandLine.code
  SliceBuffer.code
  StaticBigInt.code
  UInt128.code
  UnfoldSequence.code
  UnsafeBufferPointerSlice.code
  VarArgs.code
  Zip.code

  "${PROJECT_SOURCE_DIR}/linker-support/magic-symbols-for-install-name.c"
  "${CMAKE_CURRENT_BINARY_DIR}/AtomicInt.code"
  "${CMAKE_CURRENT_BINARY_DIR}/FloatingPointParsing.code"
  "${CMAKE_CURRENT_BINARY_DIR}/FloatingPointTypes.code"
  "${CMAKE_CURRENT_BINARY_DIR}/IntegerTypes.code"
  "${CMAKE_CURRENT_BINARY_DIR}/LegacyInt128.code"
  "${CMAKE_CURRENT_BINARY_DIR}/UnsafeBufferPointer.code"
  "${CMAKE_CURRENT_BINARY_DIR}/UnsafeRawBufferPointer.code"
  "${CMAKE_CURRENT_BINARY_DIR}/Tuple.code")

# https://github.com/languagelang/language/issues/77705 - Freestanding and Linux/Android builds both have failures to resolve.
if(NOT LINUX AND NOT ANDROID)
  target_sources(languageCore PRIVATE ObjectIdentifier+DebugDescription.code)
endif()

if(CodiraCore_ENABLE_COMMANDLINE_SUPPORT)
  target_sources(languageCore PRIVATE
    CommandLine.code)
endif()

if(CodiraCore_ENABLE_VECTOR_TYPES)
  gyb_expand(SIMDFloatConcreteOperations.code.gyb
    SIMDFloatConcreteOperations.code
    FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")
  gyb_expand(SIMDIntegerConcreteOperations.code.gyb
    SIMDIntegerConcreteOperations.code
    FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")
  gyb_expand(SIMDMaskConcreteOperations.code.gyb
    SIMDMaskConcreteOperations.code
    FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")
  gyb_expand(SIMDVectorTypes.code.gyb
    SIMDVectorTypes.code
    FLAGS "-DCMAKE_SIZEOF_VOID_P=${CodiraCore_SIZEOF_POINTER}")

  target_sources(languageCore PRIVATE
    SIMDVector.code
    "${CMAKE_CURRENT_BINARY_DIR}/SIMDFloatConcreteOperations.code"
    "${CMAKE_CURRENT_BINARY_DIR}/SIMDIntegerConcreteOperations.code"
    "${CMAKE_CURRENT_BINARY_DIR}/SIMDMaskConcreteOperations.code"
    "${CMAKE_CURRENT_BINARY_DIR}/SIMDVectorTypes.code")
endif()

set_target_properties(languageCore PROPERTIES
  Codira_MODULE_NAME Codira
  LINKER_LANGUAGE CXX)
if(NOT BUILD_SHARED_LIBS AND CMAKE_STATIC_LIBRARY_PREFIX_Codira)
  set_target_properties(languageCore PROPERTIES
    PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX_Codira})
endif()

target_compile_definitions(languageCore
  PRIVATE
    $<$<BOOL:${CodiraCore_ENABLE_REFLECTION}>:-DLANGUAGE_ENABLE_REFLECTION>
    $<$<BOOL:${CodiraCore_ENABLE_COMPACT_ABSOLUTE_FUNCTION_POINTERS}>:-DLANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER>
    $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_TARGET_LIBRARY_NAME=languageCore>
  INTERFACE
    $<$<BOOL:${CodiraCore_ENABLE_VECTOR_TYPES}>:-DLANGUAGE_STDLIB_ENABLE_VECTOR_TYPES>)

target_compile_options(languageCore PRIVATE
  "$<$<AND:$<COMPILE_LANGUAGE:Codira>,$<BOOL:${BUILD_SHARED_LIBS}>>:SHELL:-Xcc -DlanguageCore_EXPORTS>"
  # STAGING: Temporarily avoids having to write #fileID in Codira.codeinterface.
  # see also 327ea8bce2d1107a847d444651b19ca6a2901c9e
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -enable-experimental-concise-pound-file>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature AddressableParameters>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature AddressableTypes>"
  $<$<COMPILE_LANGUAGE:Codira>:-parse-stdlib>
  $<$<COMPILE_LANGUAGE:Codira>:-nostdimport>
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -group-info-path -Xfrontend ${CMAKE_CURRENT_SOURCE_DIR}/GroupInfo.json>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -disable-objc-attr-requires-foundation-module>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xfrontend -require-explicit-availability=ignore>"
  "$<$<AND:$<PLATFORM_ID:Darwin>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -previous-module-installname-map-file -Xfrontend ${CMAKE_CURRENT_SOURCE_DIR}/PreviousModuleInstallName.json>")
if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
  # Using these in MinSizeRel would result in a 15% increase in the binary size
  target_compile_options(languageCore PRIVATE
    "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xtoolchain -sil-inline-generics>"
    "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xtoolchain -sil-partial-specialization>")
endif()

# FIXME: Why is this not implicitly in the interface flags?
target_include_directories(languageCore INTERFACE
  "$<$<COMPILE_LANGUAGE:Codira>:$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CodiraCore_INSTALL_LANGUAGEMODULEDIR}>>")

target_link_libraries(languageCore
  PRIVATE
    languageRuntime
    languageLLVMSupport
    languageDemangling
    languageStdlibStubs
    languageThreading
    $<$<NOT:$<PLATFORM_ID:Darwin>>:languagert$<$<PLATFORM_ID:Windows>:T>>
  PUBLIC
    languageShims)

string(TOLOWER "${CodiraCore_OBJECT_FORMAT}" CodiraCore_OBJECT_FORMAT_lc)
if("${CodiraCore_OBJECT_FORMAT_lc}" STREQUAL "elf")
  target_link_libraries(languageCore INTERFACE
    languagert)
elseif("${CodiraCore_OBJECT_FORMAT_lc}" STREQUAL "coff")
  if(BUILD_SHARED_LIBS)
    target_link_libraries(languageCore INTERFACE
      languagert)
  else()
    target_link_libraries(languageCore INTERFACE
      languagertT)
  endif()
endif()

target_link_options(languageCore PUBLIC
  $<$<LINK_LANGUAGE:Codira>:-nostartfiles>)

if(NOT POLICY CMP0157)
  target_compile_options(languageCore PRIVATE
    $<TARGET_OBJECTS:languageRuntime>
    $<TARGET_OBJECTS:languageLLVMSupport>
    $<TARGET_OBJECTS:languageDemangling>
    $<TARGET_OBJECTS:languageStdlibStubs>
    $<TARGET_OBJECTS:languageThreading>)
endif()

if(NOT ANDROID AND NOT APPLE AND NOT LINUX AND NOT WIN32 AND UNIX)
  find_library(EXECINFO_LIBRARY execinfo)
  target_link_libraries(languageCore PRIVATE "${EXECINFO_LIBRARY}")
endif()

if(APPLE AND BUILD_SHARED_LIBS)
  target_link_options(languageCore PRIVATE "SHELL:-Xlinker -headerpad_max_install_names")
endif()

install(TARGETS languageCore
  EXPORT CodiraCoreTargets
  COMPONENT CodiraCore_runtime
  ARCHIVE DESTINATION "${CodiraCore_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraCore_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageCore)
install_language_interface(languageCore)

# Configure plist creation for Darwin platforms.
generate_plist("${CMAKE_PROJECT_NAME}" "${CMAKE_PROJECT_VERSION}" languageCore)
embed_manifest(languageCore)

include("${CodiraCore_VENDOR_MODULE_DIR}/languageCore.cmake" OPTIONAL)

# TODO: Embedded CodiraCore builds
# FIXME: Embedded builds should be separate CMake invocations because they are
#        building for a different environment. I'm not sure how CMake will
#        handle their build model though, so we'll continue to cram them in here
#        as they are for now, but should eventually tie into the rest of the
#        build graph normally.
