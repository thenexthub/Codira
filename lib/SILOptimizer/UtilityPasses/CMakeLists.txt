target_sources(languageSILOptimizer PRIVATE
  AccessSummaryDumper.cpp
  AccessPathVerification.cpp
  AccessStorageAnalysisDumper.cpp
  AccessStorageDumper.cpp
  BasicCalleePrinter.cpp
  BasicInstructionPropertyDumper.cpp
  BugReducerTester.cpp
  CFGPrinter.cpp
  CallerAnalysisPrinter.cpp
  ComputeDominanceInfo.cpp
  ComputeLoopInfo.cpp
  ConstantEvaluatorTester.cpp
  ConstantEvaluableSubsetChecker.cpp
  EpilogueARCMatcherDumper.cpp
  EpilogueRetainReleaseMatcherDumper.cpp
  FunctionOrderPrinter.cpp
  IVInfoPrinter.cpp
  InstCount.cpp
  Link.cpp
  LoopCanonicalizer.cpp
  LoopInfoPrinter.cpp
  LoopRegionPrinter.cpp
  ModulePrinter.cpp
  RCIdentityDumper.cpp
  SerializeSILPass.cpp
  SILDebugInfoGenerator.cpp
  SILSkippingChecker.cpp
  SimplifyUnreachableContainingBlocks.cpp
  StripDebugInfo.cpp
  TestRunner.cpp
  OwnershipDumper.cpp
  OwnershipVerifierTextualErrorDumper.cpp)
  
