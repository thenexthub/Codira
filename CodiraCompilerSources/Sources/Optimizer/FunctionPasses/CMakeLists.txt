# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli


language_compiler_sources(Optimizer
  AllocBoxToStack.code
  AssumeSingleThreaded.code
  AsyncDemotion.code
  BooleanLiteralFolding.code
  CleanupDebugSteps.code
  ClosureSpecialization.code
  ComputeEscapeEffects.code
  ComputeSideEffects.code
  CopyToBorrowOptimization.code
  DeadStoreElimination.code
  DeinitDevirtualizer.code
  DestroyHoisting.code
  DiagnoseInfiniteRecursion.code
  InitializeStaticGlobals.code
  LetPropertyLowering.code
  LifetimeDependenceDiagnostics.code
  LifetimeDependenceInsertion.code
  LifetimeDependenceScopeFixup.code
  ObjectOutliner.code
  ObjCBridgingOptimization.code
  MergeCondFails.code
  NamedReturnValueOptimization.code
  RedundantLoadElimination.code
  ReleaseDevirtualizer.code
  SimplificationPasses.code
  StackPromotion.code
  StripObjectHeaders.code
  TempLValueElimination.code
  TempRValueElimination.code
)
