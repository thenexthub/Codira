//===-- Lower/CustomIntrinsicCall.h -----------------------------*- C++ -*-===//
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
/// Custom intrinsic lowering for the few intrinsic that have optional
/// arguments that prevents them to be handled in a more generic way in
/// IntrinsicCall.cpp.
/// The core principle is that this interface provides the intrinsic arguments
/// via callbacks to generate fir::ExtendedValue (instead of a list of
/// precomputed fir::ExtendedValue as done in the default intrinsic call
/// lowering). This gives more flexibility to only generate references to
/// dynamically optional arguments (pointers, allocatables, OPTIONAL dummies) in
/// a safe way.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CUSTOMINTRINSICCALL_H
#define LANGUAGE_COMPABILITY_LOWER_CUSTOMINTRINSICCALL_H

#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Optimizer/Builder/IntrinsicCall.h"
#include <functional>
#include <optional>

namespace language::Compability {

namespace evaluate {
class ProcedureRef;
struct SpecificIntrinsic;
} // namespace evaluate

namespace lower {

/// Does the call \p procRef to \p intrinsic need to be handle via this custom
/// framework due to optional arguments. Otherwise, the tools from
/// IntrinsicCall.cpp should be used directly.
bool intrinsicRequiresCustomOptionalHandling(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    AbstractConverter &converter);

/// Type of callback to be provided to prepare the arguments fetching from an
/// actual argument expression.
using OperandPrepare = std::function<void(const language::Compability::lower::SomeExpr &)>;
using OperandPrepareAs = std::function<void(const language::Compability::lower::SomeExpr &,
                                            fir::LowerIntrinsicArgAs)>;

/// Type of the callback to inquire about an argument presence, once the call
/// preparation was done. An absent optional means the argument is statically
/// present. An mlir::Value means the presence must be checked at runtime, and
/// that the value contains the "is present" boolean value.
using OperandPresent = std::function<std::optional<mlir::Value>(std::size_t)>;

/// Type of the callback to generate an argument reference after the call
/// preparation was done. For optional arguments, the utility guarantees
/// these callbacks will only be called in regions where the presence was
/// verified. This means the getter callback can dereference the argument
/// without any special care.
/// For elemental intrinsics, the getter must provide the current iteration
/// element value. If the boolean argument is true, the callback must load the
/// argument before returning it.
using OperandGetter = std::function<fir::ExtendedValue(std::size_t, bool)>;

/// Given a callback \p prepareOptionalArgument to prepare optional
/// arguments and a callback \p prepareOtherArgument to prepare non-optional
/// arguments prepare the intrinsic arguments calls.
/// It is up to the caller to decide what argument preparation means,
/// the only contract is that it should later allow the caller to provide
/// callbacks to generate argument reference given an argument index without
/// any further knowledge of the argument. The function simply visits
/// the actual arguments, deciding which ones are dynamically optional,
/// and calling the callbacks accordingly in argument order.
void prepareCustomIntrinsicArgument(
    const language::Compability::evaluate::ProcedureRef &procRef,
    const language::Compability::evaluate::SpecificIntrinsic &intrinsic,
    std::optional<mlir::Type> retTy,
    const OperandPrepare &prepareOptionalArgument,
    const OperandPrepareAs &prepareOtherArgument, AbstractConverter &converter);

/// Given a callback \p getOperand to generate a reference to the i-th argument,
/// and a callback \p isPresentCheck to test if an argument is present, this
/// function lowers the intrinsic calls to \p name whose argument were
/// previously prepared with prepareCustomIntrinsicArgument. The elemental
/// aspects must be taken into account by the caller (i.e, the function should
/// be called during the loop nest generation for elemental intrinsics. It will
/// not generate any implicit loop nest on its own).
fir::ExtendedValue
lowerCustomIntrinsic(fir::FirOpBuilder &builder, mlir::Location loc,
                     toolchain::StringRef name, std::optional<mlir::Type> retTy,
                     const OperandPresent &isPresentCheck,
                     const OperandGetter &getOperand, std::size_t numOperands,
                     language::Compability::lower::StatementContext &stmtCtx);

/// DEPRECATED: NEW CODE SHOULD USE THE VERSION OF genIntrinsicCall WITHOUT A
/// StatementContext, DECLARED IN IntrinsicCall.h
/// Generate the FIR+MLIR operations for the generic intrinsic \p name
/// with argument \p args and expected result type \p resultType.
/// Returned fir::ExtendedValue is the returned Fortran intrinsic value.
fir::ExtendedValue
genIntrinsicCall(fir::FirOpBuilder &builder, mlir::Location loc,
                 toolchain::StringRef name, std::optional<mlir::Type> resultType,
                 toolchain::ArrayRef<fir::ExtendedValue> args,
                 StatementContext &stmtCtx,
                 language::Compability::lower::AbstractConverter *converter = nullptr);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_CUSTOMINTRINSICCALL_H
