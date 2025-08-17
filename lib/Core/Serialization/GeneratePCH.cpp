//===--- GeneratePCH.cpp - Sema Consumer for PCH Generation -----*- C++ -*-===//
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
//  This file defines the PCHGenerator, which as a SemaConsumer that generates
//  a PCH file.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTContext.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Lex/HeaderSearch.h"
#include "language/Core/Lex/HeaderSearchOptions.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Sema/SemaConsumer.h"
#include "language/Core/Serialization/ASTWriter.h"
#include "toolchain/Bitstream/BitstreamWriter.h"

using namespace language::Core;

PCHGenerator::PCHGenerator(
    Preprocessor &PP, ModuleCache &ModCache, StringRef OutputFile,
    StringRef isysroot, std::shared_ptr<PCHBuffer> Buffer,
    const CodeGenOptions &CodeGenOpts,
    ArrayRef<std::shared_ptr<ModuleFileExtension>> Extensions,
    bool AllowASTWithErrors, bool IncludeTimestamps,
    bool BuildingImplicitModule, bool ShouldCacheASTInMemory,
    bool GeneratingReducedBMI)
    : PP(PP), Subject(&PP), OutputFile(OutputFile), isysroot(isysroot.str()),
      Buffer(std::move(Buffer)), Stream(this->Buffer->Data),
      Writer(Stream, this->Buffer->Data, ModCache, CodeGenOpts, Extensions,
             IncludeTimestamps, BuildingImplicitModule, GeneratingReducedBMI),
      AllowASTWithErrors(AllowASTWithErrors),
      ShouldCacheASTInMemory(ShouldCacheASTInMemory) {
  this->Buffer->IsComplete = false;
}

PCHGenerator::~PCHGenerator() {
}

Module *PCHGenerator::getEmittingModule(ASTContext &) {
  Module *M = nullptr;

  if (PP.getLangOpts().isCompilingModule()) {
    M = PP.getHeaderSearchInfo().lookupModule(PP.getLangOpts().CurrentModule,
                                              SourceLocation(),
                                              /*AllowSearch*/ false);
    if (!M)
      assert(PP.getDiagnostics().hasErrorOccurred() &&
             "emitting module but current module doesn't exist");
  }

  return M;
}

DiagnosticsEngine &PCHGenerator::getDiagnostics() const {
  return PP.getDiagnostics();
}

void PCHGenerator::InitializeSema(Sema &S) {
  if (!PP.getHeaderSearchInfo()
           .getHeaderSearchOpts()
           .ModulesSerializeOnlyPreprocessor)
    Subject = &S;
}

void PCHGenerator::HandleTranslationUnit(ASTContext &Ctx) {
  // Don't create a PCH if there were fatal failures during module loading.
  if (PP.getModuleLoader().HadFatalFailure)
    return;

  bool hasErrors = PP.getDiagnostics().hasErrorOccurred();
  if (hasErrors && !AllowASTWithErrors)
    return;

  Module *Module = getEmittingModule(Ctx);

  // Errors that do not prevent the PCH from being written should not cause the
  // overall compilation to fail either.
  if (AllowASTWithErrors)
    PP.getDiagnostics().getClient()->clear();

  Buffer->Signature = Writer.WriteAST(Subject, OutputFile, Module, isysroot,
                                      ShouldCacheASTInMemory);

  Buffer->IsComplete = true;
}

ASTMutationListener *PCHGenerator::GetASTMutationListener() {
  return &Writer;
}

ASTDeserializationListener *PCHGenerator::GetASTDeserializationListener() {
  return &Writer;
}

void PCHGenerator::anchor() {}

CXX20ModulesGenerator::CXX20ModulesGenerator(Preprocessor &PP,
                                             ModuleCache &ModCache,
                                             StringRef OutputFile,
                                             const CodeGenOptions &CodeGenOpts,
                                             bool GeneratingReducedBMI,
                                             bool AllowASTWithErrors)
    : PCHGenerator(
          PP, ModCache, OutputFile, toolchain::StringRef(),
          std::make_shared<PCHBuffer>(), CodeGenOpts,
          /*Extensions=*/ArrayRef<std::shared_ptr<ModuleFileExtension>>(),
          AllowASTWithErrors, /*IncludeTimestamps=*/false,
          /*BuildingImplicitModule=*/false, /*ShouldCacheASTInMemory=*/false,
          GeneratingReducedBMI) {}

Module *CXX20ModulesGenerator::getEmittingModule(ASTContext &Ctx) {
  Module *M = Ctx.getCurrentNamedModule();
  assert(M && M->isNamedModuleUnit() &&
         "CXX20ModulesGenerator should only be used with C++20 Named modules.");
  return M;
}

void CXX20ModulesGenerator::HandleTranslationUnit(ASTContext &Ctx) {
  PCHGenerator::HandleTranslationUnit(Ctx);

  if (!isComplete())
    return;

  std::error_code EC;
  auto OS = std::make_unique<toolchain::raw_fd_ostream>(getOutputFile(), EC);
  if (EC) {
    getDiagnostics().Report(diag::err_fe_unable_to_open_output)
        << getOutputFile() << EC.message() << "\n";
    return;
  }

  *OS << getBufferPtr()->Data;
  OS->flush();
}

void CXX20ModulesGenerator::anchor() {}

void ReducedBMIGenerator::anchor() {}
