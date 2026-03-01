//===- Parser.cpp - Main dispatch module for the Parser library -----------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This library implements the functionality defined in toolchain/AsmParser/Parser.h
//
//===----------------------------------------------------------------------===//

#include "vm/core/AsmParser/Parser.h"
#include "vm/core/AsmParser/LLParser.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/ModuleSummaryIndex.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/SourceMgr.h"
#include <system_error>

using namespace vm::core;

static bool parseAssemblyInto(MemoryBufferRef F, Module *M,
                              ModuleSummaryIndex *Index, SMDiagnostic &Err,
                              SlotMapping *Slots, bool UpgradeDebugInfo,
                              DataLayoutCallbackTy DataLayoutCallback,
                              AsmParserContext *ParserContext = nullptr) {
  SourceMgr SM;
  std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(F);
  SM.AddNewSourceBuffer(std::move(Buf), SMLoc());

  std::optional<LLVMContext> OptContext;
  return LLParser(F.getBuffer(), SM, Err, M, Index,
                  M ? M->getContext() : OptContext.emplace(), Slots,
                  ParserContext)
      .Run(UpgradeDebugInfo, DataLayoutCallback);
}

bool toolchain::parseAssemblyInto(MemoryBufferRef F, Module *M,
                             ModuleSummaryIndex *Index, SMDiagnostic &Err,
                             SlotMapping *Slots,
                             DataLayoutCallbackTy DataLayoutCallback,
                             AsmParserContext *ParserContext) {
  return ::parseAssemblyInto(F, M, Index, Err, Slots,
                             /*UpgradeDebugInfo*/ true, DataLayoutCallback,
                             ParserContext);
}

std::unique_ptr<Module>
toolchain::parseAssembly(MemoryBufferRef F, SMDiagnostic &Err, LLVMContext &Context,
                    SlotMapping *Slots, DataLayoutCallbackTy DataLayoutCallback,
                    AsmParserContext *ParserContext) {
  std::unique_ptr<Module> M =
      std::make_unique<Module>(F.getBufferIdentifier(), Context);

  if (parseAssemblyInto(F, M.get(), nullptr, Err, Slots, DataLayoutCallback,
                        ParserContext))
    return nullptr;

  return M;
}

std::unique_ptr<Module> toolchain::parseAssemblyFile(StringRef Filename,
                                                SMDiagnostic &Err,
                                                LLVMContext &Context,
                                                SlotMapping *Slots) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename);
  if (std::error_code EC = FileOrErr.getError()) {
    Err = SMDiagnostic(Filename, SourceMgr::DK_Error,
                       "Could not open input file: " + EC.message());
    return nullptr;
  }

  return parseAssembly(FileOrErr.get()->getMemBufferRef(), Err, Context, Slots);
}

static ParsedModuleAndIndex
parseAssemblyWithIndex(MemoryBufferRef F, SMDiagnostic &Err,
                       LLVMContext &Context, SlotMapping *Slots,
                       bool UpgradeDebugInfo,
                       DataLayoutCallbackTy DataLayoutCallback) {
  std::unique_ptr<Module> M =
      std::make_unique<Module>(F.getBufferIdentifier(), Context);
  std::unique_ptr<ModuleSummaryIndex> Index =
      std::make_unique<ModuleSummaryIndex>(/*HaveGVs=*/true);

  if (parseAssemblyInto(F, M.get(), Index.get(), Err, Slots, UpgradeDebugInfo,
                        DataLayoutCallback))
    return {nullptr, nullptr};

  return {std::move(M), std::move(Index)};
}

ParsedModuleAndIndex toolchain::parseAssemblyWithIndex(MemoryBufferRef F,
                                                  SMDiagnostic &Err,
                                                  LLVMContext &Context,
                                                  SlotMapping *Slots) {
  return ::parseAssemblyWithIndex(
      F, Err, Context, Slots,
      /*UpgradeDebugInfo*/ true,
      [](StringRef, StringRef) { return std::nullopt; });
}

static ParsedModuleAndIndex
parseAssemblyFileWithIndex(StringRef Filename, SMDiagnostic &Err,
                           LLVMContext &Context, SlotMapping *Slots,
                           bool UpgradeDebugInfo,
                           DataLayoutCallbackTy DataLayoutCallback) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename, /*IsText=*/true);
  if (std::error_code EC = FileOrErr.getError()) {
    Err = SMDiagnostic(Filename, SourceMgr::DK_Error,
                       "Could not open input file: " + EC.message());
    return {nullptr, nullptr};
  }

  return parseAssemblyWithIndex(FileOrErr.get()->getMemBufferRef(), Err,
                                Context, Slots, UpgradeDebugInfo,
                                DataLayoutCallback);
}

ParsedModuleAndIndex
toolchain::parseAssemblyFileWithIndex(StringRef Filename, SMDiagnostic &Err,
                                 LLVMContext &Context, SlotMapping *Slots,
                                 DataLayoutCallbackTy DataLayoutCallback) {
  return ::parseAssemblyFileWithIndex(Filename, Err, Context, Slots,
                                      /*UpgradeDebugInfo*/ true,
                                      DataLayoutCallback);
}

ParsedModuleAndIndex toolchain::parseAssemblyFileWithIndexNoUpgradeDebugInfo(
    StringRef Filename, SMDiagnostic &Err, LLVMContext &Context,
    SlotMapping *Slots, DataLayoutCallbackTy DataLayoutCallback) {
  return ::parseAssemblyFileWithIndex(Filename, Err, Context, Slots,
                                      /*UpgradeDebugInfo*/ false,
                                      DataLayoutCallback);
}

std::unique_ptr<Module>
toolchain::parseAssemblyString(StringRef AsmString, SMDiagnostic &Err,
                          LLVMContext &Context, SlotMapping *Slots,
                          AsmParserContext *ParserContext) {
  MemoryBufferRef F(AsmString, "<string>");
  return parseAssembly(
      F, Err, Context, Slots, [](StringRef, StringRef) { return std::nullopt; },
      ParserContext);
}

static bool parseSummaryIndexAssemblyInto(MemoryBufferRef F,
                                          ModuleSummaryIndex &Index,
                                          SMDiagnostic &Err) {
  SourceMgr SM;
  std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(F);
  SM.AddNewSourceBuffer(std::move(Buf), SMLoc());

  // The parser holds a reference to a context that is unused when parsing the
  // index, but we need to initialize it.
  LLVMContext unusedContext;
  return LLParser(F.getBuffer(), SM, Err, nullptr, &Index, unusedContext)
      .Run(true, [](StringRef, StringRef) { return std::nullopt; });
}

std::unique_ptr<ModuleSummaryIndex>
toolchain::parseSummaryIndexAssembly(MemoryBufferRef F, SMDiagnostic &Err) {
  std::unique_ptr<ModuleSummaryIndex> Index =
      std::make_unique<ModuleSummaryIndex>(/*HaveGVs=*/false);

  if (parseSummaryIndexAssemblyInto(F, *Index, Err))
    return nullptr;

  return Index;
}

std::unique_ptr<ModuleSummaryIndex>
toolchain::parseSummaryIndexAssemblyFile(StringRef Filename, SMDiagnostic &Err) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename);
  if (std::error_code EC = FileOrErr.getError()) {
    Err = SMDiagnostic(Filename, SourceMgr::DK_Error,
                       "Could not open input file: " + EC.message());
    return nullptr;
  }

  return parseSummaryIndexAssembly(FileOrErr.get()->getMemBufferRef(), Err);
}

std::unique_ptr<ModuleSummaryIndex>
toolchain::parseSummaryIndexAssemblyString(StringRef AsmString, SMDiagnostic &Err) {
  MemoryBufferRef F(AsmString, "<string>");
  return parseSummaryIndexAssembly(F, Err);
}

Constant *toolchain::parseConstantValue(StringRef Asm, SMDiagnostic &Err,
                                   const Module &M, const SlotMapping *Slots) {
  SourceMgr SM;
  std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(Asm);
  SM.AddNewSourceBuffer(std::move(Buf), SMLoc());
  Constant *C;
  if (LLParser(Asm, SM, Err, const_cast<Module *>(&M), nullptr, M.getContext())
          .parseStandaloneConstantValue(C, Slots))
    return nullptr;
  return C;
}

Type *toolchain::parseType(StringRef Asm, SMDiagnostic &Err, const Module &M,
                      const SlotMapping *Slots) {
  unsigned Read;
  Type *Ty = parseTypeAtBeginning(Asm, Read, Err, M, Slots);
  if (!Ty)
    return nullptr;
  if (Read != Asm.size()) {
    SourceMgr SM;
    std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(Asm);
    SM.AddNewSourceBuffer(std::move(Buf), SMLoc());
    Err = SM.GetMessage(SMLoc::getFromPointer(Asm.begin() + Read),
                        SourceMgr::DK_Error, "expected end of string");
    return nullptr;
  }
  return Ty;
}
Type *toolchain::parseTypeAtBeginning(StringRef Asm, unsigned &Read,
                                 SMDiagnostic &Err, const Module &M,
                                 const SlotMapping *Slots) {
  SourceMgr SM;
  std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(Asm);
  SM.AddNewSourceBuffer(std::move(Buf), SMLoc());
  Type *Ty;
  if (LLParser(Asm, SM, Err, const_cast<Module *>(&M), nullptr, M.getContext())
          .parseTypeAtBeginning(Ty, Read, Slots))
    return nullptr;
  return Ty;
}

DIExpression *toolchain::parseDIExpressionBodyAtBeginning(StringRef Asm,
                                                     unsigned &Read,
                                                     SMDiagnostic &Err,
                                                     const Module &M,
                                                     const SlotMapping *Slots) {
  SourceMgr SM;
  std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(Asm);
  SM.AddNewSourceBuffer(std::move(Buf), SMLoc());
  MDNode *MD;
  if (LLParser(Asm, SM, Err, const_cast<Module *>(&M), nullptr, M.getContext())
          .parseDIExpressionBodyAtBeginning(MD, Read, Slots))
    return nullptr;
  return dyn_cast<DIExpression>(MD);
}
