//===- BuiltinTypeInterfaces.cpp ------------------------------------------===//
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

#include "mlir/IR/BuiltinTypes.h"
#include "vm/core/ADT/APFloat.h"

using namespace mlir;
using namespace mlir::detail;

//===----------------------------------------------------------------------===//
/// Tablegen Interface Definitions
//===----------------------------------------------------------------------===//

#include "mlir/IR/BuiltinTypeInterfaces.cpp.inc"

//===----------------------------------------------------------------------===//
// FloatType
//===----------------------------------------------------------------------===//

unsigned FloatType::getWidth() {
  return APFloat::semanticsSizeInBits(getFloatSemantics());
}

unsigned FloatType::getFPMantissaWidth() {
  return APFloat::semanticsPrecision(getFloatSemantics());
}

//===----------------------------------------------------------------------===//
// ShapedType
//===----------------------------------------------------------------------===//

int64_t ShapedType::getNumElements(ArrayRef<int64_t> shape) {
  int64_t num = 1;
  for (int64_t dim : shape) {
    num *= dim;
    assert(num >= 0 && "integer overflow in element count computation");
  }
  return num;
}
