//===- FrontendAction.h -----------------------------------------*- C++ -*-===//
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
///
/// \file
/// Defines the flang::FrontendAction interface.
///
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_FRONTENDACTION_H
#define LANGUAGE_COMPABILITY_FRONTEND_FRONTENDACTION_H

#include "language/Compability/Frontend/FrontendOptions.h"
#include "toolchain/Support/Error.h"

namespace language::Compability::frontend {
class CompilerInstance;

/// Abstract base class for actions which can be performed by the frontend.
class FrontendAction {
  FrontendInputFile currentInput;
  CompilerInstance *instance;

protected:
  /// @name Implementation Action Interface
  /// @{

  /// Callback to run the program action, using the initialized
  /// compiler instance.
  virtual void executeAction() = 0;

  /// Callback at the end of processing a single input, to determine
  /// if the output files should be erased or not.
  ///
  /// By default it returns true if a compiler error occurred.
  virtual bool shouldEraseOutputFiles();

  /// Callback at the start of processing a single input.
  ///
  /// \return True on success; on failure ExecutionAction() and
  /// EndSourceFileAction() will not be called.
  virtual bool beginSourceFileAction() { return true; }

  /// @}

public:
  FrontendAction() : instance(nullptr) {}
  virtual ~FrontendAction() = default;

  /// @name Compiler Instance Access
  /// @{

  CompilerInstance &getInstance() const {
    assert(instance && "Compiler instance not registered!");
    return *instance;
  }

  void setInstance(CompilerInstance *value) { instance = value; }

  /// @}
  /// @name Current File Information
  /// @{

  const FrontendInputFile &getCurrentInput() const { return currentInput; }

  toolchain::StringRef getCurrentFile() const {
    assert(!currentInput.isEmpty() && "No current file!");
    return currentInput.getFile();
  }

  toolchain::StringRef getCurrentFileOrBufferName() const {
    assert(!currentInput.isEmpty() && "No current file!");
    return currentInput.isFile()
               ? currentInput.getFile()
               : currentInput.getBuffer()->getBufferIdentifier();
  }
  void setCurrentInput(const FrontendInputFile &currentIntput);

  /// @}
  /// @name Public Action Interface
  /// @{

  /// Prepare the action for processing the input file \p input.
  ///
  /// This is run after the options and frontend have been initialized,
  /// but prior to executing any per-file processing.
  /// \param ci - The compiler instance this action is being run from. The
  /// action may store and use this object.
  /// \param input - The input filename and kind.
  /// \return True on success; on failure the compilation of this file should
  bool beginSourceFile(CompilerInstance &ci, const FrontendInputFile &input);

  /// Run the action.
  toolchain::Error execute();

  /// Perform any per-file post processing, deallocate per-file
  /// objects, and run statistics and output file cleanup code.
  void endSourceFile();

  /// @}
protected:
  // Prescan the current input file. Return False if fatal errors are reported,
  // True otherwise.
  bool runPrescan();
  // Parse the current input file. Return False if fatal errors are reported,
  // True otherwise.
  bool runParse(bool emitMessages);
  // Run semantic checks for the current input file. Return False if fatal
  // errors are reported, True otherwise.
  bool runSemanticChecks();
  // Generate run-time type information for derived types. This may lead to new
  // semantic errors. Return False if fatal errors are reported, True
  // otherwise.
  bool generateRtTypeTables();

  // Report fatal semantic errors. Return True if present, false otherwise.
  bool reportFatalSemanticErrors();

  // Report fatal scanning errors. Return True if present, false otherwise.
  inline bool reportFatalScanningErrors() {
    return reportFatalErrors("Could not scan %0");
  }

  // Report fatal parsing errors. Return True if present, false otherwise
  inline bool reportFatalParsingErrors() {
    return reportFatalErrors("Could not parse %0");
  }

private:
  template <unsigned N>
  bool reportFatalErrors(const char (&message)[N]);
};

} // namespace language::Compability::frontend

#endif // FORTRAN_FRONTEND_FRONTENDACTION_H
