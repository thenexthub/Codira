//===- InterpreterPass.cpp - Transform dialect interpreter pass -----------===//
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

#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/Dialect/Transform/Transforms/Passes.h"
#include "mlir/Dialect/Transform/Transforms/TransformInterpreterUtils.h"

using namespace mlir;

namespace mlir {
namespace transform {
#define GEN_PASS_DEF_INTERPRETERPASS
#include "mlir/Dialect/Transform/Transforms/Passes.h.inc"
} // namespace transform
} // namespace mlir

/// Returns the payload operation to be used as payload root:
///   - the operation nested under `passRoot` that has the given tag attribute,
///     must be unique;
///   - the `passRoot` itself if the tag is empty.
static Operation *findPayloadRoot(Operation *passRoot, StringRef tag) {
  // Fast return.
  if (tag.empty())
    return passRoot;

  // Walk to do a lookup.
  Operation *target = nullptr;
  auto tagAttrName = StringAttr::get(
      passRoot->getContext(), transform::TransformDialect::kTargetTagAttrName);
  WalkResult walkResult = passRoot->walk([&](Operation *op) {
    auto attr = op->getAttrOfType<StringAttr>(tagAttrName);
    if (!attr || attr.getValue() != tag)
      return WalkResult::advance();

    if (!target) {
      target = op;
      return WalkResult::advance();
    }

    InFlightDiagnostic diag = op->emitError()
                              << "repeated operation with the target tag '"
                              << tag << "'";
    diag.attachNote(target->getLoc()) << "previously seen operation";
    return WalkResult::interrupt();
  });

  if (!target) {
    passRoot->emitError()
        << "could not find the operation with transform.target_tag=\"" << tag
        << "\" attribute";
    return nullptr;
  }

  return walkResult.wasInterrupted() ? nullptr : target;
}

namespace {
class InterpreterPass
    : public transform::impl::InterpreterPassBase<InterpreterPass> {
  // Parses the pass arguments to bind trailing arguments of the entry point.
  std::optional<RaggedArray<transform::MappedValue>>
  parseArguments(Operation *payloadRoot) {
    MLIRContext *context = payloadRoot->getContext();

    SmallVector<SmallVector<transform::MappedValue>, 2> trailingBindings;
    trailingBindings.resize(debugBindTrailingArgs.size());

    // Construct lists of op names to match.
    SmallVector<std::optional<OperationName>> debugBindNames;
    debugBindNames.reserve(debugBindTrailingArgs.size());
    for (auto &&[position, nameString] :
         toolchain::enumerate(debugBindTrailingArgs)) {
      StringRef name = nameString;

      // Parse the integer literals.
      if (name.starts_with("#")) {
        debugBindNames.push_back(std::nullopt);
        StringRef lhs = "";
        StringRef rhs = name.drop_front();
        do {
          std::tie(lhs, rhs) = rhs.split(';');
          int64_t value;
          if (lhs.getAsInteger(10, value)) {
            emitError(UnknownLoc::get(context))
                << "couldn't parse integer pass argument " << name;
            return std::nullopt;
          }
          trailingBindings[position].push_back(
              Builder(context).getI64IntegerAttr(value));
        } while (!rhs.empty());
      } else if (name.starts_with("^")) {
        debugBindNames.emplace_back(OperationName(name.drop_front(), context));
      } else {
        debugBindNames.emplace_back(OperationName(name, context));
      }
    }

    // Collect operations or results for extra bindings.
    payloadRoot->walk([&](Operation *payload) {
      for (auto &&[position, name] : toolchain::enumerate(debugBindNames)) {
        if (!name || payload->getName() != *name)
          continue;

        if (StringRef(*std::next(debugBindTrailingArgs.begin(), position))
                .starts_with("^")) {
          toolchain::append_range(trailingBindings[position], payload->getResults());
        } else {
          trailingBindings[position].push_back(payload);
        }
      }
    });

    RaggedArray<transform::MappedValue> bindings;
    bindings.push_back(ArrayRef<Operation *>{payloadRoot});
    for (SmallVector<transform::MappedValue> &trailing : trailingBindings)
      bindings.push_back(std::move(trailing));
    return bindings;
  }

public:
  using Base::Base;

  void runOnOperation() override {
    MLIRContext *context = &getContext();
    ModuleOp transformModule =
        transform::detail::getPreloadedTransformModule(context);
    Operation *payloadRoot =
        findPayloadRoot(getOperation(), debugPayloadRootTag);
    if (!payloadRoot)
      return signalPassFailure();

    Operation *transformEntryPoint = transform::detail::findTransformEntryPoint(
        getOperation(), transformModule, entryPoint);
    if (!transformEntryPoint)
      return signalPassFailure();

    std::optional<RaggedArray<transform::MappedValue>> bindings =
        parseArguments(payloadRoot);
    if (!bindings)
      return signalPassFailure();
    if (failed(transform::applyTransformNamedSequence(
            *bindings,
            cast<transform::TransformOpInterface>(transformEntryPoint),
            transformModule,
            options.enableExpensiveChecks(!disableExpensiveChecks)))) {
      return signalPassFailure();
    }
  }

private:
  /// Transform interpreter options.
  transform::TransformOptions options;
};
} // namespace
