//===- Type.cpp - Type class ----------------------------------------------===//
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
//
// Type wrapper to simplify using TableGen Record defining a MLIR Type.
//
//===----------------------------------------------------------------------===//

#include "mlir/TableGen/Type.h"
#include "mlir/TableGen/Dialect.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;
using toolchain::Record;

TypeConstraint::TypeConstraint(const toolchain::DefInit *init)
    : TypeConstraint(init->getDef()) {}

bool TypeConstraint::isOptional() const {
  return def->isSubClassOf("Optional");
}

bool TypeConstraint::isVariadic() const {
  return def->isSubClassOf("Variadic");
}

bool TypeConstraint::isVariadicOfVariadic() const {
  return def->isSubClassOf("VariadicOfVariadic");
}

StringRef TypeConstraint::getVariadicOfVariadicSegmentSizeAttr() const {
  assert(isVariadicOfVariadic());
  return def->getValueAsString("segmentAttrName");
}

// Returns the builder call for this constraint if this is a buildable type,
// returns std::nullopt otherwise.
std::optional<StringRef> TypeConstraint::getBuilderCall() const {
  const Record *baseType = def;
  if (isVariableLength())
    baseType = baseType->getValueAsDef("baseType");

  // Check to see if this type constraint has a builder call.
  const toolchain::RecordVal *builderCall = baseType->getValue("builderCall");
  if (!builderCall || !builderCall->getValue())
    return std::nullopt;
  return TypeSwitch<const toolchain::Init *, std::optional<StringRef>>(
             builderCall->getValue())
      .Case<toolchain::StringInit>([&](auto *init) {
        StringRef value = init->getValue();
        return value.empty() ? std::optional<StringRef>() : value;
      })
      .Default(std::nullopt);
}

// Return the C++ type for this type (which may just be ::mlir::Type).
StringRef TypeConstraint::getCppType() const {
  return def->getValueAsString("cppType");
}

Type::Type(const Record *record) : TypeConstraint(record) {}

Dialect Type::getDialect() const {
  return Dialect(def->getValueAsDef("dialect"));
}
