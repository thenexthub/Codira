//===- Dialect.cpp - Dialect wrapper class --------------------------------===//
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
// Dialect wrapper to simplify using TableGen Record defining a MLIR dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/TableGen/Dialect.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;
Dialect::Dialect(const toolchain::Record *def) : def(def) {
  if (def == nullptr)
    return;
  for (StringRef dialect : def->getValueAsListOfStrings("dependentDialects"))
    dependentDialects.push_back(dialect);
}

StringRef Dialect::getName() const { return def->getValueAsString("name"); }

StringRef Dialect::getCppNamespace() const {
  return def->getValueAsString("cppNamespace");
}

std::string Dialect::getCppClassName() const {
  // Simply use the name and remove any '_' tokens.
  std::string cppName = def->getName().str();
  toolchain::erase(cppName, '_');
  return cppName;
}

static StringRef getAsStringOrEmpty(const toolchain::Record &record,
                                    StringRef fieldName) {
  if (auto *valueInit = record.getValueInit(fieldName)) {
    if (toolchain::isa<toolchain::StringInit>(valueInit))
      return record.getValueAsString(fieldName);
  }
  return "";
}

StringRef Dialect::getSummary() const {
  return getAsStringOrEmpty(*def, "summary");
}

StringRef Dialect::getDescription() const {
  return getAsStringOrEmpty(*def, "description");
}

ArrayRef<StringRef> Dialect::getDependentDialects() const {
  return dependentDialects;
}

std::optional<StringRef> Dialect::getExtraClassDeclaration() const {
  auto value = def->getValueAsString("extraClassDeclaration");
  return value.empty() ? std::optional<StringRef>() : value;
}

bool Dialect::hasCanonicalizer() const {
  return def->getValueAsBit("hasCanonicalizer");
}

bool Dialect::hasConstantMaterializer() const {
  return def->getValueAsBit("hasConstantMaterializer");
}

bool Dialect::hasNonDefaultDestructor() const {
  return def->getValueAsBit("hasNonDefaultDestructor");
}

bool Dialect::hasOperationAttrVerify() const {
  return def->getValueAsBit("hasOperationAttrVerify");
}

bool Dialect::hasRegionArgAttrVerify() const {
  return def->getValueAsBit("hasRegionArgAttrVerify");
}

bool Dialect::hasRegionResultAttrVerify() const {
  return def->getValueAsBit("hasRegionResultAttrVerify");
}

bool Dialect::hasOperationInterfaceFallback() const {
  return def->getValueAsBit("hasOperationInterfaceFallback");
}

bool Dialect::useDefaultAttributePrinterParser() const {
  return def->getValueAsBit("useDefaultAttributePrinterParser");
}

bool Dialect::useDefaultTypePrinterParser() const {
  return def->getValueAsBit("useDefaultTypePrinterParser");
}

bool Dialect::isExtensible() const {
  return def->getValueAsBit("isExtensible");
}

bool Dialect::usePropertiesForAttributes() const {
  return def->getValueAsBit("usePropertiesForAttributes");
}

const toolchain::DagInit *Dialect::getDiscardableAttributes() const {
  return def->getValueAsDag("discardableAttrs");
}

bool Dialect::operator==(const Dialect &other) const {
  return def == other.def;
}

bool Dialect::operator<(const Dialect &other) const {
  return getName() < other.getName();
}
