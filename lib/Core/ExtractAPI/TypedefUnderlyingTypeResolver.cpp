//===- ExtractAPI/TypedefUnderlyingTypeResolver.cpp -------------*- C++ -*-===//
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
///
/// \file
/// This file implements UnderlyingTypeResolver.
///
//===----------------------------------------------------------------------===//

#include "language/Core/ExtractAPI/TypedefUnderlyingTypeResolver.h"
#include "language/Core/Basic/Module.h"
#include "language/Core/Index/USRGeneration.h"

using namespace language::Core;
using namespace extractapi;

const NamedDecl *
TypedefUnderlyingTypeResolver::getUnderlyingTypeDecl(QualType Type) const {
  const NamedDecl *TypeDecl = nullptr;

  const TypedefType *TypedefTy = Type->getAs<TypedefType>();
  if (TypedefTy)
    TypeDecl = TypedefTy->getDecl();
  if (const TagType *TagTy = Type->getAs<TagType>()) {
    TypeDecl = TagTy->getOriginalDecl();
  } else if (const ObjCInterfaceType *ObjCITy =
                 Type->getAs<ObjCInterfaceType>()) {
    TypeDecl = ObjCITy->getDecl();
  }

  if (TypeDecl && TypedefTy) {
    // if this is a typedef to another typedef, use the typedef's decl for the
    // USR - this will actually be in the output, unlike a typedef to an
    // anonymous decl
    const TypedefNameDecl *TypedefDecl = TypedefTy->getDecl();
    if (TypedefDecl->getUnderlyingType()->isTypedefNameType())
      TypeDecl = TypedefDecl;
  }

  return TypeDecl;
}

SymbolReference
TypedefUnderlyingTypeResolver::getSymbolReferenceForType(QualType Type,
                                                         APISet &API) const {
  std::string TypeName = Type.getAsString();
  SmallString<128> TypeUSR;
  const NamedDecl *TypeDecl = getUnderlyingTypeDecl(Type);
  const TypedefType *TypedefTy = Type->getAs<TypedefType>();
  StringRef OwningModuleName;

  if (TypeDecl) {
    if (!TypedefTy)
      TypeName = TypeDecl->getName().str();

    language::Core::index::generateUSRForDecl(TypeDecl, TypeUSR);
    if (auto *OwningModule = TypeDecl->getImportedOwningModule())
      OwningModuleName = OwningModule->Name;
  } else {
    language::Core::index::generateUSRForType(Type, Context, TypeUSR);
  }

  return API.createSymbolReference(TypeName, TypeUSR, OwningModuleName);
}

std::string TypedefUnderlyingTypeResolver::getUSRForType(QualType Type) const {
  SmallString<128> TypeUSR;
  const NamedDecl *TypeDecl = getUnderlyingTypeDecl(Type);

  if (TypeDecl)
    language::Core::index::generateUSRForDecl(TypeDecl, TypeUSR);
  else
    language::Core::index::generateUSRForType(Type, Context, TypeUSR);

  return std::string(TypeUSR);
}
