//===- DeclFriend.cpp - C++ Friend Declaration AST Node Implementation ----===//
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
// This file implements the AST classes related to C++ friend
// declarations.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/DeclFriend.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclTemplate.h"
#include "language/Core/Basic/LLVM.h"
#include <cassert>
#include <cstddef>

using namespace language::Core;

void FriendDecl::anchor() {}

FriendDecl *FriendDecl::getNextFriendSlowCase() {
  return cast_or_null<FriendDecl>(
                           NextFriend.get(getASTContext().getExternalSource()));
}

FriendDecl *
FriendDecl::Create(ASTContext &C, DeclContext *DC, SourceLocation L,
                   FriendUnion Friend, SourceLocation FriendL,
                   SourceLocation EllipsisLoc,
                   ArrayRef<TemplateParameterList *> FriendTypeTPLists) {
#ifndef NDEBUG
  if (const auto *D = dyn_cast<NamedDecl *>(Friend)) {
    assert(isa<FunctionDecl>(D) ||
           isa<CXXRecordDecl>(D) ||
           isa<FunctionTemplateDecl>(D) ||
           isa<ClassTemplateDecl>(D));

    // As a temporary hack, we permit template instantiation to point
    // to the original declaration when instantiating members.
    assert(D->getFriendObjectKind() ||
           (cast<CXXRecordDecl>(DC)->getTemplateSpecializationKind()));
    // These template parameters are for friend types only.
    assert(FriendTypeTPLists.empty());
  }
#endif

  std::size_t Extra =
      FriendDecl::additionalSizeToAlloc<TemplateParameterList *>(
          FriendTypeTPLists.size());
  auto *FD = new (C, DC, Extra)
      FriendDecl(DC, L, Friend, FriendL, EllipsisLoc, FriendTypeTPLists);
  cast<CXXRecordDecl>(DC)->pushFriendDecl(FD);
  return FD;
}

FriendDecl *FriendDecl::CreateDeserialized(ASTContext &C, GlobalDeclID ID,
                                           unsigned FriendTypeNumTPLists) {
  std::size_t Extra =
      additionalSizeToAlloc<TemplateParameterList *>(FriendTypeNumTPLists);
  return new (C, ID, Extra) FriendDecl(EmptyShell(), FriendTypeNumTPLists);
}

FriendDecl *CXXRecordDecl::getFirstFriend() const {
  ExternalASTSource *Source = getParentASTContext().getExternalSource();
  Decl *First = data().FirstFriend.get(Source);
  return First ? cast<FriendDecl>(First) : nullptr;
}
