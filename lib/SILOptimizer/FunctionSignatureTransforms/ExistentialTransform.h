//===--- ExistentialTransform.h - Existential Specializer -----------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This contains utilities for transforming existential args to generics.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_EXISTENTIALTRANSFORM_H
#define LANGUAGE_SIL_EXISTENTIALTRANSFORM_H
#include "FunctionSignatureOpts.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Utils/Existential.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "language/SILOptimizer/Utils/SILOptFunctionBuilder.h"
#include "language/SILOptimizer/Utils/SpecializationMangler.h"
#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"

namespace language {

/// A descriptor to carry information from ExistentialTransform analysis
/// to transformation.
struct ExistentialTransformArgumentDescriptor {
  OpenedExistentialAccess AccessType;
  bool isConsumed;
};

/// ExistentialTransform creates a protocol constrained generic and a thunk.
class ExistentialTransform {
  /// Function Builder to create a new thunk.
  SILOptFunctionBuilder &FunctionBuilder;

  /// The original function to analyze and transform.
  SILFunction *F;

  /// The newly created inner function.
  SILFunction *NewF;

  /// The function signature mangler we are using.
  Mangle::FunctionSignatureSpecializationMangler &Mangler;

  /// List of arguments and their descriptors to specialize
  toolchain::SmallDenseMap<int, ExistentialTransformArgumentDescriptor>
      &ExistentialArgDescriptor;

  /// Argument to Generic Type Map for NewF.
  toolchain::SmallDenseMap<int, GenericTypeParamType *> ArgToGenericTypeMap;

  /// Allocate the argument descriptors.
  toolchain::SmallVector<ArgumentDescriptor, 4> &ArgumentDescList;

  /// Create the Devirtualized Inner Function.
  void createExistentialSpecializedFunction();

  /// Create new generic arguments from existential arguments.
  void
  convertExistentialArgTypesToGenericArgTypes(
      SmallVectorImpl<GenericTypeParamType *> &genericParams,
      SmallVectorImpl<Requirement> &requirements);

  /// Create a name for the inner function.
  std::string createExistentialSpecializedFunctionName();

  /// Create the new devirtualized protocol function signature.
  CanSILFunctionType createExistentialSpecializedFunctionType();

  /// Create the thunk.
  void populateThunkBody();

public:
  /// Constructor.
  ExistentialTransform(
      SILOptFunctionBuilder &FunctionBuilder, SILFunction *F,
      Mangle::FunctionSignatureSpecializationMangler &Mangler,
      toolchain::SmallVector<ArgumentDescriptor, 4> &ADL,
      toolchain::SmallDenseMap<int, ExistentialTransformArgumentDescriptor>
          &ExistentialArgDescriptor)
      : FunctionBuilder(FunctionBuilder), F(F), NewF(nullptr), Mangler(Mangler),
        ExistentialArgDescriptor(ExistentialArgDescriptor),
        ArgumentDescList(ADL) {}

  /// Return the optimized iner function.
  SILFunction *getExistentialSpecializedFunction() { return NewF; }

  /// External function for the optimization.
  bool run() {
    createExistentialSpecializedFunction();
    return true;
  }
};

} // end namespace language

#endif
