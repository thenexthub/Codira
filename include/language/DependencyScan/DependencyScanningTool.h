//===-------------- DependencyScanningTool.h - Codira Compiler -------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEPENDENCY_SCANNING_TOOL_H
#define LANGUAGE_DEPENDENCY_SCANNING_TOOL_H

#include "language-c/DependencyScan/DependencyScan.h"
#include "language/Frontend/Frontend.h"
#include "language/AST/ModuleDependencies.h"
#include "language/DependencyScan/ScanDependencies.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/StringSaver.h"

namespace language {
namespace dependencies {
class DependencyScanningTool;
class DependencyScanDiagnosticCollector;

struct ScanQueryInstance {
  std::unique_ptr<CompilerInstance> ScanInstance;
  std::shared_ptr<DependencyScanDiagnosticCollector> ScanDiagnostics;
};

/// Diagnostic consumer that simply collects the diagnostics emitted so-far
class DependencyScanDiagnosticCollector : public DiagnosticConsumer {
private:
  struct ScannerDiagnosticInfo {
    std::string Message;
    toolchain::SourceMgr::DiagKind Severity;
    std::optional<ScannerImportStatementInfo::ImportDiagnosticLocationInfo> ImportLocation;
  };
  std::vector<ScannerDiagnosticInfo> Diagnostics;
  toolchain::sys::SmartMutex<true> ScanningDiagnosticConsumerStateLock;

  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info) override;

protected:
  virtual void addDiagnostic(SourceManager &SM, const DiagnosticInfo &Info);

public:
  friend DependencyScanningTool;
  DependencyScanDiagnosticCollector() {}
  void reset() { Diagnostics.clear(); }
  const std::vector<ScannerDiagnosticInfo> &getDiagnostics() const {
    return Diagnostics;
  }
};

/// Given a set of arguments to a print-target-info frontend tool query, produce the
/// JSON target info.
toolchain::ErrorOr<languagescan_string_ref_t> getTargetInfo(ArrayRef<const char *> Command,
                                                    const char *main_executable_path);

/// The high-level implementation of the dependency scanner that runs on
/// an individual worker thread.
class DependencyScanningTool {
public:
  /// Construct a dependency scanning tool.
  DependencyScanningTool();

  /// Collect the full module dependency graph for the input.
  ///
  /// \returns a \c StringError with the diagnostic output if errors
  /// occurred, \c languagescan_dependency_result_t otherwise.
  toolchain::ErrorOr<languagescan_dependency_graph_t>
  getDependencies(ArrayRef<const char *> Command,
                  StringRef WorkingDirectory);

  /// Collect the set of imports for the input module
  ///
  /// \returns a \c StringError with the diagnostic output if errors
  /// occurred, \c languagescan_prescan_result_t otherwise.
  toolchain::ErrorOr<languagescan_import_set_t>
  getImports(ArrayRef<const char *> Command, StringRef WorkingDirectory);

  /// Using the specified invocation command, instantiate a CompilerInstance
  /// that will be used for this scan.
  toolchain::ErrorOr<ScanQueryInstance>
  initCompilerInstanceForScan(ArrayRef<const char *> Command,
                              StringRef WorkingDirectory,
                              std::shared_ptr<DependencyScanDiagnosticCollector> scannerDiagnosticsCollector);

private:
  /// Shared cache of module dependencies, re-used by individual full-scan queries
  /// during the lifetime of this Tool.
  std::unique_ptr<CodiraDependencyScanningService> ScanningService;

  /// Shared state mutual-exclusivity lock
  toolchain::sys::SmartMutex<true> DependencyScanningToolStateLock;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver;
};

languagescan_diagnostic_set_t *mapCollectedDiagnosticsForOutput(const DependencyScanDiagnosticCollector *diagnosticCollector);

} // end namespace dependencies
} // end namespace language

#endif // LANGUAGE_DEPENDENCY_SCANNING_TOOL_H
