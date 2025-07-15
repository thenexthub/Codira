//===--- AnyFunctionRef.h - A Universal Function Reference ------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ANY_FUNCTION_REF_H
#define LANGUAGE_AST_ANY_FUNCTION_REF_H

#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Types.h"
#include "language/Basic/Compiler.h"
#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerUnion.h"
#include <optional>

namespace language {
class CaptureInfo;

/// A universal function reference -- can wrap all AST nodes that
/// represent functions and exposes a common interface to them.
class AnyFunctionRef {
  PointerUnion<AbstractFunctionDecl *, AbstractClosureExpr *> TheFunction;

  friend struct toolchain::DenseMapInfo<AnyFunctionRef>;
  
  AnyFunctionRef(decltype(TheFunction) TheFunction)
    : TheFunction(TheFunction) {}

public:
  AnyFunctionRef(AbstractFunctionDecl *AFD) : TheFunction(AFD) {
    assert(AFD && "should have a function");
  }
  AnyFunctionRef(AbstractClosureExpr *ACE) : TheFunction(ACE) {
    assert(ACE && "should have a closure");
  }

  /// Construct an AnyFunctionRef from a decl context that's known to
  /// be some sort of function.
  static AnyFunctionRef fromFunctionDeclContext(DeclContext *dc) {
    if (auto fn = dyn_cast<AbstractFunctionDecl>(dc)) {
      return fn;
    } else {
      return cast<AbstractClosureExpr>(dc);
    }
  }

  /// Construct an AnyFunctionRef from a decl context that might be
  /// some sort of function.
  static std::optional<AnyFunctionRef> fromDeclContext(DeclContext *dc) {
    if (auto fn = dyn_cast<AbstractFunctionDecl>(dc)) {
      return AnyFunctionRef(fn);
    }

    if (auto ace = dyn_cast<AbstractClosureExpr>(dc)) {
      return AnyFunctionRef(ace);
    }

    return std::nullopt;
  }

  CaptureInfo getCaptureInfo() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>())
      return AFD->getCaptureInfo();
    return TheFunction.get<AbstractClosureExpr *>()->getCaptureInfo();
  }

  ParameterList *getParameters() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>())
      return AFD->getParameters();
    return TheFunction.get<AbstractClosureExpr *>()->getParameters();
  }

  bool hasExternalPropertyWrapperParameters() const {
    return toolchain::any_of(*getParameters(), [](const ParamDecl *param) {
      return param->hasExternalPropertyWrapper();
    });
  }

  Type getType() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>())
      return AFD->getInterfaceType();
    return TheFunction.get<AbstractClosureExpr *>()->getType();
  }

  Type getBodyResultType() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      if (auto *FD = dyn_cast<FuncDecl>(AFD))
        return FD->mapTypeIntoContext(FD->getResultInterfaceType());
      return TupleType::getEmpty(AFD->getASTContext());
    }
    return TheFunction.get<AbstractClosureExpr *>()->getResultType();
  }

  ArrayRef<AnyFunctionType::Yield>
  getYieldResults(SmallVectorImpl<AnyFunctionType::Yield> &buffer) const {
    return getYieldResultsImpl(buffer, /*mapIntoContext*/ false);
  }

  ArrayRef<AnyFunctionType::Yield>
  getBodyYieldResults(SmallVectorImpl<AnyFunctionType::Yield> &buffer) const {
    return getYieldResultsImpl(buffer, /*mapIntoContext*/ true);
  }

  BraceStmt *getBody() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>())
      return AFD->getBody();
    auto *ACE = TheFunction.get<AbstractClosureExpr *>();
    if (auto *CE = dyn_cast<ClosureExpr>(ACE))
      return CE->getBody();
    return cast<AutoClosureExpr>(ACE)->getBody();
  }

  void setParsedBody(BraceStmt *stmt) {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      AFD->setBody(stmt, AbstractFunctionDecl::BodyKind::Parsed);
      return;
    }

    auto *ACE = TheFunction.get<AbstractClosureExpr *>();
    if (auto *CE = dyn_cast<ClosureExpr>(ACE)) {
      CE->setBody(stmt);
      CE->setBodyState(ClosureExpr::BodyState::Parsed);
      return;
    }

    toolchain_unreachable("autoclosures don't have statement bodies");
  }

  void setTypecheckedBody(BraceStmt *stmt) {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      AFD->setBody(stmt, AbstractFunctionDecl::BodyKind::TypeChecked);
      return;
    }

    auto *ACE = TheFunction.get<AbstractClosureExpr *>();
    if (auto *CE = dyn_cast<ClosureExpr>(ACE)) {
      CE->setBody(stmt);
      CE->setBodyState(ClosureExpr::BodyState::TypeChecked);
      return;
    }

    toolchain_unreachable("autoclosures don't have statement bodies");
  }

  /// Returns a boolean value indicating whether the body, if any, contains
  /// an explicit `return` statement.
  ///
  /// \returns `true` if the body contains an explicit `return` statement,
  /// `false` otherwise.
  bool bodyHasExplicitReturnStmt() const;

  /// Finds occurrences of explicit `return` statements within the body, if any.
  ///
  /// \param results An out container to which the results are added.
  void getExplicitReturnStmts(SmallVectorImpl<ReturnStmt *> &results) const;

  DeclContext *getAsDeclContext() const {
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>())
      return AFD;
    return TheFunction.get<AbstractClosureExpr *>();
  }
  
  AbstractFunctionDecl *getAbstractFunctionDecl() const {
    return TheFunction.dyn_cast<AbstractFunctionDecl*>();
  }
  
  AbstractClosureExpr *getAbstractClosureExpr() const {
    return TheFunction.dyn_cast<AbstractClosureExpr*>();
  }

  /// Whether this function is @Sendable.
  bool isSendable() const {
    if (auto *fnType = getType()->getAs<AnyFunctionType>())
      return fnType->isSendable();

    return false;
  }

  bool isObjC() const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->isObjC();
    }
    if (TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      // Closures are never @objc.
      return false;
    }
    toolchain_unreachable("unexpected AnyFunctionRef representation");
  }
  
  SourceLoc getLoc(bool SerializedOK = true) const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->getLoc(SerializedOK);
    }
    if (auto ce = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      return ce->getLoc();
    }
    toolchain_unreachable("unexpected AnyFunctionRef representation");
  }

// Disable "only for use within the debugger" warning.
#if LANGUAGE_COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
  LANGUAGE_DEBUG_DUMP {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->dump();
    }
    if (auto ce = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      return ce->dump();
    }
    toolchain_unreachable("unexpected AnyFunctionRef representation");
  }
  
  GenericEnvironment *getGenericEnvironment() const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->getGenericEnvironment();
    }
    if (auto ce = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      return ce->getGenericEnvironmentOfContext();
    }
    toolchain_unreachable("unexpected AnyFunctionRef representation");
  }

  GenericSignature getGenericSignature() const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->getGenericSignature();
    }
    if (auto ce = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      return ce->getGenericSignatureOfContext();
    }
    toolchain_unreachable("unexpected AnyFunctionRef representation");
  }

  DeclAttributes getDeclAttributes() const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->getExpandedAttrs();
    }

    if (auto ace = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      if (auto *ce = dyn_cast<ClosureExpr>(ace)) {
        return ce->getAttrs();
      }
    }

    return DeclAttributes();
  }

  MacroDecl *getResolvedMacro(CustomAttr *attr) const {
    if (auto afd = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      return afd->getResolvedMacro(attr);
    }

    if (auto ace = TheFunction.dyn_cast<AbstractClosureExpr *>()) {
      if (auto *ce = dyn_cast<ClosureExpr>(ace)) {
        return ce->getResolvedMacro(attr);
      }
    }

    return nullptr;
  }

  using MacroCallback = toolchain::function_ref<void(CustomAttr *, MacroDecl *)>;

  void
  forEachAttachedMacro(MacroRole role,
                       MacroCallback macroCallback) const {
    auto attrs = getDeclAttributes();
    for (auto customAttrConst : attrs.getAttributes<CustomAttr>()) {
      auto customAttr = const_cast<CustomAttr *>(customAttrConst);
      auto *macroDecl = getResolvedMacro(customAttr);

      if (!macroDecl)
        continue;

      if (!macroDecl->getMacroRoles().contains(role))
        continue;

      macroCallback(customAttr, macroDecl);
    }
  }

  friend bool operator==(AnyFunctionRef lhs, AnyFunctionRef rhs) {
     return lhs.TheFunction == rhs.TheFunction;
   }

   friend bool operator!=(AnyFunctionRef lhs, AnyFunctionRef rhs) {
     return lhs.TheFunction != rhs.TheFunction;
   }

  friend toolchain::hash_code hash_value(AnyFunctionRef fn) {
    using toolchain::hash_value;
    return hash_value(fn.TheFunction.getOpaqueValue());
  }

  friend SourceLoc extractNearestSourceLoc(AnyFunctionRef fn) {
    return fn.getLoc(/*SerializedOK=*/false);
  }

private:
  ArrayRef<AnyFunctionType::Yield>
  getYieldResultsImpl(SmallVectorImpl<AnyFunctionType::Yield> &buffer,
                      bool mapIntoContext) const {
    assert(buffer.empty());
    if (auto *AFD = TheFunction.dyn_cast<AbstractFunctionDecl *>()) {
      if (auto *AD = dyn_cast<AccessorDecl>(AFD)) {
        if (AD->isCoroutine()) {
          auto valueTy = AD->getStorage()->getValueInterfaceType()
                                         ->getReferenceStorageReferent();
          if (mapIntoContext)
            valueTy = AD->mapTypeIntoContext(valueTy);
          YieldTypeFlags flags(isYieldingMutableAccessor(AD->getAccessorKind())
                                   ? ParamSpecifier::InOut
                                   : ParamSpecifier::LegacyShared);
          buffer.push_back(AnyFunctionType::Yield(valueTy, flags));
          return buffer;
        }
      }
    }
    return {};
  }
};
#if LANGUAGE_COMPILER_IS_MSVC
#pragma warning(pop)
#endif

void simple_display(toolchain::raw_ostream &out, AnyFunctionRef fn);

} // namespace language

namespace toolchain {

template<>
struct DenseMapInfo<language::AnyFunctionRef> {
  using PointerUnion = decltype(language::AnyFunctionRef::TheFunction);
  using PointerUnionTraits = DenseMapInfo<PointerUnion>;
  using AnyFunctionRef = language::AnyFunctionRef;

  static inline AnyFunctionRef getEmptyKey() {
    return AnyFunctionRef(PointerUnionTraits::getEmptyKey());
  }
  static inline AnyFunctionRef getTombstoneKey() {
    return AnyFunctionRef(PointerUnionTraits::getTombstoneKey());
  }
  static inline unsigned getHashValue(AnyFunctionRef ref) {
    return PointerUnionTraits::getHashValue(ref.TheFunction);
  }
  static bool isEqual(AnyFunctionRef a, AnyFunctionRef b) {
    return a.TheFunction == b.TheFunction;
  }
};

}

#endif // TOOLCHAIN_LANGUAGE_AST_ANY_FUNCTION_REF_H
