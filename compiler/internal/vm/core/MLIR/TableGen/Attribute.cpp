//===- Attribute.cpp - Attribute wrapper class ----------------------------===//
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
// Attribute wrapper to simplify using TableGen Record defining a MLIR
// Attribute.
//
//===----------------------------------------------------------------------===//

#include "mlir/TableGen/Operator.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;

using toolchain::DefInit;
using toolchain::Init;
using toolchain::Record;
using toolchain::StringInit;

// Returns the initializer's value as string if the given TableGen initializer
// is a code or string initializer. Returns the empty StringRef otherwise.
static StringRef getValueAsString(const Init *init) {
  if (const auto *str = dyn_cast<StringInit>(init))
    return str->getValue().trim();
  return {};
}

bool AttrConstraint::isSubClassOf(StringRef className) const {
  return def->isSubClassOf(className);
}

Attribute::Attribute(const Record *record) : AttrConstraint(record) {
  assert(record->isSubClassOf("Attr") &&
         "must be subclass of TableGen 'Attr' class");
}

Attribute::Attribute(const DefInit *init) : Attribute(init->getDef()) {}

bool Attribute::isDerivedAttr() const { return isSubClassOf("DerivedAttr"); }

bool Attribute::isTypeAttr() const { return isSubClassOf("TypeAttrBase"); }

bool Attribute::isSymbolRefAttr() const {
  StringRef defName = def->getName();
  if (defName == "SymbolRefAttr" || defName == "FlatSymbolRefAttr")
    return true;
  return isSubClassOf("SymbolRefAttr") || isSubClassOf("FlatSymbolRefAttr");
}

bool Attribute::isEnumAttr() const { return isSubClassOf("EnumAttrInfo"); }

StringRef Attribute::getStorageType() const {
  const auto *init = def->getValueInit("storageType");
  auto type = getValueAsString(init);
  if (type.empty())
    return "::mlir::Attribute";
  return type;
}

StringRef Attribute::getReturnType() const {
  const auto *init = def->getValueInit("returnType");
  return getValueAsString(init);
}

// Return the type constraint corresponding to the type of this attribute, or
// std::nullopt if this is not a TypedAttr.
std::optional<Type> Attribute::getValueType() const {
  if (const auto *defInit = dyn_cast<DefInit>(def->getValueInit("valueType")))
    return Type(defInit->getDef());
  return std::nullopt;
}

StringRef Attribute::getConvertFromStorageCall() const {
  const auto *init = def->getValueInit("convertFromStorage");
  return getValueAsString(init);
}

bool Attribute::isConstBuildable() const {
  const auto *init = def->getValueInit("constBuilderCall");
  return !getValueAsString(init).empty();
}

StringRef Attribute::getConstBuilderTemplate() const {
  const auto *init = def->getValueInit("constBuilderCall");
  return getValueAsString(init);
}

Attribute Attribute::getBaseAttr() const {
  if (const auto *defInit = dyn_cast<DefInit>(def->getValueInit("baseAttr"))) {
    return Attribute(defInit).getBaseAttr();
  }
  return *this;
}

bool Attribute::hasDefaultValue() const {
  const auto *init = def->getValueInit("defaultValue");
  return !getValueAsString(init).empty();
}

StringRef Attribute::getDefaultValue() const {
  const auto *init = def->getValueInit("defaultValue");
  return getValueAsString(init);
}

bool Attribute::isOptional() const { return def->getValueAsBit("isOptional"); }

StringRef Attribute::getAttrDefName() const {
  if (def->isAnonymous()) {
    return getBaseAttr().def->getName();
  }
  return def->getName();
}

StringRef Attribute::getDerivedCodeBody() const {
  assert(isDerivedAttr() && "only derived attribute has 'body' field");
  return def->getValueAsString("body");
}

Dialect Attribute::getDialect() const {
  const toolchain::RecordVal *record = def->getValue("dialect");
  if (record && record->getValue()) {
    if (const DefInit *init = dyn_cast<DefInit>(record->getValue()))
      return Dialect(init->getDef());
  }
  return Dialect(nullptr);
}

const Record &Attribute::getDef() const { return *def; }

ConstantAttr::ConstantAttr(const DefInit *init) : def(init->getDef()) {
  assert(def->isSubClassOf("ConstantAttr") &&
         "must be subclass of TableGen 'ConstantAttr' class");
}

Attribute ConstantAttr::getAttribute() const {
  return Attribute(def->getValueAsDef("attr"));
}

StringRef ConstantAttr::getConstantValue() const {
  return def->getValueAsString("value");
}

const char * ::mlir::tblgen::inferTypeOpInterface = "InferTypeOpInterface";
