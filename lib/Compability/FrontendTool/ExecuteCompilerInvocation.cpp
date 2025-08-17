//===--- ExecuteCompilerInvocation.cpp ------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file holds ExecuteCompilerInvocation(). It is split into its own file to
// minimize the impact of pulling in essentially everything else in Flang.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/CompilerInstance.h"
#include "language/Compability/Frontend/FrontendActions.h"
#include "language/Compability/Frontend/FrontendPluginRegistry.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/BuryPointer.h"
#include "toolchain/Support/CommandLine.h"

namespace language::Compability::frontend {

static std::unique_ptr<FrontendAction>
createFrontendAction(CompilerInstance &ci) {

  switch (ci.getFrontendOpts().programAction) {
  case InputOutputTest:
    return std::make_unique<InputOutputTestAction>();
  case PrintPreprocessedInput:
    return std::make_unique<PrintPreprocessedAction>();
  case ParseSyntaxOnly:
    return std::make_unique<ParseSyntaxOnlyAction>();
  case EmitFIR:
    return std::make_unique<EmitFIRAction>();
  case EmitHLFIR:
    return std::make_unique<EmitHLFIRAction>();
  case EmitLLVM:
    return std::make_unique<EmitLLVMAction>();
  case EmitLLVMBitcode:
    return std::make_unique<EmitLLVMBitcodeAction>();
  case EmitObj:
    return std::make_unique<EmitObjAction>();
  case EmitAssembly:
    return std::make_unique<EmitAssemblyAction>();
  case DebugUnparse:
    return std::make_unique<DebugUnparseAction>();
  case DebugUnparseNoSema:
    return std::make_unique<DebugUnparseNoSemaAction>();
  case DebugUnparseWithSymbols:
    return std::make_unique<DebugUnparseWithSymbolsAction>();
  case DebugUnparseWithModules:
    return std::make_unique<DebugUnparseWithModulesAction>();
  case DebugDumpSymbols:
    return std::make_unique<DebugDumpSymbolsAction>();
  case DebugDumpParseTree:
    return std::make_unique<DebugDumpParseTreeAction>();
  case DebugDumpPFT:
    return std::make_unique<DebugDumpPFTAction>();
  case DebugDumpParseTreeNoSema:
    return std::make_unique<DebugDumpParseTreeNoSemaAction>();
  case DebugDumpAll:
    return std::make_unique<DebugDumpAllAction>();
  case DebugDumpProvenance:
    return std::make_unique<DebugDumpProvenanceAction>();
  case DebugDumpParsingLog:
    return std::make_unique<DebugDumpParsingLogAction>();
  case DebugMeasureParseTree:
    return std::make_unique<DebugMeasureParseTreeAction>();
  case DebugPreFIRTree:
    return std::make_unique<DebugPreFIRTreeAction>();
  case GetDefinition:
    return std::make_unique<GetDefinitionAction>();
  case GetSymbolsSources:
    return std::make_unique<GetSymbolsSourcesAction>();
  case InitOnly:
    return std::make_unique<InitOnlyAction>();
  case PluginAction: {
    for (const FrontendPluginRegistry::entry &plugin :
         FrontendPluginRegistry::entries()) {
      if (plugin.getName() == ci.getFrontendOpts().actionName) {
        std::unique_ptr<PluginParseTreeAction> p(plugin.instantiate());
        return std::move(p);
      }
    }
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "unable to find plugin '%0'");
    ci.getDiagnostics().Report(diagID) << ci.getFrontendOpts().actionName;
    return nullptr;
  }
  }

  toolchain_unreachable("Invalid program action!");
}

static void emitUnknownDiagWarning(language::Core::DiagnosticsEngine &diags,
                                   language::Core::diag::Flavor flavor,
                                   toolchain::StringRef prefix,
                                   toolchain::StringRef opt) {
  toolchain::StringRef suggestion =
      language::Core::DiagnosticIDs::getNearestOption(flavor, opt);
  diags.Report(language::Core::diag::warn_unknown_diag_option)
      << (flavor == language::Core::diag::Flavor::WarningOrError ? 0 : 1)
      << (prefix.str() += std::string(opt)) << !suggestion.empty()
      << (prefix.str() += std::string(suggestion));
}

// Remarks are ignored by default in Diagnostic.td, hence, we have to
// enable them here before execution. Clang follows same idea using
// ProcessWarningOptions in Warnings.cpp
// This function is also responsible for emitting early warnings for
// invalid -R options.
static void
updateDiagEngineForOptRemarks(language::Core::DiagnosticsEngine &diagsEng,
                              const language::Core::DiagnosticOptions &opts) {
  toolchain::SmallVector<language::Core::diag::kind, 10> diags;
  const toolchain::IntrusiveRefCntPtr<language::Core::DiagnosticIDs> diagIDs =
      diagsEng.getDiagnosticIDs();

  for (unsigned i = 0; i < opts.Remarks.size(); i++) {
    toolchain::StringRef remarkOpt = opts.Remarks[i];
    const auto flavor = language::Core::diag::Flavor::Remark;

    // Check to see if this opt starts with "no-", if so, this is a
    // negative form of the option.
    bool isPositive = !remarkOpt.starts_with("no-");
    if (!isPositive)
      remarkOpt = remarkOpt.substr(3);

    // Verify that this is a valid optimization remarks option
    if (diagIDs->getDiagnosticsInGroup(flavor, remarkOpt, diags)) {
      emitUnknownDiagWarning(diagsEng, flavor, isPositive ? "-R" : "-Rno-",
                             remarkOpt);
      return;
    }

    diagsEng.setSeverityForGroup(flavor, remarkOpt,
                                 isPositive ? language::Core::diag::Severity::Remark
                                            : language::Core::diag::Severity::Ignored);
  }
}

bool executeCompilerInvocation(CompilerInstance *flang) {
  // Honor -help.
  if (flang->getFrontendOpts().showHelp) {
    language::Core::driver::getDriverOptTable().printHelp(
        toolchain::outs(), "flang -fc1 [options] file...", "LLVM 'Flang' Compiler",
        /*ShowHidden=*/false, /*ShowAllAliases=*/false,
        toolchain::opt::Visibility(language::Core::driver::options::FC1Option));
    return true;
  }

  // Honor -version.
  if (flang->getFrontendOpts().showVersion) {
    toolchain::cl::PrintVersionMessage();
    return true;
  }

  // Load any requested plugins.
  for (const std::string &path : flang->getFrontendOpts().plugins) {
    std::string error;
    if (toolchain::sys::DynamicLibrary::LoadLibraryPermanently(path.c_str(),
                                                          &error)) {
      unsigned diagID = flang->getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "unable to load plugin '%0': '%1'");
      flang->getDiagnostics().Report(diagID) << path << error;
    }
  }

  // Honor -mtoolchain. This should happen AFTER plugins have been loaded!
  if (!flang->getFrontendOpts().toolchainArgs.empty()) {
    unsigned numArgs = flang->getFrontendOpts().toolchainArgs.size();
    auto args = std::make_unique<const char *[]>(numArgs + 2);
    args[0] = "flang (LLVM option parsing)";

    for (unsigned i = 0; i != numArgs; ++i)
      args[i + 1] = flang->getFrontendOpts().toolchainArgs[i].c_str();

    args[numArgs + 1] = nullptr;
    toolchain::cl::ParseCommandLineOptions(numArgs + 1, args.get());
  }

  // Honor -mmlir. This should happen AFTER plugins have been loaded!
  if (!flang->getFrontendOpts().mlirArgs.empty()) {
    mlir::registerMLIRContextCLOptions();
    mlir::registerPassManagerCLOptions();
    mlir::registerAsmPrinterCLOptions();
    unsigned numArgs = flang->getFrontendOpts().mlirArgs.size();
    auto args = std::make_unique<const char *[]>(numArgs + 2);
    args[0] = "flang (MLIR option parsing)";

    for (unsigned i = 0; i != numArgs; ++i)
      args[i + 1] = flang->getFrontendOpts().mlirArgs[i].c_str();

    args[numArgs + 1] = nullptr;
    toolchain::cl::ParseCommandLineOptions(numArgs + 1, args.get());
  }

  // If there were errors in processing arguments, don't do anything else.
  if (flang->getDiagnostics().hasErrorOccurred()) {
    return false;
  }

  updateDiagEngineForOptRemarks(flang->getDiagnostics(),
                                flang->getDiagnosticOpts());

  // Create and execute the frontend action.
  std::unique_ptr<FrontendAction> act(createFrontendAction(*flang));
  if (!act)
    return false;

  bool success = flang->executeAction(*act);
  return success;
}

} // namespace language::Compability::frontend
