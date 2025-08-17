//===-- CUFDialect.td - CUF dialect base definitions -------*- tablegen -*-===//
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
/// Definition of the CUDA Fortran dialect
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_DIALECT_CUF_CUFDIALECT
#define LANGUAGE_COMPABILITY_DIALECT_CUF_CUFDIALECT

include "mlir/IR/AttrTypeBase.td"
include "mlir/IR/EnumAttr.td"
include "mlir/IR/OpBase.td"

def CUFDialect : Dialect {
  let name = "cuf";

  let summary = "CUDA Fortran dialect";

  let description = [{
    This dialect models CUDA Fortran operations. The CUF dialect operations use
    the FIR types and are tightly coupled with FIR and HLFIR.
  }];

  let useDefaultAttributePrinterParser = 1;
  let usePropertiesForAttributes = 1;
  let cppNamespace = "::cuf";
  let dependentDialects = ["fir::FIROpsDialect"];

  let extraClassDeclaration = [{
  private:
    // Register the CUF Attributes.
    void registerAttributes();
  }];
}

#endif // FORTRAN_DIALECT_CUF_CUFDIALECT
