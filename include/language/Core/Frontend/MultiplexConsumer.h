//===-- MultiplexConsumer.h - AST Consumer for PCH Generation ---*- C++ -*-===//
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
//  This file declares the MultiplexConsumer class, which can be used to
//  multiplex ASTConsumer and SemaConsumer messages to many consumers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_MULTIPLEXCONSUMER_H
#define LANGUAGE_CORE_FRONTEND_MULTIPLEXCONSUMER_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Sema/SemaConsumer.h"
#include "language/Core/Serialization/ASTDeserializationListener.h"
#include <memory>
#include <vector>

namespace language::Core {

class MultiplexASTMutationListener;

// This ASTDeserializationListener forwards its notifications to a set of
// child listeners.
class MultiplexASTDeserializationListener : public ASTDeserializationListener {
public:
  // Does NOT take ownership of the elements in L.
  MultiplexASTDeserializationListener(
      const std::vector<ASTDeserializationListener *> &L);
  void ReaderInitialized(ASTReader *Reader) override;
  void IdentifierRead(serialization::IdentifierID ID, IdentifierInfo *II) override;
  void MacroRead(serialization::MacroID ID, MacroInfo *MI) override;
  void TypeRead(serialization::TypeIdx Idx, QualType T) override;
  void DeclRead(GlobalDeclID ID, const Decl *D) override;
  void PredefinedDeclBuilt(PredefinedDeclIDs ID, const Decl *D) override;
  void SelectorRead(serialization::SelectorID iD, Selector Sel) override;
  void MacroDefinitionRead(serialization::PreprocessedEntityID,
                           MacroDefinitionRecord *MD) override;
  void ModuleRead(serialization::SubmoduleID ID, Module *Mod) override;
  void ModuleImportRead(serialization::SubmoduleID ID,
                        SourceLocation ImportLoc) override;

private:
  std::vector<ASTDeserializationListener *> Listeners;
};

// Has a list of ASTConsumers and calls each of them. Owns its children.
class MultiplexConsumer : public SemaConsumer {
public:
  // Takes ownership of the pointers in C.
  MultiplexConsumer(std::vector<std::unique_ptr<ASTConsumer>> C);
  MultiplexConsumer(std::unique_ptr<ASTConsumer> C);
  ~MultiplexConsumer() override;

  // ASTConsumer
  void Initialize(ASTContext &Context) override;
  void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override;
  bool HandleTopLevelDecl(DeclGroupRef D) override;
  void HandleInlineFunctionDefinition(FunctionDecl *D) override;
  void HandleInterestingDecl(DeclGroupRef D) override;
  void HandleTranslationUnit(ASTContext &Ctx) override;
  void HandleTagDeclDefinition(TagDecl *D) override;
  void HandleTagDeclRequiredDefinition(const TagDecl *D) override;
  void HandleCXXImplicitFunctionInstantiation(FunctionDecl *D) override;
  void HandleTopLevelDeclInObjCContainer(DeclGroupRef D) override;
  void HandleImplicitImportDecl(ImportDecl *D) override;
  void CompleteTentativeDefinition(VarDecl *D) override;
  void CompleteExternalDeclaration(DeclaratorDecl *D) override;
  void AssignInheritanceModel(CXXRecordDecl *RD) override;
  void HandleVTable(CXXRecordDecl *RD) override;
  ASTMutationListener *GetASTMutationListener() override;
  ASTDeserializationListener *GetASTDeserializationListener() override;
  void PrintStats() override;
  bool shouldSkipFunctionBody(Decl *D) override;

  // SemaConsumer
  void InitializeSema(Sema &S) override;
  void ForgetSema() override;

protected:
  std::vector<std::unique_ptr<ASTConsumer>> Consumers; // Owns these.
  std::unique_ptr<MultiplexASTMutationListener> MutationListener;
  std::unique_ptr<MultiplexASTDeserializationListener> DeserializationListener;
};

}  // end namespace language::Core

#endif
