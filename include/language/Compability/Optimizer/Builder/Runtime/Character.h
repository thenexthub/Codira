//===-- Character.h -- generate calls to character runtime API --*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_CHARACTER_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_CHARACTER_H

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace fir {
class ExtendedValue;
class FirOpBuilder;
} // namespace fir

namespace fir::runtime {

/// Generate a call to the `ADJUSTL` runtime.
/// This calls the simple runtime entry point that then calls into the more
/// complex runtime cases handling left or right adjustments.
///
/// \p resultBox must be an unallocated allocatable used for the temporary
/// result. \p StringBox must be a `fir.box` describing the `ADJUSTL` string
/// argument. Note that the \p genAdjust() helper is called to do the majority
/// of the lowering work.
void genAdjustL(fir::FirOpBuilder &builder, mlir::Location loc,
                mlir::Value resultBox, mlir::Value stringBox);

/// Generate a call to the `ADJUSTR` runtime.
/// This calls the simple runtime entry point that then calls into the more
/// complex runtime cases handling left or right adjustments.
///
/// \p resultBox must be an unallocated allocatable used for the temporary
/// result.  \p StringBox must be a fir.box describing the adjustr string
/// argument. Note that the \p genAdjust() helper is called to do the majority
/// of the lowering work.
void genAdjustR(fir::FirOpBuilder &builder, mlir::Location loc,
                mlir::Value resultBox, mlir::Value stringBox);

/// Generate call to a character comparison for two ssa-values of type
/// `boxchar`.
mlir::Value genCharCompare(fir::FirOpBuilder &builder, mlir::Location loc,
                           mlir::arith::CmpIPredicate cmp,
                           const fir::ExtendedValue &lhs,
                           const fir::ExtendedValue &rhs);

/// Generate call to a character comparison op for two unboxed variables. There
/// are 4 arguments, 2 for the lhs and 2 for the rhs. Each CHARACTER must pass a
/// reference to its buffer (`ref<char<K>>`) and its LEN type parameter (some
/// integral type).
mlir::Value genCharCompare(fir::FirOpBuilder &builder, mlir::Location loc,
                           mlir::arith::CmpIPredicate cmp, mlir::Value lhsBuff,
                           mlir::Value lhsLen, mlir::Value rhsBuff,
                           mlir::Value rhsLen);

/// Generate call to INDEX runtime.
/// This calls the simple runtime entry points based on the KIND of the string.
/// No descriptors are used.
mlir::Value genIndex(fir::FirOpBuilder &builder, mlir::Location loc, int kind,
                     mlir::Value stringBase, mlir::Value stringLen,
                     mlir::Value substringBase, mlir::Value substringLen,
                     mlir::Value back);

/// Generate call to INDEX runtime.
/// This calls the descriptor based runtime call implementation for the index
/// intrinsic.
void genIndexDescriptor(fir::FirOpBuilder &builder, mlir::Location loc,
                        mlir::Value resultBox, mlir::Value stringBox,
                        mlir::Value substringBox, mlir::Value backOpt,
                        mlir::Value kind);

/// Generate call to repeat runtime.
///   \p resultBox must be an unallocated allocatable used for the temporary
///   result. \p stringBox must be a fir.box describing repeat string argument.
///   \p ncopies must be a value representing the number of copies.
/// The runtime will always allocate the resultBox.
void genRepeat(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value resultBox, mlir::Value stringBox,
               mlir::Value ncopies);

/// Generate call to trim runtime.
///   \p resultBox must be an unallocated allocatable used for the temporary
///   result. \p stringBox must be a fir.box describing trim string argument.
/// The runtime will always allocate the resultBox.
void genTrim(fir::FirOpBuilder &builder, mlir::Location loc,
             mlir::Value resultBox, mlir::Value stringBox);

/// Generate call to scan runtime.
/// This calls the descriptor based runtime call implementation of the scan
/// intrinsics.
void genScanDescriptor(fir::FirOpBuilder &builder, mlir::Location loc,
                       mlir::Value resultBox, mlir::Value stringBox,
                       mlir::Value setBox, mlir::Value backBox,
                       mlir::Value kind);

/// Generate call to the scan runtime routine that is specialized on
/// \param kind.
/// The \param kind represents the kind of the elements in the strings.
mlir::Value genScan(fir::FirOpBuilder &builder, mlir::Location loc, int kind,
                    mlir::Value stringBase, mlir::Value stringLen,
                    mlir::Value setBase, mlir::Value setLen, mlir::Value back);

/// Generate call to verify runtime.
/// This calls the descriptor based runtime call implementation of the scan
/// intrinsics.
void genVerifyDescriptor(fir::FirOpBuilder &builder, mlir::Location loc,
                         mlir::Value resultBox, mlir::Value stringBox,
                         mlir::Value setBox, mlir::Value backBox,
                         mlir::Value kind);

/// Generate call to the verify runtime routine that is specialized on
/// \param kind.
/// The \param kind represents the kind of the elements in the strings.
mlir::Value genVerify(fir::FirOpBuilder &builder, mlir::Location loc, int kind,
                      mlir::Value stringBase, mlir::Value stringLen,
                      mlir::Value setBase, mlir::Value setLen,
                      mlir::Value back);

} // namespace fir::runtime

#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_CHARACTER_H
