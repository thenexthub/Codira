//===-- Optimizer/CodeGen/CodeGen.h -- code generation ----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_CODEGEN_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_CODEGEN_H

#include "language/Compability/Frontend/CodeGenOptions.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/raw_ostream.h"
#include <memory>

namespace fir {

class LLVMTypeConverter;

struct NameUniquer;

#define GEN_PASS_DECL_FIRTOLLVMLOWERING
#define GEN_PASS_DECL_CODEGENREWRITE
#define GEN_PASS_DECL_TARGETREWRITEPASS
#define GEN_PASS_DECL_BOXEDPROCEDUREPASS
#define GEN_PASS_DECL_LOWERREPACKARRAYSPASS
#include "language/Compability/Optimizer/CodeGen/CGPasses.h.inc"

/// FIR to LLVM translation pass options.
struct FIRToLLVMPassOptions {
  // Do not fail when type descriptors are not found when translating
  // operations that use them at the LLVM level like fir.embox. Instead,
  // just use a null pointer.
  // This is useful to test translating programs manually written where a
  // frontend did not generate type descriptor data structures. However, note
  // that such programs would crash at runtime if the derived type descriptors
  // are required by the runtime, so this is only an option to help debugging.
  bool ignoreMissingTypeDescriptors = false;
  // Similar to ignoreMissingTypeDescriptors, but generate external declaration
  // for the missing type descriptor globals instead.
  bool skipExternalRttiDefinition = false;

  // Generate TBAA information for FIR types and memory accessing operations.
  bool applyTBAA = false;

  // Force the usage of a unified tbaa tree in TBAABuilder.
  bool forceUnifiedTBAATree = false;

  // If set to true, then the global variables created
  // for the derived types have been renamed to avoid usage
  // of special symbols that may not be supported by all targets.
  // The renaming is done by the CompilerGeneratedNamesConversion pass.
  // If it is true, FIR-to-LLVM pass has to use
  // fir::NameUniquer::getTypeDescriptorAssemblyName() to take
  // the name of the global variable corresponding to a derived
  // type's descriptor.
  bool typeDescriptorsRenamedForAssembly = false;

  // Specify the calculation method for complex number division used by the
  // Conversion pass of the MLIR complex dialect.
  language::Compability::frontend::CodeGenOptions::ComplexRangeKind ComplexRange =
      language::Compability::frontend::CodeGenOptions::ComplexRangeKind::CX_Full;
};

/// Convert FIR to the LLVM IR dialect with default options.
std::unique_ptr<mlir::Pass> createFIRToLLVMPass();

/// Convert FIR to the LLVM IR dialect
std::unique_ptr<mlir::Pass> createFIRToLLVMPass(FIRToLLVMPassOptions options);

using LLVMIRLoweringPrinter =
    std::function<void(toolchain::Module &, toolchain::raw_ostream &)>;

/// Convert the LLVM IR dialect to LLVM-IR proper
std::unique_ptr<mlir::Pass> createLLVMDialectToLLVMPass(
    toolchain::raw_ostream &output,
    LLVMIRLoweringPrinter printer =
        [](toolchain::Module &m, toolchain::raw_ostream &out) { m.print(out, nullptr); });

/// Populate the given list with patterns that convert from FIR to LLVM.
void populateFIRToLLVMConversionPatterns(
    const fir::LLVMTypeConverter &converter, mlir::RewritePatternSet &patterns,
    fir::FIRToLLVMPassOptions &options);

/// Populate the pattern set with the PreCGRewrite patterns.
void populatePreCGRewritePatterns(mlir::RewritePatternSet &patterns,
                                  bool preserveDeclare);

// declarative passes
#define GEN_PASS_REGISTRATION
#include "language/Compability/Optimizer/CodeGen/CGPasses.h.inc"

} // namespace fir

#endif // FORTRAN_OPTIMIZER_CODEGEN_CODEGEN_H
