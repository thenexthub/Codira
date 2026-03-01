//===- Translation.cpp - Translation registry -----------------------------===//
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
// Definitions of the translation registry.
//
//===----------------------------------------------------------------------===//

#include "mlir/Tools/mlir-translate/Translation.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/Verifier.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Tools/ParseUtilities.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/SourceMgr.h"
#include <optional>

using namespace mlir;

//===----------------------------------------------------------------------===//
// Translation CommandLine Options
//===----------------------------------------------------------------------===//

struct TranslationOptions {
  toolchain::cl::opt<bool> noImplicitModule{
      "no-implicit-module",
      toolchain::cl::desc("Disable the parsing of an implicit top-level module op"),
      toolchain::cl::init(false)};
};

static toolchain::ManagedStatic<TranslationOptions> clOptions;

void mlir::registerTranslationCLOptions() { *clOptions; }

//===----------------------------------------------------------------------===//
// Translation Registry
//===----------------------------------------------------------------------===//

/// Get the mutable static map between registered file-to-file MLIR
/// translations.
static toolchain::StringMap<Translation> &getTranslationRegistry() {
  static toolchain::StringMap<Translation> translationBundle;
  return translationBundle;
}

/// Register the given translation.
static void registerTranslation(StringRef name, StringRef description,
                                std::optional<toolchain::Align> inputAlignment,
                                const TranslateFunction &function) {
  auto &registry = getTranslationRegistry();
  if (registry.count(name))
    toolchain::report_fatal_error(
        "Attempting to overwrite an existing <file-to-file> function");
  assert(function &&
         "Attempting to register an empty translate <file-to-file> function");
  registry[name] = Translation(function, description, inputAlignment);
}

TranslateRegistration::TranslateRegistration(
    StringRef name, StringRef description, const TranslateFunction &function) {
  registerTranslation(name, description, /*inputAlignment=*/std::nullopt,
                      function);
}

//===----------------------------------------------------------------------===//
// Translation to MLIR
//===----------------------------------------------------------------------===//

// Puts `function` into the to-MLIR translation registry unless there is already
// a function registered for the same name.
static void registerTranslateToMLIRFunction(
    StringRef name, StringRef description,
    const DialectRegistrationFunction &dialectRegistration,
    std::optional<toolchain::Align> inputAlignment,
    const TranslateSourceMgrToMLIRFunction &function) {
  auto wrappedFn = [function, dialectRegistration](
                       const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
                       raw_ostream &output, MLIRContext *context) {
    DialectRegistry registry;
    dialectRegistration(registry);
    context->appendDialectRegistry(registry);
    OwningOpRef<Operation *> op = function(sourceMgr, context);
    if (!op || failed(verify(*op)))
      return failure();
    op.get()->print(output);
    return success();
  };
  registerTranslation(name, description, inputAlignment, wrappedFn);
}

TranslateToMLIRRegistration::TranslateToMLIRRegistration(
    StringRef name, StringRef description,
    const TranslateSourceMgrToMLIRFunction &function,
    const DialectRegistrationFunction &dialectRegistration,
    std::optional<toolchain::Align> inputAlignment) {
  registerTranslateToMLIRFunction(name, description, dialectRegistration,
                                  inputAlignment, function);
}
TranslateToMLIRRegistration::TranslateToMLIRRegistration(
    StringRef name, StringRef description,
    const TranslateRawSourceMgrToMLIRFunction &function,
    const DialectRegistrationFunction &dialectRegistration,
    std::optional<toolchain::Align> inputAlignment) {
  registerTranslateToMLIRFunction(
      name, description, dialectRegistration, inputAlignment,
      [function](const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
                 MLIRContext *ctx) { return function(*sourceMgr, ctx); });
}
/// Wraps `function` with a lambda that extracts a StringRef from a source
/// manager and registers the wrapper lambda as a to-MLIR conversion.
TranslateToMLIRRegistration::TranslateToMLIRRegistration(
    StringRef name, StringRef description,
    const TranslateStringRefToMLIRFunction &function,
    const DialectRegistrationFunction &dialectRegistration,
    std::optional<toolchain::Align> inputAlignment) {
  registerTranslateToMLIRFunction(
      name, description, dialectRegistration, inputAlignment,
      [function](const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
                 MLIRContext *ctx) {
        const toolchain::MemoryBuffer *buffer =
            sourceMgr->getMemoryBuffer(sourceMgr->getMainFileID());
        return function(buffer->getBuffer(), ctx);
      });
}

//===----------------------------------------------------------------------===//
// Translation from MLIR
//===----------------------------------------------------------------------===//

TranslateFromMLIRRegistration::TranslateFromMLIRRegistration(
    StringRef name, StringRef description,
    const TranslateFromMLIRFunction &function,
    const DialectRegistrationFunction &dialectRegistration) {
  registerTranslation(
      name, description, /*inputAlignment=*/std::nullopt,
      [function,
       dialectRegistration](const std::shared_ptr<toolchain::SourceMgr> &sourceMgr,
                            raw_ostream &output, MLIRContext *context) {
        DialectRegistry registry;
        dialectRegistration(registry);
        context->appendDialectRegistry(registry);
        bool implicitModule =
            (!clOptions.isConstructed() || !clOptions->noImplicitModule);
        OwningOpRef<Operation *> op =
            parseSourceFileForTool(sourceMgr, context, implicitModule);
        if (!op || failed(verify(*op)))
          return failure();
        return function(op.get(), output);
      });
}

//===----------------------------------------------------------------------===//
// Translation Parser
//===----------------------------------------------------------------------===//

TranslationParser::TranslationParser(toolchain::cl::Option &opt)
    : toolchain::cl::parser<const Translation *>(opt) {
  for (const auto &kv : getTranslationRegistry())
    addLiteralOption(kv.first(), &kv.second, kv.second.getDescription());
}

void TranslationParser::printOptionInfo(const toolchain::cl::Option &o,
                                        size_t globalWidth) const {
  TranslationParser *tp = const_cast<TranslationParser *>(this);
  toolchain::array_pod_sort(tp->Values.begin(), tp->Values.end(),
                       [](const TranslationParser::OptionInfo *lhs,
                          const TranslationParser::OptionInfo *rhs) {
                         return lhs->Name.compare(rhs->Name);
                       });
  toolchain::cl::parser<const Translation *>::printOptionInfo(o, globalWidth);
}
