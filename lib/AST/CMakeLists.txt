
set(LANGUAGEAST_TOOLCHAIN_LINK_COMPONENTS
  bitreader bitwriter coroutines coverage irreader debuginfoDWARF
  profiledata instrumentation object objcarcopts mc mcparser
  bitreader bitwriter lto ipo option core support remarks ${TOOLCHAIN_TARGETS_TO_BUILD}
)

add_language_host_library(languageAST STATIC
  AbstractSourceFileDepGraphFactory.cpp
  AccessNotes.cpp
  AccessRequests.cpp
  ActorIsolation.cpp
  ArgumentList.cpp
  ASTContext.cpp
  ASTDemangler.cpp
  ASTDumper.cpp
  ASTMangler.cpp
  ASTNode.cpp
  ASTPrinter.cpp
  ASTScope.cpp
  ASTScopeCreation.cpp
  ASTScopeLookup.cpp
  ASTScopePrinting.cpp
  ASTScopeSourceRange.cpp
  ASTVerifier.cpp
  ASTWalker.cpp
  Attr.cpp
  AutoDiff.cpp
  Availability.cpp
  AvailabilityConstraint.cpp
  AvailabilityContext.cpp
  AvailabilityDomain.cpp
  AvailabilityQuery.cpp
  AvailabilityScope.cpp
  AvailabilityScopeBuilder.cpp
  AvailabilitySpec.cpp
  Builtins.cpp
  CaptureInfo.cpp
  ClangCodiraTypeCorrespondence.cpp
  ClangTypeConverter.cpp
  Concurrency.cpp
  ConcreteDeclRef.cpp
  ConformanceLookup.cpp
  ConformanceLookupTable.cpp
  Decl.cpp
  DeclContext.cpp
  DeclContextDumper.cpp
  DeclNameLoc.cpp
  DiagnosticArgument.cpp
  DiagnosticBridge.cpp
  DiagnosticConsumer.cpp
  DiagnosticEngine.cpp
  DiagnosticGroups.cpp
  DiagnosticList.cpp
  DistributedDecl.cpp
  DocComment.cpp
  Effects.cpp
  Evaluator.cpp
  ExistentialGeneralization.cpp
  Expr.cpp
  ExtInfo.cpp
  FeatureSet.cpp
  FineGrainedDependencies.cpp
  FineGrainedDependencyFormat.cpp
  FreestandingMacroExpansion.cpp
  FrontendSourceFileDepGraphFactory.cpp
  FunctionRefInfo.cpp
  GenericEnvironment.cpp
  GenericParamList.cpp
  GenericSignature.cpp
  Identifier.cpp
  ImportCache.cpp
  IndexSubset.cpp
  InlinableText.cpp
  LayoutConstraint.cpp
  LifetimeDependence.cpp
  LocalArchetypeRequirementCollector.cpp
  Module.cpp
  ModuleDependencies.cpp
  ModuleLoader.cpp
  ModuleNameLookup.cpp
  NameLookup.cpp
  NameLookupRequests.cpp
  OperatorNameLookup.cpp
  PackConformance.cpp
  PackExpansionMatcher.cpp
  ParameterPack.cpp
  Parameter.cpp
  Pattern.cpp
  PlatformKindUtils.cpp
  PluginLoader.cpp
  PluginRegistry.cpp
  PrettyStackTrace.cpp
  ProtocolConformance.cpp
  ProtocolConformanceRef.cpp
  RawComment.cpp
  Requirement.cpp
  RequirementEnvironment.cpp
  RequirementMachine/ApplyInverses.cpp
  RequirementMachine/ConcreteContraction.cpp
  RequirementMachine/ConcreteTypeWitness.cpp
  RequirementMachine/Diagnostics.cpp
  RequirementMachine/GenericSignatureQueries.cpp
  RequirementMachine/HomotopyReduction.cpp
  RequirementMachine/InterfaceType.cpp
  RequirementMachine/KnuthBendix.cpp
  RequirementMachine/MinimalConformances.cpp
  RequirementMachine/NameLookup.cpp
  RequirementMachine/NormalizeRewritePath.cpp
  RequirementMachine/PropertyMap.cpp
  RequirementMachine/PropertyRelations.cpp
  RequirementMachine/PropertyUnification.cpp
  RequirementMachine/RequirementBuilder.cpp
  RequirementMachine/RequirementLowering.cpp
  RequirementMachine/RequirementMachine.cpp
  RequirementMachine/RequirementMachineRequests.cpp
  RequirementMachine/RewriteContext.cpp
  RequirementMachine/RewriteLoop.cpp
  RequirementMachine/RewriteSystem.cpp
  RequirementMachine/Rule.cpp
  RequirementMachine/RuleBuilder.cpp
  RequirementMachine/SimplifySubstitutions.cpp
  RequirementMachine/Symbol.cpp
  RequirementMachine/Term.cpp
  RequirementMachine/TypeDifference.cpp
  SearchPathOptions.cpp
  SILLayout.cpp
  Stmt.cpp
  StorageImpl.cpp
  SubstitutionMap.cpp
  CodiraNameTranslation.cpp
  Type.cpp
  TypeCheckRequests.cpp
  TypeDeclFinder.cpp
  TypeJoinMeet.cpp
  TypeRepr.cpp
  TypeSubstitution.cpp
  TypeWalker.cpp
  UnqualifiedLookup.cpp
  USRGeneration.cpp

  TOOLCHAIN_LINK_COMPONENTS
  ${LANGUAGEAST_TOOLCHAIN_LINK_COMPONENTS}
  )

add_subdirectory(Bridging)

if(LANGUAGE_FORCE_OPTIMIZED_TYPECHECKER)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    target_compile_options(languageAST PRIVATE /O2 /Ob2)
  else()
    target_compile_options(languageAST PRIVATE -O3)
  endif()
endif()
target_link_libraries(languageAST INTERFACE
  clangCodeGen
  clangIndex
  clangFormat
  clangToolingCore
  clangFrontendTool
  clangDependencyScanning
  clangFrontend
  clangDriver
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangRewriteFrontend
  clangRewrite
  clangAST
  clangLex
  clangAPINotes
  clangBasic)

target_link_libraries(languageAST
  PUBLIC languageBasic
  PRIVATE languageMarkup)

if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  target_link_libraries(languageAST
    PRIVATE languageASTGen)
endif()

set_language_toolchain_is_available(languageAST)
