//===--- FrontendAction.cpp -----------------------------------------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/FrontendAction.h"
#include "language/Compability/Frontend/CompilerInstance.h"
#include "language/Compability/Frontend/FrontendActions.h"
#include "language/Compability/Frontend/FrontendOptions.h"
#include "language/Compability/Frontend/FrontendPluginRegistry.h"
#include "language/Compability/Parser/parsing.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language::Compability::frontend;

LLVM_INSTANTIATE_REGISTRY(FrontendPluginRegistry)

void FrontendAction::setCurrentInput(const FrontendInputFile &input) {
  this->currentInput = input;
}

// Call this method if BeginSourceFile fails.
// Deallocate compiler instance, input and output descriptors
static void beginSourceFileCleanUp(FrontendAction &fa, CompilerInstance &ci) {
  ci.clearOutputFiles(/*EraseFiles=*/true);
  fa.setCurrentInput(FrontendInputFile());
  fa.setInstance(nullptr);
}

bool FrontendAction::beginSourceFile(CompilerInstance &ci,
                                     const FrontendInputFile &realInput) {

  FrontendInputFile input(realInput);

  // Return immediately if the input file does not exist or is not a file. Note
  // that we cannot check this for input from stdin.
  if (input.getFile() != "-") {
    if (!toolchain::sys::fs::is_regular_file(input.getFile())) {
      // Create an diagnostic ID to report
      unsigned diagID;
      if (toolchain::vfs::getRealFileSystem()->exists(input.getFile())) {
        ci.getDiagnostics().Report(language::Core::diag::err_fe_error_reading)
            << input.getFile() << "not a regular file";
        diagID = ci.getDiagnostics().getCustomDiagID(
            language::Core::DiagnosticsEngine::Error, "%0 is not a regular file");
      } else {
        diagID = ci.getDiagnostics().getCustomDiagID(
            language::Core::DiagnosticsEngine::Error, "%0 does not exist");
      }

      // Report the diagnostic and return
      ci.getDiagnostics().Report(diagID) << input.getFile();
      beginSourceFileCleanUp(*this, ci);
      return false;
    }
  }

  assert(!instance && "Already processing a source file!");
  assert(!realInput.isEmpty() && "Unexpected empty filename!");
  setCurrentInput(realInput);
  setInstance(&ci);

  if (!ci.hasAllSources()) {
    beginSourceFileCleanUp(*this, ci);
    return false;
  }

  auto &invoc = ci.getInvocation();

  // Include command-line and predefined preprocessor macros. Use either:
  //  * `-cpp/-nocpp`, or
  //  * the file extension (if the user didn't express any preference)
  // to decide whether to include them or not.
  if ((invoc.getPreprocessorOpts().macrosFlag == PPMacrosFlag::Include) ||
      (invoc.getPreprocessorOpts().macrosFlag == PPMacrosFlag::Unknown &&
       getCurrentInput().getMustBePreprocessed())) {
    invoc.setDefaultPredefinitions();
    invoc.collectMacroDefinitions();
  }

  if (!invoc.getFortranOpts().features.IsEnabled(
          language::Compability::common::LanguageFeature::CUDA)) {
    // Enable CUDA Fortran if source file is *.cuf/*.CUF and not already
    // enabled.
    invoc.getFortranOpts().features.Enable(
        language::Compability::common::LanguageFeature::CUDA,
        getCurrentInput().getIsCUDAFortran());
  }

  // -fpreprocess-include-lines
  invoc.getFortranOpts().expandIncludeLinesInPreprocessedOutput =
      invoc.getPreprocessorOpts().preprocessIncludeLines;

  // Decide between fixed and free form (if the user didn't express any
  // preference, use the file extension to decide)
  if (invoc.getFrontendOpts().fortranForm == FortranForm::Unknown) {
    invoc.getFortranOpts().isFixedForm = getCurrentInput().getIsFixedForm();
  }

  if (!beginSourceFileAction()) {
    beginSourceFileCleanUp(*this, ci);
    return false;
  }

  return true;
}

bool FrontendAction::shouldEraseOutputFiles() {
  return getInstance().getDiagnostics().hasErrorOccurred();
}

toolchain::Error FrontendAction::execute() {
  executeAction();

  return toolchain::Error::success();
}

void FrontendAction::endSourceFile() {
  CompilerInstance &ci = getInstance();

  // Cleanup the output streams, and erase the output files if instructed by the
  // FrontendAction.
  ci.clearOutputFiles(/*EraseFiles=*/shouldEraseOutputFiles());

  setInstance(nullptr);
  setCurrentInput(FrontendInputFile());
}

bool FrontendAction::runPrescan() {
  CompilerInstance &ci = this->getInstance();
  std::string currentInputPath{getCurrentFileOrBufferName()};
  language::Compability::parser::Options parserOptions = ci.getInvocation().getFortranOpts();

  if (ci.getInvocation().getFrontendOpts().fortranForm ==
      FortranForm::Unknown) {
    // Switch between fixed and free form format based on the input file
    // extension.
    //
    // Ideally we should have all Fortran options set before entering this
    // method (i.e. before processing any specific input files). However, we
    // can't decide between fixed and free form based on the file extension
    // earlier than this.
    parserOptions.isFixedForm = getCurrentInput().getIsFixedForm();
  }

  // Prescan. In case of failure, report and return.
  ci.getParsing().Prescan(currentInputPath, parserOptions);

  return !reportFatalScanningErrors();
}

bool FrontendAction::runParse(bool emitMessages) {
  CompilerInstance &ci = this->getInstance();

  // Parse. In case of failure, report and return.
  ci.getParsing().Parse(toolchain::outs());

  if (reportFatalParsingErrors()) {
    return false;
  }

  if (emitMessages) {
    // Report any non-fatal diagnostics from getParsing now rather than
    // combining them with messages from semantics.
    const common::LanguageFeatureControl &features{
        ci.getInvocation().getFortranOpts().features};
    // Default maxErrors here because none are fatal.
    ci.getParsing().messages().Emit(toolchain::errs(), ci.getAllCookedSources(),
                                    /*echoSourceLine=*/true, &features);
  }
  return true;
}

bool FrontendAction::runSemanticChecks() {
  CompilerInstance &ci = this->getInstance();
  std::optional<parser::Program> &parseTree{ci.getParsing().parseTree()};
  assert(parseTree && "Cannot run semantic checks without a parse tree!");

  // Transfer any pending non-fatal messages from parsing to semantics
  // so that they are merged and all printed in order.
  auto &semanticsCtx{ci.createNewSemanticsContext()};
  semanticsCtx.messages().Annex(std::move(ci.getParsing().messages()));
  semanticsCtx.set_debugModuleWriter(ci.getInvocation().getDebugModuleDir());

  // Prepare semantics
  ci.setSemantics(std::make_unique<language::Compability::semantics::Semantics>(semanticsCtx,
                                                                  *parseTree));
  auto &semantics = ci.getSemantics();
  semantics.set_hermeticModuleFileOutput(
      ci.getInvocation().getHermeticModuleFileOutput());

  // Run semantic checks
  semantics.Perform();

  if (reportFatalSemanticErrors()) {
    return false;
  }

  // Report the diagnostics from parsing and the semantic checks
  semantics.EmitMessages(ci.getSemaOutputStream());

  return true;
}

bool FrontendAction::generateRtTypeTables() {
  getInstance().setRtTyTables(
      std::make_unique<language::Compability::semantics::RuntimeDerivedTypeTables>(
          BuildRuntimeDerivedTypeTables(getInstance().getSemanticsContext())));

  // The runtime derived type information table builder may find additional
  // semantic errors. Report them.
  if (reportFatalSemanticErrors()) {
    return false;
  }

  return true;
}

template <unsigned N>
bool FrontendAction::reportFatalErrors(const char (&message)[N]) {
  const common::LanguageFeatureControl &features{
      instance->getInvocation().getFortranOpts().features};
  const size_t maxErrors{instance->getInvocation().getMaxErrors()};
  const bool warningsAreErrors{instance->getInvocation().getWarnAsErr()};
  if (instance->getParsing().messages().AnyFatalError(warningsAreErrors)) {
    const unsigned diagID = instance->getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, message);
    instance->getDiagnostics().Report(diagID) << getCurrentFileOrBufferName();
    instance->getParsing().messages().Emit(
        toolchain::errs(), instance->getAllCookedSources(),
        /*echoSourceLines=*/true, &features, maxErrors, warningsAreErrors);
    return true;
  }
  if (instance->getParsing().parseTree().has_value() &&
      !instance->getParsing().consumedWholeFile()) {
    // Parsing failed without error.
    const unsigned diagID = instance->getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, message);
    instance->getDiagnostics().Report(diagID) << getCurrentFileOrBufferName();
    instance->getParsing().messages().Emit(
        toolchain::errs(), instance->getAllCookedSources(),
        /*echoSourceLine=*/true, &features, maxErrors, warningsAreErrors);
    instance->getParsing().EmitMessage(
        toolchain::errs(), instance->getParsing().finalRestingPlace(),
        "parser FAIL (final position)", "error: ", toolchain::raw_ostream::RED);
    return true;
  }
  return false;
}

bool FrontendAction::reportFatalSemanticErrors() {
  auto &diags = instance->getDiagnostics();
  auto &sema = instance->getSemantics();

  if (instance->getSemantics().AnyFatalError()) {
    unsigned diagID = diags.getCustomDiagID(language::Core::DiagnosticsEngine::Error,
                                            "Semantic errors in %0");
    diags.Report(diagID) << getCurrentFileOrBufferName();
    sema.EmitMessages(instance->getSemaOutputStream());

    return true;
  }

  return false;
}
