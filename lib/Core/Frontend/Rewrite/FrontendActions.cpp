//===--- FrontendActions.cpp ----------------------------------------------===//
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

#include "language/Core/Rewrite/Frontend/FrontendActions.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Basic/CharInfo.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Config/config.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/Frontend/Utils.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "language/Core/Rewrite/Frontend/ASTConsumers.h"
#include "language/Core/Rewrite/Frontend/FixItRewriter.h"
#include "language/Core/Rewrite/Frontend/Rewriters.h"
#include "language/Core/Serialization/ASTReader.h"
#include "language/Core/Serialization/ModuleFile.h"
#include "language/Core/Serialization/ModuleManager.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include <memory>
#include <utility>

using namespace language::Core;

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

std::unique_ptr<ASTConsumer>
HTMLPrintAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  if (std::unique_ptr<raw_ostream> OS =
          CI.createDefaultOutputFile(false, InFile))
    return CreateHTMLPrinter(std::move(OS), CI.getPreprocessor());
  return nullptr;
}

FixItAction::FixItAction() {}
FixItAction::~FixItAction() {}

std::unique_ptr<ASTConsumer>
FixItAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  return std::make_unique<ASTConsumer>();
}

namespace {
class FixItRewriteInPlace : public FixItOptions {
public:
  FixItRewriteInPlace() { InPlace = true; }

  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    toolchain_unreachable("don't call RewriteFilename for inplace rewrites");
  }
};

class FixItActionSuffixInserter : public FixItOptions {
  std::string NewSuffix;

public:
  FixItActionSuffixInserter(std::string NewSuffix, bool FixWhatYouCan)
      : NewSuffix(std::move(NewSuffix)) {
    this->FixWhatYouCan = FixWhatYouCan;
  }

  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    fd = -1;
    SmallString<128> Path(Filename);
    toolchain::sys::path::replace_extension(Path,
      NewSuffix + toolchain::sys::path::extension(Path));
    return std::string(Path);
  }
};

class FixItRewriteToTemp : public FixItOptions {
public:
  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    SmallString<128> Path;
    toolchain::sys::fs::createTemporaryFile(toolchain::sys::path::filename(Filename),
                                       toolchain::sys::path::extension(Filename).drop_front(), fd,
                                       Path);
    return std::string(Path);
  }
};
} // end anonymous namespace

bool FixItAction::BeginSourceFileAction(CompilerInstance &CI) {
  const FrontendOptions &FEOpts = getCompilerInstance().getFrontendOpts();
  if (!FEOpts.FixItSuffix.empty()) {
    FixItOpts.reset(new FixItActionSuffixInserter(FEOpts.FixItSuffix,
                                                  FEOpts.FixWhatYouCan));
  } else {
    FixItOpts.reset(new FixItRewriteInPlace);
    FixItOpts->FixWhatYouCan = FEOpts.FixWhatYouCan;
  }
  Rewriter.reset(new FixItRewriter(CI.getDiagnostics(), CI.getSourceManager(),
                                   CI.getLangOpts(), FixItOpts.get()));
  return ASTFrontendAction::BeginSourceFileAction(CI);
}

void FixItAction::EndSourceFileAction() {
  // Otherwise rewrite all files.
  Rewriter->WriteFixedFiles();
  ASTFrontendAction::EndSourceFileAction();
}

bool FixItRecompile::BeginInvocation(CompilerInstance &CI) {

  std::vector<std::pair<std::string, std::string> > RewrittenFiles;
  bool err = false;
  {
    const FrontendOptions &FEOpts = CI.getFrontendOpts();
    std::unique_ptr<FrontendAction> FixAction(new SyntaxOnlyAction());
    if (FixAction->BeginSourceFile(CI, FEOpts.Inputs[0])) {
      std::unique_ptr<FixItOptions> FixItOpts;
      if (FEOpts.FixToTemporaries)
        FixItOpts.reset(new FixItRewriteToTemp());
      else
        FixItOpts.reset(new FixItRewriteInPlace());
      FixItOpts->Silent = true;
      FixItOpts->FixWhatYouCan = FEOpts.FixWhatYouCan;
      FixItOpts->FixOnlyWarnings = FEOpts.FixOnlyWarnings;
      FixItRewriter Rewriter(CI.getDiagnostics(), CI.getSourceManager(),
                             CI.getLangOpts(), FixItOpts.get());
      if (toolchain::Error Err = FixAction->Execute()) {
        // FIXME this drops the error on the floor.
        consumeError(std::move(Err));
        return false;
      }

      err = Rewriter.WriteFixedFiles(&RewrittenFiles);

      FixAction->EndSourceFile();
      CI.setSourceManager(nullptr);
      CI.setFileManager(nullptr);
    } else {
      err = true;
    }
  }
  if (err)
    return false;
  CI.getDiagnosticClient().clear();
  CI.getDiagnostics().Reset();

  PreprocessorOptions &PPOpts = CI.getPreprocessorOpts();
  PPOpts.RemappedFiles.insert(PPOpts.RemappedFiles.end(),
                              RewrittenFiles.begin(), RewrittenFiles.end());
  PPOpts.RemappedFilesKeepOriginalName = false;

  return true;
}

#if CLANG_ENABLE_OBJC_REWRITER

std::unique_ptr<ASTConsumer>
RewriteObjCAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  if (std::unique_ptr<raw_ostream> OS =
          CI.createDefaultOutputFile(false, InFile, "cpp")) {
    if (CI.getLangOpts().ObjCRuntime.isNonFragile())
      return CreateModernObjCRewriter(std::string(InFile), std::move(OS),
                                      CI.getDiagnostics(), CI.getLangOpts(),
                                      CI.getDiagnosticOpts().NoRewriteMacros,
                                      (CI.getCodeGenOpts().getDebugInfo() !=
                                       toolchain::codegenoptions::NoDebugInfo));
    return CreateObjCRewriter(std::string(InFile), std::move(OS),
                              CI.getDiagnostics(), CI.getLangOpts(),
                              CI.getDiagnosticOpts().NoRewriteMacros);
  }
  return nullptr;
}

#endif

//===----------------------------------------------------------------------===//
// Preprocessor Actions
//===----------------------------------------------------------------------===//

void RewriteMacrosAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  std::unique_ptr<raw_ostream> OS =
      CI.createDefaultOutputFile(/*Binary=*/true, getCurrentFileOrBufferName());
  if (!OS) return;

  RewriteMacrosInInput(CI.getPreprocessor(), OS.get());
}

void RewriteTestAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  std::unique_ptr<raw_ostream> OS =
      CI.createDefaultOutputFile(/*Binary=*/false, getCurrentFileOrBufferName());
  if (!OS) return;

  DoRewriteTest(CI.getPreprocessor(), OS.get());
}

class RewriteIncludesAction::RewriteImportsListener : public ASTReaderListener {
  CompilerInstance &CI;
  std::weak_ptr<raw_ostream> Out;

  toolchain::DenseSet<const FileEntry*> Rewritten;

public:
  RewriteImportsListener(CompilerInstance &CI, std::shared_ptr<raw_ostream> Out)
      : CI(CI), Out(Out) {}

  void visitModuleFile(StringRef Filename,
                       serialization::ModuleKind Kind) override {
    auto File = CI.getFileManager().getOptionalFileRef(Filename);
    assert(File && "missing file for loaded module?");

    // Only rewrite each module file once.
    if (!Rewritten.insert(*File).second)
      return;

    serialization::ModuleFile *MF =
        CI.getASTReader()->getModuleManager().lookup(*File);
    assert(MF && "missing module file for loaded module?");

    // Not interested in PCH / preambles.
    if (!MF->isModule())
      return;

    auto OS = Out.lock();
    assert(OS && "loaded module file after finishing rewrite action?");

    (*OS) << "#pragma clang module build ";
    if (isValidAsciiIdentifier(MF->ModuleName))
      (*OS) << MF->ModuleName;
    else {
      (*OS) << '"';
      OS->write_escaped(MF->ModuleName);
      (*OS) << '"';
    }
    (*OS) << '\n';

    // Rewrite the contents of the module in a separate compiler instance.
    CompilerInstance Instance(
        std::make_shared<CompilerInvocation>(CI.getInvocation()),
        CI.getPCHContainerOperations(), &CI.getModuleCache());
    Instance.createDiagnostics(
        CI.getVirtualFileSystem(),
        new ForwardingDiagnosticConsumer(CI.getDiagnosticClient()),
        /*ShouldOwnClient=*/true);
    Instance.getFrontendOpts().DisableFree = false;
    Instance.getFrontendOpts().Inputs.clear();
    Instance.getFrontendOpts().Inputs.emplace_back(
        Filename, InputKind(Language::Unknown, InputKind::Precompiled));
    Instance.getFrontendOpts().ModuleFiles.clear();
    Instance.getFrontendOpts().ModuleMapFiles.clear();
    // Don't recursively rewrite imports. We handle them all at the top level.
    Instance.getPreprocessorOutputOpts().RewriteImports = false;

    toolchain::CrashRecoveryContext().RunSafelyOnThread([&]() {
      RewriteIncludesAction Action;
      Action.OutputStream = OS;
      Instance.ExecuteAction(Action);
    });

    (*OS) << "#pragma clang module endbuild /*" << MF->ModuleName << "*/\n";
  }
};

bool RewriteIncludesAction::BeginSourceFileAction(CompilerInstance &CI) {
  if (!OutputStream) {
    OutputStream =
        CI.createDefaultOutputFile(/*Binary=*/true, getCurrentFileOrBufferName());
    if (!OutputStream)
      return false;
  }

  auto &OS = *OutputStream;

  // If we're preprocessing a module map, start by dumping the contents of the
  // module itself before switching to the input buffer.
  auto &Input = getCurrentInput();
  if (Input.getKind().getFormat() == InputKind::ModuleMap) {
    if (Input.isFile()) {
      OS << "# 1 \"";
      OS.write_escaped(Input.getFile());
      OS << "\"\n";
    }
    getCurrentModule()->print(OS);
    OS << "#pragma clang module contents\n";
  }

  // If we're rewriting imports, set up a listener to track when we import
  // module files.
  if (CI.getPreprocessorOutputOpts().RewriteImports) {
    CI.createASTReader();
    CI.getASTReader()->addListener(
        std::make_unique<RewriteImportsListener>(CI, OutputStream));
  }

  return PreprocessorFrontendAction::BeginSourceFileAction(CI);
}

void RewriteIncludesAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();

  // If we're rewriting imports, emit the module build output first rather
  // than switching back and forth (potentially in the middle of a line).
  if (CI.getPreprocessorOutputOpts().RewriteImports) {
    std::string Buffer;
    toolchain::raw_string_ostream OS(Buffer);

    RewriteIncludesInInput(CI.getPreprocessor(), &OS,
                           CI.getPreprocessorOutputOpts());

    (*OutputStream) << OS.str();
  } else {
    RewriteIncludesInInput(CI.getPreprocessor(), OutputStream.get(),
                           CI.getPreprocessorOutputOpts());
  }

  OutputStream.reset();
}
