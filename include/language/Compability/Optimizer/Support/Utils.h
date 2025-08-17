//===-- Optimizer/Support/Utils.h -------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_UTILS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_UTILS_H

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/CUF/Attributes/CUFAttr.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Support/FatalError.h"
#include "language/Compability/Support/default-kinds.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringRef.h"

namespace fir {
/// Return the integer value of a arith::ConstantOp.
inline std::int64_t toInt(mlir::arith::ConstantOp cop) {
  return mlir::cast<mlir::IntegerAttr>(cop.getValue())
      .getValue()
      .getSExtValue();
}

// Translate front-end KINDs for use in the IR and code gen.
inline std::vector<fir::KindTy>
fromDefaultKinds(const language::Compability::common::IntrinsicTypeDefaultKinds &defKinds) {
  return {static_cast<fir::KindTy>(defKinds.GetDefaultKind(
              language::Compability::common::TypeCategory::Character)),
          static_cast<fir::KindTy>(
              defKinds.GetDefaultKind(language::Compability::common::TypeCategory::Complex)),
          static_cast<fir::KindTy>(defKinds.doublePrecisionKind()),
          static_cast<fir::KindTy>(
              defKinds.GetDefaultKind(language::Compability::common::TypeCategory::Integer)),
          static_cast<fir::KindTy>(
              defKinds.GetDefaultKind(language::Compability::common::TypeCategory::Logical)),
          static_cast<fir::KindTy>(
              defKinds.GetDefaultKind(language::Compability::common::TypeCategory::Real))};
}

inline std::string mlirTypeToString(mlir::Type type) {
  std::string result{};
  toolchain::raw_string_ostream sstream(result);
  sstream << type;
  return result;
}

inline std::optional<int> mlirFloatTypeToKind(mlir::Type type) {
  if (type.isF16())
    return 2;
  else if (type.isBF16())
    return 3;
  else if (type.isF32())
    return 4;
  else if (type.isF64())
    return 8;
  else if (type.isF80())
    return 10;
  else if (type.isF128())
    return 16;
  return std::nullopt;
}

inline std::string mlirTypeToIntrinsicFortran(fir::FirOpBuilder &builder,
                                              mlir::Type type,
                                              mlir::Location loc,
                                              const toolchain::Twine &name) {
  if (auto floatTy = mlir::dyn_cast<mlir::FloatType>(type)) {
    if (std::optional<int> kind = mlirFloatTypeToKind(type))
      return "REAL(KIND="s + std::to_string(*kind) + ")";
  } else if (auto cplxTy = mlir::dyn_cast<mlir::ComplexType>(type)) {
    if (std::optional<int> kind = mlirFloatTypeToKind(cplxTy.getElementType()))
      return "COMPLEX(KIND+"s + std::to_string(*kind) + ")";
  } else if (type.isUnsignedInteger()) {
    if (type.isInteger(8))
      return "UNSIGNED(KIND=1)";
    else if (type.isInteger(16))
      return "UNSIGNED(KIND=2)";
    else if (type.isInteger(32))
      return "UNSIGNED(KIND=4)";
    else if (type.isInteger(64))
      return "UNSIGNED(KIND=8)";
    else if (type.isInteger(128))
      return "UNSIGNED(KIND=16)";
  } else if (type.isInteger(8))
    return "INTEGER(KIND=1)";
  else if (type.isInteger(16))
    return "INTEGER(KIND=2)";
  else if (type.isInteger(32))
    return "INTEGER(KIND=4)";
  else if (type.isInteger(64))
    return "INTEGER(KIND=8)";
  else if (type.isInteger(128))
    return "INTEGER(KIND=16)";
  else if (type == fir::LogicalType::get(builder.getContext(), 1))
    return "LOGICAL(KIND=1)";
  else if (type == fir::LogicalType::get(builder.getContext(), 2))
    return "LOGICAL(KIND=2)";
  else if (type == fir::LogicalType::get(builder.getContext(), 4))
    return "LOGICAL(KIND=4)";
  else if (type == fir::LogicalType::get(builder.getContext(), 8))
    return "LOGICAL(KIND=8)";

  fir::emitFatalError(loc, "unsupported type in " + name + ": " +
                               fir::mlirTypeToString(type));
}

inline void intrinsicTypeTODO(fir::FirOpBuilder &builder, mlir::Type type,
                              mlir::Location loc,
                              const toolchain::Twine &intrinsicName) {
  TODO(loc,
       "intrinsic: " +
           fir::mlirTypeToIntrinsicFortran(builder, type, loc, intrinsicName) +
           " in " + intrinsicName);
}

inline void intrinsicTypeTODO2(fir::FirOpBuilder &builder, mlir::Type type1,
                               mlir::Type type2, mlir::Location loc,
                               const toolchain::Twine &intrinsicName) {
  TODO(loc,
       "intrinsic: {" +
           fir::mlirTypeToIntrinsicFortran(builder, type2, loc, intrinsicName) +
           ", " +
           fir::mlirTypeToIntrinsicFortran(builder, type2, loc, intrinsicName) +
           "} in " + intrinsicName);
}

inline std::pair<language::Compability::common::TypeCategory, KindMapping::KindTy>
mlirTypeToCategoryKind(mlir::Location loc, mlir::Type type) {
  if (auto floatTy = mlir::dyn_cast<mlir::FloatType>(type)) {
    if (std::optional<int> kind = mlirFloatTypeToKind(type))
      return {language::Compability::common::TypeCategory::Real, *kind};
  } else if (auto cplxTy = mlir::dyn_cast<mlir::ComplexType>(type)) {
    if (std::optional<int> kind = mlirFloatTypeToKind(cplxTy.getElementType()))
      return {language::Compability::common::TypeCategory::Complex, *kind};
  } else if (type.isInteger(8))
    return {type.isUnsignedInteger() ? language::Compability::common::TypeCategory::Unsigned
                                     : language::Compability::common::TypeCategory::Integer,
            1};
  else if (type.isInteger(16))
    return {type.isUnsignedInteger() ? language::Compability::common::TypeCategory::Unsigned
                                     : language::Compability::common::TypeCategory::Integer,
            2};
  else if (type.isInteger(32))
    return {type.isUnsignedInteger() ? language::Compability::common::TypeCategory::Unsigned
                                     : language::Compability::common::TypeCategory::Integer,
            4};
  else if (type.isInteger(64))
    return {type.isUnsignedInteger() ? language::Compability::common::TypeCategory::Unsigned
                                     : language::Compability::common::TypeCategory::Integer,
            8};
  else if (type.isInteger(128))
    return {type.isUnsignedInteger() ? language::Compability::common::TypeCategory::Unsigned
                                     : language::Compability::common::TypeCategory::Integer,
            16};
  else if (auto logicalType = mlir::dyn_cast<fir::LogicalType>(type))
    return {language::Compability::common::TypeCategory::Logical, logicalType.getFKind()};
  else if (auto charType = mlir::dyn_cast<fir::CharacterType>(type))
    return {language::Compability::common::TypeCategory::Character, charType.getFKind()};
  else if (mlir::isa<fir::RecordType>(type))
    return {language::Compability::common::TypeCategory::Derived, 0};
  fir::emitFatalError(loc, "unsupported type: " + fir::mlirTypeToString(type));
}

/// Find the fir.type_info that was created for this \p recordType in \p module,
/// if any. \p  symbolTable can be provided to speed-up the lookup. This tool
/// will match record type even if they have been "altered" in type conversion
/// passes.
fir::TypeInfoOp
lookupTypeInfoOp(fir::RecordType recordType, mlir::ModuleOp module,
                 const mlir::SymbolTable *symbolTable = nullptr);

/// Find the fir.type_info named \p name in \p module, if any. \p  symbolTable
/// can be provided to speed-up the lookup. Prefer using the equivalent with a
/// RecordType argument  unless it is certain \p name has not been altered by a
/// pass rewriting fir.type (see NameUniquer::dropTypeConversionMarkers).
fir::TypeInfoOp
lookupTypeInfoOp(toolchain::StringRef name, mlir::ModuleOp module,
                 const mlir::SymbolTable *symbolTable = nullptr);

/// Returns all lower bounds of \p component if it is an array component of \p
/// recordType with non default lower bounds. Returns nullopt if this is not an
/// array componnet of \p recordType or if its lower bounds are all ones.
std::optional<toolchain::ArrayRef<int64_t>> getComponentLowerBoundsIfNonDefault(
    fir::RecordType recordType, toolchain::StringRef component,
    mlir::ModuleOp module, const mlir::SymbolTable *symbolTable = nullptr);

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_UTILS_H
