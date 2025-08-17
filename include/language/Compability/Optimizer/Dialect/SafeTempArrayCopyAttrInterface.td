//===- SafeTempArrayCopyAttrInterface.td -------------------*- tablegen -*-===//
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
/// \file
/// This file defines SafeTempArrayCopyAttrInterface and a generic attribute
/// SafeTempArrayCopyAttr promising the SafeTempArrayCopyAttrInterface.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SAFETEMPARRAYCOPYATTRINTERFACE_TD
#define LANGUAGE_COMPABILITY_SAFETEMPARRAYCOPYATTRINTERFACE_TD

include "mlir/IR/OpBase.td"

def SafeTempArrayCopyAttrInterface
    : AttrInterface<"SafeTempArrayCopyAttrInterface"> {
  let cppNamespace = "::fir";
  let description = [{
    Interface for attributes defining whether creation of a temporary
    copy of a Fortran array is safe and/or how to produce proper
    dynamic checks to avoid it, if it is unsafe.
  }];

  let methods =
      [StaticInterfaceMethod<
           /*desc=*/[{
        Returns true iff the usage of the temporary array copy
        can be made safe applying some dynamic checks.
      }],
           /*retTy=*/"bool",
           /*methodName=*/"isDynamicallySafe",
           /*args=*/(ins)>,
       StaticInterfaceMethod<
           /*desc=*/[{
        Generate FIR that produces an i1 Value indicating
        whether the creation of the temporary array copy is safe.
        \p array is a definition of the original array.
        The implementation may assume that \p array is present
        (though, it may be empty).
      }],
           /*retTy=*/"mlir::Value",
           /*methodName=*/"genDynamicCheck",
           /*args=*/
           (ins "::mlir::Location":$loc, "::fir::FirOpBuilder &":$builder,
               "::mlir::Value":$array)>,
       StaticInterfaceMethod<
           /*desc=*/[{
        This method allows inserting any FIR right before the optional
        copy-out (from \p temp to \p array) and the deallocation
        of the temporary array (implying that the temporary copy was
        actually created).
      }],
           /*retTy=*/"void",
           /*methodName=*/"registerTempDeallocation",
           /*args=*/
           (ins "::mlir::Location":$loc, "::fir::FirOpBuilder &":$builder,
               "::mlir::Value":$array, "::mlir::Value":$temp)>,
  ];
}

def SafeTempArrayCopyAttr
    : ConfinedAttr<
          AnyAttr, [PromisedAttrInterface<SafeTempArrayCopyAttrInterface>]> {
  let description = [{
    Generic attribute implementing or promising
    the `SafeTempArrayCopyAttrInterface` interface.
  }];
}

def SafeTempArrayCopyArrayAttr
    : TypedArrayAttrBase<SafeTempArrayCopyAttr,
                         "array of SafeTempArrayCopyAttr attributes">;

def NonEmptySafeTempArrayCopyArrayAttr
    : ConfinedAttr<SafeTempArrayCopyArrayAttr, [ArrayMinCount<1>]>;

#endif // FORTRAN_SAFETEMPARRAYCOPYATTRINTERFACE_TD
