//===- ChainedIncludesSource.cpp - Chained PCHs in Memory -------*- C++ -*-===//
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
//  This file defines the ChainedIncludesSource class, which converts headers
//  to chained PCHs in memory, mainly used for testing.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/Builtins.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/TextDiagnosticPrinter.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "language/Core/Parse/ParseAST.h"
#include "language/Core/Sema/MultiplexExternalSemaSource.h"
#include "language/Core/Serialization/ASTReader.h"
#include "language/Core/Serialization/ASTWriter.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace language::Core;

namespace {
class ChainedIncludesSource : public ExternalSemaSource {
public:
  ChainedIncludesSource(std::vector<std::unique_ptr<CompilerInstance>> CIs)
      : CIs(std::move(CIs)) {}

protected:
  //===--------------------------------------------------------------------===//
  // ExternalASTSource interface.
  //===--------------------------------------------------------------------===//

  /// Return the amount of memory used by memory buffers, breaking down
  /// by heap-backed versus mmap'ed memory.
  void getMemoryBufferSizes(MemoryBufferSizes &sizes) const override {
    for (unsigned i = 0, e = CIs.size(); i != e; ++i) {
      if (const ExternalASTSource *eSrc =
          CIs[i]->getASTContext().getExternalSource()) {
        eSrc->getMemoryBufferSizes(sizes);
      }
    }
  }

private:
  std::vector<std::unique_ptr<CompilerInstance>> CIs;
};
} // end anonymous namespace

static toolchain::IntrusiveRefCntPtr<ASTReader>
createASTReader(CompilerInstance &CI, StringRef pchFile,
                SmallVectorImpl<std::unique_ptr<toolchain::MemoryBuffer>> &MemBufs,
                SmallVectorImpl<std::string> &bufNames,
                ASTDeserializationListener *deserialListener = nullptr) {
  Preprocessor &PP = CI.getPreprocessor();
  auto Reader = toolchain::makeIntrusiveRefCnt<ASTReader>(
      PP, CI.getModuleCache(), &CI.getASTContext(), CI.getPCHContainerReader(),
      CI.getCodeGenOpts(),
      /*Extensions=*/ArrayRef<std::shared_ptr<ModuleFileExtension>>(),
      /*isysroot=*/"", DisableValidationForModuleKind::PCH);
  for (unsigned ti = 0; ti < bufNames.size(); ++ti) {
    StringRef sr(bufNames[ti]);
    Reader->addInMemoryBuffer(sr, std::move(MemBufs[ti]));
  }
  Reader->setDeserializationListener(deserialListener);
  switch (Reader->ReadAST(pchFile, serialization::MK_PCH, SourceLocation(),
                          ASTReader::ARR_None)) {
  case ASTReader::Success:
    // Set the predefines buffer as suggested by the PCH reader.
    PP.setPredefines(Reader->getSuggestedPredefines());
    return Reader;

  case ASTReader::Failure:
  case ASTReader::Missing:
  case ASTReader::OutOfDate:
  case ASTReader::VersionMismatch:
  case ASTReader::ConfigurationMismatch:
  case ASTReader::HadErrors:
    break;
  }
  return nullptr;
}

IntrusiveRefCntPtr<ExternalSemaSource>
language::Core::createChainedIncludesSource(CompilerInstance &CI,
                                   IntrusiveRefCntPtr<ASTReader> &OutReader) {

  std::vector<std::string> &includes = CI.getPreprocessorOpts().ChainedIncludes;
  assert(!includes.empty() && "No '-chain-include' in options!");

  std::vector<std::unique_ptr<CompilerInstance>> CIs;
  InputKind IK = CI.getFrontendOpts().Inputs[0].getKind();

  SmallVector<std::unique_ptr<toolchain::MemoryBuffer>, 4> SerialBufs;
  SmallVector<std::string, 4> serialBufNames;

  for (unsigned i = 0, e = includes.size(); i != e; ++i) {
    bool firstInclude = (i == 0);
    std::unique_ptr<CompilerInvocation> CInvok;
    CInvok.reset(new CompilerInvocation(CI.getInvocation()));

    CInvok->getPreprocessorOpts().ChainedIncludes.clear();
    CInvok->getPreprocessorOpts().ImplicitPCHInclude.clear();
    CInvok->getPreprocessorOpts().DisablePCHOrModuleValidation =
        DisableValidationForModuleKind::PCH;
    CInvok->getPreprocessorOpts().Includes.clear();
    CInvok->getPreprocessorOpts().MacroIncludes.clear();
    CInvok->getPreprocessorOpts().Macros.clear();

    CInvok->getFrontendOpts().Inputs.clear();
    FrontendInputFile InputFile(includes[i], IK);
    CInvok->getFrontendOpts().Inputs.push_back(InputFile);

    TextDiagnosticPrinter *DiagClient =
        new TextDiagnosticPrinter(toolchain::errs(), CI.getDiagnosticOpts());
    auto Diags = toolchain::makeIntrusiveRefCnt<DiagnosticsEngine>(
        DiagnosticIDs::create(), CI.getDiagnosticOpts(), DiagClient);

    auto Clang = std::make_unique<CompilerInstance>(
        std::move(CInvok), CI.getPCHContainerOperations());
    Clang->setDiagnostics(Diags);
    Clang->setTarget(TargetInfo::CreateTargetInfo(
        Clang->getDiagnostics(), Clang->getInvocation().getTargetOpts()));
    Clang->createFileManager();
    Clang->createSourceManager(Clang->getFileManager());
    Clang->createPreprocessor(TU_Prefix);
    Clang->getDiagnosticClient().BeginSourceFile(Clang->getLangOpts(),
                                                 &Clang->getPreprocessor());
    Clang->createASTContext();

    auto Buffer = std::make_shared<PCHBuffer>();
    ArrayRef<std::shared_ptr<ModuleFileExtension>> Extensions;
    auto consumer = std::make_unique<PCHGenerator>(
        Clang->getPreprocessor(), Clang->getModuleCache(), "-", /*isysroot=*/"",
        Buffer, Clang->getCodeGenOpts(), Extensions,
        /*AllowASTWithErrors=*/true);
    Clang->getASTContext().setASTMutationListener(
                                            consumer->GetASTMutationListener());
    Clang->setASTConsumer(std::move(consumer));
    Clang->createSema(TU_Prefix, nullptr);

    if (firstInclude) {
      Preprocessor &PP = Clang->getPreprocessor();
      PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                             PP.getLangOpts());
    } else {
      assert(!SerialBufs.empty());
      SmallVector<std::unique_ptr<toolchain::MemoryBuffer>, 4> Bufs;
      // TODO: Pass through the existing MemoryBuffer instances instead of
      // allocating new ones.
      for (auto &SB : SerialBufs)
        Bufs.push_back(toolchain::MemoryBuffer::getMemBuffer(SB->getBuffer()));
      std::string pchName = includes[i-1];
      toolchain::raw_string_ostream os(pchName);
      os << ".pch" << i-1;
      serialBufNames.push_back(pchName);

      IntrusiveRefCntPtr<ASTReader> Reader;
      Reader = createASTReader(
          *Clang, pchName, Bufs, serialBufNames,
          Clang->getASTConsumer().GetASTDeserializationListener());
      if (!Reader)
        return nullptr;
      Clang->setASTReader(Reader);
      Clang->getASTContext().setExternalSource(Reader);
    }

    if (!Clang->InitializeSourceManager(InputFile))
      return nullptr;

    ParseAST(Clang->getSema());
    Clang->getDiagnosticClient().EndSourceFile();
    assert(Buffer->IsComplete && "serialization did not complete");
    auto &serialAST = Buffer->Data;
    SerialBufs.push_back(toolchain::MemoryBuffer::getMemBufferCopy(
        StringRef(serialAST.data(), serialAST.size())));
    serialAST.clear();
    CIs.push_back(std::move(Clang));
  }

  assert(!SerialBufs.empty());
  std::string pchName = includes.back() + ".pch-final";
  serialBufNames.push_back(pchName);
  OutReader = createASTReader(CI, pchName, SerialBufs, serialBufNames);
  if (!OutReader)
    return nullptr;

  auto ChainedSrc =
      toolchain::makeIntrusiveRefCnt<ChainedIncludesSource>(std::move(CIs));
  return toolchain::makeIntrusiveRefCnt<MultiplexExternalSemaSource>(
      std::move(ChainedSrc), OutReader);
}
