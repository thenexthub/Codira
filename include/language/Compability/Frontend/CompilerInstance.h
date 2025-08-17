//===-- CompilerInstance.h - Flang Compiler Instance ------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_FRONTEND_COMPILERINSTANCE_H
#define LANGUAGE_COMPABILITY_FRONTEND_COMPILERINSTANCE_H

#include "language/Compability/Frontend/CompilerInvocation.h"
#include "language/Compability/Frontend/FrontendAction.h"
#include "language/Compability/Frontend/ParserActions.h"
#include "language/Compability/Frontend/PreprocessorOptions.h"
#include "language/Compability/Semantics/runtime-type-info.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Support/StringOstream.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Target/TargetMachine.h"

namespace language::Compability::frontend {

/// Helper class for managing a single instance of the Flang compiler.
///
/// This class serves two purposes:
///  (1) It manages the various objects which are necessary to run the compiler
///  (2) It provides utility routines for constructing and manipulating the
///      common Flang objects.
///
/// The compiler instance generally owns the instance of all the objects that it
/// manages. However, clients can still share objects by manually setting the
/// object and retaking ownership prior to destroying the CompilerInstance.
///
/// The compiler instance is intended to simplify clients, but not to lock them
/// in to the compiler instance for everything. When possible, utility functions
/// come in two forms; a short form that reuses the CompilerInstance objects,
/// and a long form that takes explicit instances of any required objects.
class CompilerInstance {

  /// The options used in this compiler instance.
  std::shared_ptr<CompilerInvocation> invocation;

  /// Flang file  manager.
  std::shared_ptr<language::Compability::parser::AllSources> allSources;

  std::shared_ptr<language::Compability::parser::AllCookedSources> allCookedSources;

  std::shared_ptr<language::Compability::parser::Parsing> parsing;

  std::unique_ptr<language::Compability::semantics::Semantics> semantics;

  std::unique_ptr<language::Compability::semantics::RuntimeDerivedTypeTables> rtTyTables;

  std::unique_ptr<language::Compability::semantics::SemanticsContext> semaContext;

  std::unique_ptr<toolchain::TargetMachine> targetMachine;

  /// The stream for diagnostics from Semantics
  toolchain::raw_ostream *semaOutputStream = &toolchain::errs();

  /// The stream for diagnostics from Semantics if owned, otherwise nullptr.
  std::unique_ptr<toolchain::raw_ostream> ownedSemaOutputStream;

  /// The diagnostics engine instance.
  toolchain::IntrusiveRefCntPtr<language::Core::DiagnosticsEngine> diagnostics;

  /// Holds information about the output file.
  struct OutputFile {
    std::string filename;
    OutputFile(std::string inputFilename)
        : filename(std::move(inputFilename)) {}
  };

  /// The list of active output files.
  std::list<OutputFile> outputFiles;

  /// Holds the output stream provided by the user. Normally, users of
  /// CompilerInstance will call CreateOutputFile to obtain/create an output
  /// stream. If they want to provide their own output stream, this field will
  /// facilitate this. It is optional and will normally be just a nullptr.
  std::unique_ptr<toolchain::raw_pwrite_stream> outputStream;

  /// @name Timing
  /// Objects needed when timing is enabled.
  /// @{
  /// The timing manager.
  mlir::DefaultTimingManager timingMgr;

  /// The root of the timingScope. This will be reset in @ref executeAction if
  /// timers have been enabled.
  mlir::TimingScope timingScopeRoot;

  /// @name Timing stream
  /// The output streams to capture the timing. Three different streams are
  /// needed because the timing classes all work slightly differently. We create
  /// these streams so we have control over when and how the timing is
  /// displayed. Otherwise, the timing is only displayed when the corresponding
  /// managers/timers go out of scope.
  std::unique_ptr<language::Compability::support::string_ostream> timingStreamMLIR;
  std::unique_ptr<language::Compability::support::string_ostream> timingStreamLLVM;
  std::unique_ptr<language::Compability::support::string_ostream> timingStreamCodeGen;
  /// @}

public:
  explicit CompilerInstance();

  ~CompilerInstance();

  /// @name Compiler Invocation
  /// {

  CompilerInvocation &getInvocation() {
    assert(invocation && "Compiler instance has no invocation!");
    return *invocation;
  };

  /// Replace the current invocation.
  void setInvocation(std::shared_ptr<CompilerInvocation> value);

  /// }
  /// @name File manager
  /// {

  /// Return the current allSources.
  language::Compability::parser::AllSources &getAllSources() const { return *allSources; }

  bool hasAllSources() const { return allSources != nullptr; }

  parser::AllCookedSources &getAllCookedSources() {
    assert(allCookedSources && "Compiler instance has no AllCookedSources!");
    return *allCookedSources;
  };

  /// }
  /// @name Parser Operations
  /// {

  /// Return parsing to be used by Actions.
  language::Compability::parser::Parsing &getParsing() const { return *parsing; }

  /// }
  /// @name Semantic analysis
  /// {

  language::Compability::semantics::SemanticsContext &createNewSemanticsContext() {
    semaContext =
        getInvocation().getSemanticsCtx(*allCookedSources, getTargetMachine());
    return *semaContext;
  }

  language::Compability::semantics::SemanticsContext &getSemanticsContext() {
    return *semaContext;
  }
  const language::Compability::semantics::SemanticsContext &getSemanticsContext() const {
    return *semaContext;
  }

  /// Replace the current stream for verbose output.
  void setSemaOutputStream(toolchain::raw_ostream &value);

  /// Replace the current stream for verbose output.
  void setSemaOutputStream(std::unique_ptr<toolchain::raw_ostream> value);

  /// Get the current stream for verbose output.
  toolchain::raw_ostream &getSemaOutputStream() { return *semaOutputStream; }

  language::Compability::semantics::Semantics &getSemantics() { return *semantics; }
  const language::Compability::semantics::Semantics &getSemantics() const {
    return *semantics;
  }

  void setSemantics(std::unique_ptr<language::Compability::semantics::Semantics> sema) {
    semantics = std::move(sema);
  }

  void setRtTyTables(
      std::unique_ptr<language::Compability::semantics::RuntimeDerivedTypeTables> tables) {
    rtTyTables = std::move(tables);
  }

  language::Compability::semantics::RuntimeDerivedTypeTables &getRtTyTables() {
    assert(rtTyTables && "Missing runtime derived type tables!");
    return *rtTyTables;
  }

  /// }
  /// @name High-Level Operations
  /// {

  /// Execute the provided action against the compiler's
  /// CompilerInvocation object.
  /// \param act - The action to execute.
  /// \return - True on success.
  bool executeAction(FrontendAction &act);

  /// }
  /// @name Forwarding Methods
  /// {

  language::Core::DiagnosticOptions &getDiagnosticOpts() {
    return invocation->getDiagnosticOpts();
  }
  const language::Core::DiagnosticOptions &getDiagnosticOpts() const {
    return invocation->getDiagnosticOpts();
  }

  FrontendOptions &getFrontendOpts() { return invocation->getFrontendOpts(); }
  const FrontendOptions &getFrontendOpts() const {
    return invocation->getFrontendOpts();
  }

  PreprocessorOptions &preprocessorOpts() {
    return invocation->getPreprocessorOpts();
  }
  const PreprocessorOptions &preprocessorOpts() const {
    return invocation->getPreprocessorOpts();
  }

  /// }
  /// @name Diagnostics Engine
  /// {

  bool hasDiagnostics() const { return diagnostics != nullptr; }

  /// Get the current diagnostics engine.
  language::Core::DiagnosticsEngine &getDiagnostics() const {
    assert(diagnostics && "Compiler instance has no diagnostics!");
    return *diagnostics;
  }

  language::Core::DiagnosticConsumer &getDiagnosticClient() const {
    assert(diagnostics && diagnostics->getClient() &&
           "Compiler instance has no diagnostic client!");
    return *diagnostics->getClient();
  }

  /// {
  /// @name Output Files
  /// {

  /// Clear the output file list.
  void clearOutputFiles(bool eraseFiles);

  /// Create the default output file (based on the invocation's options) and
  /// add it to the list of tracked output files. If the name of the output
  /// file is not provided, it will be derived from the input file.
  ///
  /// \param binary     The mode to open the file in.
  /// \param baseInput  If the invocation contains no output file name (i.e.
  ///                   outputFile in FrontendOptions is empty), the input path
  ///                   name to use for deriving the output path.
  /// \param extension  The extension to use for output names derived from
  ///                   \p baseInput.
  /// \return           Null on error, ostream for the output file otherwise
  std::unique_ptr<toolchain::raw_pwrite_stream>
  createDefaultOutputFile(bool binary = true, toolchain::StringRef baseInput = "",
                          toolchain::StringRef extension = "");

  /// {
  /// @name Target Machine
  /// {

  /// Get the target machine.
  const toolchain::TargetMachine &getTargetMachine() const {
    assert(targetMachine && "target machine was not set");
    return *targetMachine;
  }
  toolchain::TargetMachine &getTargetMachine() {
    assert(targetMachine && "target machine was not set");
    return *targetMachine;
  }

  /// Sets up LLVM's TargetMachine.
  bool setUpTargetMachine();

  /// Produces the string which represents target feature
  std::string getTargetFeatures();

  /// {
  /// @name Timing
  /// @{
  bool isTimingEnabled() const { return timingMgr.isEnabled(); }

  mlir::DefaultTimingManager &getTimingManager() { return timingMgr; }
  const mlir::DefaultTimingManager &getTimingManager() const {
    return timingMgr;
  }

  mlir::TimingScope &getTimingScopeRoot() { return timingScopeRoot; }
  const mlir::TimingScope &getTimingScopeRoot() const {
    return timingScopeRoot;
  }

  /// Get the timing stream for the MLIR pass manager.
  toolchain::raw_ostream &getTimingStreamMLIR() {
    assert(timingStreamMLIR && "Timing stream for MLIR was not set");
    return *timingStreamMLIR;
  }

  /// Get the timing stream for the new LLVM pass manager.
  toolchain::raw_ostream &getTimingStreamLLVM() {
    assert(timingStreamLLVM && "Timing stream for LLVM was not set");
    return *timingStreamLLVM;
  }

  /// Get the timing stream fro the legacy LLVM pass manager.
  /// NOTE: If the codegen is updated to use the new pass manager, this should
  /// no longer be needed.
  toolchain::raw_ostream &getTimingStreamCodeGen() {
    assert(timingStreamCodeGen && "Timing stream for codegen was not set");
    return *timingStreamCodeGen;
  }
  /// @}

private:
  /// Create a new output file
  ///
  /// \param outputPath   The path to the output file.
  /// \param binary       The mode to open the file in.
  /// \return             Null on error, ostream for the output file otherwise
  toolchain::Expected<std::unique_ptr<toolchain::raw_pwrite_stream>>
  createOutputFileImpl(toolchain::StringRef outputPath, bool binary);

public:
  /// }
  /// @name Construction Utility Methods
  /// {

  /// Create a DiagnosticsEngine object
  ///
  /// If no diagnostic client is provided, this method creates a
  /// DiagnosticConsumer that is owned by the returned diagnostic object. If
  /// using directly the caller is responsible for releasing the returned
  /// DiagnosticsEngine's client eventually.
  ///
  /// \param opts - The diagnostic options; note that the created text
  /// diagnostic object contains a reference to these options.
  ///
  /// \param client - If non-NULL, a diagnostic client that will be attached to
  /// (and optionally, depending on /p shouldOwnClient, owned by) the returned
  /// DiagnosticsEngine object.
  ///
  /// \return The new object on success, or null on failure.
  static language::Core::IntrusiveRefCntPtr<language::Core::DiagnosticsEngine>
  createDiagnostics(language::Core::DiagnosticOptions &opts,
                    language::Core::DiagnosticConsumer *client = nullptr,
                    bool shouldOwnClient = true);
  void createDiagnostics(language::Core::DiagnosticConsumer *client = nullptr,
                         bool shouldOwnClient = true);

  /// }
  /// @name Output Stream Methods
  /// {
  void setOutputStream(std::unique_ptr<toolchain::raw_pwrite_stream> outStream) {
    outputStream = std::move(outStream);
  }

  bool isOutputStreamNull() { return (outputStream == nullptr); }

  // Allow the frontend compiler to write in the output stream.
  void writeOutputStream(const std::string &message) {
    *outputStream << message;
  }

  /// Get the user specified output stream.
  toolchain::raw_pwrite_stream &getOutputStream() {
    assert(outputStream &&
           "Compiler instance has no user-specified output stream!");
    return *outputStream;
  }
};

} // end namespace language::Compability::frontend
#endif // FORTRAN_FRONTEND_COMPILERINSTANCE_H
