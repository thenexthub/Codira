//===- ExternalASTSource.cpp - Abstract External AST Interface ------------===//
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
//  This file provides the default implementation of the ExternalASTSource
//  interface, which enables construction of AST nodes from some external
//  source.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ExternalASTSource.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/Basic/ASTSourceDescriptor.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/Support/ErrorHandling.h"
#include <cstdint>
#include <optional>

using namespace language::Core;

char ExternalASTSource::ID;

ExternalASTSource::~ExternalASTSource() = default;

std::optional<ASTSourceDescriptor>
ExternalASTSource::getSourceDescriptor(unsigned ID) {
  return std::nullopt;
}

ExternalASTSource::ExtKind
ExternalASTSource::hasExternalDefinitions(const Decl *D) {
  return EK_ReplyHazy;
}

bool ExternalASTSource::wasThisDeclarationADefinition(const FunctionDecl *FD) {
  return false;
}

void ExternalASTSource::FindFileRegionDecls(FileID File, unsigned Offset,
                                            unsigned Length,
                                            SmallVectorImpl<Decl *> &Decls) {}

void ExternalASTSource::CompleteRedeclChain(const Decl *D) {}

void ExternalASTSource::CompleteType(TagDecl *Tag) {}

void ExternalASTSource::CompleteType(ObjCInterfaceDecl *Class) {}

void ExternalASTSource::ReadComments() {}

void ExternalASTSource::StartedDeserializing() {}

void ExternalASTSource::FinishedDeserializing() {}

void ExternalASTSource::StartTranslationUnit(ASTConsumer *Consumer) {}

void ExternalASTSource::PrintStats() {}

bool ExternalASTSource::layoutRecordType(
    const RecordDecl *Record, uint64_t &Size, uint64_t &Alignment,
    toolchain::DenseMap<const FieldDecl *, uint64_t> &FieldOffsets,
    toolchain::DenseMap<const CXXRecordDecl *, CharUnits> &BaseOffsets,
    toolchain::DenseMap<const CXXRecordDecl *, CharUnits> &VirtualBaseOffsets) {
  return false;
}

Decl *ExternalASTSource::GetExternalDecl(GlobalDeclID ID) { return nullptr; }

Selector ExternalASTSource::GetExternalSelector(uint32_t ID) {
  return Selector();
}

uint32_t ExternalASTSource::GetNumExternalSelectors() {
   return 0;
}

Stmt *ExternalASTSource::GetExternalDeclStmt(uint64_t Offset) {
  return nullptr;
}

CXXCtorInitializer **
ExternalASTSource::GetExternalCXXCtorInitializers(uint64_t Offset) {
  return nullptr;
}

CXXBaseSpecifier *
ExternalASTSource::GetExternalCXXBaseSpecifiers(uint64_t Offset) {
  return nullptr;
}

bool ExternalASTSource::FindExternalVisibleDeclsByName(
    const DeclContext *DC, DeclarationName Name,
    const DeclContext *OriginalDC) {
  return false;
}

bool ExternalASTSource::LoadExternalSpecializations(const Decl *D, bool) {
  return false;
}

bool ExternalASTSource::LoadExternalSpecializations(
    const Decl *D, ArrayRef<TemplateArgument>) {
  return false;
}

void ExternalASTSource::completeVisibleDeclsMap(const DeclContext *DC) {}

void ExternalASTSource::FindExternalLexicalDecls(
    const DeclContext *DC, toolchain::function_ref<bool(Decl::Kind)> IsKindWeWant,
    SmallVectorImpl<Decl *> &Result) {}

void ExternalASTSource::getMemoryBufferSizes(MemoryBufferSizes &sizes) const {}

uint32_t ExternalASTSource::incrementGeneration(ASTContext &C) {
  uint32_t OldGeneration = CurrentGeneration;

  // Make sure the generation of the topmost external source for the context is
  // incremented. That might not be us.
  auto *P = C.getExternalSource();
  if (P && P != this)
    CurrentGeneration = P->incrementGeneration(C);
  else {
    // FIXME: Only bump the generation counter if the current generation number
    // has been observed?
    if (!++CurrentGeneration)
      toolchain::reportFatalUsageError("generation counter overflowed");
  }

  return OldGeneration;
}
