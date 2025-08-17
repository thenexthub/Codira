//===--- MultiplexExternalSemaSource.cpp  ---------------------------------===//
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
// This file implements the event dispatching to the subscribed clients.
//
//===----------------------------------------------------------------------===//
#include "language/Core/Sema/MultiplexExternalSemaSource.h"
#include "language/Core/Sema/Lookup.h"

using namespace language::Core;

char MultiplexExternalSemaSource::ID;

/// Constructs a new multiplexing external sema source and appends the
/// given element to it.
///
MultiplexExternalSemaSource::MultiplexExternalSemaSource(
    toolchain::IntrusiveRefCntPtr<ExternalSemaSource> S1,
    toolchain::IntrusiveRefCntPtr<ExternalSemaSource> S2) {
  Sources.push_back(std::move(S1));
  Sources.push_back(std::move(S2));
}

/// Appends new source to the source list.
///
///\param[in] source - An ExternalSemaSource.
///
void MultiplexExternalSemaSource::AddSource(
    toolchain::IntrusiveRefCntPtr<ExternalSemaSource> Source) {
  Sources.push_back(std::move(Source));
}

//===----------------------------------------------------------------------===//
// ExternalASTSource.
//===----------------------------------------------------------------------===//

Decl *MultiplexExternalSemaSource::GetExternalDecl(GlobalDeclID ID) {
  for(size_t i = 0; i < Sources.size(); ++i)
    if (Decl *Result = Sources[i]->GetExternalDecl(ID))
      return Result;
  return nullptr;
}

void MultiplexExternalSemaSource::CompleteRedeclChain(const Decl *D) {
  for (size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->CompleteRedeclChain(D);
}

Selector MultiplexExternalSemaSource::GetExternalSelector(uint32_t ID) {
  Selector Sel;
  for(size_t i = 0; i < Sources.size(); ++i) {
    Sel = Sources[i]->GetExternalSelector(ID);
    if (!Sel.isNull())
      return Sel;
  }
  return Sel;
}

uint32_t MultiplexExternalSemaSource::GetNumExternalSelectors() {
  uint32_t total = 0;
  for(size_t i = 0; i < Sources.size(); ++i)
    total += Sources[i]->GetNumExternalSelectors();
  return total;
}

Stmt *MultiplexExternalSemaSource::GetExternalDeclStmt(uint64_t Offset) {
  for(size_t i = 0; i < Sources.size(); ++i)
    if (Stmt *Result = Sources[i]->GetExternalDeclStmt(Offset))
      return Result;
  return nullptr;
}

CXXBaseSpecifier *MultiplexExternalSemaSource::GetExternalCXXBaseSpecifiers(
                                                               uint64_t Offset){
  for(size_t i = 0; i < Sources.size(); ++i)
    if (CXXBaseSpecifier *R = Sources[i]->GetExternalCXXBaseSpecifiers(Offset))
      return R;
  return nullptr;
}

CXXCtorInitializer **
MultiplexExternalSemaSource::GetExternalCXXCtorInitializers(uint64_t Offset) {
  for (auto &S : Sources)
    if (auto *R = S->GetExternalCXXCtorInitializers(Offset))
      return R;
  return nullptr;
}

ExternalASTSource::ExtKind
MultiplexExternalSemaSource::hasExternalDefinitions(const Decl *D) {
  for (const auto &S : Sources)
    if (auto EK = S->hasExternalDefinitions(D))
      if (EK != EK_ReplyHazy)
        return EK;
  return EK_ReplyHazy;
}

bool MultiplexExternalSemaSource::wasThisDeclarationADefinition(
    const FunctionDecl *FD) {
  for (const auto &S : Sources)
    if (S->wasThisDeclarationADefinition(FD))
      return true;
  return false;
}

bool MultiplexExternalSemaSource::FindExternalVisibleDeclsByName(
    const DeclContext *DC, DeclarationName Name,
    const DeclContext *OriginalDC) {
  bool AnyDeclsFound = false;
  for (size_t i = 0; i < Sources.size(); ++i)
    AnyDeclsFound |=
        Sources[i]->FindExternalVisibleDeclsByName(DC, Name, OriginalDC);
  return AnyDeclsFound;
}

bool MultiplexExternalSemaSource::LoadExternalSpecializations(
    const Decl *D, bool OnlyPartial) {
  bool Loaded = false;
  for (size_t i = 0; i < Sources.size(); ++i)
    Loaded |= Sources[i]->LoadExternalSpecializations(D, OnlyPartial);
  return Loaded;
}

bool MultiplexExternalSemaSource::LoadExternalSpecializations(
    const Decl *D, ArrayRef<TemplateArgument> TemplateArgs) {
  bool AnyNewSpecsLoaded = false;
  for (size_t i = 0; i < Sources.size(); ++i)
    AnyNewSpecsLoaded |=
        Sources[i]->LoadExternalSpecializations(D, TemplateArgs);
  return AnyNewSpecsLoaded;
}

void MultiplexExternalSemaSource::completeVisibleDeclsMap(const DeclContext *DC){
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->completeVisibleDeclsMap(DC);
}

void MultiplexExternalSemaSource::FindExternalLexicalDecls(
    const DeclContext *DC, toolchain::function_ref<bool(Decl::Kind)> IsKindWeWant,
    SmallVectorImpl<Decl *> &Result) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->FindExternalLexicalDecls(DC, IsKindWeWant, Result);
}

void MultiplexExternalSemaSource::FindFileRegionDecls(FileID File,
                                                      unsigned Offset,
                                                      unsigned Length,
                                                SmallVectorImpl<Decl *> &Decls){
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->FindFileRegionDecls(File, Offset, Length, Decls);
}

void MultiplexExternalSemaSource::CompleteType(TagDecl *Tag) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->CompleteType(Tag);
}

void MultiplexExternalSemaSource::CompleteType(ObjCInterfaceDecl *Class) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->CompleteType(Class);
}

void MultiplexExternalSemaSource::ReadComments() {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadComments();
}

void MultiplexExternalSemaSource::StartedDeserializing() {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->StartedDeserializing();
}

void MultiplexExternalSemaSource::FinishedDeserializing() {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->FinishedDeserializing();
}

void MultiplexExternalSemaSource::StartTranslationUnit(ASTConsumer *Consumer) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->StartTranslationUnit(Consumer);
}

void MultiplexExternalSemaSource::PrintStats() {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->PrintStats();
}

Module *MultiplexExternalSemaSource::getModule(unsigned ID) {
  for (size_t i = 0; i < Sources.size(); ++i)
    if (auto M = Sources[i]->getModule(ID))
      return M;
  return nullptr;
}

bool MultiplexExternalSemaSource::layoutRecordType(const RecordDecl *Record,
                                                   uint64_t &Size,
                                                   uint64_t &Alignment,
                      toolchain::DenseMap<const FieldDecl *, uint64_t> &FieldOffsets,
                  toolchain::DenseMap<const CXXRecordDecl *, CharUnits> &BaseOffsets,
          toolchain::DenseMap<const CXXRecordDecl *, CharUnits> &VirtualBaseOffsets){
  for(size_t i = 0; i < Sources.size(); ++i)
    if (Sources[i]->layoutRecordType(Record, Size, Alignment, FieldOffsets,
                                     BaseOffsets, VirtualBaseOffsets))
      return true;
  return false;
}

void MultiplexExternalSemaSource::
getMemoryBufferSizes(MemoryBufferSizes &sizes) const {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->getMemoryBufferSizes(sizes);

}

//===----------------------------------------------------------------------===//
// ExternalSemaSource.
//===----------------------------------------------------------------------===//


void MultiplexExternalSemaSource::InitializeSema(Sema &S) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->InitializeSema(S);
}

void MultiplexExternalSemaSource::ForgetSema() {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ForgetSema();
}

void MultiplexExternalSemaSource::ReadMethodPool(Selector Sel) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadMethodPool(Sel);
}

void MultiplexExternalSemaSource::updateOutOfDateSelector(Selector Sel) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->updateOutOfDateSelector(Sel);
}

void MultiplexExternalSemaSource::ReadKnownNamespaces(
                                   SmallVectorImpl<NamespaceDecl*> &Namespaces){
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadKnownNamespaces(Namespaces);
}

void MultiplexExternalSemaSource::ReadUndefinedButUsed(
    toolchain::MapVector<NamedDecl *, SourceLocation> &Undefined) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadUndefinedButUsed(Undefined);
}

void MultiplexExternalSemaSource::ReadMismatchingDeleteExpressions(
    toolchain::MapVector<FieldDecl *,
                    toolchain::SmallVector<std::pair<SourceLocation, bool>, 4>> &
        Exprs) {
  for (auto &Source : Sources)
    Source->ReadMismatchingDeleteExpressions(Exprs);
}

bool MultiplexExternalSemaSource::LookupUnqualified(LookupResult &R, Scope *S){
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->LookupUnqualified(R, S);

  return !R.empty();
}

void MultiplexExternalSemaSource::ReadTentativeDefinitions(
                                     SmallVectorImpl<VarDecl*> &TentativeDefs) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadTentativeDefinitions(TentativeDefs);
}

void MultiplexExternalSemaSource::ReadUnusedFileScopedDecls(
                                SmallVectorImpl<const DeclaratorDecl*> &Decls) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadUnusedFileScopedDecls(Decls);
}

void MultiplexExternalSemaSource::ReadDelegatingConstructors(
                                  SmallVectorImpl<CXXConstructorDecl*> &Decls) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadDelegatingConstructors(Decls);
}

void MultiplexExternalSemaSource::ReadExtVectorDecls(
                                     SmallVectorImpl<TypedefNameDecl*> &Decls) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadExtVectorDecls(Decls);
}

void MultiplexExternalSemaSource::ReadDeclsToCheckForDeferredDiags(
    toolchain::SmallSetVector<Decl *, 4> &Decls) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadDeclsToCheckForDeferredDiags(Decls);
}

void MultiplexExternalSemaSource::ReadUnusedLocalTypedefNameCandidates(
    toolchain::SmallSetVector<const TypedefNameDecl *, 4> &Decls) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadUnusedLocalTypedefNameCandidates(Decls);
}

void MultiplexExternalSemaSource::ReadReferencedSelectors(
                  SmallVectorImpl<std::pair<Selector, SourceLocation> > &Sels) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadReferencedSelectors(Sels);
}

void MultiplexExternalSemaSource::ReadWeakUndeclaredIdentifiers(
                   SmallVectorImpl<std::pair<IdentifierInfo*, WeakInfo> > &WI) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadWeakUndeclaredIdentifiers(WI);
}

void MultiplexExternalSemaSource::ReadUsedVTables(
                                  SmallVectorImpl<ExternalVTableUse> &VTables) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadUsedVTables(VTables);
}

void MultiplexExternalSemaSource::ReadPendingInstantiations(
                                           SmallVectorImpl<std::pair<ValueDecl*,
                                                   SourceLocation> > &Pending) {
  for(size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadPendingInstantiations(Pending);
}

void MultiplexExternalSemaSource::ReadLateParsedTemplates(
    toolchain::MapVector<const FunctionDecl *, std::unique_ptr<LateParsedTemplate>>
        &LPTMap) {
  for (size_t i = 0; i < Sources.size(); ++i)
    Sources[i]->ReadLateParsedTemplates(LPTMap);
}

TypoCorrection MultiplexExternalSemaSource::CorrectTypo(
                                     const DeclarationNameInfo &Typo,
                                     int LookupKind, Scope *S, CXXScopeSpec *SS,
                                     CorrectionCandidateCallback &CCC,
                                     DeclContext *MemberContext,
                                     bool EnteringContext,
                                     const ObjCObjectPointerType *OPT) {
  for (size_t I = 0, E = Sources.size(); I < E; ++I) {
    if (TypoCorrection C = Sources[I]->CorrectTypo(Typo, LookupKind, S, SS, CCC,
                                                   MemberContext,
                                                   EnteringContext, OPT))
      return C;
  }
  return TypoCorrection();
}

bool MultiplexExternalSemaSource::MaybeDiagnoseMissingCompleteType(
    SourceLocation Loc, QualType T) {
  for (size_t I = 0, E = Sources.size(); I < E; ++I) {
    if (Sources[I]->MaybeDiagnoseMissingCompleteType(Loc, T))
      return true;
  }
  return false;
}

void MultiplexExternalSemaSource::AssignedLambdaNumbering(
    CXXRecordDecl *Lambda) {
  for (auto &Source : Sources)
    Source->AssignedLambdaNumbering(Lambda);
}
