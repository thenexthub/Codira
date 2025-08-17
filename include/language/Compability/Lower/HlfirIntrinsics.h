//===-- HlfirIntrinsics.h -- lowering to HLFIR intrinsic ops ----*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//
///
/// Implements lowering of transformational intrinsics to HLFIR intrinsic
/// operations
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_HLFIRINTRINSICS_H
#define LANGUAGE_COMPABILITY_LOWER_HLFIRINTRINSICS_H

#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "toolchain/ADT/SmallVector.h"
#include <cassert>
#include <optional>
#include <string>

namespace mlir {
class Location;
class Type;
class Value;
class ValueRange;
} // namespace mlir

namespace fir {
class FirOpBuilder;
struct IntrinsicArgumentLoweringRules;
} // namespace fir

namespace language::Compability::lower {

/// This structure holds the initial lowered value of an actual argument that
/// was lowered regardless of the interface, and it holds whether or not it
/// may be absent at runtime and the dummy is optional.
struct PreparedActualArgument {

  PreparedActualArgument(hlfir::Entity actual,
                         std::optional<mlir::Value> isPresent)
      : actual{actual}, isPresent{isPresent} {}
  PreparedActualArgument(hlfir::ElementalAddrOp vectorSubscriptedActual)
      : actual{vectorSubscriptedActual}, isPresent{std::nullopt} {}
  void setElementalIndices(mlir::ValueRange &indices) {
    oneBasedElementalIndices = &indices;
  }

  /// Get the prepared actual. If this is an array argument in an elemental
  /// call, the current element value will be returned.
  hlfir::Entity getActual(mlir::Location loc, fir::FirOpBuilder &builder) const;

  void derefPointersAndAllocatables(mlir::Location loc,
                                    fir::FirOpBuilder &builder) {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual))
      actual = hlfir::derefPointersAndAllocatables(loc, builder, *actualEntity);
  }

  void loadTrivialScalar(mlir::Location loc, fir::FirOpBuilder &builder) {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual))
      actual = hlfir::loadTrivialScalar(loc, builder, *actualEntity);
  }

  /// Ensure an array expression argument is fully evaluated in memory before
  /// the call. Useful for impure elemental calls.
  hlfir::AssociateOp associateIfArrayExpr(mlir::Location loc,
                                          fir::FirOpBuilder &builder) {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual)) {
      if (!actualEntity->isVariable() && actualEntity->isArray()) {
        mlir::Type storageType = actualEntity->getType();
        hlfir::AssociateOp associate = hlfir::genAssociateExpr(
            loc, builder, *actualEntity, storageType, "adapt.impure_arg_eval");
        actual = hlfir::Entity{associate};
        return associate;
      }
    }
    return {};
  }

  bool isArray() const {
    return std::holds_alternative<hlfir::ElementalAddrOp>(actual) ||
           std::get<hlfir::Entity>(actual).isArray();
  }

  mlir::Value genShape(mlir::Location loc, fir::FirOpBuilder &builder) {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual))
      return hlfir::genShape(loc, builder, *actualEntity);
    return std::get<hlfir::ElementalAddrOp>(actual).getShape();
  }

  mlir::Value genCharLength(mlir::Location loc, fir::FirOpBuilder &builder) {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual))
      return hlfir::genCharLength(loc, builder, *actualEntity);
    auto typeParams = std::get<hlfir::ElementalAddrOp>(actual).getTypeparams();
    assert(typeParams.size() == 1 &&
           "failed to retrieve vector subscripted character length");
    return typeParams[0];
  }

  /// When the argument is polymorphic, get mold value with the same dynamic
  /// type.
  mlir::Value getPolymorphicMold(mlir::Location loc) const {
    if (auto *actualEntity = std::get_if<hlfir::Entity>(&actual))
      return *actualEntity;
    TODO(loc, "polymorphic vector subscripts");
  }

  bool handleDynamicOptional() const { return isPresent.has_value(); }
  mlir::Value getIsPresent() const {
    assert(handleDynamicOptional() && "not a dynamic optional");
    return *isPresent;
  }

  void resetOptionalAspect() { isPresent = std::nullopt; }

private:
  std::variant<hlfir::Entity, hlfir::ElementalAddrOp> actual;
  mlir::ValueRange *oneBasedElementalIndices{nullptr};
  // When the actual may be dynamically optional, "isPresent"
  // holds a boolean value indicating the presence of the
  // actual argument at runtime.
  std::optional<mlir::Value> isPresent;
};

/// Vector of pre-lowered actual arguments. nullopt if the actual is
/// "statically" absent (if it was not syntactically  provided).
using PreparedActualArguments =
    toolchain::SmallVector<std::optional<PreparedActualArgument>>;

std::optional<hlfir::EntityWithAttributes> lowerHlfirIntrinsic(
    fir::FirOpBuilder &builder, mlir::Location loc, const std::string &name,
    const language::Compability::lower::PreparedActualArguments &loweredActuals,
    const fir::IntrinsicArgumentLoweringRules *argLowering,
    mlir::Type stmtResultType);

} // namespace language::Compability::lower
#endif // FORTRAN_LOWER_HLFIRINTRINSICS_H
