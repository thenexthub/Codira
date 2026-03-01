//===- Pass.cpp - Pass related classes ------------------------------------===//
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

#include "mlir/TableGen/Pass.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;

//===----------------------------------------------------------------------===//
// PassOption
//===----------------------------------------------------------------------===//

StringRef PassOption::getCppVariableName() const {
  return def->getValueAsString("cppName");
}

StringRef PassOption::getArgument() const {
  return def->getValueAsString("argument");
}

StringRef PassOption::getType() const { return def->getValueAsString("type"); }

std::optional<StringRef> PassOption::getDefaultValue() const {
  StringRef defaultVal = def->getValueAsString("defaultValue");
  return defaultVal.empty() ? std::optional<StringRef>() : defaultVal;
}

StringRef PassOption::getDescription() const {
  return def->getValueAsString("description");
}

std::optional<StringRef> PassOption::getAdditionalFlags() const {
  StringRef additionalFlags = def->getValueAsString("additionalOptFlags");
  return additionalFlags.empty() ? std::optional<StringRef>() : additionalFlags;
}

bool PassOption::isListOption() const {
  return def->isSubClassOf("ListOption");
}

//===----------------------------------------------------------------------===//
// PassStatistic
//===----------------------------------------------------------------------===//

StringRef PassStatistic::getCppVariableName() const {
  return def->getValueAsString("cppName");
}

StringRef PassStatistic::getName() const {
  return def->getValueAsString("name");
}

StringRef PassStatistic::getDescription() const {
  return def->getValueAsString("description");
}

//===----------------------------------------------------------------------===//
// Pass
//===----------------------------------------------------------------------===//

Pass::Pass(const toolchain::Record *def) : def(def) {
  for (auto *init : def->getValueAsListOfDefs("options"))
    options.emplace_back(init);
  for (auto *init : def->getValueAsListOfDefs("statistics"))
    statistics.emplace_back(init);
  for (StringRef dialect : def->getValueAsListOfStrings("dependentDialects"))
    dependentDialects.push_back(dialect);
}

StringRef Pass::getArgument() const {
  return def->getValueAsString("argument");
}

StringRef Pass::getBaseClass() const {
  return def->getValueAsString("baseClass");
}

StringRef Pass::getSummary() const { return def->getValueAsString("summary"); }

StringRef Pass::getDescription() const {
  return def->getValueAsString("description");
}

StringRef Pass::getConstructor() const {
  return def->getValueAsString("constructor");
}

ArrayRef<StringRef> Pass::getDependentDialects() const {
  return dependentDialects;
}

ArrayRef<PassOption> Pass::getOptions() const { return options; }

ArrayRef<PassStatistic> Pass::getStatistics() const { return statistics; }
