add_language_unittest(CodiraASTTests
  ArithmeticEvaluator.cpp
  ASTDumperTests.cpp
  ASTWalkerTests.cpp
  AvailabilityContextTests.cpp
  AvailabilityDomainTests.cpp
  IndexSubsetTests.cpp
  DiagnosticBehaviorTests.cpp
  DiagnosticConsumerTests.cpp
  DiagnosticGroupsTests.cpp
  DiagnosticFormattingTests.cpp
  DiagnosticInfoTests.cpp
  SourceLocTests.cpp
  TestContext.cpp
  TypeMatchTests.cpp
  VersionRangeTests.cpp
  ImportTests.cpp
)

target_link_libraries(CodiraASTTests
   PRIVATE
   languageAST
   # FIXME: Circular dependencies.
   languageParse
   languageSema
)
