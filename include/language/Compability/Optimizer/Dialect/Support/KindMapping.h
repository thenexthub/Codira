//===-- Optimizer/Support/KindMapping.h -- support kind mapping -*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_KINDMAPPING_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_KINDMAPPING_H

#include "mlir/IR/OpDefinition.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/IR/Type.h"

namespace toolchain {
struct fltSemantics;
} // namespace toolchain

namespace fir {

/// The kind mapping is an encoded string that informs FIR how the Fortran KIND
/// values from the front-end should be converted to LLVM IR types.  This
/// encoding allows the mapping from front-end KIND values to backend LLVM IR
/// types to be customized by the front-end.
///
/// The provided string uses the following syntax.
///
///   intrinsic-key `:` kind-value (`,` intrinsic-key `:` kind-value)*
///
/// intrinsic-key is a single character for the intrinsic type.
///   'i' : INTEGER   (size in bits)
///   'l' : LOGICAL   (size in bits)
///   'a' : CHARACTER (size in bits)
///   'r' : REAL    (encoding value)
///   'c' : COMPLEX (encoding value)
///
/// kind-value is either an unsigned integer (for 'i', 'l', and 'a') or one of
/// 'Half', 'BFloat', 'Float', 'Double', 'X86_FP80', or 'FP128' (for 'r' and
/// 'c').
///
/// If LLVM adds support for new floating-point types, the final list should be
/// extended.
class KindMapping {
public:
  using KindTy = unsigned;
  using Bitsize = unsigned;
  using LLVMTypeID = toolchain::Type::TypeID;
  using MatchResult = mlir::ParseResult;

  /// KindMapping constructor with both the kind map and default kinds read from
  /// command-line options.
  explicit KindMapping(mlir::MLIRContext *context);
  /// KindMapping constructor taking a `defs` argument to specify the default
  /// kinds for intrinsic types. To set the default kinds, an ArrayRef of 6
  /// KindTy must be passed. The kinds must be the given in the following order:
  /// CHARACTER, COMPLEX, DOUBLE PRECISION, INTEGER, LOGICAL, and REAL.  The
  /// kind map is read from command-line options, if given.
  explicit KindMapping(mlir::MLIRContext *context, toolchain::ArrayRef<KindTy> defs);
  /// KindMapping constructor taking an optional `defs` argument to specify the
  /// default kinds for intrinsic types. To set the default kinds, an ArrayRef
  /// of 6 KindTy must be passed. The kinds must be the given in the following
  /// order: CHARACTER, COMPLEX, DOUBLE PRECISION, INTEGER, LOGICAL, and REAL.
  explicit KindMapping(mlir::MLIRContext *context, toolchain::StringRef map,
                       toolchain::ArrayRef<KindTy> defs = {});
  explicit KindMapping(mlir::MLIRContext *context, toolchain::StringRef map,
                       toolchain::StringRef defs)
      : KindMapping{context, map, toDefaultKinds(defs)} {}

  /// Get the size in bits of !fir.char<kind>
  Bitsize getCharacterBitsize(KindTy kind) const;

  /// Get the size in bits of !fir.int<kind>
  Bitsize getIntegerBitsize(KindTy kind) const;

  /// Get the size in bits of !fir.logical<kind>
  Bitsize getLogicalBitsize(KindTy kind) const;

  /// Get the size in bits of !fir.real<kind>
  Bitsize getRealBitsize(KindTy kind) const;

  /// Get the LLVM Type::TypeID of !fir.real<kind>
  LLVMTypeID getRealTypeID(KindTy kind) const;

  /// Get the LLVM Type::TypeID of !fir.complex<kind>
  LLVMTypeID getComplexTypeID(KindTy kind) const;

  mlir::MLIRContext *getContext() const { return context; }

  /// Get the float semantics of !fir.real<kind>
  const toolchain::fltSemantics &getFloatSemantics(KindTy kind) const;

  /// Get the default kind map as a string.
  static constexpr const char *getDefaultMap() { return ""; }

  /// Convert the current kind map to a string.
  std::string mapToString() const;

  //===--------------------------------------------------------------------===//
  // Default kinds of intrinsic types
  //===--------------------------------------------------------------------===//

  KindTy defaultCharacterKind() const;
  KindTy defaultComplexKind() const;
  KindTy defaultDoubleKind() const;
  KindTy defaultIntegerKind() const;
  KindTy defaultLogicalKind() const;
  KindTy defaultRealKind() const;

  /// Get the default kinds as a string.
  static constexpr const char *getDefaultKinds() { return "a1c4d8i4l4r4"; }

  /// Convert the current default kinds to a string.
  std::string defaultsToString() const;

  /// Translate a default kinds string into a default kind vector. This vector
  /// can be passed to the KindMapping ctor.
  static std::vector<KindTy> toDefaultKinds(toolchain::StringRef defs);

private:
  MatchResult badMapString(const toolchain::Twine &ptr);
  MatchResult parse(toolchain::StringRef kindMap);
  toolchain::LogicalResult setDefaultKinds(toolchain::ArrayRef<KindTy> defs);

  mlir::MLIRContext *context;
  toolchain::DenseMap<std::pair<char, KindTy>, Bitsize> intMap;
  toolchain::DenseMap<std::pair<char, KindTy>, LLVMTypeID> floatMap;
  toolchain::DenseMap<char, KindTy> defaultMap;
};

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_KINDMAPPING_H
