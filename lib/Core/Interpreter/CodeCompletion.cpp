//===------ CodeCompletion.cpp - Code Completion for ClangRepl -------===//
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
// This file implements the classes which performs code completion at the REPL.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Interpreter/CodeCompletion.h"
#include "language/Core/AST/ASTImporter.h"
#include "language/Core/AST/DeclLookups.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/AST/ExternalASTSource.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/Interpreter/Interpreter.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "language/Core/Sema/CodeCompleteConsumer.h"
#include "language/Core/Sema/CodeCompleteOptions.h"
#include "language/Core/Sema/Sema.h"
#include "toolchain/Support/Debug.h"
#define DEBUG_TYPE "REPLCC"

namespace language::Core {

const std::string CodeCompletionFileName = "input_line_[Completion]";

language::Core::CodeCompleteOptions getClangCompleteOpts() {
  language::Core::CodeCompleteOptions Opts;
  Opts.IncludeCodePatterns = true;
  Opts.IncludeMacros = true;
  Opts.IncludeGlobals = true;
  Opts.IncludeBriefComments = true;
  return Opts;
}

class ReplCompletionConsumer : public CodeCompleteConsumer {
public:
  ReplCompletionConsumer(std::vector<std::string> &Results,
                         ReplCodeCompleter &CC)
      : CodeCompleteConsumer(getClangCompleteOpts()),
        CCAllocator(std::make_shared<GlobalCodeCompletionAllocator>()),
        CCTUInfo(CCAllocator), Results(Results), CC(CC) {}

  // The entry of handling code completion. When the function is called, we
  // create a `Context`-based handler (see classes defined below) to handle each
  // completion result.
  void ProcessCodeCompleteResults(class Sema &S, CodeCompletionContext Context,
                                  CodeCompletionResult *InResults,
                                  unsigned NumResults) final;

  CodeCompletionAllocator &getAllocator() override { return *CCAllocator; }

  CodeCompletionTUInfo &getCodeCompletionTUInfo() override { return CCTUInfo; }

private:
  std::shared_ptr<GlobalCodeCompletionAllocator> CCAllocator;
  CodeCompletionTUInfo CCTUInfo;
  std::vector<std::string> &Results;
  ReplCodeCompleter &CC;
};

/// The class CompletionContextHandler contains four interfaces, each of
/// which handles one type of completion result.
/// Its derived classes are used to create concrete handlers based on
/// \c CodeCompletionContext.
class CompletionContextHandler {
protected:
  CodeCompletionContext CCC;
  std::vector<std::string> &Results;

private:
  Sema &S;

public:
  CompletionContextHandler(Sema &S, CodeCompletionContext CCC,
                           std::vector<std::string> &Results)
      : CCC(CCC), Results(Results), S(S) {}

  virtual ~CompletionContextHandler() = default;
  /// Converts a Declaration completion result to a completion string, and then
  /// stores it in Results.
  virtual void handleDeclaration(const CodeCompletionResult &Result) {
    auto PreferredType = CCC.getPreferredType();
    if (PreferredType.isNull()) {
      Results.push_back(Result.Declaration->getName().str());
      return;
    }

    if (auto *VD = dyn_cast<VarDecl>(Result.Declaration)) {
      auto ArgumentType = VD->getType();
      if (PreferredType->isReferenceType()) {
        QualType RT = PreferredType->castAs<ReferenceType>()->getPointeeType();
        Sema::ReferenceConversions RefConv;
        Sema::ReferenceCompareResult RefRelationship =
            S.CompareReferenceRelationship(SourceLocation(), RT, ArgumentType,
                                           &RefConv);
        switch (RefRelationship) {
        case Sema::Ref_Compatible:
        case Sema::Ref_Related:
          Results.push_back(VD->getName().str());
          break;
        case Sema::Ref_Incompatible:
          break;
        }
      } else if (S.Context.hasSameType(ArgumentType, PreferredType)) {
        Results.push_back(VD->getName().str());
      }
    }
  }

  /// Converts a Keyword completion result to a completion string, and then
  /// stores it in Results.
  virtual void handleKeyword(const CodeCompletionResult &Result) {
    auto Prefix = S.getPreprocessor().getCodeCompletionFilter();
    // Add keyword to the completion results only if we are in a type-aware
    // situation.
    if (!CCC.getBaseType().isNull() || !CCC.getPreferredType().isNull())
      return;
    if (StringRef(Result.Keyword).starts_with(Prefix))
      Results.push_back(Result.Keyword);
  }

  /// Converts a Pattern completion result to a completion string, and then
  /// stores it in Results.
  virtual void handlePattern(const CodeCompletionResult &Result) {}

  /// Converts a Macro completion result to a completion string, and then stores
  /// it in Results.
  virtual void handleMacro(const CodeCompletionResult &Result) {}
};

class DotMemberAccessHandler : public CompletionContextHandler {
public:
  DotMemberAccessHandler(Sema &S, CodeCompletionContext CCC,
                         std::vector<std::string> &Results)
      : CompletionContextHandler(S, CCC, Results) {}
  void handleDeclaration(const CodeCompletionResult &Result) override {
    auto *ID = Result.Declaration->getIdentifier();
    if (!ID)
      return;
    if (!isa<CXXMethodDecl>(Result.Declaration))
      return;
    const auto *Fun = cast<CXXMethodDecl>(Result.Declaration);
    if (Fun->getParent()->getCanonicalDecl() ==
        CCC.getBaseType()->getAsCXXRecordDecl()->getCanonicalDecl()) {
      LLVM_DEBUG(toolchain::dbgs() << "[In HandleCodeCompleteDOT] Name : "
                              << ID->getName() << "\n");
      Results.push_back(ID->getName().str());
    }
  }

  void handleKeyword(const CodeCompletionResult &Result) override {}
};

void ReplCompletionConsumer::ProcessCodeCompleteResults(
    class Sema &S, CodeCompletionContext Context,
    CodeCompletionResult *InResults, unsigned NumResults) {

  auto Prefix = S.getPreprocessor().getCodeCompletionFilter();
  CC.Prefix = Prefix;

  std::unique_ptr<CompletionContextHandler> CCH;

  // initialize fine-grained code completion handler based on the code
  // completion context.
  switch (Context.getKind()) {
  case CodeCompletionContext::CCC_DotMemberAccess:
    CCH.reset(new DotMemberAccessHandler(S, Context, this->Results));
    break;
  default:
    CCH.reset(new CompletionContextHandler(S, Context, this->Results));
  };

  for (unsigned I = 0; I < NumResults; I++) {
    auto &Result = InResults[I];
    switch (Result.Kind) {
    case CodeCompletionResult::RK_Declaration:
      if (Result.Hidden) {
        break;
      }
      if (!Result.Declaration->getDeclName().isIdentifier() ||
          !Result.Declaration->getName().starts_with(Prefix)) {
        break;
      }
      CCH->handleDeclaration(Result);
      break;
    case CodeCompletionResult::RK_Keyword:
      CCH->handleKeyword(Result);
      break;
    case CodeCompletionResult::RK_Macro:
      CCH->handleMacro(Result);
      break;
    case CodeCompletionResult::RK_Pattern:
      CCH->handlePattern(Result);
      break;
    }
  }

  std::sort(Results.begin(), Results.end());
}

class IncrementalSyntaxOnlyAction : public SyntaxOnlyAction {
  const CompilerInstance *ParentCI;

public:
  IncrementalSyntaxOnlyAction(const CompilerInstance *ParentCI)
      : ParentCI(ParentCI) {}

protected:
  void ExecuteAction() override;
};

class ExternalSource : public language::Core::ExternalASTSource {
  TranslationUnitDecl *ChildTUDeclCtxt;
  ASTContext &ParentASTCtxt;
  TranslationUnitDecl *ParentTUDeclCtxt;

  std::unique_ptr<ASTImporter> Importer;

public:
  ExternalSource(ASTContext &ChildASTCtxt, FileManager &ChildFM,
                 ASTContext &ParentASTCtxt, FileManager &ParentFM);
  bool FindExternalVisibleDeclsByName(const DeclContext *DC,
                                      DeclarationName Name,
                                      const DeclContext *OriginalDC) override;
  void
  completeVisibleDeclsMap(const language::Core::DeclContext *childDeclContext) override;
};

// This method is intended to set up `ExternalASTSource` to the running
// compiler instance before the super `ExecuteAction` triggers parsing
void IncrementalSyntaxOnlyAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  auto astContextExternalSource = toolchain::makeIntrusiveRefCnt<ExternalSource>(
      CI.getASTContext(), CI.getFileManager(), ParentCI->getASTContext(),
      ParentCI->getFileManager());
  CI.getASTContext().setExternalSource(astContextExternalSource);
  CI.getASTContext().getTranslationUnitDecl()->setHasExternalVisibleStorage(
      true);

  // Load all external decls into current context. Under the hood, it calls
  // ExternalSource::completeVisibleDeclsMap, which make all decls on the redecl
  // chain visible.
  //
  // This is crucial to code completion on dot members, since a bound variable
  // before "." would be otherwise treated out-of-scope.
  //
  // clang-repl> Foo f1;
  // clang-repl> f1.<tab>
  CI.getASTContext().getTranslationUnitDecl()->lookups();
  SyntaxOnlyAction::ExecuteAction();
}

ExternalSource::ExternalSource(ASTContext &ChildASTCtxt, FileManager &ChildFM,
                               ASTContext &ParentASTCtxt, FileManager &ParentFM)
    : ChildTUDeclCtxt(ChildASTCtxt.getTranslationUnitDecl()),
      ParentASTCtxt(ParentASTCtxt),
      ParentTUDeclCtxt(ParentASTCtxt.getTranslationUnitDecl()) {
  ASTImporter *importer =
      new ASTImporter(ChildASTCtxt, ChildFM, ParentASTCtxt, ParentFM,
                      /*MinimalImport : ON*/ true);
  Importer.reset(importer);
}

bool ExternalSource::FindExternalVisibleDeclsByName(
    const DeclContext *DC, DeclarationName Name,
    const DeclContext *OriginalDC) {

  IdentifierTable &ParentIdTable = ParentASTCtxt.Idents;

  auto ParentDeclName =
      DeclarationName(&(ParentIdTable.get(Name.getAsString())));

  DeclContext::lookup_result lookup_result =
      ParentTUDeclCtxt->lookup(ParentDeclName);

  if (!lookup_result.empty()) {
    return true;
  }
  return false;
}

void ExternalSource::completeVisibleDeclsMap(
    const DeclContext *ChildDeclContext) {
  assert(ChildDeclContext && ChildDeclContext == ChildTUDeclCtxt &&
         "No child decl context!");

  if (!ChildDeclContext->hasExternalVisibleStorage())
    return;

  for (auto *DeclCtxt = ParentTUDeclCtxt; DeclCtxt != nullptr;
       DeclCtxt = DeclCtxt->getPreviousDecl()) {
    for (auto &IDeclContext : DeclCtxt->decls()) {
      if (!toolchain::isa<NamedDecl>(IDeclContext))
        continue;

      NamedDecl *Decl = toolchain::cast<NamedDecl>(IDeclContext);

      auto DeclOrErr = Importer->Import(Decl);
      if (!DeclOrErr) {
        // if an error happens, it usually means the decl has already been
        // imported or the decl is a result of a failed import.  But in our
        // case, every import is fresh each time code completion is
        // triggered. So Import usually doesn't fail. If it does, it just means
        // the related decl can't be used in code completion and we can safely
        // drop it.
        toolchain::consumeError(DeclOrErr.takeError());
        continue;
      }

      if (!toolchain::isa<NamedDecl>(*DeclOrErr))
        continue;

      NamedDecl *importedNamedDecl = toolchain::cast<NamedDecl>(*DeclOrErr);

      SetExternalVisibleDeclsForName(ChildDeclContext,
                                     importedNamedDecl->getDeclName(),
                                     importedNamedDecl);

      if (!toolchain::isa<CXXRecordDecl>(importedNamedDecl))
        continue;

      auto *Record = toolchain::cast<CXXRecordDecl>(importedNamedDecl);

      if (auto Err = Importer->ImportDefinition(Decl)) {
        // the same as above
        consumeError(std::move(Err));
        continue;
      }

      Record->setHasLoadedFieldsFromExternalStorage(true);
      LLVM_DEBUG(toolchain::dbgs()
                 << "\nCXXRecrod : " << Record->getName() << " size(methods): "
                 << std::distance(Record->method_begin(), Record->method_end())
                 << " has def?:  " << Record->hasDefinition()
                 << " # (methods): "
                 << std::distance(Record->getDefinition()->method_begin(),
                                  Record->getDefinition()->method_end())
                 << "\n");
      for (auto *Meth : Record->methods())
        SetExternalVisibleDeclsForName(ChildDeclContext, Meth->getDeclName(),
                                       Meth);
    }
    ChildDeclContext->setHasExternalLexicalStorage(false);
  }
}

void ReplCodeCompleter::codeComplete(CompilerInstance *InterpCI,
                                     toolchain::StringRef Content, unsigned Line,
                                     unsigned Col,
                                     const CompilerInstance *ParentCI,
                                     std::vector<std::string> &CCResults) {
  auto consumer = ReplCompletionConsumer(CCResults, *this);

  auto diag = InterpCI->getDiagnosticsPtr();
  std::unique_ptr<ASTUnit> AU(ASTUnit::LoadFromCompilerInvocationAction(
      InterpCI->getInvocationPtr(), std::make_shared<PCHContainerOperations>(),
      nullptr, diag));
  toolchain::SmallVector<language::Core::StoredDiagnostic, 8> sd = {};
  toolchain::SmallVector<const toolchain::MemoryBuffer *, 1> tb = {};
  InterpCI->getFrontendOpts().Inputs[0] = FrontendInputFile(
      CodeCompletionFileName, Language::CXX, InputKind::Source);
  auto Act = std::make_unique<IncrementalSyntaxOnlyAction>(ParentCI);
  std::unique_ptr<toolchain::MemoryBuffer> MB =
      toolchain::MemoryBuffer::getMemBufferCopy(Content, CodeCompletionFileName);
  toolchain::SmallVector<ASTUnit::RemappedFile, 4> RemappedFiles;

  RemappedFiles.push_back(std::make_pair(CodeCompletionFileName, MB.get()));
  // we don't want the AU destructor to release the memory buffer that MB
  // owns twice, because MB handles its resource on its own.
  AU->setOwnsRemappedFileBuffers(false);
  AU->CodeComplete(CodeCompletionFileName, 1, Col, RemappedFiles, false, false,
                   false, consumer,
                   std::make_shared<language::Core::PCHContainerOperations>(), diag,
                   InterpCI->getLangOpts(), AU->getSourceManagerPtr(),
                   AU->getFileManagerPtr(), sd, tb, std::move(Act));
}

} // namespace language::Core
