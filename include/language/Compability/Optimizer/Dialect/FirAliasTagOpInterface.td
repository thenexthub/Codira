//===-- FirAliasTagOpInterface.td --------------------------*- tablegen -*-===//
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

include "mlir/IR/Interfaces.td"

def FirAliasTagOpInterface : OpInterface<"FirAliasTagOpInterface"> {
  let description = [{
    An interface for memory operations that can carry alias analysis metadata.
    It provides setters and getters for the operation's alias analysis
    attributes. The default implementations of the interface methods expect
    the operation to have an attribute of type ArrayAttr named tbaa.
    Unlike the mlir::LLVM::AliasAnalysisOpInterface, this only supports tbaa.
  }];

  let cppNamespace = "::fir";
  let verify = [{ return detail::verifyFirAliasTagOpInterface($_op); }];

  let methods = [
    InterfaceMethod<
      /*desc=*/        "Returns the tbaa attribute or nullptr",
      /*returnType=*/  "mlir::ArrayAttr",
      /*methodName=*/  "getTBAATagsOrNull",
      /*args=*/        (ins),
      /*methodBody=*/  [{}],
      /*defaultImpl=*/ [{
        auto op = mlir::cast<ConcreteOp>(this->getOperation());
        return op.getTbaaAttr();
      }]
      >,
    InterfaceMethod<
      /*desc=*/        "Sets the tbaa attribute",
      /*returnType=*/  "void",
      /*methodName=*/  "setTBAATags",
      /*args=*/        (ins "const mlir::ArrayAttr":$attr),
      /*methodBody=*/  [{}],
      /*defaultImpl=*/ [{
        auto op = mlir::cast<ConcreteOp>(this->getOperation());
        op.setTbaaAttr(attr);
      }]
      >,
    InterfaceMethod<
      /*desc=*/        "Returns a list of all pointer operands accessed by the "
                       "operation",
      /*returnType=*/  "::toolchain::SmallVector<::mlir::Value>",
      /*methodName=*/  "getAccessedOperands",
      /*args=*/        (ins),
      /*methodBody=*/  [{}],
      /*defaultImpl=*/ [{
        auto op = mlir::cast<ConcreteOp>(this->getOperation());
        return {op.getMemref()};
      }]
      >
  ];
}
