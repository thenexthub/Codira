//===---------------------------------------------------------------------===//
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
//===---------------------------------------------------------------------===//

#ifndef CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE
#define CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/LoopLikeInterface.td"

def LoopOpInterface : OpInterface<"LoopOpInterface", [
  DeclareOpInterfaceMethods<RegionBranchOpInterface>,
  DeclareOpInterfaceMethods<LoopLikeOpInterface>
]> {
  let description = [{
    Contains helper functions to query properties and perform transformations
    on a loop.
  }];
  let cppNamespace = "::cir";

  let methods = [
    InterfaceMethod<[{
        Returns the loop's conditional region.
      }],
      /*retTy=*/"mlir::Region &",
      /*methodName=*/"getCond"
    >,
    InterfaceMethod<[{
        Returns the loop's body region.
      }],
      /*retTy=*/"mlir::Region &",
      /*methodName=*/"getBody"
    >,
    InterfaceMethod<[{
        Returns a pointer to the loop's step region or nullptr.
      }],
      /*retTy=*/"mlir::Region *",
      /*methodName=*/"maybeGetStep",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/"return nullptr;"
    >,
    InterfaceMethod<[{
        Returns the first region to be executed in the loop.
      }],
      /*retTy=*/"mlir::Region &",
      /*methodName=*/"getEntry",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/"return $_op.getCond();"
    >,
    InterfaceMethod<[{
        Returns a list of regions in order of execution.
      }],
      /*retTy=*/"toolchain::SmallVector<mlir::Region *>",
      /*methodName=*/"getRegionsInExecutionOrder",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return toolchain::SmallVector<mlir::Region *, 2>{&$_op.getRegion(0), &$_op.getRegion(1)};
      }]
    >,
    InterfaceMethod<[{
        Recursively walks the body of the loop in pre-order while skipping
        nested loops and executing a callback on every other operation.
      }],
      /*retTy=*/"mlir::WalkResult",
      /*methodName=*/"walkBodySkippingNestedLoops",
      /*args=*/(ins "::toolchain::function_ref<mlir::WalkResult (mlir::Operation *)>":$callback),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return $_op.getBody().template walk<mlir::WalkOrder::PreOrder>([&](mlir::Operation *op) {
          if (mlir::isa<LoopOpInterface>(op))
            return mlir::WalkResult::skip();
          return callback(op);
        });
      }]
    >
  ];

  let extraClassDeclaration = [{
    /// Generic method to retrieve the successors of a LoopOpInterface operation.
    static void getLoopOpSuccessorRegions(
        ::cir::LoopOpInterface op, ::mlir::RegionBranchPoint point,
        ::mlir::SmallVectorImpl<::mlir::RegionSuccessor> &regions);
  }];

  let verify = [{
    /// Verify invariants of the LoopOpInterface.
    return detail::verifyLoopOpInterface($_op);
  }];
}

#endif // CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE
