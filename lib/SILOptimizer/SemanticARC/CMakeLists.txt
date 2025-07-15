target_sources(languageSILOptimizer PRIVATE
  SemanticARCOpts.cpp
  OwnershipLiveRange.cpp
  BorrowScopeOpts.cpp
  CopyValueOpts.cpp
  OwnedToGuaranteedPhiOpt.cpp
  Context.cpp
  SemanticARCOptVisitor.cpp
  OwnershipConversionElimination.cpp
  RedundantMoveValueElimination.cpp
  )
