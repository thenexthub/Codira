//===----------------------------------------------------------------------===//
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
// These classes wrap the information about a call or function
// definition used to handle ABI compliancy.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_LIB_CODEGEN_CIRGENCALL_H
#define CLANG_LIB_CODEGEN_CIRGENCALL_H

#include "CIRGenValue.h"
#include "mlir/IR/Operation.h"
#include "language/Core/AST/GlobalDecl.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core::CIRGen {

class CIRGenFunction;

/// Abstract information about a function or function prototype.
class CIRGenCalleeInfo {
  const language::Core::FunctionProtoType *calleeProtoTy;
  language::Core::GlobalDecl calleeDecl;

public:
  explicit CIRGenCalleeInfo() : calleeProtoTy(nullptr), calleeDecl() {}
  CIRGenCalleeInfo(const language::Core::FunctionProtoType *calleeProtoTy,
                   language::Core::GlobalDecl calleeDecl)
      : calleeProtoTy(calleeProtoTy), calleeDecl(calleeDecl) {}
  CIRGenCalleeInfo(language::Core::GlobalDecl calleeDecl)
      : calleeProtoTy(nullptr), calleeDecl(calleeDecl) {}

  const language::Core::FunctionProtoType *getCalleeFunctionProtoType() const {
    return calleeProtoTy;
  }
  language::Core::GlobalDecl getCalleeDecl() const { return calleeDecl; }
};

class CIRGenCallee {
  enum class SpecialKind : uintptr_t {
    Invalid,
    Builtin,
    PseudoDestructor,

    Last = Builtin,
  };

  struct BuiltinInfoStorage {
    const language::Core::FunctionDecl *decl;
    unsigned id;
  };
  struct PseudoDestructorInfoStorage {
    const language::Core::CXXPseudoDestructorExpr *expr;
  };

  SpecialKind kindOrFunctionPtr;

  union {
    CIRGenCalleeInfo abstractInfo;
    BuiltinInfoStorage builtinInfo;
    PseudoDestructorInfoStorage pseudoDestructorInfo;
  };

  explicit CIRGenCallee(SpecialKind kind) : kindOrFunctionPtr(kind) {}

public:
  CIRGenCallee() : kindOrFunctionPtr(SpecialKind::Invalid) {}

  CIRGenCallee(const CIRGenCalleeInfo &abstractInfo, mlir::Operation *funcPtr)
      : kindOrFunctionPtr(SpecialKind(reinterpret_cast<uintptr_t>(funcPtr))),
        abstractInfo(abstractInfo) {
    assert(funcPtr && "configuring callee without function pointer");
  }

  static CIRGenCallee
  forDirect(mlir::Operation *funcPtr,
            const CIRGenCalleeInfo &abstractInfo = CIRGenCalleeInfo()) {
    return CIRGenCallee(abstractInfo, funcPtr);
  }

  bool isBuiltin() const { return kindOrFunctionPtr == SpecialKind::Builtin; }

  const language::Core::FunctionDecl *getBuiltinDecl() const {
    assert(isBuiltin());
    return builtinInfo.decl;
  }
  unsigned getBuiltinID() const {
    assert(isBuiltin());
    return builtinInfo.id;
  }

  static CIRGenCallee forBuiltin(unsigned builtinID,
                                 const language::Core::FunctionDecl *builtinDecl) {
    CIRGenCallee result(SpecialKind::Builtin);
    result.builtinInfo.decl = builtinDecl;
    result.builtinInfo.id = builtinID;
    return result;
  }

  static CIRGenCallee
  forPseudoDestructor(const language::Core::CXXPseudoDestructorExpr *expr) {
    CIRGenCallee result(SpecialKind::PseudoDestructor);
    result.pseudoDestructorInfo.expr = expr;
    return result;
  }

  bool isPseudoDestructor() const {
    return kindOrFunctionPtr == SpecialKind::PseudoDestructor;
  }

  const CXXPseudoDestructorExpr *getPseudoDestructorExpr() const {
    assert(isPseudoDestructor());
    return pseudoDestructorInfo.expr;
  }

  bool isOrdinary() const {
    return uintptr_t(kindOrFunctionPtr) > uintptr_t(SpecialKind::Last);
  }

  /// If this is a delayed callee computation of some sort, prepare a concrete
  /// callee
  CIRGenCallee prepareConcreteCallee(CIRGenFunction &cgf) const;

  CIRGenCalleeInfo getAbstractInfo() const {
    assert(!cir::MissingFeatures::opCallVirtual());
    assert(isOrdinary());
    return abstractInfo;
  }

  mlir::Operation *getFunctionPointer() const {
    assert(isOrdinary());
    return reinterpret_cast<mlir::Operation *>(kindOrFunctionPtr);
  }

  void setFunctionPointer(mlir::Operation *functionPtr) {
    assert(isOrdinary());
    kindOrFunctionPtr = SpecialKind(reinterpret_cast<uintptr_t>(functionPtr));
  }
};

/// Type for representing both the decl and type of parameters to a function.
/// The decl must be either a ParmVarDecl or ImplicitParamDecl.
class FunctionArgList : public toolchain::SmallVector<const language::Core::VarDecl *, 16> {};

struct CallArg {
private:
  union {
    RValue rv;
    LValue lv; // This argument is semantically a load from this l-value
  };
  bool hasLV;

  /// A data-flow flag to make sure getRValue and/or copyInto are not
  /// called twice for duplicated IR emission.
  [[maybe_unused]] mutable bool isUsed;

public:
  language::Core::QualType ty;

  CallArg(RValue rv, language::Core::QualType ty)
      : rv(rv), hasLV(false), isUsed(false), ty(ty) {}

  CallArg(LValue lv, language::Core::QualType ty)
      : lv(lv), hasLV(true), isUsed(false), ty(ty) {}

  bool hasLValue() const { return hasLV; }

  LValue getKnownLValue() const {
    assert(hasLV && !isUsed);
    return lv;
  }

  RValue getKnownRValue() const {
    assert(!hasLV && !isUsed);
    return rv;
  }

  bool isAggregate() const { return hasLV || rv.isAggregate(); }
};

class CallArgList : public toolchain::SmallVector<CallArg, 8> {
public:
  void add(RValue rvalue, language::Core::QualType type) { emplace_back(rvalue, type); }

  void addUncopiedAggregate(LValue lvalue, language::Core::QualType type) {
    emplace_back(lvalue, type);
  }

  /// Add all the arguments from another CallArgList to this one. After doing
  /// this, the old CallArgList retains its list of arguments, but must not
  /// be used to emit a call.
  void addFrom(const CallArgList &other) {
    insert(end(), other.begin(), other.end());
    // Classic codegen has handling for these here. We may not need it here for
    // CIR, but if not we should implement equivalent handling in lowering.
    assert(!cir::MissingFeatures::writebacks());
    assert(!cir::MissingFeatures::cleanupsToDeactivate());
    assert(!cir::MissingFeatures::stackBase());
  }
};

/// Contains the address where the return value of a function can be stored, and
/// whether the address is volatile or not.
class ReturnValueSlot {
  Address addr = Address::invalid();

public:
  ReturnValueSlot() = default;
  ReturnValueSlot(Address addr) : addr(addr) {}

  Address getValue() const { return addr; }
};

} // namespace language::Core::CIRGen

#endif // CLANG_LIB_CODEGEN_CIRGENCALL_H
