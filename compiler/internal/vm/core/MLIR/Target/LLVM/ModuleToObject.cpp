//===- ModuleToObject.cpp - Module to object base class ---------*- C++ -*-===//
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
// This file implements the base class for transforming Operations into binary
// objects.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVM/ModuleToObject.h"

#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/BuiltinAttributeInterfaces.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/IRReader/IRReader.h"
#include "vm/core/Linker/Linker.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Transforms/IPO/Internalize.h"

using namespace mlir;
using namespace mlir::LLVM;

ModuleToObject::ModuleToObject(
    Operation &module, StringRef triple, StringRef chip, StringRef features,
    int optLevel, function_ref<void(toolchain::Module &)> initialLlvmIRCallback,
    function_ref<void(toolchain::Module &)> linkedLlvmIRCallback,
    function_ref<void(toolchain::Module &)> optimizedLlvmIRCallback,
    function_ref<void(StringRef)> isaCallback)
    : module(module), triple(triple), chip(chip), features(features),
      optLevel(optLevel), initialLlvmIRCallback(initialLlvmIRCallback),
      linkedLlvmIRCallback(linkedLlvmIRCallback),
      optimizedLlvmIRCallback(optimizedLlvmIRCallback),
      isaCallback(isaCallback) {}

ModuleToObject::~ModuleToObject() = default;

Operation &ModuleToObject::getOperation() { return module; }

FailureOr<toolchain::TargetMachine *> ModuleToObject::getOrCreateTargetMachine() {
  if (targetMachine)
    return targetMachine.get();
  // Load the target.
  std::string error;
  toolchain::Triple parsedTriple(triple);
  const toolchain::Target *target =
      toolchain::TargetRegistry::lookupTarget(parsedTriple, error);
  if (!target)
    return getOperation().emitError()
           << "Failed to lookup target for triple '" << triple << "' " << error;

  // Create the target machine using the target.
  targetMachine.reset(
      target->createTargetMachine(parsedTriple, chip, features, {}, {}));
  if (!targetMachine)
    return getOperation().emitError()
           << "Failed to create target machine for triple '" << triple << "'";

  return targetMachine.get();
}

std::unique_ptr<toolchain::Module>
ModuleToObject::loadBitcodeFile(toolchain::LLVMContext &context, StringRef path) {
  toolchain::SMDiagnostic error;
  std::unique_ptr<toolchain::Module> library =
      toolchain::getLazyIRFileModule(path, error, context);
  if (!library) {
    getOperation().emitError() << "Failed loading file from " << path
                               << ", error: " << error.getMessage();
    return nullptr;
  }
  if (failed(handleBitcodeFile(*library))) {
    return nullptr;
  }
  return library;
}

LogicalResult ModuleToObject::loadBitcodeFilesFromList(
    toolchain::LLVMContext &context, ArrayRef<Attribute> librariesToLink,
    SmallVector<std::unique_ptr<toolchain::Module>> &llvmModules,
    bool failureOnError) {
  for (Attribute linkLib : librariesToLink) {
    // Attributes in this list can be either list of file paths using
    // StringAttr, or a resource attribute pointing to the LLVM bitcode in
    // memory.
    if (auto filePath = dyn_cast<StringAttr>(linkLib)) {
      // Test if the path exists, if it doesn't abort.
      if (!toolchain::sys::fs::is_regular_file(filePath.strref())) {
        getOperation().emitError()
            << "File path: " << filePath << " does not exist or is not a file.";
        return failure();
      }
      // Load the file or abort on error.
      if (auto bcFile = loadBitcodeFile(context, filePath))
        llvmModules.push_back(std::move(bcFile));
      else if (failureOnError)
        return failure();
      continue;
    }
    if (auto blobAttr = dyn_cast<BlobAttr>(linkLib)) {
      // Load the file or abort on error.
      toolchain::SMDiagnostic error;
      ArrayRef<char> data = blobAttr.getData();
      std::unique_ptr<toolchain::MemoryBuffer> buffer =
          toolchain::MemoryBuffer::getMemBuffer(StringRef(data.data(), data.size()),
                                           "blobLinkedLib",
                                           /*RequiresNullTerminator=*/false);
      std::unique_ptr<toolchain::Module> mod =
          getLazyIRModule(std::move(buffer), error, context);
      if (mod) {
        if (failed(handleBitcodeFile(*mod)))
          return failure();
        llvmModules.push_back(std::move(mod));
      } else if (failureOnError) {
        getOperation().emitError()
            << "Couldn't load LLVM library for linking: " << error.getMessage();
        return failure();
      }
      continue;
    }
    if (failureOnError) {
      getOperation().emitError()
          << "Unknown attribute describing LLVM library to load: " << linkLib;
      return failure();
    }
  }
  return success();
}

std::unique_ptr<toolchain::Module>
ModuleToObject::translateToLLVMIR(toolchain::LLVMContext &llvmContext) {
  return translateModuleToLLVMIR(&getOperation(), llvmContext);
}

LogicalResult
ModuleToObject::linkFiles(toolchain::Module &module,
                          SmallVector<std::unique_ptr<toolchain::Module>> &&libs) {
  if (libs.empty())
    return success();
  toolchain::Linker linker(module);
  for (std::unique_ptr<toolchain::Module> &libModule : libs) {
    // This bitcode linking imports the library functions into the module,
    // allowing LLVM optimization passes (which must run after linking) to
    // optimize across the libraries and the module's code. We also only import
    // symbols if they are referenced by the module or a previous library since
    // there will be no other source of references to those symbols in this
    // compilation and since we don't want to bloat the resulting code object.
    bool err = linker.linkInModule(
        std::move(libModule), toolchain::Linker::Flags::LinkOnlyNeeded,
        [](toolchain::Module &m, const StringSet<> &gvs) {
          toolchain::internalizeModule(m, [&gvs](const toolchain::GlobalValue &gv) {
            return !gv.hasName() || (gvs.count(gv.getName()) == 0);
          });
        });
    // True is linker failure
    if (err) {
      getOperation().emitError("Unrecoverable failure during bitcode linking.");
      // We have no guaranties about the state of `ret`, so bail
      return failure();
    }
  }
  return success();
}

LogicalResult ModuleToObject::optimizeModule(toolchain::Module &module,

                                             int optLevel) {
  if (optLevel < 0 || optLevel > 3)
    return getOperation().emitError()
           << "Invalid optimization level: " << optLevel << ".";

  FailureOr<toolchain::TargetMachine *> targetMachine = getOrCreateTargetMachine();
  if (failed(targetMachine))
    return getOperation().emitError()
           << "Target Machine unavailable for triple " << triple
           << ", can't optimize with LLVM\n";
  (*targetMachine)->setOptLevel(static_cast<toolchain::CodeGenOptLevel>(optLevel));

  auto transformer =
      makeOptimizingTransformer(optLevel, /*sizeLevel=*/0, *targetMachine);
  auto error = transformer(&module);
  if (error) {
    InFlightDiagnostic mlirError = getOperation().emitError();
    toolchain::handleAllErrors(
        std::move(error), [&mlirError](const toolchain::ErrorInfoBase &ei) {
          mlirError << "Could not optimize LLVM IR: " << ei.message() << "\n";
        });
    return mlirError;
  }
  return success();
}

FailureOr<SmallString<0>> ModuleToObject::translateModuleToISA(
    toolchain::Module &llvmModule, toolchain::TargetMachine &targetMachine,
    function_ref<InFlightDiagnostic()> emitError) {
  SmallString<0> targetISA;
  toolchain::raw_svector_ostream stream(targetISA);

  { // Drop pstream after this to prevent the ISA from being stuck buffering
    toolchain::buffer_ostream pstream(stream);
    toolchain::legacy::PassManager codegenPasses;

    if (targetMachine.addPassesToEmitFile(codegenPasses, pstream, nullptr,
                                          toolchain::CodeGenFileType::AssemblyFile))
      return emitError() << "Target machine cannot emit assembly";

    codegenPasses.run(llvmModule);
  }
  return targetISA;
}

void ModuleToObject::setDataLayoutAndTriple(toolchain::Module &module) {
  // Create the target machine.
  FailureOr<toolchain::TargetMachine *> targetMachine = getOrCreateTargetMachine();
  if (failed(targetMachine))
    return;

  // Set the data layout and target triple of the module.
  module.setDataLayout((*targetMachine)->createDataLayout());
  module.setTargetTriple((*targetMachine)->getTargetTriple());
}

FailureOr<SmallVector<char, 0>>
ModuleToObject::moduleToObject(toolchain::Module &llvmModule) {
  SmallVector<char, 0> binaryData;
  // Write the LLVM module bitcode to a buffer.
  toolchain::raw_svector_ostream outputStream(binaryData);
  toolchain::WriteBitcodeToFile(llvmModule, outputStream);
  return binaryData;
}

std::optional<SmallVector<char, 0>> ModuleToObject::run() {
  // Translate the module to LLVM IR.
  toolchain::LLVMContext llvmContext;
  std::unique_ptr<toolchain::Module> llvmModule = translateToLLVMIR(llvmContext);
  if (!llvmModule) {
    getOperation().emitError() << "Failed creating the toolchain::Module.";
    return std::nullopt;
  }
  setDataLayoutAndTriple(*llvmModule);

  if (initialLlvmIRCallback)
    initialLlvmIRCallback(*llvmModule);

  // Link bitcode files.
  handleModulePreLink(*llvmModule);
  {
    auto libs = loadBitcodeFiles(*llvmModule);
    if (!libs)
      return std::nullopt;
    if (!libs->empty())
      if (failed(linkFiles(*llvmModule, std::move(*libs))))
        return std::nullopt;
    handleModulePostLink(*llvmModule);
  }

  if (linkedLlvmIRCallback)
    linkedLlvmIRCallback(*llvmModule);

  // Optimize the module.
  if (failed(optimizeModule(*llvmModule, optLevel)))
    return std::nullopt;

  if (optimizedLlvmIRCallback)
    optimizedLlvmIRCallback(*llvmModule);

  // Return the serialized object.
  return moduleToObject(*llvmModule);
}
