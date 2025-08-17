//===- CIRDialect.td - CIR dialect -------------------------*- tablegen -*-===//
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
// This file declares the CIR dialect.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_DIALECT_IR_CIRDIALECT_TD
#define CLANG_CIR_DIALECT_IR_CIRDIALECT_TD

include "mlir/IR/OpBase.td"

def CIR_Dialect : Dialect {
  let name = "cir";

  // A short one-line summary of our dialect.
  let summary = "A high-level dialect for analyzing and optimizing Clang "
                "supported languages";

  let cppNamespace = "::cir";

  let useDefaultAttributePrinterParser = 0;
  let useDefaultTypePrinterParser = 0;

  // Enable constant materialization for the CIR dialect. This generates a
  // declaration for the cir::CIRDialect::materializeConstant function. This
  // hook is necessary for canonicalization to properly handle attributes
  // returned by fold methods, allowing them to be materialized as constant
  // operations in the IR.
  let hasConstantMaterializer = 1;

  let extraClassDeclaration = [{
    static toolchain::StringRef getTripleAttrName() { return "cir.triple"; }
    static toolchain::StringRef getOptInfoAttrName() { return "cir.opt_info"; }
    static toolchain::StringRef getCalleeAttrName() { return "callee"; }
    static toolchain::StringRef getNoThrowAttrName() { return "nothrow"; }
    static toolchain::StringRef getSideEffectAttrName() { return "side_effect"; }
    static toolchain::StringRef getModuleLevelAsmAttrName() { return "cir.module_asm"; }

    void registerAttributes();
    void registerTypes();

    mlir::Type parseType(mlir::DialectAsmParser &parser) const override;
    void printType(mlir::Type type,
                   mlir::DialectAsmPrinter &printer) const override;

    mlir::Attribute parseAttribute(mlir::DialectAsmParser &parser,
                                   mlir::Type type) const override;

    void printAttribute(mlir::Attribute attr,
                        mlir::DialectAsmPrinter &os) const override;
  }];
}

#endif // CLANG_CIR_DIALECT_IR_CIRDIALECT_TD
