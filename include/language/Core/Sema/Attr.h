//===----- Attr.h --- Helper functions for attribute handling in Sema -----===//
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
//  This file provides helpers for Sema functions that handle attributes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_ATTR_H
#define LANGUAGE_CORE_SEMA_ATTR_H

#include "language/Core/AST/Attr.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/AttributeCommonInfo.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/ParsedAttr.h"
#include "language/Core/Sema/SemaBase.h"
#include "toolchain/Support/Casting.h"

namespace language::Core {

/// isFuncOrMethodForAttrSubject - Return true if the given decl has function
/// type (function or function-typed variable) or an Objective-C
/// method.
inline bool isFuncOrMethodForAttrSubject(const Decl *D) {
  return (D->getFunctionType() != nullptr) || toolchain::isa<ObjCMethodDecl>(D);
}

/// Return true if the given decl has function type (function or
/// function-typed variable) or an Objective-C method or a block.
inline bool isFunctionOrMethodOrBlockForAttrSubject(const Decl *D) {
  return isFuncOrMethodForAttrSubject(D) || toolchain::isa<BlockDecl>(D);
}

/// Return true if the given decl has a declarator that should have
/// been processed by Sema::GetTypeForDeclarator.
inline bool hasDeclarator(const Decl *D) {
  // In some sense, TypedefDecl really *ought* to be a DeclaratorDecl.
  return isa<DeclaratorDecl>(D) || isa<BlockDecl>(D) ||
         isa<TypedefNameDecl>(D) || isa<ObjCPropertyDecl>(D);
}

/// hasFunctionProto - Return true if the given decl has a argument
/// information. This decl should have already passed
/// isFuncOrMethodForAttrSubject or isFunctionOrMethodOrBlockForAttrSubject.
inline bool hasFunctionProto(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return isa<FunctionProtoType>(FnTy);
  return isa<ObjCMethodDecl>(D) || isa<BlockDecl>(D);
}

/// getFunctionOrMethodNumParams - Return number of function or method
/// parameters. It is an error to call this on a K&R function (use
/// hasFunctionProto first).
inline unsigned getFunctionOrMethodNumParams(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionProtoType>(FnTy)->getNumParams();
  if (const auto *BD = dyn_cast<BlockDecl>(D))
    return BD->getNumParams();
  return cast<ObjCMethodDecl>(D)->param_size();
}

inline const ParmVarDecl *getFunctionOrMethodParam(const Decl *D,
                                                   unsigned Idx) {
  if (const auto *FD = dyn_cast<FunctionDecl>(D))
    return FD->getParamDecl(Idx);
  if (const auto *MD = dyn_cast<ObjCMethodDecl>(D))
    return MD->getParamDecl(Idx);
  if (const auto *BD = dyn_cast<BlockDecl>(D))
    return BD->getParamDecl(Idx);
  return nullptr;
}

inline QualType getFunctionOrMethodParamType(const Decl *D, unsigned Idx) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionProtoType>(FnTy)->getParamType(Idx);
  if (const auto *BD = dyn_cast<BlockDecl>(D))
    return BD->getParamDecl(Idx)->getType();

  return cast<ObjCMethodDecl>(D)->parameters()[Idx]->getType();
}

inline SourceRange getFunctionOrMethodParamRange(const Decl *D, unsigned Idx) {
  if (auto *PVD = getFunctionOrMethodParam(D, Idx))
    return PVD->getSourceRange();
  return SourceRange();
}

inline QualType getFunctionOrMethodResultType(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return FnTy->getReturnType();
  return cast<ObjCMethodDecl>(D)->getReturnType();
}

inline SourceRange getFunctionOrMethodResultSourceRange(const Decl *D) {
  if (const auto *FD = dyn_cast<FunctionDecl>(D))
    return FD->getReturnTypeSourceRange();
  if (const auto *MD = dyn_cast<ObjCMethodDecl>(D))
    return MD->getReturnTypeSourceRange();
  return SourceRange();
}

inline bool isFunctionOrMethodVariadic(const Decl *D) {
  if (const FunctionType *FnTy = D->getFunctionType())
    return cast<FunctionProtoType>(FnTy)->isVariadic();
  if (const auto *BD = dyn_cast<BlockDecl>(D))
    return BD->isVariadic();
  return cast<ObjCMethodDecl>(D)->isVariadic();
}

inline bool isInstanceMethod(const Decl *D) {
  if (const auto *MethodDecl = dyn_cast<CXXMethodDecl>(D))
    return MethodDecl->isInstance();
  return false;
}

/// Diagnose mutually exclusive attributes when present on a given
/// declaration. Returns true if diagnosed.
template <typename AttrTy>
bool checkAttrMutualExclusion(SemaBase &S, Decl *D, const ParsedAttr &AL) {
  if (const auto *A = D->getAttr<AttrTy>()) {
    S.Diag(AL.getLoc(), diag::err_attributes_are_not_compatible)
        << AL << A
        << (AL.isRegularKeywordAttribute() || A->isRegularKeywordAttribute());
    S.Diag(A->getLocation(), diag::note_conflicting_attribute);
    return true;
  }
  return false;
}

template <typename AttrTy>
bool checkAttrMutualExclusion(SemaBase &S, Decl *D, const Attr &AL) {
  if (const auto *A = D->getAttr<AttrTy>()) {
    S.Diag(AL.getLocation(), diag::err_attributes_are_not_compatible)
        << &AL << A
        << (AL.isRegularKeywordAttribute() || A->isRegularKeywordAttribute());
    Diag(A->getLocation(), diag::note_conflicting_attribute);
    return true;
  }
  return false;
}

template <typename... DiagnosticArgs>
const SemaBase::SemaDiagnosticBuilder &
appendDiagnostics(const SemaBase::SemaDiagnosticBuilder &Bldr) {
  return Bldr;
}

template <typename T, typename... DiagnosticArgs>
const SemaBase::SemaDiagnosticBuilder &
appendDiagnostics(const SemaBase::SemaDiagnosticBuilder &Bldr, T &&ExtraArg,
                  DiagnosticArgs &&...ExtraArgs) {
  return appendDiagnostics(Bldr << std::forward<T>(ExtraArg),
                           std::forward<DiagnosticArgs>(ExtraArgs)...);
}

/// Applies the given attribute to the Decl without performing any
/// additional semantic checking.
template <typename AttrType>
void handleSimpleAttribute(SemaBase &S, Decl *D,
                           const AttributeCommonInfo &CI) {
  D->addAttr(::new (S.getASTContext()) AttrType(S.getASTContext(), CI));
}

/// Add an attribute @c AttrType to declaration @c D, provided that
/// @c PassesCheck is true.
/// Otherwise, emit diagnostic @c DiagID, passing in all parameters
/// specified in @c ExtraArgs.
template <typename AttrType, typename... DiagnosticArgs>
void handleSimpleAttributeOrDiagnose(SemaBase &S, Decl *D,
                                     const AttributeCommonInfo &CI,
                                     bool PassesCheck, unsigned DiagID,
                                     DiagnosticArgs &&...ExtraArgs) {
  if (!PassesCheck) {
    SemaBase::SemaDiagnosticBuilder DB = S.Diag(D->getBeginLoc(), DiagID);
    appendDiagnostics(DB, std::forward<DiagnosticArgs>(ExtraArgs)...);
    return;
  }
  handleSimpleAttribute<AttrType>(S, D, CI);
}

} // namespace language::Core
#endif // LANGUAGE_CORE_SEMA_ATTR_H
