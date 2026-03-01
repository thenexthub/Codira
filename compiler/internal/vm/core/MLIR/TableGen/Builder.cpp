//===- Builder.cpp - Builder definitions ----------------------------------===//
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

#include "mlir/TableGen/Builder.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;
using toolchain::DagInit;
using toolchain::DefInit;
using toolchain::Init;
using toolchain::Record;
using toolchain::StringInit;

//===----------------------------------------------------------------------===//
// Builder::Parameter
//===----------------------------------------------------------------------===//

/// Return a string containing the C++ type of this parameter.
StringRef Builder::Parameter::getCppType() const {
  if (const auto *stringInit = dyn_cast<StringInit>(def))
    return stringInit->getValue();
  const Record *record = cast<DefInit>(def)->getDef();
  // Inlining the first part of `Record::getValueAsString` to give better
  // error messages.
  const toolchain::RecordVal *type = record->getValue("type");
  if (!type || !type->getValue()) {
    toolchain::PrintFatalError("Builder DAG arguments must be either strings or "
                          "defs which inherit from CArg");
  }
  return record->getValueAsString("type");
}

/// Return an optional string containing the default value to use for this
/// parameter.
std::optional<StringRef> Builder::Parameter::getDefaultValue() const {
  if (isa<StringInit>(def))
    return std::nullopt;
  const Record *record = cast<DefInit>(def)->getDef();
  std::optional<StringRef> value =
      record->getValueAsOptionalString("defaultValue");
  return value && !value->empty() ? value : std::nullopt;
}

//===----------------------------------------------------------------------===//
// Builder
//===----------------------------------------------------------------------===//

Builder::Builder(const Record *record, ArrayRef<SMLoc> loc) : def(record) {
  // Initialize the parameters of the builder.
  const DagInit *dag = def->getValueAsDag("dagParams");
  auto *defInit = dyn_cast<DefInit>(dag->getOperator());
  if (!defInit || defInit->getDef()->getName() != "ins")
    PrintFatalError(def->getLoc(), "expected 'ins' in builders");

  bool seenDefaultValue = false;
  for (unsigned i = 0, e = dag->getNumArgs(); i < e; ++i) {
    const StringInit *paramName = dag->getArgName(i);
    const Init *paramValue = dag->getArg(i);
    Parameter param(paramName ? paramName->getValue()
                              : std::optional<StringRef>(),
                    paramValue);

    // Similarly to C++, once an argument with a default value is detected, the
    // following arguments must have default values as well.
    if (param.getDefaultValue()) {
      seenDefaultValue = true;
    } else if (seenDefaultValue) {
      PrintFatalError(loc,
                      "expected an argument with default value after other "
                      "arguments with default values");
    }
    parameters.emplace_back(param);
  }
}

/// Return an optional string containing the body of the builder.
std::optional<StringRef> Builder::getBody() const {
  std::optional<StringRef> body = def->getValueAsOptionalString("body");
  return body && !body->empty() ? body : std::nullopt;
}

std::optional<StringRef> Builder::getDeprecatedMessage() const {
  std::optional<StringRef> message =
      def->getValueAsOptionalString("odsCppDeprecated");
  return message && !message->empty() ? message : std::nullopt;
}
