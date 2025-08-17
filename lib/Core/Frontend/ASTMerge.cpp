//===-- ASTMerge.cpp - AST Merging Frontend Action --------------*- C++ -*-===//
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
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/ASTDiagnostic.h"
#include "language/Core/AST/ASTImporter.h"
#include "language/Core/AST/ASTImporterSharedState.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendActions.h"

using namespace language::Core;

std::unique_ptr<ASTConsumer>
ASTMergeAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  return AdaptedAction->CreateASTConsumer(CI, InFile);
}

bool ASTMergeAction::BeginSourceFileAction(CompilerInstance &CI) {
  // FIXME: This is a hack. We need a better way to communicate the
  // AST file, compiler instance, and file name than member variables
  // of FrontendAction.
  AdaptedAction->setCurrentInput(getCurrentInput(), takeCurrentASTUnit());
  AdaptedAction->setCompilerInstance(&CI);
  return AdaptedAction->BeginSourceFileAction(CI);
}

void ASTMergeAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  CI.getDiagnostics().getClient()->BeginSourceFile(
                                             CI.getASTContext().getLangOpts());
  CI.getDiagnostics().SetArgToStringFn(&FormatASTNodeDiagnosticArgument,
                                       &CI.getASTContext());
  IntrusiveRefCntPtr<DiagnosticIDs>
      DiagIDs(CI.getDiagnostics().getDiagnosticIDs());
  auto SharedState = std::make_shared<ASTImporterSharedState>(
      *CI.getASTContext().getTranslationUnitDecl());
  for (unsigned I = 0, N = ASTFiles.size(); I != N; ++I) {
    auto Diags = toolchain::makeIntrusiveRefCnt<DiagnosticsEngine>(
        DiagIDs, CI.getDiagnosticOpts(),
        new ForwardingDiagnosticConsumer(*CI.getDiagnostics().getClient()),
        /*ShouldOwnClient=*/true);
    std::unique_ptr<ASTUnit> Unit = ASTUnit::LoadFromASTFile(
        ASTFiles[I], CI.getPCHContainerReader(), ASTUnit::LoadEverything,
        nullptr, Diags, CI.getFileSystemOpts(), CI.getHeaderSearchOpts());

    if (!Unit)
      continue;

    ASTImporter Importer(CI.getASTContext(), CI.getFileManager(),
                         Unit->getASTContext(), Unit->getFileManager(),
                         /*MinimalImport=*/false, SharedState);

    TranslationUnitDecl *TU = Unit->getASTContext().getTranslationUnitDecl();
    for (auto *D : TU->decls()) {
      // Don't re-import __va_list_tag, __builtin_va_list.
      if (const auto *ND = dyn_cast<NamedDecl>(D))
        if (IdentifierInfo *II = ND->getIdentifier())
          if (II->isStr("__va_list_tag") || II->isStr("__builtin_va_list"))
            continue;

      toolchain::Expected<Decl *> ToDOrError = Importer.Import(D);

      if (ToDOrError) {
        DeclGroupRef DGR(*ToDOrError);
        CI.getASTConsumer().HandleTopLevelDecl(DGR);
      } else {
        toolchain::consumeError(ToDOrError.takeError());
      }
    }
  }

  AdaptedAction->ExecuteAction();
  CI.getDiagnostics().getClient()->EndSourceFile();
}

void ASTMergeAction::EndSourceFileAction() {
  return AdaptedAction->EndSourceFileAction();
}

ASTMergeAction::ASTMergeAction(std::unique_ptr<FrontendAction> adaptedAction,
                               ArrayRef<std::string> ASTFiles)
: AdaptedAction(std::move(adaptedAction)), ASTFiles(ASTFiles.begin(), ASTFiles.end()) {
  assert(AdaptedAction && "ASTMergeAction needs an action to adapt");
}

ASTMergeAction::~ASTMergeAction() {
}

bool ASTMergeAction::usesPreprocessorOnly() const {
  return AdaptedAction->usesPreprocessorOnly();
}

TranslationUnitKind ASTMergeAction::getTranslationUnitKind() {
  return AdaptedAction->getTranslationUnitKind();
}

bool ASTMergeAction::hasPCHSupport() const {
  return AdaptedAction->hasPCHSupport();
}

bool ASTMergeAction::hasASTFileSupport() const {
  return AdaptedAction->hasASTFileSupport();
}

bool ASTMergeAction::hasCodeCompletionSupport() const {
  return AdaptedAction->hasCodeCompletionSupport();
}
