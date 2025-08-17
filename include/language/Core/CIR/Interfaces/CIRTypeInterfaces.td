//===----------------------------------------------------------------------===//
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
// Defines cir type interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_INTERFACES_CIRTYPEINTERFACES_TD
#define CLANG_CIR_INTERFACES_CIRTYPEINTERFACES_TD

include "mlir/IR/OpBase.td"

def CIR_IntTypeInterface : TypeInterface<"IntTypeInterface"> {
  let description = [{
    Contains helper functions to query properties about an integer type.
  }];
  let cppNamespace = "::cir";
  let methods = [
    InterfaceMethod<[{
        Returns true if this is a signed integer type.
      }],
      /*retTy=*/"bool",
      /*methodName=*/"isSigned",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return $_type.isSigned();
      }]
    >,
    InterfaceMethod<[{
        Returns true if this is an unsigned integer type.
      }],
      /*retTy=*/"bool",
      /*methodName=*/"isUnsigned",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return $_type.isUnsigned();
      }]
    >,
    InterfaceMethod<[{
        Returns the bit width of this integer type.
      }],
      /*retTy=*/"unsigned",
      /*methodName=*/"getWidth",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return $_type.getWidth();
      }]
    >
  ];
}

def CIR_FPTypeInterface : TypeInterface<"FPTypeInterface"> {
  let description = [{
    Contains helper functions to query properties about a floating-point type.
  }];
  let cppNamespace = "::cir";

  let methods = [
    InterfaceMethod<[{
        Returns the bit width of this floating-point type.
      }],
      /*retTy=*/"unsigned",
      /*methodName=*/"getWidth",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
          return toolchain::APFloat::semanticsSizeInBits($_type.getFloatSemantics());
        }]
    >,
    InterfaceMethod<[{
        Return the mantissa width.
      }],
      /*retTy=*/"unsigned",
      /*methodName=*/"getFPMantissaWidth",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
          return toolchain::APFloat::semanticsPrecision($_type.getFloatSemantics());
        }]
    >,
    InterfaceMethod<[{
        Return the float semantics of this floating-point type.
      }],
      /*retTy=*/"const toolchain::fltSemantics &",
      /*methodName=*/"getFloatSemantics"
    >,
  ];
}

def CIR_SizedTypeInterface : TypeInterface<"SizedTypeInterface"> {
  let description = [{
    Annotates types that have known size. Types that don't have a size are
    abstract types and void.
  }];
  let cppNamespace = "::cir";
  let methods = [
    InterfaceMethod<[{
        Returns true if this is a sized type. This mirrors sizedness from the
        clang AST, where a type is sized if it has a known size.
        By default type defining this interface returns true,
        but this can be overridden if sizedness depends on properties of the type.
        For example, whether a struct is not sized if it is incomplete.
      }],
      /*retTy=*/"bool",
      /*methodName=*/"isSized",
      /*args=*/(ins),
      /*methodBody=*/"",
      /*defaultImplementation=*/[{
        return true;
      }]
    >,
  ];
}

#endif // CLANG_CIR_INTERFACES_CIRTYPEINTERFACES_TD
