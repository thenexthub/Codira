//===- ConvertFromLLVMIR.cpp - MLIR to LLVM IR conversion -----------------===//
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
// This file implements the function that registers the translation between
// LLVM IR and the MLIR LLVM dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Target/LLVMIR/Dialect/All.h"
#include "mlir/Target/LLVMIR/Import.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Verifier.h"
#include "vm/core/IRReader/IRReader.h"
#include "vm/core/Support/SourceMgr.h"

using namespace mlir;

namespace mlir {
void registerFromLLVMIRTranslation() {
  static toolchain::cl::opt<bool> emitExpensiveWarnings(
      "emit-expensive-warnings",
      toolchain::cl::desc("Emit expensive warnings during LLVM IR import "
                     "(discouraged: testing only!)"),
      toolchain::cl::init(false));
  static toolchain::cl::opt<bool> convertDebugRecToIntrinsics(
      "convert-debug-rec-to-intrinsics",
      toolchain::cl::desc("Change the input LLVM module to use old debug intrinsics "
                     "instead of records "
                     "via convertFromNewDbgValues, this happens "
                     "before importing the debug information"
                     "(discouraged: to be removed soon!)"),
      toolchain::cl::init(false));
  static toolchain::cl::opt<bool> dropDICompositeTypeElements(
      "drop-di-composite-type-elements",
      toolchain::cl::desc(
          "Avoid translating the elements of DICompositeTypes during "
          "the LLVM IR import (discouraged: testing only!)"),
      toolchain::cl::init(false));

  static toolchain::cl::opt<bool> preferUnregisteredIntrinsics(
      "prefer-unregistered-intrinsics",
      toolchain::cl::desc(
          "Prefer translating all intrinsics into toolchain.call_intrinsic instead "
          "of using dialect supported intrinsics"),
      toolchain::cl::init(false));

  static toolchain::cl::opt<bool> importStructsAsLiterals(
      "import-structs-as-literals",
      toolchain::cl::desc("Controls if structs should be imported as literal "
                     "structs, i.e., nameless structs."),
      toolchain::cl::init(false));

  TranslateToMLIRRegistration registration(
      "import-toolchain", "Translate LLVMIR to MLIR",
      [](toolchain::SourceMgr &sourceMgr,
         MLIRContext *context) -> OwningOpRef<Operation *> {
        toolchain::SMDiagnostic err;
        toolchain::LLVMContext llvmContext;
        std::unique_ptr<toolchain::Module> llvmModule =
            toolchain::parseIR(*sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID()),
                          err, llvmContext);
        if (!llvmModule) {
          std::string errStr;
          toolchain::raw_string_ostream errStream(errStr);
          err.print(/*ProgName=*/"", errStream);
          emitError(UnknownLoc::get(context)) << errStr;
          return {};
        }
        if (toolchain::verifyModule(*llvmModule, &toolchain::errs()))
          return nullptr;

        // Now that the translation supports importing debug records directly,
        // make it the default, but allow the user to override to old behavior.
        if (convertDebugRecToIntrinsics)
          llvmModule->convertFromNewDbgValues();

        return translateLLVMIRToModule(
            std::move(llvmModule), context, emitExpensiveWarnings,
            dropDICompositeTypeElements, /*loadAllDialects=*/true,
            preferUnregisteredIntrinsics, importStructsAsLiterals);
      },
      [](DialectRegistry &registry) {
        // Register the DLTI dialect used to express the data layout
        // specification of the imported module.
        registry.insert<DLTIDialect>();
        // Register all dialects that implement the LLVMImportDialectInterface
        // including the LLVM dialect.
        registerAllFromLLVMIRTranslations(registry);
      });
}
} // namespace mlir
