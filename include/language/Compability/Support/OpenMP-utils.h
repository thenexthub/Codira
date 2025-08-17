//===-- language/Compability/Support/OpenMP-utils.h --------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SUPPORT_OPENMP_UTILS_H_
#define LANGUAGE_COMPABILITY_SUPPORT_OPENMP_UTILS_H_

#include "language/Compability/Semantics/symbol.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/Value.h"

#include "toolchain/ADT/ArrayRef.h"

namespace language::Compability::common::openmp {
/// Structure holding the information needed to create and bind entry block
/// arguments associated to a single clause.
struct EntryBlockArgsEntry {
  toolchain::ArrayRef<const language::Compability::semantics::Symbol *> syms;
  toolchain::ArrayRef<mlir::Value> vars;

  bool isValid() const {
    // This check allows specifying a smaller number of symbols than values
    // because in some case cases a single symbol generates multiple block
    // arguments.
    return syms.size() <= vars.size();
  }
};

/// Structure holding the information needed to create and bind entry block
/// arguments associated to all clauses that can define them.
struct EntryBlockArgs {
  EntryBlockArgsEntry hasDeviceAddr;
  toolchain::ArrayRef<mlir::Value> hostEvalVars;
  EntryBlockArgsEntry inReduction;
  EntryBlockArgsEntry map;
  EntryBlockArgsEntry priv;
  EntryBlockArgsEntry reduction;
  EntryBlockArgsEntry taskReduction;
  EntryBlockArgsEntry useDeviceAddr;
  EntryBlockArgsEntry useDevicePtr;

  bool isValid() const {
    return hasDeviceAddr.isValid() && inReduction.isValid() && map.isValid() &&
        priv.isValid() && reduction.isValid() && taskReduction.isValid() &&
        useDeviceAddr.isValid() && useDevicePtr.isValid();
  }

  auto getSyms() const {
    return toolchain::concat<const semantics::Symbol *const>(hasDeviceAddr.syms,
        inReduction.syms, map.syms, priv.syms, reduction.syms,
        taskReduction.syms, useDeviceAddr.syms, useDevicePtr.syms);
  }

  auto getVars() const {
    return toolchain::concat<const mlir::Value>(hasDeviceAddr.vars, hostEvalVars,
        inReduction.vars, map.vars, priv.vars, reduction.vars,
        taskReduction.vars, useDeviceAddr.vars, useDevicePtr.vars);
  }
};

/// Create an entry block for the given region, including the clause-defined
/// arguments specified.
///
/// \param [in] builder - MLIR operation builder.
/// \param [in]    args - entry block arguments information for the given
///                       operation.
/// \param [in]  region - Empty region in which to create the entry block.
mlir::Block *genEntryBlock(
    mlir::OpBuilder &builder, const EntryBlockArgs &args, mlir::Region &region);
} // namespace language::Compability::common::openmp

#endif // FORTRAN_SUPPORT_OPENMP_UTILS_H_
