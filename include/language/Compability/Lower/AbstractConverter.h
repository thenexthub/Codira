//===-- Lower/AbstractConverter.h -------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_ABSTRACTCONVERTER_H
#define LANGUAGE_COMPABILITY_LOWER_ABSTRACTCONVERTER_H

#include "language/Compability/Lower/LoweringOptions.h"
#include "language/Compability/Lower/PFTDefs.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Semantics/symbol.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"

namespace mlir {
class SymbolTable;
class StateStack;
}

namespace fir {
class KindMapping;
class FirOpBuilder;
} // namespace fir

namespace language::Compability {
namespace common {
template <typename>
class Reference;
}

namespace evaluate {
struct DataRef;
template <typename>
class Expr;
class FoldingContext;
struct SomeType;
} // namespace evaluate

namespace parser {
class CharBlock;
}
namespace semantics {
class Symbol;
class Scope;
class DerivedTypeSpec;
} // namespace semantics

namespace lower {
class SymMap;
struct SymbolBox;
namespace pft {
struct Variable;
struct FunctionLikeUnit;
} // namespace pft

using SomeExpr = language::Compability::evaluate::Expr<language::Compability::evaluate::SomeType>;
using SymbolRef = language::Compability::common::Reference<const language::Compability::semantics::Symbol>;
using TypeConstructionStack =
    toolchain::DenseMap<const language::Compability::semantics::Scope *, mlir::Type>;
class StatementContext;

using ExprToValueMap = toolchain::DenseMap<const SomeExpr *, mlir::Value>;

//===----------------------------------------------------------------------===//
// AbstractConverter interface
//===----------------------------------------------------------------------===//

/// The abstract interface for converter implementations to lower Fortran
/// front-end fragments such as expressions, types, etc. to the FIR dialect of
/// MLIR.
class AbstractConverter {
public:
  //===--------------------------------------------------------------------===//
  // Symbols
  //===--------------------------------------------------------------------===//

  /// Get the mlir instance of a symbol.
  virtual mlir::Value getSymbolAddress(SymbolRef sym) = 0;

  virtual fir::ExtendedValue
  symBoxToExtendedValue(const language::Compability::lower::SymbolBox &symBox) = 0;

  virtual fir::ExtendedValue
  getSymbolExtendedValue(const language::Compability::semantics::Symbol &sym,
                         language::Compability::lower::SymMap *symMap = nullptr) = 0;

  /// Get the binding of an implied do variable by name.
  virtual mlir::Value impliedDoBinding(toolchain::StringRef name) = 0;

  /// Copy the binding of src to target symbol.
  virtual void copySymbolBinding(SymbolRef src, SymbolRef target) = 0;

  /// Binds the symbol to an fir extended value. The symbol binding will be
  /// added or replaced at the inner-most level of the local symbol map.
  virtual void bindSymbol(SymbolRef sym, const fir::ExtendedValue &exval) = 0;

  /// Override lowering of expression with pre-lowered values.
  /// Associate mlir::Value to evaluate::Expr. All subsequent call to
  /// genExprXXX() will replace any occurrence of an overridden
  /// expression in the expression tree by the pre-lowered values.
  virtual void overrideExprValues(const ExprToValueMap *) = 0;
  void resetExprOverrides() { overrideExprValues(nullptr); }
  virtual const ExprToValueMap *getExprOverrides() = 0;

  /// Get the label set associated with a symbol.
  virtual bool lookupLabelSet(SymbolRef sym, pft::LabelSet &labelSet) = 0;

  /// Get the code defined by a label
  virtual pft::Evaluation *lookupLabel(pft::Label label) = 0;

  /// For a given symbol which is host-associated, create a clone using
  /// parameters from the host-associated symbol.
  /// The clone is default initialized if its type has any default
  /// initialization unless `skipDefaultInit` is set.
  virtual bool
  createHostAssociateVarClone(const language::Compability::semantics::Symbol &sym,
                              bool skipDefaultInit) = 0;

  virtual void
  createHostAssociateVarCloneDealloc(const language::Compability::semantics::Symbol &sym) = 0;

  /// For a host-associated symbol (a symbol associated with another symbol from
  /// an enclosing scope), either:
  ///
  /// * if \p hostIsSource == true: copy \p sym's value *from* its corresponding
  /// host symbol,
  ///
  /// * if \p hostIsSource == false: copy \p sym's value *to* its corresponding
  /// host symbol.
  virtual void
  copyHostAssociateVar(const language::Compability::semantics::Symbol &sym,
                       mlir::OpBuilder::InsertPoint *copyAssignIP = nullptr,
                       bool hostIsSource = true) = 0;

  virtual void copyVar(mlir::Location loc, mlir::Value dst, mlir::Value src,
                       fir::FortranVariableFlagsEnum attrs) = 0;

  /// For a given symbol, check if it is present in the inner-most
  /// level of the symbol map.
  virtual bool
  isPresentShallowLookup(const language::Compability::semantics::Symbol &sym) = 0;

  /// Collect the set of symbols with \p flag in \p eval
  /// region if \p collectSymbols is true. Otherwise, collect the
  /// set of the host symbols with \p flag of the associated symbols in \p eval
  /// region if collectHostAssociatedSymbols is true. This allows gathering
  /// host association details of symbols particularly in nested directives
  /// irrespective of \p flag \p, and can be useful where host
  /// association details are needed in flag-agnostic manner.
  virtual void collectSymbolSet(
      pft::Evaluation &eval,
      toolchain::SetVector<const language::Compability::semantics::Symbol *> &symbolSet,
      language::Compability::semantics::Symbol::Flag flag, bool collectSymbols = true,
      bool collectHostAssociatedSymbols = false) = 0;

  /// For the given literal constant \p expression, returns a unique name
  /// that can be used to create a global object to represent this
  /// literal constant. It will return the same name for equivalent
  /// literal constant expressions. \p eleTy specifies the data type
  /// of the constant elements. For array constants it specifies
  /// the array's element type.
  virtual toolchain::StringRef
  getUniqueLitName(mlir::Location loc,
                   std::unique_ptr<language::Compability::lower::SomeExpr> expression,
                   mlir::Type eleTy) = 0;

  //===--------------------------------------------------------------------===//
  // Expressions
  //===--------------------------------------------------------------------===//

  /// Generate the address of the location holding the expression, \p expr.
  /// If \p expr is a Designator that is not compile time contiguous, the
  /// address returned is the one of a contiguous temporary storage holding the
  /// expression value. The clean-up for this temporary is added to \p context.
  virtual fir::ExtendedValue genExprAddr(const SomeExpr &expr,
                                         StatementContext &context,
                                         mlir::Location *locPtr = nullptr) = 0;

  /// Generate the address of the location holding the expression, \p expr.
  fir::ExtendedValue genExprAddr(mlir::Location loc, const SomeExpr *expr,
                                 StatementContext &stmtCtx) {
    return genExprAddr(*expr, stmtCtx, &loc);
  }
  fir::ExtendedValue genExprAddr(mlir::Location loc, const SomeExpr &expr,
                                 StatementContext &stmtCtx) {
    return genExprAddr(expr, stmtCtx, &loc);
  }

  /// Generate the computations of the expression to produce a value.
  virtual fir::ExtendedValue genExprValue(const SomeExpr &expr,
                                          StatementContext &context,
                                          mlir::Location *locPtr = nullptr) = 0;

  /// Generate the computations of the expression, \p expr, to produce a value.
  fir::ExtendedValue genExprValue(mlir::Location loc, const SomeExpr *expr,
                                  StatementContext &stmtCtx) {
    return genExprValue(*expr, stmtCtx, &loc);
  }
  fir::ExtendedValue genExprValue(mlir::Location loc, const SomeExpr &expr,
                                  StatementContext &stmtCtx) {
    return genExprValue(expr, stmtCtx, &loc);
  }

  /// Generate or get a fir.box describing the expression. If SomeExpr is
  /// a Designator, the fir.box describes an entity over the Designator base
  /// storage without making a temporary.
  virtual fir::ExtendedValue genExprBox(mlir::Location loc,
                                        const SomeExpr &expr,
                                        StatementContext &stmtCtx) = 0;

  /// Generate the address of the box describing the variable designated
  /// by the expression. The expression must be an allocatable or pointer
  /// designator.
  virtual fir::MutableBoxValue genExprMutableBox(mlir::Location loc,
                                                 const SomeExpr &expr) = 0;

  /// Get FoldingContext that is required for some expression
  /// analysis.
  virtual language::Compability::evaluate::FoldingContext &getFoldingContext() = 0;

  /// Host associated variables are grouped as a tuple. This returns that value,
  /// which is itself a reference. Use bindTuple() to set this value.
  virtual mlir::Value hostAssocTupleValue() = 0;

  /// Record a binding for the ssa-value of the host assoications tuple for this
  /// function.
  virtual void bindHostAssocTuple(mlir::Value val) = 0;

  /// Returns fir.dummy_scope operation's result value to be used
  /// as dummy_scope operand of hlfir.declare operations for the dummy
  /// arguments of this function.
  virtual mlir::Value dummyArgsScopeValue() const = 0;

  /// Returns true if the given symbol is a dummy argument of this function.
  /// Note that it returns false for all the symbols after all the variables
  /// are instantiated for this function, i.e. it can only be used reliably
  /// during the instatiation of the variables.
  virtual bool
  isRegisteredDummySymbol(language::Compability::semantics::SymbolRef symRef) const = 0;

  /// Returns the FunctionLikeUnit being lowered, if any.
  virtual const language::Compability::lower::pft::FunctionLikeUnit *
  getCurrentFunctionUnit() const = 0;

  //===--------------------------------------------------------------------===//
  // Types
  //===--------------------------------------------------------------------===//

  /// Generate the type of an Expr
  virtual mlir::Type genType(const SomeExpr &) = 0;
  /// Generate the type of a Symbol
  virtual mlir::Type genType(SymbolRef) = 0;
  /// Generate the type from a category
  virtual mlir::Type genType(language::Compability::common::TypeCategory tc) = 0;
  /// Generate the type from a category and kind and length parameters.
  virtual mlir::Type
  genType(language::Compability::common::TypeCategory tc, int kind,
          toolchain::ArrayRef<std::int64_t> lenParameters = {}) = 0;
  /// Generate the type from a DerivedTypeSpec.
  virtual mlir::Type genType(const language::Compability::semantics::DerivedTypeSpec &) = 0;
  /// Generate the type from a Variable
  virtual mlir::Type genType(const pft::Variable &) = 0;

  /// Register a runtime derived type information object symbol to ensure its
  /// object will be generated as a global.
  virtual void
  registerTypeInfo(mlir::Location loc, SymbolRef typeInfoSym,
                   const language::Compability::semantics::DerivedTypeSpec &typeSpec,
                   fir::RecordType type) = 0;

  /// Get stack of derived type in construction. This is an internal entry point
  /// for the type conversion utility to allow lowering recursive derived types.
  virtual TypeConstructionStack &getTypeConstructionStack() = 0;

  //===--------------------------------------------------------------------===//
  // Locations
  //===--------------------------------------------------------------------===//

  /// Get the converter's current location
  virtual mlir::Location getCurrentLocation() = 0;
  /// Generate a dummy location
  virtual mlir::Location genUnknownLocation() = 0;
  /// Generate the location as converted from a CharBlock
  virtual mlir::Location genLocation(const language::Compability::parser::CharBlock &) = 0;

  /// Get the converter's current scope
  virtual const language::Compability::semantics::Scope &getCurrentScope() = 0;

  //===--------------------------------------------------------------------===//
  // FIR/MLIR
  //===--------------------------------------------------------------------===//

  /// Get the OpBuilder
  virtual fir::FirOpBuilder &getFirOpBuilder() = 0;
  /// Get the ModuleOp
  virtual mlir::ModuleOp getModuleOp() = 0;
  /// Get the MLIRContext
  virtual mlir::MLIRContext &getMLIRContext() = 0;
  /// Unique a symbol (add a containing scope specific prefix)
  virtual std::string mangleName(const language::Compability::semantics::Symbol &) = 0;
  /// Unique a derived type (add a containing scope specific prefix)
  virtual std::string
  mangleName(const language::Compability::semantics::DerivedTypeSpec &) = 0;
  /// Unique a compiler generated name (add a containing scope specific prefix)
  virtual std::string mangleName(std::string &) = 0;
  /// Unique a compiler generated name (add a provided scope specific prefix)
  virtual std::string mangleName(std::string &, const semantics::Scope &) = 0;
  /// Return the field name for a derived type component inside a fir.record
  /// type.
  virtual std::string
  getRecordTypeFieldName(const language::Compability::semantics::Symbol &component) = 0;

  /// Get the KindMap.
  virtual const fir::KindMapping &getKindMap() = 0;

  virtual language::Compability::lower::StatementContext &getFctCtx() = 0;

  AbstractConverter(const language::Compability::lower::LoweringOptions &loweringOptions)
      : loweringOptions(loweringOptions) {}
  virtual ~AbstractConverter() = default;

  //===--------------------------------------------------------------------===//
  // Miscellaneous
  //===--------------------------------------------------------------------===//

  /// Generate IR for Evaluation \p eval.
  virtual void genEval(pft::Evaluation &eval,
                       bool unstructuredContext = true) = 0;

  /// Return options controlling lowering behavior.
  const language::Compability::lower::LoweringOptions &getLoweringOptions() const {
    return loweringOptions;
  }

  /// Find the symbol in one level up of symbol map such as for host-association
  /// in OpenMP code or return null.
  virtual language::Compability::lower::SymbolBox
  lookupOneLevelUpSymbol(const language::Compability::semantics::Symbol &sym) = 0;

  /// Find the symbol in the inner-most level of the local map or return null.
  virtual language::Compability::lower::SymbolBox
  shallowLookupSymbol(const language::Compability::semantics::Symbol &sym) = 0;

  /// Return the mlir::SymbolTable associated to the ModuleOp.
  /// Look-ups are faster using it than using module.lookup<>,
  /// but the module op should be queried in case of failure
  /// because this symbol table is not guaranteed to contain
  /// all the symbols from the ModuleOp (the symbol table should
  /// always be provided to the builder helper creating globals and
  /// functions in order to be in sync).
  virtual mlir::SymbolTable *getMLIRSymbolTable() = 0;

  virtual mlir::StateStack &getStateStack() = 0;

private:
  /// Options controlling lowering behavior.
  const language::Compability::lower::LoweringOptions &loweringOptions;
};

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_ABSTRACTCONVERTER_H
