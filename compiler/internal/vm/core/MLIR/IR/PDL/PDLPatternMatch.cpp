//===- PDLPatternMatch.cpp - Base classes for PDL pattern match
//------------===//
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

#include "mlir/IR/PatternMatch.h"
#include "vm/core/Support/InterleavedRange.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// PDLValue
//===----------------------------------------------------------------------===//

void PDLValue::print(raw_ostream &os) const {
  if (!value) {
    os << "<NULL-PDLValue>";
    return;
  }
  switch (kind) {
  case Kind::Attribute:
    os << cast<Attribute>();
    break;
  case Kind::Operation:
    os << *cast<Operation *>();
    break;
  case Kind::Type:
    os << cast<Type>();
    break;
  case Kind::TypeRange:
    os << toolchain::interleaved(cast<TypeRange>());
    break;
  case Kind::Value:
    os << cast<Value>();
    break;
  case Kind::ValueRange:
    os << toolchain::interleaved(cast<ValueRange>());
    break;
  }
}

void PDLValue::print(raw_ostream &os, Kind kind) {
  switch (kind) {
  case Kind::Attribute:
    os << "Attribute";
    break;
  case Kind::Operation:
    os << "Operation";
    break;
  case Kind::Type:
    os << "Type";
    break;
  case Kind::TypeRange:
    os << "TypeRange";
    break;
  case Kind::Value:
    os << "Value";
    break;
  case Kind::ValueRange:
    os << "ValueRange";
    break;
  }
}

//===----------------------------------------------------------------------===//
// PDLPatternModule
//===----------------------------------------------------------------------===//

void PDLPatternModule::mergeIn(PDLPatternModule &&other) {
  // Ignore the other module if it has no patterns.
  if (!other.pdlModule)
    return;

  // Steal the functions and config of the other module.
  for (auto &it : other.constraintFunctions)
    registerConstraintFunction(it.first(), std::move(it.second));
  for (auto &it : other.rewriteFunctions)
    registerRewriteFunction(it.first(), std::move(it.second));
  for (auto &it : other.configs)
    configs.emplace_back(std::move(it));
  for (auto &it : other.configMap)
    configMap.insert(it);

  // Steal the other state if we have no patterns.
  if (!pdlModule) {
    pdlModule = std::move(other.pdlModule);
    return;
  }

  // Merge the pattern operations from the other module into this one.
  Block *block = pdlModule->getBody();
  block->getOperations().splice(block->end(),
                                other.pdlModule->getBody()->getOperations());
}

void PDLPatternModule::attachConfigToPatterns(ModuleOp module,
                                              PDLPatternConfigSet &configSet) {
  // Attach the configuration to the symbols within the module. We only add
  // to symbols to avoid hardcoding any specific operation names here (given
  // that we don't depend on any PDL dialect). We can't use
  // cast<SymbolOpInterface> here because patterns may be optional symbols.
  module->walk([&](Operation *op) {
    if (op->hasTrait<SymbolOpInterface::Trait>())
      configMap[op] = &configSet;
  });
}

//===----------------------------------------------------------------------===//
// Function Registry
//===----------------------------------------------------------------------===//

void PDLPatternModule::registerConstraintFunction(
    StringRef name, PDLConstraintFunction constraintFn) {
  // TODO: Is it possible to diagnose when `name` is already registered to
  // a function that is not equivalent to `constraintFn`?
  // Allow existing mappings in the case multiple patterns depend on the same
  // constraint.
  constraintFunctions.try_emplace(name, std::move(constraintFn));
}

void PDLPatternModule::registerRewriteFunction(StringRef name,
                                               PDLRewriteFunction rewriteFn) {
  // TODO: Is it possible to diagnose when `name` is already registered to
  // a function that is not equivalent to `rewriteFn`?
  // Allow existing mappings in the case multiple patterns depend on the same
  // rewrite.
  rewriteFunctions.try_emplace(name, std::move(rewriteFn));
}
