//===- Lower/ConvertVariable.h -- lowering of variables to FIR --*- C++ -*-===//
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
/// Instantiation of pft::Variable in FIR/MLIR.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERT_VARIABLE_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERT_VARIABLE_H

#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Semantics/symbol.h"
#include "mlir/IR/Value.h"
#include "toolchain/ADT/DenseMap.h"

namespace cuf {
class DataAttributeAttr;
}

namespace fir {
class ExtendedValue;
class FirOpBuilder;
class GlobalOp;
class FortranVariableFlagsAttr;
} // namespace fir

namespace language::Compability {
namespace semantics {
class Scope;
} // namespace semantics

namespace lower {
class AbstractConverter;
class CallerInterface;
class StatementContext;
class SymMap;
namespace pft {
struct Variable;
}

/// AggregateStoreMap is used to keep track of instantiated aggregate stores
/// when lowering a scope containing equivalences (aliases). It must only be
/// owned by the code lowering a scope and provided to instantiateVariable.
using AggregateStoreKey =
    std::tuple<const language::Compability::semantics::Scope *, std::size_t>;
using AggregateStoreMap = toolchain::DenseMap<AggregateStoreKey, mlir::Value>;

/// Instantiate variable \p var and add it to \p symMap.
/// The AbstractConverter builder must be set.
/// The AbstractConverter own symbol mapping is not used during the
/// instantiation and can be different form \p symMap.
void instantiateVariable(AbstractConverter &, const pft::Variable &var,
                         SymMap &symMap, AggregateStoreMap &storeMap);

/// Does this variable have a default initialization?
bool hasDefaultInitialization(const language::Compability::semantics::Symbol &sym);

/// Call default initialization runtime routine to initialize \p var.
void defaultInitializeAtRuntime(language::Compability::lower::AbstractConverter &converter,
                                const language::Compability::semantics::Symbol &sym,
                                language::Compability::lower::SymMap &symMap);

/// Call clone initialization runtime routine to initialize \p sym's value.
void initializeCloneAtRuntime(language::Compability::lower::AbstractConverter &converter,
                              const language::Compability::semantics::Symbol &sym,
                              language::Compability::lower::SymMap &symMap);

/// Create a fir::GlobalOp given a module variable definition. This is intended
/// to be used when lowering a module definition, not when lowering variables
/// used from a module. For used variables instantiateVariable must directly be
/// called.
void defineModuleVariable(AbstractConverter &, const pft::Variable &var);

/// Create fir::GlobalOp for all common blocks, including their initial values
/// if they have one. This should be called before lowering any scopes so that
/// common block globals are available when a common appear in a scope.
void defineCommonBlocks(
    AbstractConverter &,
    const std::vector<std::pair<semantics::SymbolRef, std::size_t>>
        &commonBlocks);

/// The COMMON block is a global structure. \p commonValue is the base address
/// of the COMMON block. As the offset from the symbol \p sym, generate the
/// COMMON block member value (commonValue + offset) for the symbol.
mlir::Value genCommonBlockMember(AbstractConverter &converter,
                                 mlir::Location loc,
                                 const language::Compability::semantics::Symbol &sym,
                                 mlir::Value commonValue);

/// Lower a symbol attributes given an optional storage \p and add it to the
/// provided symbol map. If \preAlloc is not provided, a temporary storage will
/// be allocated. This is a low level function that should only be used if
/// instantiateVariable cannot be called.
void mapSymbolAttributes(AbstractConverter &, const pft::Variable &, SymMap &,
                         StatementContext &, mlir::Value preAlloc = {});
void mapSymbolAttributes(AbstractConverter &, const semantics::SymbolRef &,
                         SymMap &, StatementContext &,
                         mlir::Value preAlloc = {});

/// Instantiate the variables that appear in the specification expressions
/// of the result of a function call. The instantiated variables are added
/// to \p symMap.
void mapCallInterfaceSymbolsForResult(
    AbstractConverter &, const language::Compability::lower::CallerInterface &caller,
    SymMap &symMap);

/// Instantiate the variables that appear in the specification expressions
/// of a dummy argument of a procedure call. The instantiated variables are
/// added to \p symMap.
void mapCallInterfaceSymbolsForDummyArgument(
    AbstractConverter &, const language::Compability::lower::CallerInterface &caller,
    SymMap &symMap, const language::Compability::semantics::Symbol &dummySymbol);

// TODO: consider saving the initial expression symbol dependence analysis in
// in the PFT variable and dealing with the dependent symbols instantiation in
// the fir::GlobalOp body at the fir::GlobalOp creation point rather than by
// having genExtAddrInInitializer and genInitialDataTarget custom entry points
// here to deal with this while lowering the initial expression value.

/// Create initial-data-target fir.box in a global initializer region.
/// This handles the local instantiation of the target variable.
mlir::Value genInitialDataTarget(language::Compability::lower::AbstractConverter &,
                                 mlir::Location, mlir::Type boxType,
                                 const SomeExpr &initialTarget,
                                 bool couldBeInEquivalence = false);

/// Create the global op and its init if it has one
fir::GlobalOp defineGlobal(language::Compability::lower::AbstractConverter &converter,
                           const language::Compability::lower::pft::Variable &var,
                           toolchain::StringRef globalName, mlir::StringAttr linkage,
                           cuf::DataAttributeAttr dataAttr = {});

/// Generate address \p addr inside an initializer.
fir::ExtendedValue
genExtAddrInInitializer(language::Compability::lower::AbstractConverter &converter,
                        mlir::Location loc, const SomeExpr &addr);

/// Create a global variable for an intrinsic module object.
void createIntrinsicModuleGlobal(language::Compability::lower::AbstractConverter &converter,
                                 const pft::Variable &);

/// Create a global variable for a compiler generated object that describes a
/// derived type for the runtime.
void createRuntimeTypeInfoGlobal(language::Compability::lower::AbstractConverter &converter,
                                 const language::Compability::semantics::Symbol &typeInfoSym);

/// Translate the Fortran attributes of \p sym into the FIR variable attribute
/// representation.
fir::FortranVariableFlagsAttr
translateSymbolAttributes(mlir::MLIRContext *mlirContext,
                          const language::Compability::semantics::Symbol &sym,
                          fir::FortranVariableFlagsEnum extraFlags =
                              fir::FortranVariableFlagsEnum::None);

/// Map a symbol to a given fir::ExtendedValue. This will generate an
/// hlfir.declare when lowering to HLFIR and map the hlfir.declare result to the
/// symbol.
void genDeclareSymbol(language::Compability::lower::AbstractConverter &converter,
                      language::Compability::lower::SymMap &symMap,
                      const language::Compability::semantics::Symbol &sym,
                      const fir::ExtendedValue &exv,
                      fir::FortranVariableFlagsEnum extraFlags =
                          fir::FortranVariableFlagsEnum::None,
                      bool force = false);

/// Given the Fortran type of a Cray pointee, return the fir.box type used to
/// track the cray pointee as Fortran pointer.
mlir::Type getCrayPointeeBoxType(mlir::Type);

/// If the given array symbol must be repacked into contiguous
/// memory, generate fir.pack_array for the given box array value.
/// The returned extended value is a box with the same properties
/// as the original.
fir::ExtendedValue genPackArray(language::Compability::lower::AbstractConverter &converter,
                                const language::Compability::semantics::Symbol &sym,
                                fir::ExtendedValue exv);

/// Given an operation defining the variable corresponding
/// to the given symbol, generate fir.unpack_array operation
/// that reverts the effect of fir.pack_array.
/// \p def is expected to be hlfir.declare operation.
void genUnpackArray(language::Compability::lower::AbstractConverter &converter,
                    mlir::Location loc, fir::FortranVariableOpInterface def,
                    const language::Compability::semantics::Symbol &sym);

} // namespace lower
} // namespace language::Compability
#endif // FORTRAN_LOWER_CONVERT_VARIABLE_H
