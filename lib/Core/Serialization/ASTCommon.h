//===- ASTCommon.h - Common stuff for ASTReader/ASTWriter -*- C++ -*-=========//
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
//  This file defines common functions that both ASTReader and ASTWriter use.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_SERIALIZATION_ASTCOMMON_H
#define LANGUAGE_CORE_LIB_SERIALIZATION_ASTCOMMON_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclFriend.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Serialization/ASTBitCodes.h"

namespace language::Core {

namespace serialization {

enum class DeclUpdateKind {
  CXXAddedImplicitMember,
  CXXAddedAnonymousNamespace,
  CXXAddedFunctionDefinition,
  CXXAddedVarDefinition,
  CXXPointOfInstantiation,
  CXXInstantiatedClassDefinition,
  CXXInstantiatedDefaultArgument,
  CXXInstantiatedDefaultMemberInitializer,
  CXXResolvedDtorDelete,
  CXXResolvedExceptionSpec,
  CXXDeducedReturnType,
  DeclMarkedUsed,
  ManglingNumber,
  StaticLocalNumber,
  DeclMarkedOpenMPThreadPrivate,
  DeclMarkedOpenMPAllocate,
  DeclMarkedOpenMPDeclareTarget,
  DeclExported,
  AddedAttrToRecord
};

TypeIdx TypeIdxFromBuiltin(const BuiltinType *BT);

unsigned ComputeHash(Selector Sel);

/// Retrieve the "definitive" declaration that provides all of the
/// visible entries for the given declaration context, if there is one.
///
/// The "definitive" declaration is the only place where we need to look to
/// find information about the declarations within the given declaration
/// context. For example, C++ and Objective-C classes, C structs/unions, and
/// Objective-C protocols, categories, and extensions are all defined in a
/// single place in the source code, so they have definitive declarations
/// associated with them. C++ namespaces, on the other hand, can have
/// multiple definitions.
const DeclContext *getDefinitiveDeclContext(const DeclContext *DC);

/// Determine whether the given declaration kind is redeclarable.
bool isRedeclarableDeclKind(unsigned Kind);

/// Determine whether the given declaration needs an anonymous
/// declaration number.
bool needsAnonymousDeclarationNumber(const NamedDecl *D);

/// Visit each declaration within \c DC that needs an anonymous
/// declaration number and call \p Visit with the declaration and its number.
template<typename Fn> void numberAnonymousDeclsWithin(const DeclContext *DC,
                                                      Fn Visit) {
  unsigned Index = 0;
  for (Decl *LexicalD : DC->decls()) {
    // For a friend decl, we care about the declaration within it, if any.
    if (auto *FD = dyn_cast<FriendDecl>(LexicalD))
      LexicalD = FD->getFriendDecl();

    auto *ND = dyn_cast_or_null<NamedDecl>(LexicalD);
    if (!ND || !needsAnonymousDeclarationNumber(ND))
      continue;

    Visit(ND, Index++);
  }
}

/// Determine whether the given declaration will be included in the per-module
/// initializer if it needs to be eagerly handed to the AST consumer. If so, we
/// should not hand it to the consumer when deserializing it, nor include it in
/// the list of eagerly deserialized declarations.
inline bool isPartOfPerModuleInitializer(const Decl *D) {
  if (isa<ImportDecl>(D))
    return true;
  // Template instantiations are notionally in an "instantiation unit" rather
  // than in any particular translation unit, so they need not be part of any
  // particular (sub)module's per-module initializer.
  if (auto *VD = dyn_cast<VarDecl>(D))
    return !isTemplateInstantiation(VD->getTemplateSpecializationKind());
  return false;
}

} // namespace serialization

} // namespace language::Core

#endif
