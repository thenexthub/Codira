//===- GenInfo.cpp - Generator info -----------------------------*- C++ -*-===//
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

#include "mlir/TableGen/GenInfo.h"

#include "mlir/TableGen/GenNameParser.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ManagedStatic.h"

using namespace mlir;

static toolchain::ManagedStatic<std::vector<GenInfo>> generatorRegistry;

GenRegistration::GenRegistration(StringRef arg, StringRef description,
                                 const GenFunction &function) {
  generatorRegistry->emplace_back(arg, description, function);
}

GenNameParser::GenNameParser(toolchain::cl::Option &opt)
    : toolchain::cl::parser<const GenInfo *>(opt) {
  for (const auto &kv : *generatorRegistry) {
    addLiteralOption(kv.getGenArgument(), &kv, kv.getGenDescription());
  }
}

void GenNameParser::printOptionInfo(const toolchain::cl::Option &o,
                                    size_t globalWidth) const {
  GenNameParser *tp = const_cast<GenNameParser *>(this);
  toolchain::array_pod_sort(tp->Values.begin(), tp->Values.end(),
                       [](const GenNameParser::OptionInfo *vT1,
                          const GenNameParser::OptionInfo *vT2) {
                         return vT1->Name.compare(vT2->Name);
                       });
  using toolchain::cl::parser;
  parser<const GenInfo *>::printOptionInfo(o, globalWidth);
}
