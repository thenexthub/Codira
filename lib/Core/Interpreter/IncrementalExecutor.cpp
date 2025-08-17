//===--- IncrementalExecutor.cpp - Incremental Execution --------*- C++ -*-===//
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
// This file implements the class which performs incremental code execution.
//
//===----------------------------------------------------------------------===//

#include "IncrementalExecutor.h"

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "language/Core/Interpreter/PartialTranslationUnit.h"
#include "toolchain/ExecutionEngine/ExecutionEngine.h"
#include "toolchain/ExecutionEngine/Orc/CompileUtils.h"
#include "toolchain/ExecutionEngine/Orc/Debugging/DebuggerSupport.h"
#include "toolchain/ExecutionEngine/Orc/ExecutionUtils.h"
#include "toolchain/ExecutionEngine/Orc/IRCompileLayer.h"
#include "toolchain/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "toolchain/ExecutionEngine/Orc/LLJIT.h"
#include "toolchain/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "toolchain/ExecutionEngine/Orc/TargetProcess/JITLoaderGDB.h"
#include "toolchain/ExecutionEngine/SectionMemoryManager.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/TargetSelect.h"

// Force linking some of the runtimes that helps attaching to a debugger.
LLVM_ATTRIBUTE_USED void linkComponents() {
  toolchain::errs() << (void *)&toolchain_orc_registerJITLoaderGDBWrapper
               << (void *)&toolchain_orc_registerJITLoaderGDBAllocAction;
}

namespace language::Core {
IncrementalExecutor::IncrementalExecutor(toolchain::orc::ThreadSafeContext &TSC)
    : TSCtx(TSC) {}

toolchain::Expected<std::unique_ptr<toolchain::orc::LLJITBuilder>>
IncrementalExecutor::createDefaultJITBuilder(
    toolchain::orc::JITTargetMachineBuilder JTMB) {
  auto JITBuilder = std::make_unique<toolchain::orc::LLJITBuilder>();
  JITBuilder->setJITTargetMachineBuilder(std::move(JTMB));
  JITBuilder->setPrePlatformSetup([](toolchain::orc::LLJIT &J) {
    // Try to enable debugging of JIT'd code (only works with JITLink for
    // ELF and MachO).
    consumeError(toolchain::orc::enableDebuggerSupport(J));
    return toolchain::Error::success();
  });
  return std::move(JITBuilder);
}

IncrementalExecutor::IncrementalExecutor(toolchain::orc::ThreadSafeContext &TSC,
                                         toolchain::orc::LLJITBuilder &JITBuilder,
                                         toolchain::Error &Err)
    : TSCtx(TSC) {
  using namespace toolchain::orc;
  toolchain::ErrorAsOutParameter EAO(&Err);

  if (auto JitOrErr = JITBuilder.create())
    Jit = std::move(*JitOrErr);
  else {
    Err = JitOrErr.takeError();
    return;
  }
}

IncrementalExecutor::~IncrementalExecutor() {}

toolchain::Error IncrementalExecutor::addModule(PartialTranslationUnit &PTU) {
  toolchain::orc::ResourceTrackerSP RT =
      Jit->getMainJITDylib().createResourceTracker();
  ResourceTrackers[&PTU] = RT;

  return Jit->addIRModule(RT, {std::move(PTU.TheModule), TSCtx});
}

toolchain::Error IncrementalExecutor::removeModule(PartialTranslationUnit &PTU) {

  toolchain::orc::ResourceTrackerSP RT = std::move(ResourceTrackers[&PTU]);
  if (!RT)
    return toolchain::Error::success();

  ResourceTrackers.erase(&PTU);
  if (toolchain::Error Err = RT->remove())
    return Err;
  return toolchain::Error::success();
}

// Clean up the JIT instance.
toolchain::Error IncrementalExecutor::cleanUp() {
  // This calls the global dtors of registered modules.
  return Jit->deinitialize(Jit->getMainJITDylib());
}

toolchain::Error IncrementalExecutor::runCtors() const {
  return Jit->initialize(Jit->getMainJITDylib());
}

toolchain::Expected<toolchain::orc::ExecutorAddr>
IncrementalExecutor::getSymbolAddress(toolchain::StringRef Name,
                                      SymbolNameKind NameKind) const {
  using namespace toolchain::orc;
  auto SO = makeJITDylibSearchOrder({&Jit->getMainJITDylib(),
                                     Jit->getPlatformJITDylib().get(),
                                     Jit->getProcessSymbolsJITDylib().get()});

  ExecutionSession &ES = Jit->getExecutionSession();

  auto SymOrErr =
      ES.lookup(SO, (NameKind == LinkerName) ? ES.intern(Name)
                                             : Jit->mangleAndIntern(Name));
  if (auto Err = SymOrErr.takeError())
    return std::move(Err);
  return SymOrErr->getAddress();
}

} // namespace language::Core
