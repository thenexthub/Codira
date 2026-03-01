//===- UniformSupport.cpp - Support utilities for uniform quant -----------===//
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

#include "mlir/Dialect/Quant/Utils/UniformSupport.h"
#include "mlir/IR/BuiltinTypes.h"
#include "vm/core/ADT/STLExtras.h"
#include <numeric>

using namespace mlir;
using namespace mlir::quant;

static bool isQuantizablePrimitiveType(Type inputType) {
  return isa<FloatType>(inputType);
}

ExpressedToQuantizedConverter
ExpressedToQuantizedConverter::forInputType(Type inputType) {
  if (isa<TensorType, VectorType>(inputType)) {
    Type elementType = cast<ShapedType>(inputType).getElementType();
    if (!isQuantizablePrimitiveType(elementType))
      return ExpressedToQuantizedConverter{inputType, nullptr};
    return ExpressedToQuantizedConverter{inputType, elementType};
  }
  // Supported primitive type (which just is the expressed type).
  if (isQuantizablePrimitiveType(inputType))
    return ExpressedToQuantizedConverter{inputType, inputType};
  // Unsupported.
  return ExpressedToQuantizedConverter{inputType, nullptr};
}

Type ExpressedToQuantizedConverter::convert(QuantizedType elementalType) const {
  assert(expressedType && "convert() on unsupported conversion");
  if (auto tensorType = dyn_cast<RankedTensorType>(inputType))
    return RankedTensorType::get(tensorType.getShape(), elementalType);
  if (isa<UnrankedTensorType>(inputType))
    return UnrankedTensorType::get(elementalType);
  if (auto vectorType = dyn_cast<VectorType>(inputType))
    return VectorType::get(vectorType.getShape(), elementalType);

  // If the expressed types match, just use the new elemental type.
  if (elementalType.getExpressedType() == expressedType)
    return elementalType;
  // Unsupported.
  return nullptr;
}

ElementsAttr
UniformQuantizedPerAxisValueConverter::convert(Attribute realValue) {
  if (auto attr = dyn_cast<DenseFPElementsAttr>(realValue)) {
    return convert(attr);
  }
  // TODO: handles sparse elements attribute
  return nullptr;
}

DenseElementsAttr
UniformQuantizedPerAxisValueConverter::convert(DenseFPElementsAttr attr) {
  // Creates the converter for each chunk. Normally the size of the
  // quantization dim is 3, so we can cache all the converters.
  ShapedType type = attr.getType();
  size_t dimSize = type.getDimSize(quantizationDim);
  if (dimSize != scales.size()) {
    return {};
  }
  SmallVector<UniformQuantizedValueConverter, 4> converters;
  converters.reserve(dimSize);
  for (int i = 0, e = dimSize; i != e; ++i) {
    converters.push_back(getPerChunkConverter(i));
  }

  // Scan the elements of the dense elements attributes and quantize them by
  // using the right quantization parameters.
  int64_t flattenIndex = 0;
  auto shape = type.getShape();
  int64_t chunkSize = toolchain::product_of(shape.drop_front(quantizationDim + 1));
  Type newElementType = IntegerType::get(attr.getContext(), storageBitWidth);
  return attr.mapValues(newElementType, [&](const APFloat &old) {
    int chunkIndex = (flattenIndex++) / chunkSize;
    return converters[chunkIndex % dimSize].quantizeFloatToInt(old);
  });
}
