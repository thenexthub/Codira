//===---- IRReader.cpp - Reader for LLVM IR files -------------------------===//
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

#include "vm/core/IRReader/IRReader.h"
#include "vm/core-c/IRReader.h"
#include "vm/core/AsmParser/AsmParserContext.h"
#include "vm/core/AsmParser/Parser.h"
#include "vm/core/Bitcode/BitcodeReader.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/Timer.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstring>
#include <optional>
#include <system_error>

using namespace vm::core;

namespace vm::core {
  extern bool TimePassesIsEnabled;
}

const char TimeIRParsingGroupName[] = "irparse";
const char TimeIRParsingGroupDescription[] = "LLVM IR Parsing";
const char TimeIRParsingName[] = "parse";
const char TimeIRParsingDescription[] = "Parse IR";

std::unique_ptr<Module>
toolchain::getLazyIRModule(std::unique_ptr<MemoryBuffer> Buffer, SMDiagnostic &Err,
                      LLVMContext &Context, bool ShouldLazyLoadMetadata) {
  if (isBitcode((const unsigned char *)Buffer->getBufferStart(),
                (const unsigned char *)Buffer->getBufferEnd())) {
    Expected<std::unique_ptr<Module>> ModuleOrErr = getOwningLazyBitcodeModule(
        std::move(Buffer), Context, ShouldLazyLoadMetadata);
    if (Error E = ModuleOrErr.takeError()) {
      handleAllErrors(std::move(E), [&](ErrorInfoBase &EIB) {
        Err = SMDiagnostic(Buffer->getBufferIdentifier(), SourceMgr::DK_Error,
                           EIB.message());
      });
      return nullptr;
    }
    return std::move(ModuleOrErr.get());
  }

  return parseAssembly(Buffer->getMemBufferRef(), Err, Context);
}

std::unique_ptr<Module> toolchain::getLazyIRFileModule(StringRef Filename,
                                                  SMDiagnostic &Err,
                                                  LLVMContext &Context,
                                                  bool ShouldLazyLoadMetadata) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename);
  if (std::error_code EC = FileOrErr.getError()) {
    Err = SMDiagnostic(Filename, SourceMgr::DK_Error,
                       "Could not open input file: " + EC.message());
    return nullptr;
  }

  return getLazyIRModule(std::move(FileOrErr.get()), Err, Context,
                         ShouldLazyLoadMetadata);
}

std::unique_ptr<Module> toolchain::parseIR(MemoryBufferRef Buffer, SMDiagnostic &Err,
                                      LLVMContext &Context,
                                      ParserCallbacks Callbacks,
                                      toolchain::AsmParserContext *ParserContext) {
  NamedRegionTimer T(TimeIRParsingName, TimeIRParsingDescription,
                     TimeIRParsingGroupName, TimeIRParsingGroupDescription,
                     TimePassesIsEnabled);
  if (isBitcode((const unsigned char *)Buffer.getBufferStart(),
                (const unsigned char *)Buffer.getBufferEnd())) {
    Expected<std::unique_ptr<Module>> ModuleOrErr =
        parseBitcodeFile(Buffer, Context, Callbacks);
    if (Error E = ModuleOrErr.takeError()) {
      handleAllErrors(std::move(E), [&](ErrorInfoBase &EIB) {
        Err = SMDiagnostic(Buffer.getBufferIdentifier(), SourceMgr::DK_Error,
                           EIB.message());
      });
      return nullptr;
    }
    return std::move(ModuleOrErr.get());
  }

  return parseAssembly(Buffer, Err, Context, nullptr,
                       Callbacks.DataLayout.value_or(
                           [](StringRef, StringRef) { return std::nullopt; }),
                       ParserContext);
}

std::unique_ptr<Module> toolchain::parseIRFile(StringRef Filename, SMDiagnostic &Err,
                                          LLVMContext &Context,
                                          ParserCallbacks Callbacks,
                                          AsmParserContext *ParserContext) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename, /*IsText=*/true);
  if (std::error_code EC = FileOrErr.getError()) {
    Err = SMDiagnostic(Filename, SourceMgr::DK_Error,
                       "Could not open input file: " + EC.message());
    return nullptr;
  }

  return parseIR(FileOrErr.get()->getMemBufferRef(), Err, Context, Callbacks,
                 ParserContext);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMParseIRInContext(LLVMContextRef ContextRef,
                              LLVMMemoryBufferRef MemBuf, LLVMModuleRef *OutM,
                              char **OutMessage) {
  std::unique_ptr<MemoryBuffer> MB(unwrap(MemBuf));
  return LLVMParseIRInContext2(ContextRef, wrap(MB.get()), OutM, OutMessage);
}

LLVMBool LLVMParseIRInContext2(LLVMContextRef ContextRef,
                               LLVMMemoryBufferRef MemBuf, LLVMModuleRef *OutM,
                               char **OutMessage) {
  SMDiagnostic Diag;

  *OutM = wrap(parseIR(*unwrap(MemBuf), Diag, *unwrap(ContextRef)).release());

  if (*OutM)
    return 0;

  if (OutMessage) {
    std::string Buf;
    raw_string_ostream OS(Buf);
    Diag.print(nullptr, OS, /*ShowColors=*/false);
    *OutMessage = strdup(Buf.c_str());
  }

  return 1;
}
