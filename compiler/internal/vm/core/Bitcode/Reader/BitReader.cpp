//===-- BitReader.cpp -----------------------------------------------------===//
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

#include "vm/core-c/BitReader.h"
#include "vm/core-c/Core.h"
#include "vm/core/Bitcode/BitcodeReader.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/MemoryBuffer.h"
#include <cstring>
#include <string>

using namespace vm::core;

/* Builds a module from the bitcode in the specified memory buffer, returning a
   reference to the module via the OutModule parameter. Returns 0 on success.
   Optionally returns a human-readable error message via OutMessage. */
LLVMBool LLVMParseBitcode(LLVMMemoryBufferRef MemBuf, LLVMModuleRef *OutModule,
                          char **OutMessage) {
  return LLVMParseBitcodeInContext(getGlobalContextForCAPI(), MemBuf, OutModule,
                                   OutMessage);
}

LLVMBool LLVMParseBitcode2(LLVMMemoryBufferRef MemBuf,
                           LLVMModuleRef *OutModule) {
  return LLVMParseBitcodeInContext2(getGlobalContextForCAPI(), MemBuf,
                                    OutModule);
}

LLVMBool LLVMParseBitcodeInContext(LLVMContextRef ContextRef,
                                   LLVMMemoryBufferRef MemBuf,
                                   LLVMModuleRef *OutModule,
                                   char **OutMessage) {
  MemoryBufferRef Buf = unwrap(MemBuf)->getMemBufferRef();
  LLVMContext &Ctx = *unwrap(ContextRef);

  Expected<std::unique_ptr<Module>> ModuleOrErr = parseBitcodeFile(Buf, Ctx);
  if (Error Err = ModuleOrErr.takeError()) {
    std::string Message;
    handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) {
      Message = EIB.message();
    });
    if (OutMessage)
      *OutMessage = strdup(Message.c_str());
    *OutModule = wrap((Module *)nullptr);
    return 1;
  }

  *OutModule = wrap(ModuleOrErr.get().release());
  return 0;
}

LLVMBool LLVMParseBitcodeInContext2(LLVMContextRef ContextRef,
                                    LLVMMemoryBufferRef MemBuf,
                                    LLVMModuleRef *OutModule) {
  MemoryBufferRef Buf = unwrap(MemBuf)->getMemBufferRef();
  LLVMContext &Ctx = *unwrap(ContextRef);

  ErrorOr<std::unique_ptr<Module>> ModuleOrErr =
      expectedToErrorOrAndEmitErrors(Ctx, parseBitcodeFile(Buf, Ctx));
  if (ModuleOrErr.getError()) {
    *OutModule = wrap((Module *)nullptr);
    return 1;
  }

  *OutModule = wrap(ModuleOrErr.get().release());
  return 0;
}

/* Reads a module from the specified path, returning via the OutModule parameter
   a module provider which performs lazy deserialization. Returns 0 on success.
   Optionally returns a human-readable error message via OutMessage. */
LLVMBool LLVMGetBitcodeModuleInContext(LLVMContextRef ContextRef,
                                       LLVMMemoryBufferRef MemBuf,
                                       LLVMModuleRef *OutM, char **OutMessage) {
  LLVMContext &Ctx = *unwrap(ContextRef);
  std::unique_ptr<MemoryBuffer> Owner(unwrap(MemBuf));
  Expected<std::unique_ptr<Module>> ModuleOrErr =
      getOwningLazyBitcodeModule(std::move(Owner), Ctx);
  // Release the buffer if we didn't take ownership of it since we never owned
  // it anyway.
  (void)Owner.release();

  if (Error Err = ModuleOrErr.takeError()) {
    std::string Message;
    handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) {
      Message = EIB.message();
    });
    if (OutMessage)
      *OutMessage = strdup(Message.c_str());
    *OutM = wrap((Module *)nullptr);
    return 1;
  }

  *OutM = wrap(ModuleOrErr.get().release());

  return 0;
}

LLVMBool LLVMGetBitcodeModuleInContext2(LLVMContextRef ContextRef,
                                        LLVMMemoryBufferRef MemBuf,
                                        LLVMModuleRef *OutM) {
  LLVMContext &Ctx = *unwrap(ContextRef);
  std::unique_ptr<MemoryBuffer> Owner(unwrap(MemBuf));

  ErrorOr<std::unique_ptr<Module>> ModuleOrErr = expectedToErrorOrAndEmitErrors(
      Ctx, getOwningLazyBitcodeModule(std::move(Owner), Ctx));
  Owner.release();

  if (ModuleOrErr.getError()) {
    *OutM = wrap((Module *)nullptr);
    return 1;
  }

  *OutM = wrap(ModuleOrErr.get().release());
  return 0;
}

LLVMBool LLVMGetBitcodeModule(LLVMMemoryBufferRef MemBuf, LLVMModuleRef *OutM,
                              char **OutMessage) {
  return LLVMGetBitcodeModuleInContext(getGlobalContextForCAPI(), MemBuf, OutM,
                                       OutMessage);
}

LLVMBool LLVMGetBitcodeModule2(LLVMMemoryBufferRef MemBuf,
                               LLVMModuleRef *OutM) {
  return LLVMGetBitcodeModuleInContext2(getGlobalContextForCAPI(), MemBuf,
                                        OutM);
}
