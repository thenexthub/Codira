//===- ASTImporterLookupTable.cpp - ASTImporter specific lookup -----------===//
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
//  This file defines the ASTImporterLookupTable class which implements a
//  lookup procedure for the import mechanism.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTImporterLookupTable.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "toolchain/Support/FormatVariadic.h"

namespace language::Core {

namespace {

struct Builder : RecursiveASTVisitor<Builder> {
  ASTImporterLookupTable &LT;
  Builder(ASTImporterLookupTable &LT) : LT(LT) {}

  bool VisitTypedefNameDecl(TypedefNameDecl *D) {
    QualType Ty = D->getUnderlyingType();
    Ty = Ty.getCanonicalType();
    if (const auto *RTy = dyn_cast<RecordType>(Ty)) {
      LT.add(RTy->getAsRecordDecl());
      // iterate over the field decls, adding them
      for (auto *it : RTy->getAsRecordDecl()->fields()) {
        LT.add(it);
      }
    }
    return true;
  }

  bool VisitNamedDecl(NamedDecl *D) {
    LT.add(D);
    return true;
  }
  // In most cases the FriendDecl contains the declaration of the befriended
  // class as a child node, so it is discovered during the recursive
  // visitation. However, there are cases when the befriended class is not a
  // child, thus it must be fetched explicitly from the FriendDecl, and only
  // then can we add it to the lookup table.
  bool VisitFriendDecl(FriendDecl *D) {
    if (D->getFriendType()) {
      QualType Ty = D->getFriendType()->getType();
      // A FriendDecl with a dependent type (e.g. ClassTemplateSpecialization)
      // always has that decl as child node.
      // However, there are non-dependent cases which does not have the
      // type as a child node. We have to dig up that type now.
      if (!Ty->isDependentType()) {
        if (const auto *RTy = dyn_cast<RecordType>(Ty))
          LT.add(RTy->getAsCXXRecordDecl());
        else if (const auto *SpecTy = dyn_cast<TemplateSpecializationType>(Ty))
          LT.add(SpecTy->getAsCXXRecordDecl());
        else if (const auto *SubstTy =
                     dyn_cast<SubstTemplateTypeParmType>(Ty)) {
          if (SubstTy->getAsCXXRecordDecl())
            LT.add(SubstTy->getAsCXXRecordDecl());
        } else {
          if (isa<TypedefType>(Ty)) {
            // We do not put friend typedefs to the lookup table because
            // ASTImporter does not organize typedefs into redecl chains.
          } else if (isa<UsingType>(Ty)) {
            // Similar to TypedefType, not putting into lookup table.
          } else {
            toolchain_unreachable("Unhandled type of friend class");
          }
        }
      }
    }
    return true;
  }

  // Override default settings of base.
  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }
};

} // anonymous namespace

ASTImporterLookupTable::ASTImporterLookupTable(TranslationUnitDecl &TU) {
  Builder B(*this);
  B.TraverseDecl(&TU);
  // The VaList declaration may be created on demand only or not traversed.
  // To ensure it is present and found during import, add it to the table now.
  if (auto *D =
          dyn_cast_or_null<NamedDecl>(TU.getASTContext().getVaListTagDecl())) {
    // On some platforms (AArch64) the VaList declaration can be inside a 'std'
    // namespace. This is handled specially and not visible by AST traversal.
    // ASTImporter must be able to find this namespace to import the VaList
    // declaration (and the namespace) correctly.
    if (auto *Ns = dyn_cast<NamespaceDecl>(D->getDeclContext()))
      add(&TU, Ns);
    add(D->getDeclContext(), D);
  }
}

void ASTImporterLookupTable::add(DeclContext *DC, NamedDecl *ND) {
  DeclList &Decls = LookupTable[DC][ND->getDeclName()];
  // Inserts if and only if there is no element in the container equal to it.
  Decls.insert(ND);
}

void ASTImporterLookupTable::remove(DeclContext *DC, NamedDecl *ND) {
  const DeclarationName Name = ND->getDeclName();
  DeclList &Decls = LookupTable[DC][Name];
  bool EraseResult = Decls.remove(ND);
  (void)EraseResult;
#ifndef NDEBUG
  if (!EraseResult) {
    std::string Message =
        toolchain::formatv(
            "Trying to remove not contained Decl '{0}' of type {1} from a {2}",
            Name.getAsString(), ND->getDeclKindName(), DC->getDeclKindName())
            .str();
    toolchain_unreachable(Message.c_str());
  }
#endif
}

void ASTImporterLookupTable::add(NamedDecl *ND) {
  assert(ND);
  DeclContext *DC = ND->getDeclContext();
  add(DC, ND);
  DeclContext *ReDC = DC->getRedeclContext();
  if (DC != ReDC)
    add(ReDC, ND);
}

void ASTImporterLookupTable::remove(NamedDecl *ND) {
  assert(ND);
  DeclContext *DC = ND->getDeclContext();
  remove(DC, ND);
  DeclContext *ReDC = DC->getRedeclContext();
  if (DC != ReDC)
    remove(ReDC, ND);
}

void ASTImporterLookupTable::update(NamedDecl *ND, DeclContext *OldDC) {
  assert(OldDC != ND->getDeclContext() &&
         "DeclContext should be changed before update");
  if (contains(ND->getDeclContext(), ND)) {
    assert(!contains(OldDC, ND) &&
           "Decl should not be found in the old context if already in the new");
    return;
  }

  remove(OldDC, ND);
  add(ND);
}

void ASTImporterLookupTable::updateForced(NamedDecl *ND, DeclContext *OldDC) {
  LookupTable[OldDC][ND->getDeclName()].remove(ND);
  add(ND);
}

ASTImporterLookupTable::LookupResult
ASTImporterLookupTable::lookup(DeclContext *DC, DeclarationName Name) const {
  auto DCI = LookupTable.find(DC);
  if (DCI == LookupTable.end())
    return {};

  const auto &FoundNameMap = DCI->second;
  auto NamesI = FoundNameMap.find(Name);
  if (NamesI == FoundNameMap.end())
    return {};

  return NamesI->second;
}

bool ASTImporterLookupTable::contains(DeclContext *DC, NamedDecl *ND) const {
  return lookup(DC, ND->getDeclName()).contains(ND);
}

void ASTImporterLookupTable::dump(DeclContext *DC) const {
  auto DCI = LookupTable.find(DC);
  if (DCI == LookupTable.end())
    toolchain::errs() << "empty\n";
  const auto &FoundNameMap = DCI->second;
  for (const auto &Entry : FoundNameMap) {
    DeclarationName Name = Entry.first;
    toolchain::errs() << "==== Name: ";
    Name.dump();
    const DeclList& List = Entry.second;
    for (NamedDecl *ND : List) {
      ND->dump();
    }
  }
}

void ASTImporterLookupTable::dump() const {
  for (const auto &Entry : LookupTable) {
    DeclContext *DC = Entry.first;
    toolchain::errs() << "== DC:" << cast<Decl>(DC) << "\n";
    dump(DC);
  }
}

} // namespace language::Core
