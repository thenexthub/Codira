//===- EnumInfo.cpp - EnumInfo wrapper class ----------------------------===//
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

#include "mlir/TableGen/EnumInfo.h"
#include "mlir/TableGen/Attribute.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;

using toolchain::DefInit;
using toolchain::Init;
using toolchain::Record;

EnumCase::EnumCase(const Record *record) : def(record) {
  assert(def->isSubClassOf("EnumCase") &&
         "must be subclass of TableGen 'EnumCase' class");
}

EnumCase::EnumCase(const DefInit *init) : EnumCase(init->getDef()) {}

StringRef EnumCase::getSymbol() const {
  return def->getValueAsString("symbol");
}

StringRef EnumCase::getStr() const { return def->getValueAsString("str"); }

int64_t EnumCase::getValue() const { return def->getValueAsInt("value"); }

const Record &EnumCase::getDef() const { return *def; }

EnumInfo::EnumInfo(const Record *record) : def(record) {
  assert(isSubClassOf("EnumInfo") &&
         "must be subclass of TableGen 'EnumInfo' class");
}

EnumInfo::EnumInfo(const Record &record) : EnumInfo(&record) {}

EnumInfo::EnumInfo(const DefInit *init) : EnumInfo(init->getDef()) {}

bool EnumInfo::isSubClassOf(StringRef className) const {
  return def->isSubClassOf(className);
}

bool EnumInfo::isEnumAttr() const { return isSubClassOf("EnumAttrInfo"); }

std::optional<Attribute> EnumInfo::asEnumAttr() const {
  if (isEnumAttr())
    return Attribute(def);
  return std::nullopt;
}

bool EnumInfo::isBitEnum() const { return isSubClassOf("BitEnumBase"); }

StringRef EnumInfo::getEnumClassName() const {
  return def->getValueAsString("className");
}

StringRef EnumInfo::getSummary() const {
  return def->getValueAsString("summary");
}

StringRef EnumInfo::getDescription() const {
  return def->getValueAsString("description");
}

StringRef EnumInfo::getCppNamespace() const {
  return def->getValueAsString("cppNamespace");
}

int64_t EnumInfo::getBitwidth() const { return def->getValueAsInt("bitwidth"); }

StringRef EnumInfo::getUnderlyingType() const {
  return def->getValueAsString("underlyingType");
}

StringRef EnumInfo::getUnderlyingToSymbolFnName() const {
  return def->getValueAsString("underlyingToSymbolFnName");
}

StringRef EnumInfo::getStringToSymbolFnName() const {
  return def->getValueAsString("stringToSymbolFnName");
}

StringRef EnumInfo::getSymbolToStringFnName() const {
  return def->getValueAsString("symbolToStringFnName");
}

StringRef EnumInfo::getSymbolToStringFnRetType() const {
  return def->getValueAsString("symbolToStringFnRetType");
}

StringRef EnumInfo::getMaxEnumValFnName() const {
  return def->getValueAsString("maxEnumValFnName");
}

std::vector<EnumCase> EnumInfo::getAllCases() const {
  const auto *inits = def->getValueAsListInit("enumerants");

  std::vector<EnumCase> cases;
  cases.reserve(inits->size());

  for (const Init *init : *inits) {
    cases.emplace_back(cast<DefInit>(init));
  }

  return cases;
}

bool EnumInfo::genSpecializedAttr() const {
  return isSubClassOf("EnumAttrInfo") &&
         def->getValueAsBit("genSpecializedAttr");
}

const Record *EnumInfo::getBaseAttrClass() const {
  return def->getValueAsDef("baseAttrClass");
}

StringRef EnumInfo::getSpecializedAttrClassName() const {
  return def->getValueAsString("specializedAttrClassName");
}

bool EnumInfo::printBitEnumPrimaryGroups() const {
  return def->getValueAsBit("printBitEnumPrimaryGroups");
}

bool EnumInfo::printBitEnumQuoted() const {
  return def->getValueAsBit("printBitEnumQuoted");
}

const Record &EnumInfo::getDef() const { return *def; }
