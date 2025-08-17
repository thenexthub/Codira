//===-- ConvertCall.h -- lowering of calls ----------------------*- C++ -*-===//
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
/// Implements the conversion from evaluate::ProcedureRef to FIR.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERTCALL_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERTCALL_H

#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/CallInterface.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include <optional>

namespace language::Compability::lower {

/// Data structure packaging the SSA value(s) produced for the result of lowered
/// function calls.
using LoweredResult =
    std::variant<fir::ExtendedValue, hlfir::EntityWithAttributes>;

/// Given a call site for which the arguments were already lowered, generate
/// the call and return the result. This function deals with explicit result
/// allocation and lowering if needed. It also deals with passing the host
/// link to internal procedures.
/// \p isElemental must be set to true if elemental call is being produced.
/// It is only used for HLFIR.
/// The returned boolean indicates if finalization has been emitted in
/// \p stmtCtx for the result.
std::pair<LoweredResult, bool> genCallOpAndResult(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx,
    language::Compability::lower::CallerInterface &caller, mlir::FunctionType callSiteType,
    std::optional<mlir::Type> resultType, bool isElemental = false);

/// If \p arg is the address of a function with a denoted host-association tuple
/// argument, then return the host-associations tuple value of the current
/// procedure. Otherwise, return nullptr.
mlir::Value argumentHostAssocs(language::Compability::lower::AbstractConverter &converter,
                               mlir::Value arg);

/// Is \p procRef an intrinsic module procedure that should be lowered as
/// intrinsic procedures (with Optimizer/Builder/IntrinsicCall.h)?
bool isIntrinsicModuleProcRef(const language::Compability::evaluate::ProcedureRef &procRef);

/// Lower a ProcedureRef to HLFIR. If this is a function call, return the
/// lowered result value. Return nothing otherwise.
std::optional<hlfir::EntityWithAttributes> convertCallToHLFIR(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const evaluate::ProcedureRef &procRef, std::optional<mlir::Type> resultType,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx);

void convertUserDefinedAssignmentToHLFIR(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const evaluate::ProcedureRef &procRef, hlfir::Entity lhs, hlfir::Entity rhs,
    language::Compability::lower::SymMap &symMap);
} // namespace language::Compability::lower
#endif // FORTRAN_LOWER_CONVERTCALL_H
