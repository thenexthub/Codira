//===----------------- Wasm.cpp - Wasm Interpreter --------------*- C++ -*-===//
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
// This file implements interpreter support for code execution in WebAssembly.
//
//===----------------------------------------------------------------------===//

#include "Wasm.h"
#include "IncrementalExecutor.h"

#include <toolchain/IR/LegacyPassManager.h>
#include <toolchain/IR/Module.h>
#include <toolchain/MC/TargetRegistry.h>
#include <toolchain/Target/TargetMachine.h>

#include <clang/Interpreter/Interpreter.h>

#include <string>

namespace lld {
enum Flavor {
  Invalid,
  Gnu,     // -flavor gnu
  MinGW,   // -flavor gnu MinGW
  WinLink, // -flavor link
  Darwin,  // -flavor darwin
  Wasm,    // -flavor wasm
};

using Driver = bool (*)(toolchain::ArrayRef<const char *>, toolchain::raw_ostream &,
                        toolchain::raw_ostream &, bool, bool);

struct DriverDef {
  Flavor f;
  Driver d;
};

struct Result {
  int retCode;
  bool canRunAgain;
};

Result lldMain(toolchain::ArrayRef<const char *> args, toolchain::raw_ostream &stdoutOS,
               toolchain::raw_ostream &stderrOS, toolchain::ArrayRef<DriverDef> drivers);

namespace wasm {
bool link(toolchain::ArrayRef<const char *> args, toolchain::raw_ostream &stdoutOS,
          toolchain::raw_ostream &stderrOS, bool exitEarly, bool disableOutput);
} // namespace wasm
} // namespace lld

#include <dlfcn.h>

namespace language::Core {

WasmIncrementalExecutor::WasmIncrementalExecutor(
    toolchain::orc::ThreadSafeContext &TSC)
    : IncrementalExecutor(TSC) {}

toolchain::Error WasmIncrementalExecutor::addModule(PartialTranslationUnit &PTU) {
  std::string ErrorString;

  const toolchain::Target *Target = toolchain::TargetRegistry::lookupTarget(
      PTU.TheModule->getTargetTriple(), ErrorString);
  if (!Target) {
    return toolchain::make_error<toolchain::StringError>("Failed to create Wasm Target: ",
                                               toolchain::inconvertibleErrorCode());
  }

  toolchain::TargetOptions TO = toolchain::TargetOptions();
  toolchain::TargetMachine *TargetMachine = Target->createTargetMachine(
      PTU.TheModule->getTargetTriple(), "", "", TO, toolchain::Reloc::Model::PIC_);
  PTU.TheModule->setDataLayout(TargetMachine->createDataLayout());
  std::string ObjectFileName = PTU.TheModule->getName().str() + ".o";
  std::string BinaryFileName = PTU.TheModule->getName().str() + ".wasm";

  std::error_code Error;
  toolchain::raw_fd_ostream ObjectFileOutput(toolchain::StringRef(ObjectFileName), Error);

  toolchain::legacy::PassManager PM;
  if (TargetMachine->addPassesToEmitFile(PM, ObjectFileOutput, nullptr,
                                         toolchain::CodeGenFileType::ObjectFile)) {
    return toolchain::make_error<toolchain::StringError>(
        "Wasm backend cannot produce object.", toolchain::inconvertibleErrorCode());
  }

  if (!PM.run(*PTU.TheModule)) {

    return toolchain::make_error<toolchain::StringError>("Failed to emit Wasm object.",
                                               toolchain::inconvertibleErrorCode());
  }

  ObjectFileOutput.close();

  std::vector<const char *> LinkerArgs = {"wasm-ld",
                                          "-shared",
                                          "--import-memory",
                                          "--experimental-pic",
                                          "--stack-first",
                                          "--allow-undefined",
                                          ObjectFileName.c_str(),
                                          "-o",
                                          BinaryFileName.c_str()};

  const lld::DriverDef WasmDriver = {lld::Flavor::Wasm, &lld::wasm::link};
  std::vector<lld::DriverDef> WasmDriverArgs;
  WasmDriverArgs.push_back(WasmDriver);
  lld::Result Result =
      lld::lldMain(LinkerArgs, toolchain::outs(), toolchain::errs(), WasmDriverArgs);

  if (Result.retCode)
    return toolchain::make_error<toolchain::StringError>(
        "Failed to link incremental module", toolchain::inconvertibleErrorCode());

  void *LoadedLibModule =
      dlopen(BinaryFileName.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (LoadedLibModule == nullptr) {
    toolchain::errs() << dlerror() << '\n';
    return toolchain::make_error<toolchain::StringError>(
        "Failed to load incremental module", toolchain::inconvertibleErrorCode());
  }

  return toolchain::Error::success();
}

toolchain::Error WasmIncrementalExecutor::removeModule(PartialTranslationUnit &PTU) {
  return toolchain::make_error<toolchain::StringError>("Not implemented yet",
                                             toolchain::inconvertibleErrorCode());
}

toolchain::Error WasmIncrementalExecutor::runCtors() const {
  // This seems to be automatically done when using dlopen()
  return toolchain::Error::success();
}

toolchain::Error WasmIncrementalExecutor::cleanUp() {
  // Can't call cleanUp through IncrementalExecutor as it
  // tries to deinitialize JIT which hasn't been initialized
  return toolchain::Error::success();
}

toolchain::Expected<toolchain::orc::ExecutorAddr>
WasmIncrementalExecutor::getSymbolAddress(toolchain::StringRef Name,
                                          SymbolNameKind NameKind) const {
  void *Sym = dlsym(RTLD_DEFAULT, Name.str().c_str());
  if (!Sym) {
    return toolchain::make_error<toolchain::StringError>("dlsym failed for symbol: " +
                                                   Name.str(),
                                               toolchain::inconvertibleErrorCode());
  }

  return toolchain::orc::ExecutorAddr::fromPtr(Sym);
}

WasmIncrementalExecutor::~WasmIncrementalExecutor() = default;

} // namespace language::Core
