// SValBuilder.h - Construction of SVals from evaluating expressions -*- C++ -*-
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
//  This file defines SValBuilder, a class that defines the interface for
//  "symbolical evaluators" which construct an SVal from an expression.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SVALBUILDER_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SVALBUILDER_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprObjC.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/BasicValueFactory.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"
#include "toolchain/ADT/ImmutableList.h"
#include <cstdint>
#include <optional>

namespace language::Core {

class AnalyzerOptions;
class BlockDecl;
class CXXBoolLiteralExpr;
class CXXMethodDecl;
class CXXRecordDecl;
class DeclaratorDecl;
class FunctionDecl;
class LocationContext;
class StackFrameContext;
class Stmt;

namespace ento {

class CallEvent;
class ConditionTruthVal;
class ProgramStateManager;
class StoreRef;
class SValBuilder {
  virtual void anchor();

protected:
  ASTContext &Context;

  /// Manager of APSInt values.
  BasicValueFactory BasicVals;

  /// Manages the creation of symbols.
  SymbolManager SymMgr;

  /// Manages the creation of memory regions.
  MemRegionManager MemMgr;

  ProgramStateManager &StateMgr;

  const AnalyzerOptions &AnOpts;

  /// The scalar type to use for array indices.
  const QualType ArrayIndexTy;

  /// The width of the scalar type used for array indices.
  const unsigned ArrayIndexWidth;

public:
  SValBuilder(toolchain::BumpPtrAllocator &alloc, ASTContext &context,
              ProgramStateManager &stateMgr);

  virtual ~SValBuilder() = default;

  SVal evalCast(SVal V, QualType CastTy, QualType OriginalTy);

  // Handles casts of type CK_IntegralCast.
  SVal evalIntegralCast(ProgramStateRef state, SVal val, QualType castTy,
                        QualType originalType);

  SVal evalMinus(NonLoc val);
  SVal evalComplement(NonLoc val);

  /// Create a new value which represents a binary expression with two non-
  /// location operands.
  virtual SVal evalBinOpNN(ProgramStateRef state, BinaryOperator::Opcode op,
                           NonLoc lhs, NonLoc rhs, QualType resultTy) = 0;

  /// Create a new value which represents a binary expression with two memory
  /// location operands.
  virtual SVal evalBinOpLL(ProgramStateRef state, BinaryOperator::Opcode op,
                           Loc lhs, Loc rhs, QualType resultTy) = 0;

  /// Create a new value which represents a binary expression with a memory
  /// location and non-location operands. For example, this would be used to
  /// evaluate a pointer arithmetic operation.
  virtual SVal evalBinOpLN(ProgramStateRef state, BinaryOperator::Opcode op,
                           Loc lhs, NonLoc rhs, QualType resultTy) = 0;

  /// Evaluates a given SVal. If the SVal has only one possible (integer) value,
  /// that value is returned. Otherwise, returns NULL.
  virtual const toolchain::APSInt *getKnownValue(ProgramStateRef state, SVal val) = 0;

  /// Tries to get the minimal possible (integer) value of a given SVal. This
  /// always returns the value of a ConcreteInt, but may return NULL if the
  /// value is symbolic and the constraint manager cannot provide a useful
  /// answer.
  virtual const toolchain::APSInt *getMinValue(ProgramStateRef state, SVal val) = 0;

  /// Tries to get the maximal possible (integer) value of a given SVal. This
  /// always returns the value of a ConcreteInt, but may return NULL if the
  /// value is symbolic and the constraint manager cannot provide a useful
  /// answer.
  virtual const toolchain::APSInt *getMaxValue(ProgramStateRef state, SVal val) = 0;

  /// Simplify symbolic expressions within a given SVal. Return an SVal
  /// that represents the same value, but is hopefully easier to work with
  /// than the original SVal.
  virtual SVal simplifySVal(ProgramStateRef State, SVal Val) = 0;

  /// Constructs a symbolic expression for two non-location values.
  SVal makeSymExprValNN(BinaryOperator::Opcode op,
                        NonLoc lhs, NonLoc rhs, QualType resultTy);

  SVal evalUnaryOp(ProgramStateRef state, UnaryOperator::Opcode opc,
                 SVal operand, QualType type);

  SVal evalBinOp(ProgramStateRef state, BinaryOperator::Opcode op,
                 SVal lhs, SVal rhs, QualType type);

  /// \return Whether values in \p lhs and \p rhs are equal at \p state.
  ConditionTruthVal areEqual(ProgramStateRef state, SVal lhs, SVal rhs);

  SVal evalEQ(ProgramStateRef state, SVal lhs, SVal rhs);

  DefinedOrUnknownSVal evalEQ(ProgramStateRef state, DefinedOrUnknownSVal lhs,
                              DefinedOrUnknownSVal rhs);

  ASTContext &getContext() { return Context; }
  const ASTContext &getContext() const { return Context; }

  ProgramStateManager &getStateManager() { return StateMgr; }

  QualType getConditionType() const {
    return Context.getLangOpts().CPlusPlus ? Context.BoolTy : Context.IntTy;
  }

  QualType getArrayIndexType() const {
    return ArrayIndexTy;
  }

  BasicValueFactory &getBasicValueFactory() { return BasicVals; }
  const BasicValueFactory &getBasicValueFactory() const { return BasicVals; }

  SymbolManager &getSymbolManager() { return SymMgr; }
  const SymbolManager &getSymbolManager() const { return SymMgr; }

  MemRegionManager &getRegionManager() { return MemMgr; }
  const MemRegionManager &getRegionManager() const { return MemMgr; }

  const AnalyzerOptions &getAnalyzerOptions() const { return AnOpts; }

  // Forwarding methods to SymbolManager.

  const SymbolConjured *conjureSymbol(ConstCFGElementRef Elem,
                                      const LocationContext *LCtx,
                                      QualType type, unsigned visitCount,
                                      const void *symbolTag = nullptr) {
    return SymMgr.conjureSymbol(Elem, LCtx, type, visitCount, symbolTag);
  }

  /// Construct an SVal representing '0' for the specified type.
  DefinedOrUnknownSVal makeZeroVal(QualType type);

  /// Make a unique symbol for value of region.
  DefinedOrUnknownSVal getRegionValueSymbolVal(const TypedValueRegion *region);

  /// Create a new symbol with a unique 'name'.
  ///
  /// We resort to conjured symbols when we cannot construct a derived symbol.
  /// The advantage of symbols derived/built from other symbols is that we
  /// preserve the relation between related(or even equivalent) expressions, so
  /// conjured symbols should be used sparingly.
  DefinedOrUnknownSVal conjureSymbolVal(const void *symbolTag,
                                        ConstCFGElementRef elem,
                                        const LocationContext *LCtx,
                                        unsigned count);
  DefinedOrUnknownSVal conjureSymbolVal(const void *symbolTag,
                                        ConstCFGElementRef elem,
                                        const LocationContext *LCtx,
                                        QualType type, unsigned count);
  DefinedOrUnknownSVal conjureSymbolVal(ConstCFGElementRef elem,
                                        const LocationContext *LCtx,
                                        QualType type, unsigned visitCount);
  DefinedOrUnknownSVal conjureSymbolVal(const CallEvent &call, QualType type,
                                        unsigned visitCount,
                                        const void *symbolTag = nullptr);
  DefinedOrUnknownSVal conjureSymbolVal(const CallEvent &call,
                                        unsigned visitCount,
                                        const void *symbolTag = nullptr);

  /// Conjure a symbol representing heap allocated memory region.
  DefinedSVal getConjuredHeapSymbolVal(ConstCFGElementRef elem,
                                       const LocationContext *LCtx,
                                       QualType type, unsigned Count);

  /// Create an SVal representing the result of an alloca()-like call, that is,
  /// an AllocaRegion on the stack.
  ///
  /// After calling this function, it's a good idea to set the extent of the
  /// returned AllocaRegion.
  loc::MemRegionVal getAllocaRegionVal(const Expr *E,
                                       const LocationContext *LCtx,
                                       unsigned Count);

  DefinedOrUnknownSVal getDerivedRegionValueSymbolVal(
      SymbolRef parentSymbol, const TypedValueRegion *region);

  DefinedSVal getMetadataSymbolVal(const void *symbolTag,
                                   const MemRegion *region,
                                   const Expr *expr, QualType type,
                                   const LocationContext *LCtx,
                                   unsigned count);

  DefinedSVal getMemberPointer(const NamedDecl *ND);

  DefinedSVal getFunctionPointer(const FunctionDecl *func);

  DefinedSVal getBlockPointer(const BlockDecl *block, CanQualType locTy,
                              const LocationContext *locContext,
                              unsigned blockCount);

  /// Returns the value of \p E, if it can be determined in a non-path-sensitive
  /// manner.
  ///
  /// If \p E is not a constant or cannot be modeled, returns \c std::nullopt.
  std::optional<SVal> getConstantVal(const Expr *E);

  NonLoc makeCompoundVal(QualType type, toolchain::ImmutableList<SVal> vals) {
    return nonloc::CompoundVal(BasicVals.getCompoundValData(type, vals));
  }

  NonLoc makeLazyCompoundVal(const StoreRef &store,
                             const TypedValueRegion *region) {
    return nonloc::LazyCompoundVal(
        BasicVals.getLazyCompoundValData(store, region));
  }

  NonLoc makePointerToMember(const DeclaratorDecl *DD) {
    return nonloc::PointerToMember(DD);
  }

  NonLoc makePointerToMember(const PointerToMemberData *PTMD) {
    return nonloc::PointerToMember(PTMD);
  }

  NonLoc makeZeroArrayIndex() {
    return nonloc::ConcreteInt(BasicVals.getValue(0, ArrayIndexTy));
  }

  NonLoc makeArrayIndex(uint64_t idx) {
    return nonloc::ConcreteInt(BasicVals.getValue(idx, ArrayIndexTy));
  }

  SVal convertToArrayIndex(SVal val);

  nonloc::ConcreteInt makeIntVal(const IntegerLiteral* integer) {
    return nonloc::ConcreteInt(
        BasicVals.getValue(integer->getValue(),
                     integer->getType()->isUnsignedIntegerOrEnumerationType()));
  }

  nonloc::ConcreteInt makeBoolVal(const ObjCBoolLiteralExpr *boolean) {
    return makeTruthVal(boolean->getValue(), boolean->getType());
  }

  nonloc::ConcreteInt makeBoolVal(const CXXBoolLiteralExpr *boolean);

  nonloc::ConcreteInt makeIntVal(const toolchain::APSInt& integer) {
    return nonloc::ConcreteInt(BasicVals.getValue(integer));
  }

  loc::ConcreteInt makeIntLocVal(const toolchain::APSInt &integer) {
    return loc::ConcreteInt(BasicVals.getValue(integer));
  }

  NonLoc makeIntVal(const toolchain::APInt& integer, bool isUnsigned) {
    return nonloc::ConcreteInt(BasicVals.getValue(integer, isUnsigned));
  }

  DefinedSVal makeIntVal(uint64_t integer, QualType type) {
    if (Loc::isLocType(type))
      return loc::ConcreteInt(BasicVals.getValue(integer, type));

    return nonloc::ConcreteInt(BasicVals.getValue(integer, type));
  }

  NonLoc makeIntVal(uint64_t integer, bool isUnsigned) {
    return nonloc::ConcreteInt(BasicVals.getIntValue(integer, isUnsigned));
  }

  NonLoc makeIntValWithWidth(QualType ptrType, uint64_t integer) {
    return nonloc::ConcreteInt(BasicVals.getValue(integer, ptrType));
  }

  NonLoc makeLocAsInteger(Loc loc, unsigned bits) {
    return nonloc::LocAsInteger(BasicVals.getPersistentSValWithData(loc, bits));
  }

  nonloc::SymbolVal makeNonLoc(const SymExpr *lhs, BinaryOperator::Opcode op,
                               APSIntPtr rhs, QualType type);

  nonloc::SymbolVal makeNonLoc(APSIntPtr rhs, BinaryOperator::Opcode op,
                               const SymExpr *lhs, QualType type);

  nonloc::SymbolVal makeNonLoc(const SymExpr *lhs, BinaryOperator::Opcode op,
                               const SymExpr *rhs, QualType type);

  NonLoc makeNonLoc(const SymExpr *operand, UnaryOperator::Opcode op,
                    QualType type);

  /// Create a NonLoc value for cast.
  nonloc::SymbolVal makeNonLoc(const SymExpr *operand, QualType fromTy,
                               QualType toTy);

  nonloc::ConcreteInt makeTruthVal(bool b, QualType type) {
    return nonloc::ConcreteInt(BasicVals.getTruthValue(b, type));
  }

  nonloc::ConcreteInt makeTruthVal(bool b) {
    return nonloc::ConcreteInt(BasicVals.getTruthValue(b));
  }

  /// Create NULL pointer, with proper pointer bit-width for given address
  /// space.
  /// \param type pointer type.
  loc::ConcreteInt makeNullWithType(QualType type) {
    // We cannot use the `isAnyPointerType()`.
    assert((type->isPointerType() || type->isObjCObjectPointerType() ||
            type->isBlockPointerType() || type->isNullPtrType() ||
            type->isReferenceType()) &&
           "makeNullWithType must use pointer type");

    // The `sizeof(T&)` is `sizeof(T)`, thus we replace the reference with a
    // pointer. Here we assume that references are actually implemented by
    // pointers under-the-hood.
    type = type->isReferenceType()
               ? Context.getPointerType(type->getPointeeType())
               : type;
    return loc::ConcreteInt(BasicVals.getZeroWithTypeSize(type));
  }

  loc::MemRegionVal makeLoc(SymbolRef sym) {
    return loc::MemRegionVal(MemMgr.getSymbolicRegion(sym));
  }

  loc::MemRegionVal makeLoc(const MemRegion *region) {
    return loc::MemRegionVal(region);
  }

  loc::GotoLabel makeLoc(const AddrLabelExpr *expr) {
    return loc::GotoLabel(expr->getLabel());
  }

  loc::ConcreteInt makeLoc(const toolchain::APSInt &integer) {
    return loc::ConcreteInt(BasicVals.getValue(integer));
  }

  /// Return MemRegionVal on success cast, otherwise return std::nullopt.
  std::optional<loc::MemRegionVal>
  getCastedMemRegionVal(const MemRegion *region, QualType type);

  /// Make an SVal that represents the given symbol. This follows the convention
  /// of representing Loc-type symbols (symbolic pointers and references)
  /// as Loc values wrapping the symbol rather than as plain symbol values.
  DefinedSVal makeSymbolVal(SymbolRef Sym) {
    if (Loc::isLocType(Sym->getType()))
      return makeLoc(Sym);
    return nonloc::SymbolVal(Sym);
  }

  /// Return a memory region for the 'this' object reference.
  loc::MemRegionVal getCXXThis(const CXXMethodDecl *D,
                               const StackFrameContext *SFC);

  /// Return a memory region for the 'this' object reference.
  loc::MemRegionVal getCXXThis(const CXXRecordDecl *D,
                               const StackFrameContext *SFC);
};

SValBuilder* createSimpleSValBuilder(toolchain::BumpPtrAllocator &alloc,
                                     ASTContext &context,
                                     ProgramStateManager &stateMgr);

} // namespace ento

} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SVALBUILDER_H
