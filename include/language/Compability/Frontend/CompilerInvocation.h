//===- CompilerInvocation.h - Compiler Invocation Helper Data ---*- C -*-===//
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

#ifndef LANGUAGE_COMPABILITY_FRONTEND_COMPILERINVOCATION_H
#define LANGUAGE_COMPABILITY_FRONTEND_COMPILERINVOCATION_H

#include "language/Compability/Frontend/CodeGenOptions.h"
#include "language/Compability/Frontend/FrontendOptions.h"
#include "language/Compability/Frontend/PreprocessorOptions.h"
#include "language/Compability/Frontend/TargetOptions.h"
#include "language/Compability/Lower/LoweringOptions.h"
#include "language/Compability/Parser/options.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Support/LangOptions.h"
#include "mlir/Support/Timing.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "toolchain/Option/ArgList.h"
#include <memory>

namespace toolchain {
class TargetMachine;
} // namespace toolchain

namespace language::Compability::frontend {

/// Fill out Opts based on the options given in Args.
///
/// When errors are encountered, return false and, if Diags is non-null,
/// report the error(s).
bool parseDiagnosticArgs(language::Core::DiagnosticOptions &opts,
                         toolchain::opt::ArgList &args);

class CompilerInvocationBase {
public:
  /// Options controlling the diagnostic engine.
  std::shared_ptr<language::Core::DiagnosticOptions> diagnosticOpts;
  /// Options for the preprocessor.
  std::shared_ptr<language::Compability::frontend::PreprocessorOptions> preprocessorOpts;

  CompilerInvocationBase();
  CompilerInvocationBase(const CompilerInvocationBase &x);
  ~CompilerInvocationBase();

  language::Core::DiagnosticOptions &getDiagnosticOpts() {
    return *diagnosticOpts.get();
  }
  const language::Core::DiagnosticOptions &getDiagnosticOpts() const {
    return *diagnosticOpts.get();
  }

  PreprocessorOptions &getPreprocessorOpts() { return *preprocessorOpts; }
  const PreprocessorOptions &getPreprocessorOpts() const {
    return *preprocessorOpts;
  }
};

class CompilerInvocation : public CompilerInvocationBase {
  /// Options for the frontend driver
  // TODO: Merge with or translate to parserOpts_. We shouldn't need two sets of
  // options.
  FrontendOptions frontendOpts;

  /// Options for Flang parser
  // TODO: Merge with or translate to frontendOpts. We shouldn't need two sets
  // of options.
  language::Compability::parser::Options parserOpts;

  /// Options controlling lowering.
  language::Compability::lower::LoweringOptions loweringOpts;

  /// Options controlling the target.
  language::Compability::frontend::TargetOptions targetOpts;

  /// Options controlling IRgen and the backend.
  language::Compability::frontend::CodeGenOptions codeGenOpts;

  /// Options controlling language dialect.
  language::Compability::common::LangOptions langOpts;

  // The original invocation of the compiler driver.
  // This string will be set as the return value from the COMPILER_OPTIONS
  // intrinsic of iso_fortran_env.
  std::string allCompilerInvocOpts;

  /// Semantic options
  // TODO: Merge with or translate to frontendOpts. We shouldn't need two sets
  // of options.
  std::string moduleDir = ".";

  std::string moduleFileSuffix = ".mod";

  bool debugModuleDir = false;
  bool hermeticModuleFileOutput = false;

  // Executable name
  const char *argv0;

  /// This flag controls the unparsing and is used to decide whether to print
  /// out the semantically analyzed version of an object or expression or the
  /// plain version that does not include any information from semantic
  /// analysis.
  bool useAnalyzedObjectsForUnparse = true;

  // Fortran Dialect options
  language::Compability::common::IntrinsicTypeDefaultKinds defaultKinds;

  // Fortran Error options
  size_t maxErrors = 0;
  bool warnAsErr = false;
  // Fortran Warning options
  bool enableConformanceChecks = false;
  bool enableUsageChecks = false;
  bool disableWarnings = false;

  /// Used in e.g. unparsing to dump the analyzed rather than the original
  /// parse-tree objects.
  language::Compability::parser::AnalyzedObjectsAsFortran asFortran{
      [](toolchain::raw_ostream &o, const language::Compability::evaluate::GenericExprWrapper &x) {
        if (x.v) {
          x.v->AsFortran(o);
        } else {
          o << "(bad expression)";
        }
      },
      [](toolchain::raw_ostream &o,
         const language::Compability::evaluate::GenericAssignmentWrapper &x) {
        if (x.v) {
          x.v->AsFortran(o);
        } else {
          o << "(bad assignment)";
        }
      },
      [](toolchain::raw_ostream &o, const language::Compability::evaluate::ProcedureRef &x) {
        x.AsFortran(o << "CALL ");
      },
  };

  /// Whether to time the invocation. Set when -ftime-report or -ftime-report=
  /// is enabled.
  bool enableTimers;

public:
  CompilerInvocation() = default;

  FrontendOptions &getFrontendOpts() { return frontendOpts; }
  const FrontendOptions &getFrontendOpts() const { return frontendOpts; }

  language::Compability::parser::Options &getFortranOpts() { return parserOpts; }
  const language::Compability::parser::Options &getFortranOpts() const { return parserOpts; }

  TargetOptions &getTargetOpts() { return targetOpts; }
  const TargetOptions &getTargetOpts() const { return targetOpts; }

  CodeGenOptions &getCodeGenOpts() { return codeGenOpts; }
  const CodeGenOptions &getCodeGenOpts() const { return codeGenOpts; }

  language::Compability::common::LangOptions &getLangOpts() { return langOpts; }
  const language::Compability::common::LangOptions &getLangOpts() const { return langOpts; }

  language::Compability::lower::LoweringOptions &getLoweringOpts() { return loweringOpts; }
  const language::Compability::lower::LoweringOptions &getLoweringOpts() const {
    return loweringOpts;
  }

  /// Creates and configures semantics context based on the compilation flags.
  std::unique_ptr<language::Compability::semantics::SemanticsContext>
  getSemanticsCtx(language::Compability::parser::AllCookedSources &allCookedSources,
                  const toolchain::TargetMachine &);

  std::string &getModuleDir() { return moduleDir; }
  const std::string &getModuleDir() const { return moduleDir; }

  std::string &getModuleFileSuffix() { return moduleFileSuffix; }
  const std::string &getModuleFileSuffix() const { return moduleFileSuffix; }

  bool &getDebugModuleDir() { return debugModuleDir; }
  const bool &getDebugModuleDir() const { return debugModuleDir; }

  bool &getHermeticModuleFileOutput() { return hermeticModuleFileOutput; }
  const bool &getHermeticModuleFileOutput() const {
    return hermeticModuleFileOutput;
  }
  size_t &getMaxErrors() { return maxErrors; }
  const size_t &getMaxErrors() const { return maxErrors; }

  bool &getWarnAsErr() { return warnAsErr; }
  const bool &getWarnAsErr() const { return warnAsErr; }

  bool &getUseAnalyzedObjectsForUnparse() {
    return useAnalyzedObjectsForUnparse;
  }
  const bool &getUseAnalyzedObjectsForUnparse() const {
    return useAnalyzedObjectsForUnparse;
  }

  bool &getEnableConformanceChecks() { return enableConformanceChecks; }
  const bool &getEnableConformanceChecks() const {
    return enableConformanceChecks;
  }

  const char *getArgv0() { return argv0; }

  bool &getEnableUsageChecks() { return enableUsageChecks; }
  const bool &getEnableUsageChecks() const { return enableUsageChecks; }

  bool &getDisableWarnings() { return disableWarnings; }
  const bool &getDisableWarnings() const { return disableWarnings; }

  language::Compability::parser::AnalyzedObjectsAsFortran &getAsFortran() {
    return asFortran;
  }
  const language::Compability::parser::AnalyzedObjectsAsFortran &getAsFortran() const {
    return asFortran;
  }

  language::Compability::common::IntrinsicTypeDefaultKinds &getDefaultKinds() {
    return defaultKinds;
  }
  const language::Compability::common::IntrinsicTypeDefaultKinds &getDefaultKinds() const {
    return defaultKinds;
  }

  bool getEnableTimers() const { return enableTimers; }

  /// Create a compiler invocation from a list of input options.
  /// \returns true on success.
  /// \returns false if an error was encountered while parsing the arguments
  /// \param [out] res - The resulting invocation.
  static bool createFromArgs(CompilerInvocation &res,
                             toolchain::ArrayRef<const char *> commandLineArgs,
                             language::Core::DiagnosticsEngine &diags,
                             const char *argv0 = nullptr);

  // Enables the std=f2018 conformance check
  void setEnableConformanceChecks() { enableConformanceChecks = true; }

  // Enables the usage checks
  void setEnableUsageChecks() { enableUsageChecks = true; }

  // Disables all Warnings
  void setDisableWarnings() { disableWarnings = true; }

  /// Useful setters
  void setArgv0(const char *dir) { argv0 = dir; }

  void setModuleDir(std::string &dir) { moduleDir = dir; }

  void setModuleFileSuffix(const char *suffix) {
    moduleFileSuffix = std::string(suffix);
  }

  void setDebugModuleDir(bool flag) { debugModuleDir = flag; }
  void setHermeticModuleFileOutput(bool flag) {
    hermeticModuleFileOutput = flag;
  }

  void setMaxErrors(size_t maxErrors) { this->maxErrors = maxErrors; }
  void setWarnAsErr(bool flag) { warnAsErr = flag; }

  void setUseAnalyzedObjectsForUnparse(bool flag) {
    useAnalyzedObjectsForUnparse = flag;
  }

  /// Set the Fortran options to predefined defaults.
  // TODO: We should map frontendOpts_ to parserOpts_ instead. For that, we
  // need to extend frontendOpts_ first. Next, we need to add the corresponding
  // compiler driver options in libclangDriver.
  void setDefaultFortranOpts();

  /// Set the default predefinitions.
  void setDefaultPredefinitions();

  /// Collect the macro definitions from preprocessorOpts_ and prepare them for
  /// the parser (i.e. copy into parserOpts_)
  void collectMacroDefinitions();

  /// Set the Fortran options to user-specified values.
  /// These values are found in the preprocessor options.
  void setFortranOpts();

  /// Set the Semantic Options
  void setSemanticsOpts(language::Compability::parser::AllCookedSources &);

  /// Set \p loweringOptions controlling lowering behavior based
  /// on the \p optimizationLevel.
  void setLoweringOptions();
};

} // end namespace language::Compability::frontend
#endif // FORTRAN_FRONTEND_COMPILERINVOCATION_H
