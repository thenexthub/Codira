//===--- FrontendActions.cpp ----------------------------------------------===//
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

#include "language/Compability/Frontend/FrontendActions.h"
#include "language/Compability/Frontend/CompilerInstance.h"
#include "language/Compability/Frontend/CompilerInvocation.h"
#include "language/Compability/Frontend/FrontendOptions.h"
#include "language/Compability/Frontend/ParserActions.h"
#include "language/Compability/Lower/Bridge.h"
#include "language/Compability/Lower/Support/Verifier.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "language/Compability/Optimizer/Passes/Pipelines.h"
#include "language/Compability/Optimizer/Support/DataLayout.h"
#include "language/Compability/Optimizer/Support/InitFIR.h"
#include "language/Compability/Optimizer/Support/Utils.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Semantics/runtime-type-info.h"
#include "language/Compability/Semantics/unparse-with-symbols.h"
#include "language/Compability/Support/default-kinds.h"
#include "language/Compability/Tools/CrossToolHelpers.h"

#include "mlir/IR/Dialect.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/Import.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/FileSystemOptions.h"
#include "language/Core/Driver/DriverDiagnostic.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Analysis/TargetLibraryInfo.h"
#include "toolchain/Analysis/TargetTransformInfo.h"
#include "toolchain/Bitcode/BitcodeWriterPass.h"
#include "toolchain/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "toolchain/IR/LLVMRemarkStreamer.h"
#include "toolchain/IR/LegacyPassManager.h"
#include "toolchain/IR/Verifier.h"
#include "toolchain/IRPrinter/IRPrintingPasses.h"
#include "toolchain/IRReader/IRReader.h"
#include "toolchain/Linker/Linker.h"
#include "toolchain/Object/OffloadBinary.h"
#include "toolchain/Passes/PassBuilder.h"
#include "toolchain/Passes/PassPlugin.h"
#include "toolchain/Passes/StandardInstrumentations.h"
#include "toolchain/ProfileData/InstrProfCorrelator.h"
#include "toolchain/Support/AMDGPUAddrSpace.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/PGOOptions.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/SourceMgr.h"
#include "toolchain/Support/ToolOutputFile.h"
#include "toolchain/Target/TargetMachine.h"
#include "toolchain/TargetParser/RISCVISAInfo.h"
#include "toolchain/TargetParser/RISCVTargetParser.h"
#include "toolchain/Transforms/IPO/Internalize.h"
#include "toolchain/Transforms/Instrumentation/InstrProfiling.h"
#include "toolchain/Transforms/Utils/ModuleUtils.h"
#include <memory>
#include <system_error>

using namespace language::Compability::frontend;

constexpr toolchain::StringLiteral timingIdParse = "Parse";
constexpr toolchain::StringLiteral timingIdMLIRGen = "MLIR generation";
constexpr toolchain::StringLiteral timingIdMLIRPasses =
    "MLIR translation/optimization";
constexpr toolchain::StringLiteral timingIdLLVMIRGen = "LLVM IR generation";
constexpr toolchain::StringLiteral timingIdLLVMIRPasses = "LLVM IR optimizations";
constexpr toolchain::StringLiteral timingIdBackend =
    "Assembly/Object code generation";

// Declare plugin extension function declarations.
#define HANDLE_EXTENSION(Ext)                                                  \
  toolchain::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "toolchain/Support/Extension.def"

/// Save the given \c mlirModule to a temporary .mlir file, in a location
/// decided by the -save-temps flag. No files are produced if the flag is not
/// specified.
static bool saveMLIRTempFile(const CompilerInvocation &ci,
                             mlir::ModuleOp mlirModule,
                             toolchain::StringRef inputFile,
                             toolchain::StringRef outputTag) {
  if (!ci.getCodeGenOpts().SaveTempsDir.has_value())
    return true;

  const toolchain::StringRef compilerOutFile = ci.getFrontendOpts().outputFile;
  const toolchain::StringRef saveTempsDir = ci.getCodeGenOpts().SaveTempsDir.value();
  auto dir = toolchain::StringSwitch<toolchain::StringRef>(saveTempsDir)
                 .Case("cwd", "")
                 .Case("obj", toolchain::sys::path::parent_path(compilerOutFile))
                 .Default(saveTempsDir);

  // Build path from the compiler output file name, triple, cpu and OpenMP
  // information
  toolchain::SmallString<256> path(dir);
  toolchain::sys::path::append(path, toolchain::sys::path::stem(inputFile) + "-" +
                                    outputTag + ".mlir");

  std::error_code ec;
  toolchain::ToolOutputFile out(path, ec, toolchain::sys::fs::OF_Text);
  if (ec)
    return false;

  mlirModule->print(out.os());
  out.os().close();
  out.keep();

  return true;
}

//===----------------------------------------------------------------------===//
// Custom BeginSourceFileAction
//===----------------------------------------------------------------------===//

bool PrescanAction::beginSourceFileAction() { return runPrescan(); }

bool PrescanAndParseAction::beginSourceFileAction() {
  return runPrescan() && runParse(/*emitMessages=*/true);
}

bool PrescanAndSemaAction::beginSourceFileAction() {
  return runPrescan() && runParse(/*emitMessages=*/false) &&
         runSemanticChecks() && generateRtTypeTables();
}

bool PrescanAndSemaDebugAction::beginSourceFileAction() {
  // This is a "debug" action for development purposes. To facilitate this, the
  // semantic checks are made to succeed unconditionally to prevent this action
  // from exiting early (i.e. in the presence of semantic errors). We should
  // never do this in actions intended for end-users or otherwise regular
  // compiler workflows!
  return runPrescan() && runParse(/*emitMessages=*/false) &&
         (runSemanticChecks() || true) && (generateRtTypeTables() || true);
}

static void addDependentLibs(mlir::ModuleOp mlirModule, CompilerInstance &ci) {
  const std::vector<std::string> &libs =
      ci.getInvocation().getCodeGenOpts().DependentLibs;
  if (libs.empty()) {
    return;
  }
  // dependent-lib is currently only supported on Windows, so the list should be
  // empty on non-Windows platforms
  assert(
      toolchain::Triple(ci.getInvocation().getTargetOpts().triple).isOSWindows() &&
      "--dependent-lib is only supported on Windows");
  // Add linker options specified by --dependent-lib
  auto builder = mlir::OpBuilder(mlirModule.getRegion());
  for (const std::string &lib : libs) {
    mlir::LLVM::LinkerOptionsOp::create(
        builder, mlirModule.getLoc(),
        builder.getStrArrayAttr({"/DEFAULTLIB:" + lib}));
  }
}

bool CodeGenAction::beginSourceFileAction() {
  // Delete previous LLVM module depending on old context before making a new
  // one.
  if (toolchainModule)
    toolchainModule.reset(nullptr);
  toolchainCtx = std::make_unique<toolchain::LLVMContext>();
  CompilerInstance &ci = this->getInstance();
  mlir::DefaultTimingManager &timingMgr = ci.getTimingManager();
  mlir::TimingScope &timingScopeRoot = ci.getTimingScopeRoot();

  // This will provide timing information even when the input is an LLVM IR or
  // MLIR file. That is fine because those do have to be parsed, so the label
  // is still accurate.
  mlir::TimingScope timingScopeParse = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdParse, timingMgr));

  // If the input is an LLVM file, just parse it and return.
  if (this->getCurrentInput().getKind().getLanguage() == Language::LLVM_IR) {
    toolchain::SMDiagnostic err;
    toolchainModule = toolchain::parseIRFile(getCurrentInput().getFile(), err, *toolchainCtx);
    if (!toolchainModule || toolchain::verifyModule(*toolchainModule, &toolchain::errs())) {
      err.print("flang", toolchain::errs());
      unsigned diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "Could not parse IR");
      ci.getDiagnostics().Report(diagID);
      return false;
    }

    return true;
  }

  // Reset MLIR module if it was set before overriding the old context.
  if (mlirModule)
    mlirModule = mlir::OwningOpRef<mlir::ModuleOp>(nullptr);
  // Load the MLIR dialects required by Flang
  mlirCtx = std::make_unique<mlir::MLIRContext>();
  fir::support::loadDialects(*mlirCtx);
  fir::support::registerLLVMTranslation(*mlirCtx);
  mlir::DialectRegistry registry;
  fir::acc::registerOpenACCExtensions(registry);
  fir::omp::registerOpenMPExtensions(registry);
  mlirCtx->appendDialectRegistry(registry);

  const toolchain::TargetMachine &targetMachine = ci.getTargetMachine();

  // If the input is an MLIR file, just parse it and return.
  if (this->getCurrentInput().getKind().getLanguage() == Language::MLIR) {
    toolchain::SourceMgr sourceMgr;
    toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> fileOrErr =
        toolchain::MemoryBuffer::getFileOrSTDIN(getCurrentInput().getFile());
    sourceMgr.AddNewSourceBuffer(std::move(*fileOrErr), toolchain::SMLoc());
    mlir::OwningOpRef<mlir::ModuleOp> module =
        mlir::parseSourceFile<mlir::ModuleOp>(sourceMgr, mlirCtx.get());

    if (!module || mlir::failed(module->verifyInvariants())) {
      unsigned diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "Could not parse FIR");
      ci.getDiagnostics().Report(diagID);
      return false;
    }

    mlirModule = std::move(module);
    const toolchain::DataLayout &dl = targetMachine.createDataLayout();
    fir::support::setMLIRDataLayout(*mlirModule, dl);
    return true;
  }

  // Otherwise, generate an MLIR module from the input Fortran source
  if (getCurrentInput().getKind().getLanguage() != Language::Fortran) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error,
        "Invalid input type - expecting a Fortran file");
    ci.getDiagnostics().Report(diagID);
    return false;
  }
  bool res = runPrescan() && runParse(/*emitMessages=*/false) &&
             runSemanticChecks() && generateRtTypeTables();
  if (!res)
    return res;

  timingScopeParse.stop();
  mlir::TimingScope timingScopeMLIRGen = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdMLIRGen, timingMgr));

  // Create a LoweringBridge
  const common::IntrinsicTypeDefaultKinds &defKinds =
      ci.getSemanticsContext().defaultKinds();
  fir::KindMapping kindMap(mlirCtx.get(), toolchain::ArrayRef<fir::KindTy>{
                                              fir::fromDefaultKinds(defKinds)});
  lower::LoweringBridge lb = language::Compability::lower::LoweringBridge::create(
      *mlirCtx, ci.getSemanticsContext(), defKinds,
      ci.getSemanticsContext().intrinsics(),
      ci.getSemanticsContext().targetCharacteristics(), getAllCooked(ci),
      ci.getInvocation().getTargetOpts().triple, kindMap,
      ci.getInvocation().getLoweringOpts(),
      ci.getInvocation().getFrontendOpts().envDefaults,
      ci.getInvocation().getFrontendOpts().features, targetMachine,
      ci.getInvocation().getTargetOpts(), ci.getInvocation().getCodeGenOpts());

  if (ci.getInvocation().getFrontendOpts().features.IsEnabled(
          language::Compability::common::LanguageFeature::OpenMP)) {
    setOffloadModuleInterfaceAttributes(lb.getModule(),
                                        ci.getInvocation().getLangOpts());
    setOpenMPVersionAttribute(lb.getModule(),
                              ci.getInvocation().getLangOpts().OpenMPVersion);
  }

  // Create a parse tree and lower it to FIR
  parseAndLowerTree(ci, lb);

  // Fetch module from lb, so we can set
  mlirModule = lb.getModuleAndRelease();

  // Add target specific items like dependent libraries, target specific
  // constants etc.
  addDependentLibs(*mlirModule, ci);
  timingScopeMLIRGen.stop();

  // run the default passes.
  mlir::PassManager pm((*mlirModule)->getName(),
                       mlir::OpPassManager::Nesting::Implicit);
  (void)mlir::applyPassManagerCLOptions(pm);
  // Add OpenMP-related passes
  // WARNING: These passes must be run immediately after the lowering to ensure
  // that the FIR is correct with respect to OpenMP operations/attributes.
  bool isOpenMPEnabled =
      ci.getInvocation().getFrontendOpts().features.IsEnabled(
          language::Compability::common::LanguageFeature::OpenMP);
  bool isOpenMPSimd = ci.getInvocation().getLangOpts().OpenMPSimd;

  fir::OpenMPFIRPassPipelineOpts opts;

  using DoConcurrentMappingKind =
      language::Compability::frontend::CodeGenOptions::DoConcurrentMappingKind;
  opts.doConcurrentMappingKind =
      ci.getInvocation().getCodeGenOpts().getDoConcurrentMapping();

  if (opts.doConcurrentMappingKind != DoConcurrentMappingKind::DCMK_None &&
      !isOpenMPEnabled) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Warning,
        "OpenMP is required for lowering `do concurrent` loops to OpenMP."
        "Enable OpenMP using `-fopenmp`."
        "`do concurrent` loops will be serialized.");
    ci.getDiagnostics().Report(diagID);
    opts.doConcurrentMappingKind = DoConcurrentMappingKind::DCMK_None;
  }

  if (opts.doConcurrentMappingKind != DoConcurrentMappingKind::DCMK_None) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Warning,
        "Mapping `do concurrent` to OpenMP is still experimental.");
    ci.getDiagnostics().Report(diagID);
  }

  if (isOpenMPEnabled) {
    opts.isTargetDevice = false;
    if (auto offloadMod = toolchain::dyn_cast<mlir::omp::OffloadModuleInterface>(
            mlirModule->getOperation()))
      opts.isTargetDevice = offloadMod.getIsTargetDevice();
  }

  // WARNING: This pipeline must be run immediately after the lowering to
  // ensure that the FIR is correct with respect to OpenMP operations/
  // attributes.
  if (isOpenMPEnabled || isOpenMPSimd)
    fir::createOpenMPFIRPassPipeline(pm, opts);

  pm.enableVerifier(/*verifyPasses=*/true);
  pm.addPass(std::make_unique<language::Compability::lower::VerifierPass>());
  pm.enableTiming(timingScopeMLIRGen);

  if (mlir::failed(pm.run(*mlirModule))) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error,
        "verification of lowering to FIR failed");
    ci.getDiagnostics().Report(diagID);
    return false;
  }
  timingScopeMLIRGen.stop();

  // Print initial full MLIR module, before lowering or transformations, if
  // -save-temps has been specified.
  if (!saveMLIRTempFile(ci.getInvocation(), *mlirModule, getCurrentFile(),
                        "fir")) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Saving MLIR temp file failed");
    ci.getDiagnostics().Report(diagID);
    return false;
  }

  return true;
}

//===----------------------------------------------------------------------===//
// Custom ExecuteAction
//===----------------------------------------------------------------------===//
void InputOutputTestAction::executeAction() {
  CompilerInstance &ci = getInstance();

  // Create a stream for errors
  std::string buf;
  toolchain::raw_string_ostream errorStream{buf};

  // Read the input file
  language::Compability::parser::AllSources &allSources{ci.getAllSources()};
  std::string path{getCurrentFileOrBufferName()};
  const language::Compability::parser::SourceFile *sf;
  if (path == "-")
    sf = allSources.ReadStandardInput(errorStream);
  else
    sf = allSources.Open(path, errorStream, std::optional<std::string>{"."s});
  toolchain::ArrayRef<char> fileContent = sf->content();

  // Output file descriptor to receive the contents of the input file.
  std::unique_ptr<toolchain::raw_ostream> os;

  // Copy the contents from the input file to the output file
  if (!ci.isOutputStreamNull()) {
    // An output stream (outputStream_) was set earlier
    ci.writeOutputStream(fileContent.data());
  } else {
    // No pre-set output stream - create an output file
    os = ci.createDefaultOutputFile(
        /*binary=*/true, getCurrentFileOrBufferName(), "txt");
    if (!os)
      return;
    (*os) << fileContent.data();
  }
}

void PrintPreprocessedAction::executeAction() {
  std::string buf;
  toolchain::raw_string_ostream outForPP{buf};

  CompilerInstance &ci = this->getInstance();
  formatOrDumpPrescanner(buf, outForPP, ci);

  // If a pre-defined output stream exists, dump the preprocessed content there
  if (!ci.isOutputStreamNull()) {
    // Send the output to the pre-defined output buffer.
    ci.writeOutputStream(buf);
    return;
  }

  // Create a file and save the preprocessed output there
  std::unique_ptr<toolchain::raw_pwrite_stream> os{ci.createDefaultOutputFile(
      /*Binary=*/true, /*InFile=*/getCurrentFileOrBufferName())};
  if (!os) {
    return;
  }

  (*os) << buf;
}

void DebugDumpProvenanceAction::executeAction() {
  dumpProvenance(this->getInstance());
}

void ParseSyntaxOnlyAction::executeAction() {}

void DebugUnparseNoSemaAction::executeAction() {
  debugUnparseNoSema(this->getInstance(), toolchain::outs());
}

void DebugUnparseAction::executeAction() {
  CompilerInstance &ci = this->getInstance();
  auto os{ci.createDefaultOutputFile(
      /*Binary=*/false, /*InFile=*/getCurrentFileOrBufferName())};

  debugUnparseNoSema(ci, *os);
  reportFatalSemanticErrors();
}

void DebugUnparseWithSymbolsAction::executeAction() {
  debugUnparseWithSymbols(this->getInstance());
  reportFatalSemanticErrors();
}

void DebugUnparseWithModulesAction::executeAction() {
  debugUnparseWithModules(this->getInstance());
  reportFatalSemanticErrors();
}

void DebugDumpSymbolsAction::executeAction() {
  CompilerInstance &ci = this->getInstance();

  if (!ci.getRtTyTables().schemata) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error,
        "could not find module file for __fortran_type_info");
    ci.getDiagnostics().Report(diagID);
    toolchain::errs() << "\n";
    return;
  }

  // Dump symbols
  ci.getSemantics().DumpSymbols(toolchain::outs());
}

void DebugDumpAllAction::executeAction() {
  CompilerInstance &ci = this->getInstance();

  // Dump parse tree
  dumpTree(ci);

  if (!ci.getRtTyTables().schemata) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error,
        "could not find module file for __fortran_type_info");
    ci.getDiagnostics().Report(diagID);
    toolchain::errs() << "\n";
    return;
  }

  // Dump symbols
  toolchain::outs() << "=====================";
  toolchain::outs() << " Flang: symbols dump ";
  toolchain::outs() << "=====================\n";
  ci.getSemantics().DumpSymbols(toolchain::outs());
}

void DebugDumpParseTreeNoSemaAction::executeAction() {
  dumpTree(this->getInstance());
}

void DebugDumpParseTreeAction::executeAction() {
  dumpTree(this->getInstance());

  // Report fatal semantic errors
  reportFatalSemanticErrors();
}

void DebugMeasureParseTreeAction::executeAction() {
  CompilerInstance &ci = this->getInstance();
  debugMeasureParseTree(ci, getCurrentFileOrBufferName());
}

void DebugPreFIRTreeAction::executeAction() {
  // Report and exit if fatal semantic errors are present
  if (reportFatalSemanticErrors()) {
    return;
  }

  dumpPreFIRTree(this->getInstance());
}

void DebugDumpParsingLogAction::executeAction() {
  debugDumpParsingLog(this->getInstance());
}

void GetDefinitionAction::executeAction() {
  CompilerInstance &ci = this->getInstance();

  // Report and exit if fatal semantic errors are present
  if (reportFatalSemanticErrors()) {
    return;
  }

  parser::AllCookedSources &cs = ci.getAllCookedSources();
  unsigned diagID = ci.getDiagnostics().getCustomDiagID(
      language::Core::DiagnosticsEngine::Error, "Symbol not found");

  auto gdv = ci.getInvocation().getFrontendOpts().getDefVals;
  auto charBlock{cs.GetCharBlockFromLineAndColumns(gdv.line, gdv.startColumn,
                                                   gdv.endColumn)};
  if (!charBlock) {
    ci.getDiagnostics().Report(diagID);
    return;
  }

  toolchain::outs() << "String range: >" << charBlock->ToString() << "<\n";

  auto *symbol{
      ci.getSemanticsContext().FindScope(*charBlock).FindSymbol(*charBlock)};
  if (!symbol) {
    ci.getDiagnostics().Report(diagID);
    return;
  }

  toolchain::outs() << "Found symbol name: " << symbol->name().ToString() << "\n";

  auto sourceInfo{cs.GetSourcePositionRange(symbol->name())};
  if (!sourceInfo) {
    toolchain_unreachable(
        "Failed to obtain SourcePosition."
        "TODO: Please, write a test and replace this with a diagnostic!");
    return;
  }

  toolchain::outs() << "Found symbol name: " << symbol->name().ToString() << "\n";
  toolchain::outs() << symbol->name().ToString() << ": " << sourceInfo->first.path
               << ", " << sourceInfo->first.line << ", "
               << sourceInfo->first.column << "-" << sourceInfo->second.column
               << "\n";
}

void GetSymbolsSourcesAction::executeAction() {
  CompilerInstance &ci = this->getInstance();

  // Report and exit if fatal semantic errors are present
  if (reportFatalSemanticErrors()) {
    return;
  }

  ci.getSemantics().DumpSymbolsSources(toolchain::outs());
}

//===----------------------------------------------------------------------===//
// CodeGenActions
//===----------------------------------------------------------------------===//

CodeGenAction::~CodeGenAction() = default;

static toolchain::OptimizationLevel
mapToLevel(const language::Compability::frontend::CodeGenOptions &opts) {
  switch (opts.OptimizationLevel) {
  default:
    toolchain_unreachable("Invalid optimization level!");
  case 0:
    return toolchain::OptimizationLevel::O0;
  case 1:
    return toolchain::OptimizationLevel::O1;
  case 2:
    return toolchain::OptimizationLevel::O2;
  case 3:
    return toolchain::OptimizationLevel::O3;
  }
}

// Lower using HLFIR then run the FIR to HLFIR pipeline
void CodeGenAction::lowerHLFIRToFIR() {
  assert(mlirModule && "The MLIR module has not been generated yet.");

  CompilerInstance &ci = this->getInstance();
  const CodeGenOptions &opts = ci.getInvocation().getCodeGenOpts();
  toolchain::OptimizationLevel level = mapToLevel(opts);
  mlir::DefaultTimingManager &timingMgr = ci.getTimingManager();
  mlir::TimingScope &timingScopeRoot = ci.getTimingScopeRoot();

  fir::support::loadDialects(*mlirCtx);

  // Set-up the MLIR pass manager
  mlir::PassManager pm((*mlirModule)->getName(),
                       mlir::OpPassManager::Nesting::Implicit);

  pm.addPass(std::make_unique<language::Compability::lower::VerifierPass>());
  pm.enableVerifier(/*verifyPasses=*/true);

  fir::EnableOpenMP enableOpenMP = fir::EnableOpenMP::None;
  if (ci.getInvocation().getFrontendOpts().features.IsEnabled(
          language::Compability::common::LanguageFeature::OpenMP))
    enableOpenMP = fir::EnableOpenMP::Full;
  if (ci.getInvocation().getLangOpts().OpenMPSimd)
    enableOpenMP = fir::EnableOpenMP::Simd;
  // Create the pass pipeline
  fir::createHLFIRToFIRPassPipeline(pm, enableOpenMP, level);
  (void)mlir::applyPassManagerCLOptions(pm);

  mlir::TimingScope timingScopeMLIRPasses = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdMLIRPasses, timingMgr));
  pm.enableTiming(timingScopeMLIRPasses);
  if (!mlir::succeeded(pm.run(*mlirModule))) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Lowering to FIR failed");
    ci.getDiagnostics().Report(diagID);
  }
}

static std::optional<std::pair<unsigned, unsigned>>
getAArch64VScaleRange(CompilerInstance &ci) {
  const auto &langOpts = ci.getInvocation().getLangOpts();

  if (langOpts.VScaleMin || langOpts.VScaleMax)
    return std::pair<unsigned, unsigned>(
        langOpts.VScaleMin ? langOpts.VScaleMin : 1, langOpts.VScaleMax);

  std::string featuresStr = ci.getTargetFeatures();
  if (featuresStr.find("+sve") != std::string::npos)
    return std::pair<unsigned, unsigned>(1, 16);

  return std::nullopt;
}

static std::optional<std::pair<unsigned, unsigned>>
getRISCVVScaleRange(CompilerInstance &ci) {
  const auto &langOpts = ci.getInvocation().getLangOpts();
  const auto targetOpts = ci.getInvocation().getTargetOpts();
  const toolchain::Triple triple(targetOpts.triple);

  auto parseResult = toolchain::RISCVISAInfo::parseFeatures(
      triple.isRISCV64() ? 64 : 32, targetOpts.featuresAsWritten);
  if (!parseResult) {
    std::string buffer;
    toolchain::raw_string_ostream outputErrMsg(buffer);
    handleAllErrors(parseResult.takeError(), [&](toolchain::StringError &errMsg) {
      outputErrMsg << errMsg.getMessage();
    });
    ci.getDiagnostics().Report(language::Core::diag::err_invalid_feature_combination)
        << buffer;
    return std::nullopt;
  }

  toolchain::RISCVISAInfo *const isaInfo = parseResult->get();

  // RISCV::RVVBitsPerBlock is 64.
  unsigned vscaleMin = isaInfo->getMinVLen() / toolchain::RISCV::RVVBitsPerBlock;

  if (langOpts.VScaleMin || langOpts.VScaleMax) {
    // Treat Zvl*b as a lower bound on vscale.
    vscaleMin = std::max(vscaleMin, langOpts.VScaleMin);
    unsigned vscaleMax = langOpts.VScaleMax;
    if (vscaleMax != 0 && vscaleMax < vscaleMin)
      vscaleMax = vscaleMin;
    return std::pair<unsigned, unsigned>(vscaleMin ? vscaleMin : 1, vscaleMax);
  }

  if (vscaleMin > 0) {
    unsigned vscaleMax = isaInfo->getMaxVLen() / toolchain::RISCV::RVVBitsPerBlock;
    return std::make_pair(vscaleMin, vscaleMax);
  }

  return std::nullopt;
}

// TODO: We should get this from TargetInfo. However, that depends on
// too much of clang, so for now, replicate the functionality.
static std::optional<std::pair<unsigned, unsigned>>
getVScaleRange(CompilerInstance &ci) {
  const toolchain::Triple triple(ci.getInvocation().getTargetOpts().triple);

  if (triple.isAArch64())
    return getAArch64VScaleRange(ci);
  if (triple.isRISCV())
    return getRISCVVScaleRange(ci);

  // All other architectures that don't support scalable vectors (i.e. don't
  // need vscale)
  return std::nullopt;
}

// Lower the previously generated MLIR module into an LLVM IR module
void CodeGenAction::generateLLVMIR() {
  assert(mlirModule && "The MLIR module has not been generated yet.");

  CompilerInstance &ci = this->getInstance();
  CompilerInvocation &invoc = ci.getInvocation();
  const CodeGenOptions &opts = invoc.getCodeGenOpts();
  const auto &mathOpts = invoc.getLoweringOpts().getMathOptions();
  toolchain::OptimizationLevel level = mapToLevel(opts);
  mlir::DefaultTimingManager &timingMgr = ci.getTimingManager();
  mlir::TimingScope &timingScopeRoot = ci.getTimingScopeRoot();

  fir::support::loadDialects(*mlirCtx);
  mlir::DialectRegistry registry;
  fir::support::registerNonCodegenDialects(registry);
  fir::support::addFIRExtensions(registry);
  mlirCtx->appendDialectRegistry(registry);
  fir::support::registerLLVMTranslation(*mlirCtx);

  // Set-up the MLIR pass manager
  mlir::PassManager pm((*mlirModule)->getName(),
                       mlir::OpPassManager::Nesting::Implicit);

  pm.addPass(std::make_unique<language::Compability::lower::VerifierPass>());
  pm.enableVerifier(/*verifyPasses=*/true);

  MLIRToLLVMPassPipelineConfig config(level, opts, mathOpts);
  fir::registerDefaultInlinerPass(config);

  if (auto vsr = getVScaleRange(ci)) {
    config.VScaleMin = vsr->first;
    config.VScaleMax = vsr->second;
  }

  config.Reciprocals = opts.Reciprocals;
  config.PreferVectorWidth = opts.PreferVectorWidth;

  if (ci.getInvocation().getFrontendOpts().features.IsEnabled(
          language::Compability::common::LanguageFeature::OpenMP))
    config.EnableOpenMP = true;

  if (ci.getInvocation().getLangOpts().OpenMPSimd)
    config.EnableOpenMPSimd = true;

  if (ci.getInvocation().getLoweringOpts().getIntegerWrapAround())
    config.NSWOnLoopVarInc = false;

  config.ComplexRange = opts.getComplexRange();

  // Create the pass pipeline
  fir::createMLIRToLLVMPassPipeline(pm, config, getCurrentFile());
  (void)mlir::applyPassManagerCLOptions(pm);

  // run the pass manager
  mlir::TimingScope timingScopeMLIRPasses = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdMLIRPasses, timingMgr));
  pm.enableTiming(timingScopeMLIRPasses);
  if (!mlir::succeeded(pm.run(*mlirModule))) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Lowering to LLVM IR failed");
    ci.getDiagnostics().Report(diagID);
  }
  timingScopeMLIRPasses.stop();

  // Print final MLIR module, just before translation into LLVM IR, if
  // -save-temps has been specified.
  if (!saveMLIRTempFile(ci.getInvocation(), *mlirModule, getCurrentFile(),
                        "toolchainir")) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Saving MLIR temp file failed");
    ci.getDiagnostics().Report(diagID);
    return;
  }

  // Translate to LLVM IR
  mlir::TimingScope timingScopeLLVMIRGen = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdLLVMIRGen, timingMgr));
  std::optional<toolchain::StringRef> moduleName = mlirModule->getName();
  toolchainModule = mlir::translateModuleToLLVMIR(
      *mlirModule, *toolchainCtx, moduleName ? *moduleName : "FIRModule");

  if (!toolchainModule) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "failed to create the LLVM module");
    ci.getDiagnostics().Report(diagID);
    return;
  }

  // Set PIC/PIE level LLVM module flags.
  if (opts.PICLevel > 0) {
    toolchainModule->setPICLevel(static_cast<toolchain::PICLevel::Level>(opts.PICLevel));
    if (opts.IsPIE)
      toolchainModule->setPIELevel(
          static_cast<toolchain::PIELevel::Level>(opts.PICLevel));
  }

  const TargetOptions &targetOpts = ci.getInvocation().getTargetOpts();
  const toolchain::Triple triple(targetOpts.triple);

  // Set mcmodel level LLVM module flags
  std::optional<toolchain::CodeModel::Model> cm = getCodeModel(opts.CodeModel);
  if (cm.has_value()) {
    toolchainModule->setCodeModel(*cm);
    if ((cm == toolchain::CodeModel::Medium || cm == toolchain::CodeModel::Large) &&
        triple.getArch() == toolchain::Triple::x86_64) {
      toolchainModule->setLargeDataThreshold(opts.LargeDataThreshold);
    }
  }

  if (triple.isRISCV() && !targetOpts.abi.empty())
    toolchainModule->addModuleFlag(
        toolchain::Module::Error, "target-abi",
        toolchain::MDString::get(toolchainModule->getContext(), targetOpts.abi));

  if (triple.isAMDGPU() ||
      (triple.isSPIRV() && triple.getVendor() == toolchain::Triple::AMD)) {
    // Emit amdhsa_code_object_version module flag, which is code object version
    // times 100.
    if (opts.CodeObjectVersion != toolchain::CodeObjectVersionKind::COV_None) {
      toolchainModule->addModuleFlag(toolchain::Module::Error,
                                "amdhsa_code_object_version",
                                opts.CodeObjectVersion);
    }
  }
}

static std::unique_ptr<toolchain::raw_pwrite_stream>
getOutputStream(CompilerInstance &ci, toolchain::StringRef inFile,
                BackendActionTy action) {
  switch (action) {
  case BackendActionTy::Backend_EmitAssembly:
    return ci.createDefaultOutputFile(
        /*Binary=*/false, inFile, /*extension=*/"s");
  case BackendActionTy::Backend_EmitLL:
    return ci.createDefaultOutputFile(
        /*Binary=*/false, inFile, /*extension=*/"ll");
  case BackendActionTy::Backend_EmitFIR:
  case BackendActionTy::Backend_EmitHLFIR:
    return ci.createDefaultOutputFile(
        /*Binary=*/false, inFile, /*extension=*/"mlir");
  case BackendActionTy::Backend_EmitBC:
    return ci.createDefaultOutputFile(
        /*Binary=*/true, inFile, /*extension=*/"bc");
  case BackendActionTy::Backend_EmitObj:
    return ci.createDefaultOutputFile(
        /*Binary=*/true, inFile, /*extension=*/"o");
  }

  toolchain_unreachable("Invalid action!");
}

/// Generate target-specific machine-code or assembly file from the input LLVM
/// module.
///
/// \param [in] diags Diagnostics engine for reporting errors
/// \param [in] tm Target machine to aid the code-gen pipeline set-up
/// \param [in] act Backend act to run (assembly vs machine-code generation)
/// \param [in] toolchainModule LLVM module to lower to assembly/machine-code
/// \param [in] codeGenOpts options configuring codegen pipeline
/// \param [out] os Output stream to emit the generated code to
static void generateMachineCodeOrAssemblyImpl(language::Core::DiagnosticsEngine &diags,
                                              toolchain::TargetMachine &tm,
                                              BackendActionTy act,
                                              toolchain::Module &toolchainModule,
                                              const CodeGenOptions &codeGenOpts,
                                              toolchain::raw_pwrite_stream &os) {
  assert(((act == BackendActionTy::Backend_EmitObj) ||
          (act == BackendActionTy::Backend_EmitAssembly)) &&
         "Unsupported action");

  // Set-up the pass manager, i.e create an LLVM code-gen pass pipeline.
  // Currently only the legacy pass manager is supported.
  // TODO: Switch to the new PM once it's available in the backend.
  toolchain::legacy::PassManager codeGenPasses;
  codeGenPasses.add(
      createTargetTransformInfoWrapperPass(tm.getTargetIRAnalysis()));

  toolchain::Triple triple(toolchainModule.getTargetTriple());
  toolchain::TargetLibraryInfoImpl *tlii =
      toolchain::driver::createTLII(triple, codeGenOpts.getVecLib());
  codeGenPasses.add(new toolchain::TargetLibraryInfoWrapperPass(*tlii));

  toolchain::CodeGenFileType cgft = (act == BackendActionTy::Backend_EmitAssembly)
                                   ? toolchain::CodeGenFileType::AssemblyFile
                                   : toolchain::CodeGenFileType::ObjectFile;
  if (tm.addPassesToEmitFile(codeGenPasses, os, nullptr, cgft)) {
    unsigned diagID =
        diags.getCustomDiagID(language::Core::DiagnosticsEngine::Error,
                              "emission of this file type is not supported");
    diags.Report(diagID);
    return;
  }

  // Run the passes
  codeGenPasses.run(toolchainModule);

  // Cleanup
  delete tlii;
}

void CodeGenAction::runOptimizationPipeline(toolchain::raw_pwrite_stream &os) {
  CompilerInstance &ci = getInstance();
  const CodeGenOptions &opts = ci.getInvocation().getCodeGenOpts();
  language::Core::DiagnosticsEngine &diags = ci.getDiagnostics();
  toolchain::OptimizationLevel level = mapToLevel(opts);

  toolchain::TargetMachine *targetMachine = &ci.getTargetMachine();
  // Create the analysis managers.
  toolchain::LoopAnalysisManager lam;
  toolchain::FunctionAnalysisManager fam;
  toolchain::CGSCCAnalysisManager cgam;
  toolchain::ModuleAnalysisManager mam;

  // Create the pass manager builder.
  toolchain::PassInstrumentationCallbacks pic;
  toolchain::PipelineTuningOptions pto;
  std::optional<toolchain::PGOOptions> pgoOpt;

  if (opts.hasProfileIRInstr()) {
    // -fprofile-generate.
    pgoOpt = toolchain::PGOOptions(opts.InstrProfileOutput.empty()
                                  ? toolchain::driver::getDefaultProfileGenName()
                                  : opts.InstrProfileOutput,
                              "", "", opts.MemoryProfileUsePath, nullptr,
                              toolchain::PGOOptions::IRInstr,
                              toolchain::PGOOptions::NoCSAction,
                              toolchain::PGOOptions::ColdFuncOpt::Default, false,
                              /*PseudoProbeForProfiling=*/false, false);
  } else if (opts.hasProfileIRUse()) {
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS =
        toolchain::vfs::getRealFileSystem();
    // -fprofile-use.
    auto CSAction = opts.hasProfileCSIRUse() ? toolchain::PGOOptions::CSIRUse
                                             : toolchain::PGOOptions::NoCSAction;
    pgoOpt = toolchain::PGOOptions(
        opts.ProfileInstrumentUsePath, "", opts.ProfileRemappingFile,
        opts.MemoryProfileUsePath, VFS, toolchain::PGOOptions::IRUse, CSAction,
        toolchain::PGOOptions::ColdFuncOpt::Default, false);
  }

  toolchain::StandardInstrumentations si(toolchainModule->getContext(),
                                    opts.DebugPassManager);
  si.registerCallbacks(pic, &mam);
  if (ci.isTimingEnabled())
    si.getTimePasses().setOutStream(ci.getTimingStreamLLVM());
  pto.LoopUnrolling = opts.UnrollLoops;
  pto.LoopInterchange = opts.InterchangeLoops;
  pto.LoopInterleaving = opts.UnrollLoops;
  pto.LoopVectorization = opts.VectorizeLoop;
  pto.SLPVectorization = opts.VectorizeSLP;

  toolchain::PassBuilder pb(targetMachine, pto, pgoOpt, &pic);

  // Attempt to load pass plugins and register their callbacks with PB.
  for (auto &pluginFile : opts.LLVMPassPlugins) {
    auto passPlugin = toolchain::PassPlugin::Load(pluginFile);
    if (passPlugin) {
      passPlugin->registerPassBuilderCallbacks(pb);
    } else {
      diags.Report(language::Core::diag::err_fe_unable_to_load_plugin)
          << pluginFile << passPlugin.takeError();
    }
  }
  // Register static plugin extensions.
#define HANDLE_EXTENSION(Ext)                                                  \
  get##Ext##PluginInfo().RegisterPassBuilderCallbacks(pb);
#include "toolchain/Support/Extension.def"

  // Register the target library analysis directly and give it a customized
  // preset TLI depending on -fveclib
  toolchain::Triple triple(toolchainModule->getTargetTriple());
  toolchain::TargetLibraryInfoImpl *tlii =
      toolchain::driver::createTLII(triple, opts.getVecLib());
  fam.registerPass([&] { return toolchain::TargetLibraryAnalysis(*tlii); });

  // Register all the basic analyses with the managers.
  pb.registerModuleAnalyses(mam);
  pb.registerCGSCCAnalyses(cgam);
  pb.registerFunctionAnalyses(fam);
  pb.registerLoopAnalyses(lam);
  pb.crossRegisterProxies(lam, fam, cgam, mam);

  // Create the pass manager.
  toolchain::ModulePassManager mpm;
  if (opts.PrepareForFullLTO)
    mpm = pb.buildLTOPreLinkDefaultPipeline(level);
  else if (opts.PrepareForThinLTO)
    mpm = pb.buildThinLTOPreLinkDefaultPipeline(level);
  else
    mpm = pb.buildPerModuleDefaultPipeline(level);

  if (action == BackendActionTy::Backend_EmitBC)
    mpm.addPass(toolchain::BitcodeWriterPass(os));
  else if (action == BackendActionTy::Backend_EmitLL)
    mpm.addPass(toolchain::PrintModulePass(os));

  // FIXME: This should eventually be replaced by a first-class driver option.
  // This should be done for both flang and clang simultaneously.
  // Print a textual, '-passes=' compatible, representation of pipeline if
  // requested. In this case, don't run the passes. This mimics the behavior of
  // clang.
  if (toolchain::PrintPipelinePasses) {
    mpm.printPipeline(toolchain::outs(), [&pic](toolchain::StringRef className) {
      auto passName = pic.getPassNameForClassName(className);
      return passName.empty() ? className : passName;
    });
    toolchain::outs() << "\n";
    return;
  }

  // Run the passes.
  mpm.run(*toolchainModule, mam);

  // Print the timers to the associated output stream and reset them.
  if (ci.isTimingEnabled())
    si.getTimePasses().print();

  // Cleanup
  delete tlii;
}

// This class handles optimization remark messages requested if
// any of -Rpass, -Rpass-analysis or -Rpass-missed flags were provided
class BackendRemarkConsumer : public toolchain::DiagnosticHandler {

  const CodeGenOptions &codeGenOpts;
  language::Core::DiagnosticsEngine &diags;

public:
  BackendRemarkConsumer(language::Core::DiagnosticsEngine &diags,
                        const CodeGenOptions &codeGenOpts)
      : codeGenOpts(codeGenOpts), diags(diags) {}

  bool isAnalysisRemarkEnabled(toolchain::StringRef passName) const override {
    return codeGenOpts.OptimizationRemarkAnalysis.patternMatches(passName);
  }
  bool isMissedOptRemarkEnabled(toolchain::StringRef passName) const override {
    return codeGenOpts.OptimizationRemarkMissed.patternMatches(passName);
  }
  bool isPassedOptRemarkEnabled(toolchain::StringRef passName) const override {
    return codeGenOpts.OptimizationRemark.patternMatches(passName);
  }

  bool isAnyRemarkEnabled() const override {
    return codeGenOpts.OptimizationRemarkAnalysis.hasValidPattern() ||
           codeGenOpts.OptimizationRemarkMissed.hasValidPattern() ||
           codeGenOpts.OptimizationRemark.hasValidPattern();
  }

  void
  emitOptimizationMessage(const toolchain::DiagnosticInfoOptimizationBase &diagInfo,
                          unsigned diagID) {
    // We only support warnings and remarks.
    assert(diagInfo.getSeverity() == toolchain::DS_Remark ||
           diagInfo.getSeverity() == toolchain::DS_Warning);

    std::string msg;
    toolchain::raw_string_ostream msgStream(msg);

    if (diagInfo.isLocationAvailable()) {
      // Clang contains a SourceManager class which handles loading
      // and caching of source files into memory and it can be used to
      // query SourceLocation data. The SourceLocation data is what is
      // needed here as it contains the full include stack which gives
      // line and column number as well as file name and location.
      // Since Flang doesn't have SourceManager, send file name and absolute
      // path through msgStream, to use for printing.
      msgStream << diagInfo.getLocationStr() << ";;"
                << diagInfo.getAbsolutePath() << ";;";
    }

    msgStream << diagInfo.getMsg();

    // Emit message.
    diags.Report(diagID) << language::Core::AddFlagValue(diagInfo.getPassName()) << msg;
  }

  void optimizationRemarkHandler(
      const toolchain::DiagnosticInfoOptimizationBase &diagInfo) {
    auto passName = diagInfo.getPassName();
    if (diagInfo.isPassed()) {
      if (codeGenOpts.OptimizationRemark.patternMatches(passName))
        // Optimization remarks are active only if the -Rpass flag has a regular
        // expression that matches the name of the pass name in \p d.
        emitOptimizationMessage(
            diagInfo, language::Core::diag::remark_fe_backend_optimization_remark);

      return;
    }

    if (diagInfo.isMissed()) {
      if (codeGenOpts.OptimizationRemarkMissed.patternMatches(passName))
        // Missed optimization remarks are active only if the -Rpass-missed
        // flag has a regular expression that matches the name of the pass
        // name in \p d.
        emitOptimizationMessage(
            diagInfo,
            language::Core::diag::remark_fe_backend_optimization_remark_missed);

      return;
    }

    assert(diagInfo.isAnalysis() && "Unknown remark type");

    bool shouldAlwaysPrint = false;
    auto *ora = toolchain::dyn_cast<toolchain::OptimizationRemarkAnalysis>(&diagInfo);
    if (ora)
      shouldAlwaysPrint = ora->shouldAlwaysPrint();

    if (shouldAlwaysPrint ||
        codeGenOpts.OptimizationRemarkAnalysis.patternMatches(passName))
      emitOptimizationMessage(
          diagInfo,
          language::Core::diag::remark_fe_backend_optimization_remark_analysis);
  }

  bool handleDiagnostics(const toolchain::DiagnosticInfo &di) override {
    switch (di.getKind()) {
    case toolchain::DK_OptimizationRemark:
      optimizationRemarkHandler(toolchain::cast<toolchain::OptimizationRemark>(di));
      break;
    case toolchain::DK_OptimizationRemarkMissed:
      optimizationRemarkHandler(toolchain::cast<toolchain::OptimizationRemarkMissed>(di));
      break;
    case toolchain::DK_OptimizationRemarkAnalysis:
      optimizationRemarkHandler(
          toolchain::cast<toolchain::OptimizationRemarkAnalysis>(di));
      break;
    case toolchain::DK_MachineOptimizationRemark:
      optimizationRemarkHandler(
          toolchain::cast<toolchain::MachineOptimizationRemark>(di));
      break;
    case toolchain::DK_MachineOptimizationRemarkMissed:
      optimizationRemarkHandler(
          toolchain::cast<toolchain::MachineOptimizationRemarkMissed>(di));
      break;
    case toolchain::DK_MachineOptimizationRemarkAnalysis:
      optimizationRemarkHandler(
          toolchain::cast<toolchain::MachineOptimizationRemarkAnalysis>(di));
      break;
    default:
      break;
    }
    return true;
  }
};

void CodeGenAction::embedOffloadObjects() {
  CompilerInstance &ci = this->getInstance();
  const auto &cgOpts = ci.getInvocation().getCodeGenOpts();

  for (toolchain::StringRef offloadObject : cgOpts.OffloadObjects) {
    toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> objectOrErr =
        toolchain::MemoryBuffer::getFileOrSTDIN(offloadObject);
    if (std::error_code ec = objectOrErr.getError()) {
      auto diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "could not open '%0' for embedding");
      ci.getDiagnostics().Report(diagID) << offloadObject;
      return;
    }
    toolchain::embedBufferInModule(
        *toolchainModule, **objectOrErr, ".toolchain.offloading",
        toolchain::Align(toolchain::object::OffloadBinary::getAlignment()));
  }
}

void CodeGenAction::linkBuiltinBCLibs() {
  auto options = language::Core::FileSystemOptions();
  language::Core::FileManager fileManager(options);
  CompilerInstance &ci = this->getInstance();
  const auto &cgOpts = ci.getInvocation().getCodeGenOpts();

  std::vector<std::unique_ptr<toolchain::Module>> modules;

  // Load LLVM modules
  for (toolchain::StringRef bcLib : cgOpts.BuiltinBCLibs) {
    auto BCBuf = fileManager.getBufferForFile(bcLib);
    if (!BCBuf) {
      auto diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "could not open '%0' for linking");
      ci.getDiagnostics().Report(diagID) << bcLib;
      return;
    }

    toolchain::Expected<std::unique_ptr<toolchain::Module>> ModuleOrErr =
        getOwningLazyBitcodeModule(std::move(*BCBuf), *toolchainCtx);
    if (!ModuleOrErr) {
      auto diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "error loading '%0' for linking");
      ci.getDiagnostics().Report(diagID) << bcLib;
      return;
    }
    modules.push_back(std::move(ModuleOrErr.get()));
  }

  // Link modules and internalize functions
  for (auto &module : modules) {
    bool Err;
    Err = toolchain::Linker::linkModules(
        *toolchainModule, std::move(module), toolchain::Linker::Flags::LinkOnlyNeeded,
        [](toolchain::Module &M, const toolchain::StringSet<> &GVS) {
          toolchain::internalizeModule(M, [&GVS](const toolchain::GlobalValue &GV) {
            return !GV.hasName() || (GVS.count(GV.getName()) == 0);
          });
        });
    if (Err) {
      auto diagID = ci.getDiagnostics().getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "link error when linking '%0'");
      ci.getDiagnostics().Report(diagID) << module->getSourceFileName();
      return;
    }
  }
}

static void reportOptRecordError(toolchain::Error e, language::Core::DiagnosticsEngine &diags,
                                 const CodeGenOptions &codeGenOpts) {
  handleAllErrors(
      std::move(e),
      [&](const toolchain::LLVMRemarkSetupFileError &e) {
        diags.Report(language::Core::diag::err_cannot_open_file)
            << codeGenOpts.OptRecordFile << e.message();
      },
      [&](const toolchain::LLVMRemarkSetupPatternError &e) {
        diags.Report(language::Core::diag::err_drv_optimization_remark_pattern)
            << e.message() << codeGenOpts.OptRecordPasses;
      },
      [&](const toolchain::LLVMRemarkSetupFormatError &e) {
        diags.Report(language::Core::diag::err_drv_optimization_remark_format)
            << codeGenOpts.OptRecordFormat;
      });
}

void CodeGenAction::executeAction() {
  CompilerInstance &ci = this->getInstance();

  language::Core::DiagnosticsEngine &diags = ci.getDiagnostics();
  const CodeGenOptions &codeGenOpts = ci.getInvocation().getCodeGenOpts();
  const TargetOptions &targetOpts = ci.getInvocation().getTargetOpts();
  language::Compability::lower::LoweringOptions &loweringOpts =
      ci.getInvocation().getLoweringOpts();
  mlir::DefaultTimingManager &timingMgr = ci.getTimingManager();
  mlir::TimingScope &timingScopeRoot = ci.getTimingScopeRoot();

  // If the output stream is a file, generate it and define the corresponding
  // output stream. If a pre-defined output stream is available, we will use
  // that instead.
  //
  // NOTE: `os` is a smart pointer that will be destroyed at the end of this
  // method. However, it won't be written to until `codeGenPasses` is
  // destroyed. By defining `os` before `codeGenPasses`, we make sure that the
  // output stream won't be destroyed before it is written to. This only
  // applies when an output file is used (i.e. there is no pre-defined output
  // stream).
  // TODO: Revisit once the new PM is ready (i.e. when `codeGenPasses` is
  // updated to use it).
  std::unique_ptr<toolchain::raw_pwrite_stream> os;
  if (ci.isOutputStreamNull()) {
    os = getOutputStream(ci, getCurrentFileOrBufferName(), action);

    if (!os) {
      unsigned diagID = diags.getCustomDiagID(
          language::Core::DiagnosticsEngine::Error, "failed to create the output file");
      diags.Report(diagID);
      return;
    }
  }

  if (action == BackendActionTy::Backend_EmitFIR) {
    if (loweringOpts.getLowerToHighLevelFIR()) {
      lowerHLFIRToFIR();
    }
    mlirModule->print(ci.isOutputStreamNull() ? *os : ci.getOutputStream());
    return;
  }

  if (action == BackendActionTy::Backend_EmitHLFIR) {
    assert(loweringOpts.getLowerToHighLevelFIR() &&
           "Lowering must have been configured to emit HLFIR");
    mlirModule->print(ci.isOutputStreamNull() ? *os : ci.getOutputStream());
    return;
  }

  // Generate an LLVM module if it's not already present (it will already be
  // present if the input file is an LLVM IR/BC file).
  if (!toolchainModule)
    generateLLVMIR();

  // This will already have been started in generateLLVMIR(). But we need to
  // continue operating on the module, so we continue timing it.
  mlir::TimingScope timingScopeLLVMIRGen = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdLLVMIRGen, timingMgr));

  // If generating the LLVM module failed, abort! No need for further error
  // reporting since generateLLVMIR() does this already.
  if (!toolchainModule)
    return;

  // Set the triple based on the targetmachine (this comes compiler invocation
  // and the command-line target option if specified, or the default if not
  // given on the command-line).
  toolchain::TargetMachine &targetMachine = ci.getTargetMachine();

  targetMachine.Options.MCOptions.AsmVerbose = targetOpts.asmVerbose;

  const toolchain::Triple &theTriple = targetMachine.getTargetTriple();

  if (toolchainModule->getTargetTriple() != theTriple) {
    diags.Report(language::Core::diag::warn_fe_override_module) << theTriple.str();
  }

  // Always set the triple and data layout, to make sure they match and are set.
  // Note that this overwrites any datalayout stored in the LLVM-IR. This avoids
  // an assert for incompatible data layout when the code-generation happens.
  toolchainModule->setTargetTriple(theTriple);
  toolchainModule->setDataLayout(targetMachine.createDataLayout());

  // Link in builtin bitcode libraries
  if (!codeGenOpts.BuiltinBCLibs.empty())
    linkBuiltinBCLibs();

  // Embed offload objects specified with -fembed-offload-object
  if (!codeGenOpts.OffloadObjects.empty())
    embedOffloadObjects();
  timingScopeLLVMIRGen.stop();

  BackendRemarkConsumer remarkConsumer(diags, codeGenOpts);

  toolchainModule->getContext().setDiagnosticHandler(
      std::make_unique<BackendRemarkConsumer>(remarkConsumer));

  // write optimization-record
  toolchain::Expected<std::unique_ptr<toolchain::ToolOutputFile>> optRecordFileOrErr =
      setupLLVMOptimizationRemarks(
          toolchainModule->getContext(), codeGenOpts.OptRecordFile,
          codeGenOpts.OptRecordPasses, codeGenOpts.OptRecordFormat,
          /*DiagnosticsWithHotness=*/false,
          /*DiagnosticsHotnessThreshold=*/0);

  if (toolchain::Error e = optRecordFileOrErr.takeError()) {
    reportOptRecordError(std::move(e), diags, codeGenOpts);
    return;
  }

  std::unique_ptr<toolchain::ToolOutputFile> optRecordFile =
      std::move(*optRecordFileOrErr);

  if (optRecordFile) {
    optRecordFile->keep();
    optRecordFile->os().flush();
  }

  // Run LLVM's middle-end (i.e. the optimizer).
  mlir::TimingScope timingScopeLLVMIRPasses = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdLLVMIRPasses, timingMgr));
  runOptimizationPipeline(ci.isOutputStreamNull() ? *os : ci.getOutputStream());
  timingScopeLLVMIRPasses.stop();

  if (action == BackendActionTy::Backend_EmitLL ||
      action == BackendActionTy::Backend_EmitBC) {
    // This action has effectively been completed in runOptimizationPipeline.
    return;
  }

  // Run LLVM's backend and generate either assembly or machine code
  mlir::TimingScope timingScopeBackend = timingScopeRoot.nest(
      mlir::TimingIdentifier::get(timingIdBackend, timingMgr));
  if (action == BackendActionTy::Backend_EmitAssembly ||
      action == BackendActionTy::Backend_EmitObj) {
    generateMachineCodeOrAssemblyImpl(
        diags, targetMachine, action, *toolchainModule, codeGenOpts,
        ci.isOutputStreamNull() ? *os : ci.getOutputStream());
    if (timingMgr.isEnabled())
      toolchain::reportAndResetTimings(&ci.getTimingStreamCodeGen());
    return;
  }
}

void InitOnlyAction::executeAction() {
  CompilerInstance &ci = this->getInstance();
  unsigned diagID = ci.getDiagnostics().getCustomDiagID(
      language::Core::DiagnosticsEngine::Warning,
      "Use `-init-only` for testing purposes only");
  ci.getDiagnostics().Report(diagID);
}

void PluginParseTreeAction::executeAction() {}

void DebugDumpPFTAction::executeAction() {
  dumpPreFIRTree(this->getInstance());
}

language::Compability::parser::Parsing &PluginParseTreeAction::getParsing() {
  return getInstance().getParsing();
}

std::unique_ptr<toolchain::raw_pwrite_stream>
PluginParseTreeAction::createOutputFile(toolchain::StringRef extension = "") {

  std::unique_ptr<toolchain::raw_pwrite_stream> os{
      getInstance().createDefaultOutputFile(
          /*Binary=*/false, /*InFile=*/getCurrentFileOrBufferName(),
          extension)};
  return os;
}
