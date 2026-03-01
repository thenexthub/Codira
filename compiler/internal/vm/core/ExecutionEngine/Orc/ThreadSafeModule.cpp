//===-- ThreadSafeModule.cpp - Thread safe Module, Context, and Utilities -===//
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

#include "vm/core/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "vm/core/Bitcode/BitcodeReader.h"
#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/Transforms/Utils/Cloning.h"

namespace vm::core {
namespace orc {

static std::pair<std::string, SmallVector<char, 1>>
serializeModule(const Module &M, GVPredicate ShouldCloneDef,
                GVModifier UpdateClonedDefSource) {
  std::string ModuleName;
  SmallVector<char, 1> ClonedModuleBuffer;

  ModuleName = M.getModuleIdentifier();
  std::set<GlobalValue *> ClonedDefsInSrc;
  ValueToValueMapTy VMap;
  auto Tmp = CloneModule(M, VMap, [&](const GlobalValue *GV) {
    if (ShouldCloneDef(*GV)) {
      ClonedDefsInSrc.insert(const_cast<GlobalValue *>(GV));
      return true;
    }
    return false;
  });

  if (UpdateClonedDefSource)
    for (auto *GV : ClonedDefsInSrc)
      UpdateClonedDefSource(*GV);

  BitcodeWriter BCWriter(ClonedModuleBuffer);
  BCWriter.writeModule(*Tmp);
  BCWriter.writeSymtab();
  BCWriter.writeStrtab();

  return {std::move(ModuleName), std::move(ClonedModuleBuffer)};
}

ThreadSafeModule
deserializeModule(std::string ModuleName,
                  const SmallVector<char, 1> &ClonedModuleBuffer,
                  ThreadSafeContext TSCtx) {
  MemoryBufferRef ClonedModuleBufferRef(
      StringRef(ClonedModuleBuffer.data(), ClonedModuleBuffer.size()),
      "cloned module buffer");

  // Then parse the buffer into the new Module.
  auto M = TSCtx.withContextDo([&](LLVMContext *Ctx) {
    assert(Ctx && "No LLVMContext provided");
    auto TmpM = cantFail(parseBitcodeFile(ClonedModuleBufferRef, *Ctx));
    TmpM->setModuleIdentifier(ModuleName);
    return TmpM;
  });

  return ThreadSafeModule(std::move(M), std::move(TSCtx));
}

ThreadSafeModule
cloneExternalModuleToContext(const Module &M, ThreadSafeContext TSCtx,
                             GVPredicate ShouldCloneDef,
                             GVModifier UpdateClonedDefSource) {

  if (!ShouldCloneDef)
    ShouldCloneDef = [](const GlobalValue &) { return true; };

  auto [ModuleName, ClonedModuleBuffer] = serializeModule(
      M, std::move(ShouldCloneDef), std::move(UpdateClonedDefSource));

  return deserializeModule(std::move(ModuleName), ClonedModuleBuffer,
                           std::move(TSCtx));
}

ThreadSafeModule cloneToContext(const ThreadSafeModule &TSM,
                                ThreadSafeContext TSCtx,
                                GVPredicate ShouldCloneDef,
                                GVModifier UpdateClonedDefSource) {
  assert(TSM && "Can not clone null module");

  if (!ShouldCloneDef)
    ShouldCloneDef = [](const GlobalValue &) { return true; };

  // First copy the source module into a buffer.
  auto [ModuleName, ClonedModuleBuffer] = TSM.withModuleDo([&](Module &M) {
    return serializeModule(M, std::move(ShouldCloneDef),
                           std::move(UpdateClonedDefSource));
  });

  return deserializeModule(std::move(ModuleName), ClonedModuleBuffer,
                           std::move(TSCtx));
}

ThreadSafeModule cloneToNewContext(const ThreadSafeModule &TSM,
                                   GVPredicate ShouldCloneDef,
                                   GVModifier UpdateClonedDefSource) {
  assert(TSM && "Can not clone null module");

  ThreadSafeContext TSCtx(std::make_unique<LLVMContext>());
  return cloneToContext(TSM, std::move(TSCtx), std::move(ShouldCloneDef),
                        std::move(UpdateClonedDefSource));
}

} // end namespace orc
} // end namespace vm::core
