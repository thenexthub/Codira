//===-- ModelInjector.cpp ---------------------------------------*- C++ -*-===//
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

#include "ModelInjector.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Basic/Stack.h"
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendAction.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Serialization/ASTReader.h"
#include "language/Core/StaticAnalyzer/Frontend/FrontendActions.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/FileSystem.h"
#include <utility>

using namespace language::Core;
using namespace ento;

ModelInjector::ModelInjector(CompilerInstance &CI) : CI(CI) {}

Stmt *ModelInjector::getBody(const FunctionDecl *D) {
  onBodySynthesis(D);
  return Bodies[D->getName()];
}

Stmt *ModelInjector::getBody(const ObjCMethodDecl *D) {
  onBodySynthesis(D);
  return Bodies[D->getName()];
}

void ModelInjector::onBodySynthesis(const NamedDecl *D) {

  // FIXME: what about overloads? Declarations can be used as keys but what
  // about file name index? Mangled names may not be suitable for that either.
  if (Bodies.count(D->getName()) != 0)
    return;

  toolchain::IntrusiveRefCntPtr<SourceManager> SM = CI.getSourceManagerPtr();
  FileID mainFileID = SM->getMainFileID();

  toolchain::StringRef modelPath = CI.getAnalyzerOpts().ModelPath;

  toolchain::SmallString<128> fileName;

  if (!modelPath.empty())
    fileName =
        toolchain::StringRef(modelPath.str() + "/" + D->getName().str() + ".model");
  else
    fileName = toolchain::StringRef(D->getName().str() + ".model");

  if (!toolchain::sys::fs::exists(fileName.str())) {
    Bodies[D->getName()] = nullptr;
    return;
  }

  auto Invocation = std::make_shared<CompilerInvocation>(CI.getInvocation());

  FrontendOptions &FrontendOpts = Invocation->getFrontendOpts();
  InputKind IK = Language::CXX; // FIXME
  FrontendOpts.Inputs.clear();
  FrontendOpts.Inputs.emplace_back(fileName, IK);
  FrontendOpts.DisableFree = true;

  Invocation->getDiagnosticOpts().VerifyDiagnostics = 0;

  // Modules are parsed by a separate CompilerInstance, so this code mimics that
  // behavior for models
  CompilerInstance Instance(std::move(Invocation),
                            CI.getPCHContainerOperations());
  Instance.createDiagnostics(
      CI.getVirtualFileSystem(),
      new ForwardingDiagnosticConsumer(CI.getDiagnosticClient()),
      /*ShouldOwnClient=*/true);

  Instance.getDiagnostics().setSourceManager(SM.get());

  // The instance wants to take ownership, however DisableFree frontend option
  // is set to true to avoid double free issues
  Instance.setFileManager(CI.getFileManagerPtr());
  Instance.setSourceManager(SM);
  Instance.setPreprocessor(CI.getPreprocessorPtr());
  Instance.setASTContext(CI.getASTContextPtr());

  Instance.getPreprocessor().InitializeForModelFile();

  ParseModelFileAction parseModelFile(Bodies);

  toolchain::CrashRecoveryContext CRC;

  CRC.RunSafelyOnThread([&]() { Instance.ExecuteAction(parseModelFile); },
                        DesiredStackSize);

  Instance.getPreprocessor().FinalizeForModelFile();

  Instance.resetAndLeakSourceManager();
  Instance.resetAndLeakFileManager();
  Instance.resetAndLeakPreprocessor();

  // The preprocessor enters to the main file id when parsing is started, so
  // the main file id is changed to the model file during parsing and it needs
  // to be reset to the former main file id after parsing of the model file
  // is done.
  SM->setMainFileID(mainFileID);
}
