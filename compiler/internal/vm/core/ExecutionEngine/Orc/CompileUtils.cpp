//===------ CompileUtils.cpp - Utilities for compiling IR in the JIT ------===//
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

#include "vm/core/ExecutionEngine/Orc/CompileUtils.h"

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ExecutionEngine/ObjectCache.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/SmallVectorMemoryBuffer.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
namespace orc {

IRSymbolMapper::ManglingOptions
irManglingOptionsFromTargetOptions(const TargetOptions &Opts) {
  IRSymbolMapper::ManglingOptions MO;

  MO.EmulatedTLS = Opts.EmulatedTLS;

  return MO;
}

/// Compile a Module to an ObjectFile.
Expected<SimpleCompiler::CompileResult> SimpleCompiler::operator()(Module &M) {
  if (M.getDataLayout().isDefault())
    M.setDataLayout(TM.createDataLayout());

  CompileResult CachedObject = tryToLoadFromObjectCache(M);
  if (CachedObject)
    return std::move(CachedObject);

  SmallVector<char, 0> ObjBufferSV;

  {
    raw_svector_ostream ObjStream(ObjBufferSV);

    legacy::PassManager PM;
    MCContext *Ctx;
    if (TM.addPassesToEmitMC(PM, Ctx, ObjStream))
      return make_error<StringError>("Target does not support MC emission",
                                     inconvertibleErrorCode());
    PM.run(M);
  }

  auto ObjBuffer = std::make_unique<SmallVectorMemoryBuffer>(
      std::move(ObjBufferSV), M.getModuleIdentifier() + "-jitted-objectbuffer",
      /*RequiresNullTerminator=*/false);

  auto Obj = object::ObjectFile::createObjectFile(ObjBuffer->getMemBufferRef());

  if (!Obj)
    return Obj.takeError();

  notifyObjectCompiled(M, *ObjBuffer);
  return std::move(ObjBuffer);
}

SimpleCompiler::CompileResult
SimpleCompiler::tryToLoadFromObjectCache(const Module &M) {
  if (!ObjCache)
    return CompileResult();

  return ObjCache->getObject(&M);
}

void SimpleCompiler::notifyObjectCompiled(const Module &M,
                                          const MemoryBuffer &ObjBuffer) {
  if (ObjCache)
    ObjCache->notifyObjectCompiled(&M, ObjBuffer.getMemBufferRef());
}

ConcurrentIRCompiler::ConcurrentIRCompiler(JITTargetMachineBuilder JTMB,
                                           ObjectCache *ObjCache)
    : IRCompiler(irManglingOptionsFromTargetOptions(JTMB.getOptions())),
      JTMB(std::move(JTMB)), ObjCache(ObjCache) {}

Expected<std::unique_ptr<MemoryBuffer>>
ConcurrentIRCompiler::operator()(Module &M) {
  auto TM = cantFail(JTMB.createTargetMachine());
  SimpleCompiler C(*TM, ObjCache);
  return C(M);
}

} // end namespace orc
} // end namespace vm::core
