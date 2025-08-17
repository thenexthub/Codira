//===--- Record.cpp - struct and class metadata for the VM ------*- C++ -*-===//
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

#include "Record.h"
#include "language/Core/AST/ASTContext.h"

using namespace language::Core;
using namespace language::Core::interp;

Record::Record(const RecordDecl *Decl, BaseList &&SrcBases,
               FieldList &&SrcFields, VirtualBaseList &&SrcVirtualBases,
               unsigned VirtualSize, unsigned BaseSize)
    : Decl(Decl), Bases(std::move(SrcBases)), Fields(std::move(SrcFields)),
      BaseSize(BaseSize), VirtualSize(VirtualSize), IsUnion(Decl->isUnion()),
      IsAnonymousUnion(IsUnion && Decl->isAnonymousStructOrUnion()) {
  for (Base &V : SrcVirtualBases)
    VirtualBases.push_back({V.Decl, V.Offset + BaseSize, V.Desc, V.R});

  for (Base &B : Bases)
    BaseMap[B.Decl] = &B;
  for (Field &F : Fields)
    FieldMap[F.Decl] = &F;
  for (Base &V : VirtualBases)
    VirtualBaseMap[V.Decl] = &V;
}

std::string Record::getName() const {
  std::string Ret;
  toolchain::raw_string_ostream OS(Ret);
  Decl->getNameForDiagnostic(OS, Decl->getASTContext().getPrintingPolicy(),
                             /*Qualified=*/true);
  return Ret;
}

const Record::Field *Record::getField(const FieldDecl *FD) const {
  auto It = FieldMap.find(FD->getFirstDecl());
  assert(It != FieldMap.end() && "Missing field");
  return It->second;
}

const Record::Base *Record::getBase(const RecordDecl *FD) const {
  auto It = BaseMap.find(FD);
  assert(It != BaseMap.end() && "Missing base");
  return It->second;
}

const Record::Base *Record::getBase(QualType T) const {
  if (auto *RT = T->getAs<RecordType>()) {
    const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
    return BaseMap.lookup(RD);
  }
  return nullptr;
}

const Record::Base *Record::getVirtualBase(const RecordDecl *FD) const {
  auto It = VirtualBaseMap.find(FD);
  assert(It != VirtualBaseMap.end() && "Missing virtual base");
  return It->second;
}
