//===------------ DependencyScanningTool.cpp - Codira Compiler -------------===//
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

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/TargetInfo.h"
#include "language/Basic/ColorUtils.h"
#include "language/DependencyScan/DependencyScanningTool.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "language/DependencyScan/SerializedModuleDependencyCacheFormat.h"
#include "language/DependencyScan/StringUtils.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualOutputBackends.h"

#include <sstream>

namespace language {
namespace dependencies {

// Global mutex for target info queries since they are executed separately .
toolchain::sys::SmartMutex<true> TargetInfoMutex;

toolchain::ErrorOr<languagescan_string_ref_t> getTargetInfo(ArrayRef<const char *> Command,
                                                    const char *main_executable_path) {
  toolchain::sys::SmartScopedLock<true> Lock(TargetInfoMutex);

  // Parse arguments.
  std::string CommandString;
  for (const auto *c : Command) {
    CommandString.append(c);
    CommandString.append(" ");
  }
  SmallVector<const char *, 4> Args;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  // Ensure that we use the Windows command line parsing on Windows as we need
  // to ensure that we properly handle paths.
  if (toolchain::Triple(toolchain::sys::getProcessTriple()).isOSWindows()) 
    toolchain::cl::TokenizeWindowsCommandLine(CommandString, Saver, Args);
  else
    toolchain::cl::TokenizeGNUCommandLine(CommandString, Saver, Args);
  SourceManager dummySM;
  DiagnosticEngine DE(dummySM);
  CompilerInvocation Invocation;
  if (Invocation.parseArgs(Args, DE, nullptr, {}, main_executable_path)) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Store the result to a string.
  std::string ResultStr;
  toolchain::raw_string_ostream StrOS(ResultStr);
  language::targetinfo::printTargetInfo(Invocation, StrOS);
  return c_string_utils::create_clone(ResultStr.c_str());
}

void DependencyScanDiagnosticCollector::handleDiagnostic(SourceManager &SM,
                      const DiagnosticInfo &Info) {
  addDiagnostic(SM, Info);
  for (auto ChildInfo : Info.ChildDiagnosticInfo) {
    addDiagnostic(SM, *ChildInfo);
  }
}

void DependencyScanDiagnosticCollector::addDiagnostic(
    SourceManager &SM, const DiagnosticInfo &Info) {
  toolchain::sys::SmartScopedLock<true> Lock(ScanningDiagnosticConsumerStateLock);

  // Determine what kind of diagnostic we're emitting.
  toolchain::SourceMgr::DiagKind SMKind;
  switch (Info.Kind) {
  case DiagnosticKind::Error:
    SMKind = toolchain::SourceMgr::DK_Error;
    break;
  case DiagnosticKind::Warning:
    SMKind = toolchain::SourceMgr::DK_Warning;
    break;
  case DiagnosticKind::Note:
    SMKind = toolchain::SourceMgr::DK_Note;
    break;
  case DiagnosticKind::Remark:
    SMKind = toolchain::SourceMgr::DK_Remark;
    break;
  }

  // Translate ranges.
  SmallVector<toolchain::SMRange, 2> Ranges;
  for (auto R : Info.Ranges)
    Ranges.push_back(getRawRange(SM, R));

  // Translate fix-its.
  SmallVector<toolchain::SMFixIt, 2> FixIts;
  for (DiagnosticInfo::FixIt F : Info.FixIts)
    FixIts.push_back(getRawFixIt(SM, F));

  // Display the diagnostic.
  std::string FormattedMessage;
  toolchain::raw_string_ostream Stream(FormattedMessage);
  // Actually substitute the diagnostic arguments into the diagnostic text.
  toolchain::SmallString<256> Text;
  {
    toolchain::raw_svector_ostream Out(Text);
    DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                           Info.FormatArgs);
    auto Msg = SM.GetMessage(Info.Loc, SMKind, Text, Ranges, FixIts, true);
    Msg.print(nullptr, Stream, false, false, false);
    Stream.flush();
  }

  if (Info.Loc && Info.Loc.isValid()) {
    auto bufferIdentifier = SM.getDisplayNameForLoc(Info.Loc);
    auto lineAndColumnNumbers = SM.getLineAndColumnInBuffer(Info.Loc);
    auto importLocation = ScannerImportStatementInfo::ImportDiagnosticLocationInfo(
      bufferIdentifier.str(), lineAndColumnNumbers.first,
      lineAndColumnNumbers.second);
    Diagnostics.push_back(
      ScannerDiagnosticInfo{FormattedMessage, SMKind, importLocation});
  } else {
    Diagnostics.push_back(
      ScannerDiagnosticInfo{FormattedMessage, SMKind, std::nullopt});
  }
}

languagescan_diagnostic_set_t *mapCollectedDiagnosticsForOutput(
    const DependencyScanDiagnosticCollector *diagnosticCollector) {
  auto collectedDiagnostics = diagnosticCollector->getDiagnostics();
  auto numDiagnostics = collectedDiagnostics.size();
  languagescan_diagnostic_set_t *diagnosticOutput = new languagescan_diagnostic_set_t;
  diagnosticOutput->count = numDiagnostics;
  diagnosticOutput->diagnostics =
      new languagescan_diagnostic_info_t[numDiagnostics];
  for (size_t i = 0; i < numDiagnostics; ++i) {
    const auto &Diagnostic = collectedDiagnostics[i];
    languagescan_diagnostic_info_s *diagnosticInfo =
        new languagescan_diagnostic_info_s;
    diagnosticInfo->message =
        language::c_string_utils::create_clone(Diagnostic.Message.c_str());
    switch (Diagnostic.Severity) {
    case toolchain::SourceMgr::DK_Error:
      diagnosticInfo->severity = LANGUAGESCAN_DIAGNOSTIC_SEVERITY_ERROR;
      break;
    case toolchain::SourceMgr::DK_Warning:
      diagnosticInfo->severity = LANGUAGESCAN_DIAGNOSTIC_SEVERITY_WARNING;
      break;
    case toolchain::SourceMgr::DK_Note:
      diagnosticInfo->severity = LANGUAGESCAN_DIAGNOSTIC_SEVERITY_NOTE;
      break;
    case toolchain::SourceMgr::DK_Remark:
      diagnosticInfo->severity = LANGUAGESCAN_DIAGNOSTIC_SEVERITY_REMARK;
      break;
    }

    if (Diagnostic.ImportLocation.has_value()) {
      auto importLocation = Diagnostic.ImportLocation.value();
      languagescan_source_location_s *sourceLoc = new languagescan_source_location_s;
      if (importLocation.bufferIdentifier.empty())
        sourceLoc->buffer_identifier = language::c_string_utils::create_null();
      else
        sourceLoc->buffer_identifier = language::c_string_utils::create_clone(
            importLocation.bufferIdentifier.c_str());
      sourceLoc->line_number = importLocation.lineNumber;
      sourceLoc->column_number = importLocation.columnNumber;
      diagnosticInfo->source_location = sourceLoc;
    } else {
      diagnosticInfo->source_location = nullptr;
    }

    diagnosticOutput->diagnostics[i] = diagnosticInfo;
  }
  return diagnosticOutput;
}

// Generate an instance of the `languagescan_dependency_graph_s` which contains no
// module dependnecies but captures the diagnostics emitted during the attempted
// scan query.
static languagescan_dependency_graph_t generateHollowDiagnosticOutput(
    const DependencyScanDiagnosticCollector &ScanDiagnosticConsumer) {
  // Create a dependency graph instance
  languagescan_dependency_graph_t hollowResult = new languagescan_dependency_graph_s;

  // Populate the `modules` with a single info for the main module
  // containing no dependencies
  languagescan_dependency_set_t *dependencySet = new languagescan_dependency_set_t;
  dependencySet->count = 1;
  dependencySet->modules = new languagescan_dependency_info_t[1];
  languagescan_dependency_info_s *hollowMainModuleInfo =
      new languagescan_dependency_info_s;
  dependencySet->modules[0] = hollowMainModuleInfo;
  hollowResult->dependencies = dependencySet;

  // Other main module details empty
  hollowMainModuleInfo->direct_dependencies =
      c_string_utils::create_empty_set();
  hollowMainModuleInfo->source_files = c_string_utils::create_empty_set();
  hollowMainModuleInfo->module_path = c_string_utils::create_null();
  hollowResult->main_module_name = c_string_utils::create_clone("unknown");
  hollowMainModuleInfo->module_name =
      c_string_utils::create_clone("languageTextual:unknown");

  // Hollow info details
  languagescan_module_details_s *hollowDetails = new languagescan_module_details_s;
  hollowDetails->kind = LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL;
  hollowDetails->language_textual_details = {c_string_utils::create_null(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_null(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_empty_set(),
                                          c_string_utils::create_null(),
                                          false,
                                          false,
                                          c_string_utils::create_null(),
                                          c_string_utils::create_null(),
                                          c_string_utils::create_null(),
                                          nullptr,
                                          c_string_utils::create_null(),
                                          c_string_utils::create_null(),
                                          c_string_utils::create_null()};
  hollowMainModuleInfo->details = hollowDetails;

  // Empty Link Library set
  languagescan_link_library_set_t *hollowLinkLibrarySet =
      new languagescan_link_library_set_t;
  hollowLinkLibrarySet->count = 0;
  hollowLinkLibrarySet->link_libraries = nullptr;
  hollowMainModuleInfo->link_libraries = hollowLinkLibrarySet;

  // Empty Import set
  languagescan_import_info_set_t *hollowImportInfoSet =
      new languagescan_import_info_set_t;
  hollowImportInfoSet->count = 0;
  hollowImportInfoSet->imports = nullptr;
  hollowMainModuleInfo->imports = hollowImportInfoSet;

  // Populate the diagnostic info
  hollowResult->diagnostics =
      mapCollectedDiagnosticsForOutput(&ScanDiagnosticConsumer);
  return hollowResult;
}

// Generate an instance of the `languagescan_import_set_t` which contains no
// imports but captures the diagnostics emitted during the attempted
// scan query.
static languagescan_import_set_t generateHollowDiagnosticOutputImportSet(
    const DependencyScanDiagnosticCollector &ScanDiagnosticConsumer) {
  // Create an dependency graph instance
  languagescan_import_set_t hollowResult = new languagescan_import_set_s;
  hollowResult->imports = c_string_utils::create_empty_set();
  hollowResult->diagnostics =
      mapCollectedDiagnosticsForOutput(&ScanDiagnosticConsumer);
  return hollowResult;
}

DependencyScanningTool::DependencyScanningTool()
    : ScanningService(std::make_unique<CodiraDependencyScanningService>()),
      Alloc(), Saver(Alloc) {}

toolchain::ErrorOr<languagescan_dependency_graph_t>
DependencyScanningTool::getDependencies(
    ArrayRef<const char *> Command,
    StringRef WorkingDirectory) {
  // There may be errors as early as in instance initialization, so we must ensure
  // we can catch those.
  auto ScanDiagnosticConsumer =
    std::make_shared<DependencyScanDiagnosticCollector>();

  // The primary instance used to scan the query Codira source-code
  auto QueryContextOrErr = initCompilerInstanceForScan(Command,
                                                       WorkingDirectory,
                                                       ScanDiagnosticConsumer);
  if (QueryContextOrErr.getError())
    return generateHollowDiagnosticOutput(*ScanDiagnosticConsumer);

  auto QueryContext = std::move(*QueryContextOrErr);

  // Local scan cache instance, wrapping the shared global cache.
  ModuleDependenciesCache cache(
      QueryContext.ScanInstance->getMainModule()->getNameStr().str(),
      QueryContext.ScanInstance->getInvocation().getModuleScanningHash());
  // Execute the scanning action, retrieving the in-memory result
  auto DependenciesOrErr = performModuleScan(*ScanningService,
                                             *QueryContext.ScanInstance.get(),
                                             cache,
                                             QueryContext.ScanDiagnostics.get());
  if (DependenciesOrErr.getError())
    return generateHollowDiagnosticOutput(*ScanDiagnosticConsumer);

  return std::move(*DependenciesOrErr);
}

toolchain::ErrorOr<languagescan_import_set_t>
DependencyScanningTool::getImports(ArrayRef<const char *> Command,
                                   StringRef WorkingDirectory) {
  // There may be errors as early as in instance initialization, so we must ensure
  // we can catch those
  auto ScanDiagnosticConsumer = std::make_shared<DependencyScanDiagnosticCollector>();
  // The primary instance used to scan the query Codira source-code
  auto QueryContextOrErr = initCompilerInstanceForScan(Command,
                                                       WorkingDirectory,
                                                       ScanDiagnosticConsumer);
  if (QueryContextOrErr.getError())
    return generateHollowDiagnosticOutputImportSet(*ScanDiagnosticConsumer);

  auto QueryContext = std::move(*QueryContextOrErr);

  // Local scan cache instance, wrapping the shared global cache.
  ModuleDependenciesCache cache(
      QueryContext.ScanInstance->getMainModule()->getNameStr().str(),
      QueryContext.ScanInstance->getInvocation().getModuleScanningHash());
  auto DependenciesOrErr = performModulePrescan(*ScanningService,
                                                *QueryContext.ScanInstance.get(),
                                                cache,
                                                QueryContext.ScanDiagnostics.get());
  if (DependenciesOrErr.getError())
    return generateHollowDiagnosticOutputImportSet(*ScanDiagnosticConsumer);

  return std::move(*DependenciesOrErr);
}

toolchain::ErrorOr<ScanQueryInstance>
DependencyScanningTool::initCompilerInstanceForScan(
    ArrayRef<const char *> CommandArgs,
    StringRef WorkingDir,
    std::shared_ptr<DependencyScanDiagnosticCollector> scannerDiagnosticsCollector) {
  // The remainder of this method operates on shared state in the
  // scanning service
  toolchain::sys::SmartScopedLock<true> Lock(DependencyScanningToolStateLock);
  // FIXME: Instead, target-info and supported-features queries must use
  // `DependencyScanningToolStateLock`, but this currently requires further
  // client-side API plumbing.
  toolchain::sys::SmartScopedLock<true> TargetInfoLock(TargetInfoMutex);

  // State unique to an individual scan
  auto Instance = std::make_unique<CompilerInstance>();
  Instance->addDiagnosticConsumer(scannerDiagnosticsCollector.get());

  // Basic error checking on the arguments
  if (CommandArgs.empty()) {
    Instance->getDiags().diagnose(SourceLoc(), diag::error_no_frontend_args);
    return std::make_error_code(std::errc::invalid_argument);
  }

  CompilerInvocation Invocation;
  SmallString<128> WorkingDirectory(WorkingDir);
  if (WorkingDirectory.empty())
    toolchain::sys::fs::current_path(WorkingDirectory);

  // Parse/tokenize arguments.
  std::string CommandString;
  for (const auto *c : CommandArgs) {
    CommandString.append(c);
    CommandString.append(" ");
  }
  SmallVector<const char *, 4> Args;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  // Ensure that we use the Windows command line parsing on Windows as we need
  // to ensure that we properly handle paths.
  if (toolchain::Triple(toolchain::sys::getProcessTriple()).isOSWindows())
    toolchain::cl::TokenizeWindowsCommandLine(CommandString, Saver, Args);
  else
    toolchain::cl::TokenizeGNUCommandLine(CommandString, Saver, Args);

  if (Invocation.parseArgs(Args, Instance->getDiags(),
                           nullptr, WorkingDirectory, "/tmp/foo")) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Setup the instance
  std::string InstanceSetupError;
  if (Instance->setup(Invocation, InstanceSetupError)) {
    return std::make_error_code(std::errc::not_supported);
  }
  Invocation.getFrontendOptions().LLVMArgs.clear();

  // Setup the caching service after the instance finishes setup.
  if (ScanningService->setupCachingDependencyScanningService(*Instance))
    return std::make_error_code(std::errc::invalid_argument);

  (void)Instance->getMainModule();

  return ScanQueryInstance{std::move(Instance), 
                           scannerDiagnosticsCollector};
}

} // namespace dependencies
} // namespace language
