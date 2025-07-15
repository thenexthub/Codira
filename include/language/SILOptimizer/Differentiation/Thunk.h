//===--- Thunk.h - Automatic differentiation thunks -----------*- C++ -*---===//
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
// Automatic differentiation thunk generation utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_THUNK_H
#define LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_THUNK_H

#include "language/AST/AutoDiff.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/SILBuilder.h"

namespace language {

class SILOptFunctionBuilder;
class SILModule;
class SILLocation;
class SILValue;
class ExistentialArchetypeType;
class GenericEnvironment;
class SubstitutionMap;
class ArchetypeType;

//===----------------------------------------------------------------------===//
// Helpers
//===----------------------------------------------------------------------===//

namespace autodiff {

class ADContext;

//===----------------------------------------------------------------------===//
// Thunk helpers
//===----------------------------------------------------------------------===//
// These helpers are copied/adapted from SILGen. They should be refactored and
// moved to a shared location.
//===----------------------------------------------------------------------===//


/// Get or create a reabstraction thunk from `fromType` to `toType`, to be
/// called in `caller`.
SILFunction *getOrCreateReabstractionThunk(SILOptFunctionBuilder &fb,
                                           SILModule &module, SILLocation loc,
                                           SILFunction *caller,
                                           CanSILFunctionType fromType,
                                           CanSILFunctionType toType);

SILValue reabstractCoroutine(
    SILBuilder &builder, SILOptFunctionBuilder &fb, SILLocation loc,
    SILValue fn, CanSILFunctionType toType,
    std::function<SubstitutionMap(SubstitutionMap)> remapSubstitutions);

/// Reabstracts the given function-typed value `fn` to the target type `toType`.
/// Remaps substitutions using `remapSubstitutions`.
SILValue reabstractFunction(
    SILBuilder &builder, SILOptFunctionBuilder &fb, SILLocation loc,
    SILValue fn, CanSILFunctionType toType,
    std::function<SubstitutionMap(SubstitutionMap)> remapSubstitutions);

/// Get or create a derivative function parameter index subset thunk from
/// `actualIndices` to `desiredIndices` for the given associated function
/// value and original function operand. Returns a pair of the parameter
/// index subset thunk and its interface substitution map (used to partially
/// apply the thunk).
/// Calls `getOrCreateSubsetParametersThunkForLinearMap` to thunk the linear
/// map returned by the derivative function.
std::pair<SILFunction *, SubstitutionMap>
getOrCreateSubsetParametersThunkForDerivativeFunction(
    SILOptFunctionBuilder &fb, SILValue origFnOperand, SILValue derivativeFn,
    AutoDiffDerivativeFunctionKind kind, const AutoDiffConfig &desiredConfig,
    const AutoDiffConfig &actualConfig, ADContext &adContext);

/// Get or create a derivative function parameter index subset thunk from
/// `actualIndices` to `desiredIndices` for the given associated function
/// value and original function operand. Returns a pair of the parameter
/// index subset thunk and its interface substitution map (used to partially
/// apply the thunk).
std::pair<SILFunction *, SubstitutionMap>
getOrCreateSubsetParametersThunkForLinearMap(
    SILOptFunctionBuilder &fb, SILFunction *assocFn,
    CanSILFunctionType origFnType, CanSILFunctionType linearMapType,
    CanSILFunctionType targetType, AutoDiffDerivativeFunctionKind kind,
    const AutoDiffConfig &desiredConfig, const AutoDiffConfig &actualConfig,
    ADContext &adContext);

} // end namespace autodiff

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_MANDATORY_DIFFERENTIATION_THUNK_H
