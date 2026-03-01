//===- Types.cpp - MLIR Type Classes --------------------------------------===//
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
#include "mlir/IR/Dialect.h"

using namespace mlir;
using namespace mlir::detail;

//===----------------------------------------------------------------------===//
// AbstractType
//===----------------------------------------------------------------------===//

void AbstractType::walkImmediateSubElements(
    Type type, function_ref<void(Attribute)> walkAttrsFn,
    function_ref<void(Type)> walkTypesFn) const {
  walkImmediateSubElementsFn(type, walkAttrsFn, walkTypesFn);
}

Type AbstractType::replaceImmediateSubElements(Type type,
                                               ArrayRef<Attribute> replAttrs,
                                               ArrayRef<Type> replTypes) const {
  return replaceImmediateSubElementsFn(type, replAttrs, replTypes);
}

//===----------------------------------------------------------------------===//
// Type
//===----------------------------------------------------------------------===//

MLIRContext *Type::getContext() const { return getDialect().getContext(); }

bool Type::isBF16() const { return toolchain::isa<BFloat16Type>(*this); }
bool Type::isF16() const { return toolchain::isa<Float16Type>(*this); }
bool Type::isTF32() const { return toolchain::isa<FloatTF32Type>(*this); }
bool Type::isF32() const { return toolchain::isa<Float32Type>(*this); }
bool Type::isF64() const { return toolchain::isa<Float64Type>(*this); }
bool Type::isF80() const { return toolchain::isa<Float80Type>(*this); }
bool Type::isF128() const { return toolchain::isa<Float128Type>(*this); }

bool Type::isFloat() const { return toolchain::isa<FloatType>(*this); }

/// Return true if this is a float type with the specified width.
bool Type::isFloat(unsigned width) const {
  if (auto fltTy = toolchain::dyn_cast<FloatType>(*this))
    return fltTy.getWidth() == width;
  return false;
}

bool Type::isIndex() const { return toolchain::isa<IndexType>(*this); }

bool Type::isInteger() const { return toolchain::isa<IntegerType>(*this); }

bool Type::isInteger(unsigned width) const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.getWidth() == width;
  return false;
}

bool Type::isSignlessInteger() const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isSignless();
  return false;
}

bool Type::isSignlessInteger(unsigned width) const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isSignless() && intTy.getWidth() == width;
  return false;
}

bool Type::isSignedInteger() const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isSigned();
  return false;
}

bool Type::isSignedInteger(unsigned width) const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isSigned() && intTy.getWidth() == width;
  return false;
}

bool Type::isUnsignedInteger() const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isUnsigned();
  return false;
}

bool Type::isUnsignedInteger(unsigned width) const {
  if (auto intTy = toolchain::dyn_cast<IntegerType>(*this))
    return intTy.isUnsigned() && intTy.getWidth() == width;
  return false;
}

bool Type::isSignlessIntOrIndex() const {
  return isSignlessInteger() || toolchain::isa<IndexType>(*this);
}

bool Type::isSignlessIntOrIndexOrFloat() const {
  return isSignlessInteger() || toolchain::isa<IndexType, FloatType>(*this);
}

bool Type::isSignlessIntOrFloat() const {
  return isSignlessInteger() || toolchain::isa<FloatType>(*this);
}

bool Type::isIntOrIndex() const {
  return toolchain::isa<IntegerType>(*this) || isIndex();
}

bool Type::isIntOrFloat() const {
  return toolchain::isa<IntegerType, FloatType>(*this);
}

bool Type::isIntOrIndexOrFloat() const { return isIntOrFloat() || isIndex(); }

unsigned Type::getIntOrFloatBitWidth() const {
  assert(isIntOrFloat() && "only integers and floats have a bitwidth");
  if (auto intType = toolchain::dyn_cast<IntegerType>(*this))
    return intType.getWidth();
  return toolchain::cast<FloatType>(*this).getWidth();
}
