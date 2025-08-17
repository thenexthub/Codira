//===--- TestAST.cpp ------------------------------------------------------===//
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

#include "language/Core/Testing/TestAST.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/Frontend/TextDiagnostic.h"
#include "language/Core/Testing/CommandLineArgs.h"
#include "toolchain/ADT/ScopeExit.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/VirtualFileSystem.h"

#include "gtest/gtest.h"
#include <string>

namespace language::Core {
namespace {

// Captures diagnostics into a vector, optionally reporting errors to gtest.
class StoreDiagnostics : public DiagnosticConsumer {
  std::vector<StoredDiagnostic> &Out;
  bool ReportErrors;
  LangOptions LangOpts;

public:
  StoreDiagnostics(std::vector<StoredDiagnostic> &Out, bool ReportErrors)
      : Out(Out), ReportErrors(ReportErrors) {}

  void BeginSourceFile(const LangOptions &LangOpts,
                       const Preprocessor *) override {
    this->LangOpts = LangOpts;
  }

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    Out.emplace_back(DiagLevel, Info);
    if (ReportErrors && DiagLevel >= DiagnosticsEngine::Error) {
      std::string Text;
      toolchain::raw_string_ostream OS(Text);
      TextDiagnostic Renderer(OS, LangOpts,
                              Info.getDiags()->getDiagnosticOptions());
      Renderer.emitStoredDiagnostic(Out.back());
      ADD_FAILURE() << Text;
    }
  }
};

// Fills in the bits of a CompilerInstance that weren't initialized yet.
// Provides "empty" ASTContext etc if we fail before parsing gets started.
void createMissingComponents(CompilerInstance &Clang) {
  if (!Clang.hasDiagnostics())
    Clang.createDiagnostics(*toolchain::vfs::getRealFileSystem());
  if (!Clang.hasFileManager())
    Clang.createFileManager();
  if (!Clang.hasSourceManager())
    Clang.createSourceManager(Clang.getFileManager());
  if (!Clang.hasTarget())
    Clang.createTarget();
  if (!Clang.hasPreprocessor())
    Clang.createPreprocessor(TU_Complete);
  if (!Clang.hasASTConsumer())
    Clang.setASTConsumer(std::make_unique<ASTConsumer>());
  if (!Clang.hasASTContext())
    Clang.createASTContext();
  if (!Clang.hasSema())
    Clang.createSema(TU_Complete, /*CodeCompleteConsumer=*/nullptr);
}

} // namespace

TestAST::TestAST(const TestInputs &In) {
  Clang = std::make_unique<CompilerInstance>();
  // If we don't manage to finish parsing, create CompilerInstance components
  // anyway so that the test will see an empty AST instead of crashing.
  auto RecoverFromEarlyExit =
      toolchain::make_scope_exit([&] { createMissingComponents(*Clang); });

  std::string Filename = In.FileName;
  if (Filename.empty())
    Filename = getFilenameForTesting(In.Language).str();

  // Set up a VFS with only the virtual file visible.
  auto VFS = toolchain::makeIntrusiveRefCnt<toolchain::vfs::InMemoryFileSystem>();
  if (auto Err = VFS->setCurrentWorkingDirectory(In.WorkingDir))
    ADD_FAILURE() << "Failed to setWD: " << Err.message();
  VFS->addFile(Filename, /*ModificationTime=*/0,
               toolchain::MemoryBuffer::getMemBufferCopy(In.Code, Filename));
  for (const auto &Extra : In.ExtraFiles)
    VFS->addFile(
        Extra.getKey(), /*ModificationTime=*/0,
        toolchain::MemoryBuffer::getMemBufferCopy(Extra.getValue(), Extra.getKey()));

  // Extra error conditions are reported through diagnostics, set that up first.
  bool ErrorOK = In.ErrorOK || toolchain::StringRef(In.Code).contains("error-ok");
  Clang->createDiagnostics(*VFS, new StoreDiagnostics(Diagnostics, !ErrorOK));

  // Parse cc1 argv, (typically [-std=c++20 input.cc]) into CompilerInvocation.
  std::vector<const char *> Argv;
  std::vector<std::string> LangArgs = getCC1ArgsForTesting(In.Language);
  for (const auto &S : LangArgs)
    Argv.push_back(S.c_str());
  for (const auto &S : In.ExtraArgs)
    Argv.push_back(S.c_str());
  Argv.push_back(Filename.c_str());
  if (!CompilerInvocation::CreateFromArgs(Clang->getInvocation(), Argv,
                                          Clang->getDiagnostics(), "clang")) {
    ADD_FAILURE() << "Failed to create invocation";
    return;
  }
  assert(!Clang->getInvocation().getFrontendOpts().DisableFree);

  Clang->createFileManager(VFS);

  // Running the FrontendAction creates the other components: SourceManager,
  // Preprocessor, ASTContext, Sema. Preprocessor needs TargetInfo to be set.
  EXPECT_TRUE(Clang->createTarget());
  Action =
      In.MakeAction ? In.MakeAction() : std::make_unique<SyntaxOnlyAction>();
  const FrontendInputFile &Main = Clang->getFrontendOpts().Inputs.front();
  if (!Action->BeginSourceFile(*Clang, Main)) {
    ADD_FAILURE() << "Failed to BeginSourceFile()";
    Action.reset(); // Don't call EndSourceFile if BeginSourceFile failed.
    return;
  }
  if (auto Err = Action->Execute())
    ADD_FAILURE() << "Failed to Execute(): " << toolchain::toString(std::move(Err));

  // Action->EndSourceFile() would destroy the ASTContext, we want to keep it.
  // But notify the preprocessor we're done now.
  Clang->getPreprocessor().EndSourceFile();
  // We're done gathering diagnostics, detach the consumer so we can destroy it.
  Clang->getDiagnosticClient().EndSourceFile();
  Clang->getDiagnostics().setClient(new DiagnosticConsumer(),
                                    /*ShouldOwnClient=*/true);
}

void TestAST::clear() {
  if (Action) {
    // We notified the preprocessor of EOF already, so detach it first.
    // Sema needs the PP alive until after EndSourceFile() though.
    auto PP = Clang->getPreprocessorPtr(); // Keep PP alive for now.
    Clang->setPreprocessor(nullptr);       // Detach so we don't send EOF twice.
    Action->EndSourceFile();               // Destroy ASTContext and Sema.
    // Now Sema is gone, PP can safely be destroyed.
  }
  Action.reset();
  Clang.reset();
  Diagnostics.clear();
}

TestAST &TestAST::operator=(TestAST &&M) {
  clear();
  Action = std::move(M.Action);
  Clang = std::move(M.Clang);
  Diagnostics = std::move(M.Diagnostics);
  return *this;
}

TestAST::TestAST(TestAST &&M) { *this = std::move(M); }

TestAST::~TestAST() { clear(); }

} // end namespace language::Core
