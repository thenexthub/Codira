set(generated_tests UnicodeGraphemeBreakTest.cpp.gyb)

handle_gyb_sources(
    gyb_dependency_targets
    generated_tests
    ARCH ${LANGUAGE_HOST_VARIANT_ARCH})

add_language_unittest(CodiraBasicTests
  BlotMapVectorTest.cpp
  BlocklistTest.cpp
  CacheTest.cpp
  ClangImporterOptionsTest.cpp
  ClusteredBitVectorTest.cpp
  DemangleTest.cpp
  DiverseStackTest.cpp
  EditorPlaceholderTest.cpp
  EnumMapTest.cpp
  EncodedSequenceTest.cpp
  ExponentialGrowthAppendingBinaryByteStreamTests.cpp
  FileSystemTest.cpp
  FileTypes.cpp
  FixedBitSetTest.cpp
  FrozenMultiMapTest.cpp
  ImmutablePointerSetTest.cpp
  JSONSerialization.cpp
  OptionSetTest.cpp
  Options.cpp
  OwnedStringTest.cpp
  MultiMapCacheTest.cpp
  PointerIntEnumTest.cpp
  PrefixMapTest.cpp
  RangeTest.cpp
  SmallMapTest.cpp
  SourceManagerTest.cpp
  StableHasher.cpp
  STLExtrasTest.cpp
  StringExtrasTest.cpp
  SuccessorMapTest.cpp
  TaggedUnionTest.cpp
  ThreadSafeRefCntPointerTest.cpp
  TransformRangeTest.cpp
  TypeLookupError.cpp
  UnicodeTest.cpp
  ${generated_tests}
  )

add_dependencies(CodiraBasicTests "${gyb_dependency_targets}")

target_link_libraries(CodiraBasicTests
  PRIVATE
  languageBasic
  languageOption
  languageThreading
  clangBasic
  LLVMOption
  LLVMTestingSupport
  )

if(LANGUAGE_HOST_VARIANT STREQUAL "windows")
  target_link_libraries(CodiraBasicTests PRIVATE Synchronization)
endif()
