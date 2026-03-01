//===- ExecutionEngine.cpp - C API for MLIR JIT ---------------------------===//
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

#include "mlir-c/ExecutionEngine.h"
#include "mlir/CAPI/ExecutionEngine.h"
#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Support.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/OpenMP/OpenMPToLLVMIRTranslation.h"
#include "vm/core/ExecutionEngine/Orc/Mangling.h"
#include "vm/core/Support/TargetSelect.h"

using namespace mlir;

extern "C" MlirExecutionEngine
mlirExecutionEngineCreate(MlirModule op, int optLevel, int numPaths,
                          const MlirStringRef *sharedLibPaths,
                          bool enableObjectDump, bool enablePIC) {
  static bool initOnce = [] {
    toolchain::InitializeNativeTarget();
    toolchain::InitializeNativeTargetAsmParser(); // needed for inline_asm
    toolchain::InitializeNativeTargetAsmPrinter();
    return true;
  }();
  (void)initOnce;

  auto &ctx = *unwrap(op)->getContext();
  mlir::registerBuiltinDialectTranslation(ctx);
  mlir::registerLLVMDialectTranslation(ctx);
  mlir::registerOpenMPDialectTranslation(ctx);

  auto tmBuilderOrError = toolchain::orc::JITTargetMachineBuilder::detectHost();
  if (!tmBuilderOrError) {
    consumeError(tmBuilderOrError.takeError());
    return MlirExecutionEngine{nullptr};
  }
  if (enablePIC)
    tmBuilderOrError->setRelocationModel(toolchain::Reloc::PIC_);
  auto tmOrError = tmBuilderOrError->createTargetMachine();
  if (!tmOrError) {
    consumeError(tmOrError.takeError());
    return MlirExecutionEngine{nullptr};
  }

  SmallVector<StringRef> libPaths;
  for (unsigned i = 0; i < static_cast<unsigned>(numPaths); ++i)
    libPaths.push_back(sharedLibPaths[i].data);

  // Create a transformer to run all LLVM optimization passes at the
  // specified optimization level.
  auto transformer = mlir::makeOptimizingTransformer(
      optLevel, /*sizeLevel=*/0, /*targetMachine=*/tmOrError->get());
  ExecutionEngineOptions jitOptions;
  jitOptions.transformer = transformer;
  jitOptions.jitCodeGenOptLevel = static_cast<toolchain::CodeGenOptLevel>(optLevel);
  jitOptions.sharedLibPaths = libPaths;
  jitOptions.enableObjectDump = enableObjectDump;
  auto jitOrError = ExecutionEngine::create(unwrap(op), jitOptions,
                                            std::move(tmOrError.get()));
  if (!jitOrError) {
    consumeError(jitOrError.takeError());
    return MlirExecutionEngine{nullptr};
  }
  return wrap(jitOrError->release());
}

extern "C" void mlirExecutionEngineInitialize(MlirExecutionEngine jit) {
  unwrap(jit)->initialize();
}

extern "C" void mlirExecutionEngineDestroy(MlirExecutionEngine jit) {
  delete (unwrap(jit));
}

extern "C" MlirLogicalResult
mlirExecutionEngineInvokePacked(MlirExecutionEngine jit, MlirStringRef name,
                                void **arguments) {
  const std::string ifaceName = ("_mlir_ciface_" + unwrap(name)).str();
  toolchain::Error error = unwrap(jit)->invokePacked(
      ifaceName, MutableArrayRef<void *>{arguments, (size_t)0});
  if (error)
    return wrap(failure());
  return wrap(success());
}

extern "C" void *mlirExecutionEngineLookupPacked(MlirExecutionEngine jit,
                                                 MlirStringRef name) {
  auto optionalFPtr =
      toolchain::expectedToOptional(unwrap(jit)->lookupPacked(unwrap(name)));
  if (!optionalFPtr)
    return nullptr;
  return reinterpret_cast<void *>(*optionalFPtr);
}

extern "C" void *mlirExecutionEngineLookup(MlirExecutionEngine jit,
                                           MlirStringRef name) {
  auto optionalFPtr =
      toolchain::expectedToOptional(unwrap(jit)->lookup(unwrap(name)));
  if (!optionalFPtr)
    return nullptr;
  return *optionalFPtr;
}

extern "C" void mlirExecutionEngineRegisterSymbol(MlirExecutionEngine jit,
                                                  MlirStringRef name,
                                                  void *sym) {
  unwrap(jit)->registerSymbols([&](toolchain::orc::MangleAndInterner interner) {
    toolchain::orc::SymbolMap symbolMap;
    symbolMap[interner(unwrap(name))] = {toolchain::orc::ExecutorAddr::fromPtr(sym),
                                         toolchain::JITSymbolFlags::Exported};
    return symbolMap;
  });
}

extern "C" void mlirExecutionEngineDumpToObjectFile(MlirExecutionEngine jit,
                                                    MlirStringRef name) {
  unwrap(jit)->dumpToObjectFile(unwrap(name));
}
