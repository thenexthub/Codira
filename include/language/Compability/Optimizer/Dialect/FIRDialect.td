//===-- FIRDialect.td - FIR dialect definition -------------*- tablegen -*-===//
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
///
/// \file
/// Definition of the FIR dialect
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_DIALECT_FIR_DIALECT
#define LANGUAGE_COMPABILITY_DIALECT_FIR_DIALECT

include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/LoopLikeInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def FIROpsDialect : Dialect {
  let name = "fir";
  let cppNamespace = "::fir";
  let useDefaultTypePrinterParser = 0;
  let useDefaultAttributePrinterParser = 0;
  let usePropertiesForAttributes = 1;
  let dependentDialects = [
    // Arith dialect provides FastMathFlagsAttr
    // supported by some FIR operations.
    "mlir::arith::ArithDialect",
    // TBAA Tag types
    "mlir::LLVM::LLVMDialect"
  ];
  let extraClassDeclaration = [{
  private:
    // Register the builtin Attributes.
    void registerAttributes();
    // Register the builtin Types.
    void registerTypes();
    // Register external interfaces on operations of
    // this dialect.
    void registerOpExternalInterfaces();
  public:
    mlir::Type parseType(mlir::DialectAsmParser &parser) const override;
    void printType(mlir::Type ty, mlir::DialectAsmPrinter &p) const override;
 
    mlir::Attribute parseAttribute(mlir::DialectAsmParser &parser,
                                   mlir::Type type) const override;
    void printAttribute(mlir::Attribute attr,
                        mlir::DialectAsmPrinter &p) const override;

    // Return string name of fir.runtime attribute.
    static constexpr toolchain::StringRef getFirRuntimeAttrName() {
      return "fir.runtime";
    }
    // Return string name of fir.memory attributes.
    // It is attached to fir.call operations to convey
    // toolchain.memory attributes to LLVM IR.
    // Its value is intended to be mlir::LLVM::MemoryEffectsAttr.
    // TODO: we should probably make it an inherent attribute
    // of fir.call, though, it is supposed to be a short-lived
    // attribute that appears right before CodeGen and only
    // meaningful for LLVM, so it is unclear if embedding
    // it into fir.call makes sense.
    static constexpr toolchain::StringRef getFirCallMemoryAttrName() {
      return "fir.toolchain_memory";
    }
  }];
}

#endif // FORTRAN_DIALECT_FIR_DIALECT
