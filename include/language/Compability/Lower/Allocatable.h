//===-- Allocatable.h -- Allocatable statements lowering ------------------===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_ALLOCATABLE_H
#define LANGUAGE_COMPABILITY_LOWER_ALLOCATABLE_H

#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Optimizer/Builder/MutableBox.h"
#include "language/Compability/Runtime/allocator-registry-consts.h"
#include "toolchain/ADT/StringRef.h"

namespace mlir {
class Value;
class ValueRange;
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace language::Compability {
namespace parser {
struct AllocateStmt;
struct DeallocateStmt;
} // namespace parser

namespace semantics {
class Symbol;
class DerivedTypeSpec;
} // namespace semantics

namespace lower {
struct SymbolBox;

class StatementContext;

bool isArraySectionWithoutVectorSubscript(const SomeExpr &expr);

/// Lower an allocate statement to fir.
void genAllocateStmt(AbstractConverter &converter,
                     const parser::AllocateStmt &stmt, mlir::Location loc);

/// Lower a deallocate statement to fir.
void genDeallocateStmt(AbstractConverter &converter,
                       const parser::DeallocateStmt &stmt, mlir::Location loc);

void genDeallocateBox(AbstractConverter &converter,
                      const fir::MutableBoxValue &box, mlir::Location loc,
                      const language::Compability::semantics::Symbol *sym = nullptr,
                      mlir::Value declaredTypeDesc = {});

/// Deallocate an allocatable if it is allocated at the end of its lifetime.
void genDeallocateIfAllocated(AbstractConverter &converter,
                              const fir::MutableBoxValue &box,
                              mlir::Location loc,
                              const language::Compability::semantics::Symbol *sym = nullptr);

/// Create a MutableBoxValue for an allocatable or pointer entity.
/// If the variables is a local variable that is not a dummy, it will be
/// initialized to unallocated/diassociated status.
fir::MutableBoxValue
createMutableBox(AbstractConverter &converter, mlir::Location loc,
                 const pft::Variable &var, mlir::Value boxAddr,
                 mlir::ValueRange nonDeferredParams, bool alwaysUseBox,
                 unsigned allocator = kDefaultAllocator);

/// Assign a boxed value to a boxed variable, \p box (known as a
/// MutableBoxValue). Expression \p source will be lowered to build the
/// assignment. If \p lbounds is not empty, it is used to define the result's
/// lower bounds. Otherwise, the lower bounds from \p source will be used.
void associateMutableBox(AbstractConverter &converter, mlir::Location loc,
                         const fir::MutableBoxValue &box,
                         const SomeExpr &source, mlir::ValueRange lbounds,
                         StatementContext &stmtCtx);

/// Is \p expr a reference to an entity with the ALLOCATABLE attribute?
bool isWholeAllocatable(const SomeExpr &expr);

/// Is \p expr a reference to an entity with the POINTER attribute?
bool isWholePointer(const SomeExpr &expr);

/// Read the length from \p box for an assumed length character allocatable or
/// pointer dummy argument given by \p sym.
mlir::Value getAssumedCharAllocatableOrPointerLen(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const language::Compability::semantics::Symbol &sym, mlir::Value box);

/// Retrieve the address of a type descriptor from its derived type spec.
mlir::Value
getTypeDescAddr(AbstractConverter &converter, mlir::Location loc,
                const language::Compability::semantics::DerivedTypeSpec &typeSpec);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_ALLOCATABLE_H
