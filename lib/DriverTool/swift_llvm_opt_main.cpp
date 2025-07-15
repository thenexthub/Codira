//===--- language_toolchain_opt_main.cpp ------------------------------------------===//
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
///
/// \file
///
/// This is a simple reimplementation of opt that includes support for Codira-
/// specific LLVM passes. It is meant to make it easier to handle issues related
/// to transitioning to the new LLVM pass manager (which lacks the dynamism of
/// the old pass manager) and also problems during the code base transition to
/// that pass manager. Additionally it will enable a user to exactly simulate
/// Codira's LLVM pass pipeline by using the same pass pipeline building
/// machinery in IRGen, something not possible with opt.
///
//===----------------------------------------------------------------------===//

#include "language/Subsystems.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/AST/IRGenOptions.h"
#include "language/LLVMPasses/PassesFwd.h"
#include "language/LLVMPasses/Passes.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/Analysis/CallGraph.h"
#include "toolchain/Analysis/CallGraphSCCPass.h"
#include "toolchain/Analysis/LoopPass.h"
#include "toolchain/Analysis/RegionPass.h"
#include "toolchain/Analysis/TargetLibraryInfo.h"
#include "toolchain/Analysis/TargetTransformInfo.h"
#include "toolchain/Bitcode/BitcodeWriterPass.h"
#include "toolchain/CodeGen/CommandFlags.h"
#include "toolchain/CodeGen/TargetPassConfig.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/DebugInfo.h"
#include "toolchain/IR/IRPrintingPasses.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Passes/PassBuilder.h"
#include "toolchain/Passes/PassPlugin.h"
#include "toolchain/Passes/StandardInstrumentations.h"
#include "toolchain/IR/Verifier.h"
#include "toolchain/IRPrinter/IRPrintingPasses.h"
#include "toolchain/IRReader/IRReader.h"
#include "toolchain/InitializePasses.h"
#include "toolchain/LinkAllIR.h"
#include "toolchain/LinkAllPasses.h"
#include "toolchain/TargetParser/SubtargetFeature.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/PluginLoader.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/SourceMgr.h"
#include "toolchain/Support/SystemUtils.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Support/ToolOutputFile.h"
#include "toolchain/Target/TargetMachine.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/Transforms/Scalar/LoopPassManager.h"

using namespace language;

static toolchain::codegen::RegisterCodeGenFlags CGF;

//===----------------------------------------------------------------------===//
//                            Option Declarations
//===----------------------------------------------------------------------===//

struct CodiraLLVMOptOptions {
  toolchain::cl::opt<bool>
    Optimized = toolchain::cl::opt<bool>("O", toolchain::cl::desc("Optimization level O. Similar to language -O"));

  toolchain::cl::opt<std::string>
    TargetTriple = toolchain::cl::opt<std::string>("mtriple",
                   toolchain::cl::desc("Override target triple for module"));

  toolchain::cl::opt<bool>
    PrintStats = toolchain::cl::opt<bool>("print-stats",
                 toolchain::cl::desc("Should LLVM Statistics be printed"));

  toolchain::cl::opt<std::string>
    InputFilename = toolchain::cl::opt<std::string>(toolchain::cl::Positional,
                                            toolchain::cl::desc("<input file>"),
                                            toolchain::cl::init("-"),
                                            toolchain::cl::value_desc("filename"));

  toolchain::cl::opt<std::string>
    OutputFilename = toolchain::cl::opt<std::string>("o", toolchain::cl::desc("Override output filename"),
                     toolchain::cl::value_desc("filename"));

  toolchain::cl::opt<std::string>
    DefaultDataLayout = toolchain::cl::opt<std::string>(
      "default-data-layout",
      toolchain::cl::desc("data layout string to use if not specified by module"),
      toolchain::cl::value_desc("layout-string"), toolchain::cl::init(""));
};

static toolchain::cl::opt<std::string> PassPipeline(
    "passes",
    toolchain::cl::desc(
        "A textual description of the pass pipeline. To have analysis passes "
        "available before a certain pass, add 'require<foo-analysis>'."));
//===----------------------------------------------------------------------===//
//                               Helper Methods
//===----------------------------------------------------------------------===//

static toolchain::CodeGenOptLevel GetCodeGenOptLevel(const CodiraLLVMOptOptions &options) {
  // TODO: Is this the right thing to do here?
  if (options.Optimized)
    return toolchain::CodeGenOptLevel::Default;
  return toolchain::CodeGenOptLevel::None;
}

// Returns the TargetMachine instance or zero if no triple is provided.
static toolchain::TargetMachine *
getTargetMachine(toolchain::Triple TheTriple, StringRef CPUStr,
                 StringRef FeaturesStr, const toolchain::TargetOptions &targetOptions,
                 const CodiraLLVMOptOptions &options) {
  std::string Error;
  const auto *TheTarget = toolchain::TargetRegistry::lookupTarget(
      toolchain::codegen::getMArch(), TheTriple, Error);
  // Some modules don't specify a triple, and this is okay.
  if (!TheTarget) {
    return nullptr;
  }

  return TheTarget->createTargetMachine(
      TheTriple.getTriple(), CPUStr, FeaturesStr, targetOptions,
      std::optional<toolchain::Reloc::Model>(toolchain::codegen::getExplicitRelocModel()),
      toolchain::codegen::getExplicitCodeModel(), GetCodeGenOptLevel(options));
}

//===----------------------------------------------------------------------===//
//                            Main Implementation
//===----------------------------------------------------------------------===//

int language_toolchain_opt_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  CodiraLLVMOptOptions options;

  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(), "Codira LLVM optimizer\n");

  if (options.PrintStats)
    toolchain::EnableStatistics();

  toolchain::SMDiagnostic Err;

  // Load the input module...
  auto LLVMContext = std::make_unique<toolchain::LLVMContext>();
  std::unique_ptr<toolchain::Module> M =
      parseIRFile(options.InputFilename, Err, *LLVMContext.get());

  if (!M) {
    Err.print(argv[0], toolchain::errs());
    return 1;
  }

  if (verifyModule(*M, &toolchain::errs())) {
    toolchain::errs() << argv[0] << ": " << options.InputFilename
           << ": error: input module is broken!\n";
    return 1;
  }

  // If we are supposed to override the target triple, do so now.
  if (!options.TargetTriple.empty())
    M->setTargetTriple(toolchain::Triple::normalize(options.TargetTriple));

  // Figure out what stream we are supposed to write to...
  std::unique_ptr<toolchain::ToolOutputFile> Out;
  // Default to standard output.
  if (options.OutputFilename.empty())
    options.OutputFilename = "-";

  std::error_code EC;
  Out.reset(
      new toolchain::ToolOutputFile(options.OutputFilename, EC, toolchain::sys::fs::OF_None));
  if (EC) {
    toolchain::errs() << EC.message() << '\n';
    return 1;
  }

  toolchain::Triple ModuleTriple(M->getTargetTriple());
  std::string CPUStr, FeaturesStr;
  toolchain::TargetMachine *Machine = nullptr;
  const toolchain::TargetOptions targetOptions =
      toolchain::codegen::InitTargetOptionsFromCodeGenFlags(ModuleTriple);

  if (ModuleTriple.getArch()) {
    CPUStr = toolchain::codegen::getCPUStr();
    FeaturesStr = toolchain::codegen::getFeaturesStr();
    Machine = getTargetMachine(ModuleTriple, CPUStr, FeaturesStr, targetOptions, options);
  }

  std::unique_ptr<toolchain::TargetMachine> TM(Machine);

  // Override function attributes based on CPUStr, FeaturesStr, and command line
  // flags.
  toolchain::codegen::setFunctionAttributes(CPUStr, FeaturesStr, *M);

  if (options.Optimized) {
    IRGenOptions Opts;
    Opts.OptMode = OptimizationMode::ForSpeed;
    Opts.OutputKind = IRGenOutputKind::LLVMAssemblyAfterOptimization;

    // Then perform the optimizations.
    SourceManager SM;
    DiagnosticEngine Diags(SM);
    performLLVMOptimizations(Opts, Diags, nullptr, M.get(), TM.get(),
                             &Out->os());
  } else {
    std::string Pipeline = PassPipeline;
    toolchain::TargetLibraryInfoImpl TLII(ModuleTriple);
    if (TM)
      TM->setPGOOption(std::nullopt);
    toolchain::LoopAnalysisManager LAM;
    toolchain::FunctionAnalysisManager FAM;
    toolchain::CGSCCAnalysisManager CGAM;
    toolchain::ModuleAnalysisManager MAM;

    std::optional<toolchain::PGOOptions> P = std::nullopt;
    toolchain::PassInstrumentationCallbacks PIC;
    toolchain::PrintPassOptions PrintPassOpts;

    PrintPassOpts.Verbose = false;
    PrintPassOpts.SkipAnalyses = false;
    toolchain::StandardInstrumentations SI(M->getContext(), false, false, PrintPassOpts);
    SI.registerCallbacks(PIC, &MAM);

    toolchain::PipelineTuningOptions PTO;
    // LoopUnrolling defaults on to true and DisableLoopUnrolling is initialized
    // to false above so we shouldn't necessarily need to check whether or not the
    // option has been enabled.
    PTO.LoopUnrolling = true;
    toolchain::PassBuilder PB(TM.get(), PTO, P, &PIC);

    PB.registerPipelineParsingCallback(
                [ModuleTriple](StringRef Name, toolchain::ModulePassManager &PM,
                   ArrayRef<toolchain::PassBuilder::PipelineElement>) {
                  if (Name == "language-merge-functions") {
                    if (ModuleTriple.isArm64e())
                      PM.addPass(CodiraMergeFunctionsPass(true, 0));
                    else
                      PM.addPass(CodiraMergeFunctionsPass(false, 0));
                    return true;
                  }
                  return false;
                });
    PB.registerPipelineParsingCallback(
                [ModuleTriple](StringRef Name, toolchain::FunctionPassManager &PM,
                   ArrayRef<toolchain::PassBuilder::PipelineElement>) {
                  if (Name == "language-toolchain-arc-optimize") {
                      PM.addPass(CodiraARCOptPass());
                    return true;
                  }
                  return false;
                });
    PB.registerPipelineParsingCallback(
                [ModuleTriple](StringRef Name, toolchain::FunctionPassManager &PM,
                   ArrayRef<toolchain::PassBuilder::PipelineElement>) {
                  if (Name == "language-toolchain-arc-contract") {
                      PM.addPass(CodiraARCContractPass());
                    return true;
                  }
                  return false;
                });
    auto AA = PB.buildDefaultAAPipeline();
    AA.registerFunctionAnalysis<CodiraAA>();

    // Register the AA manager first so that our version is the one used.
    FAM.registerPass([&] { return std::move(AA); });
    FAM.registerPass([&] { return CodiraAA(); });
    // Register our TargetLibraryInfoImpl.
    FAM.registerPass([&] { return toolchain::TargetLibraryAnalysis(TLII); });

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    toolchain::ModulePassManager MPM;
    if (!Pipeline.empty()) {
      if (auto Err = PB.parsePassPipeline(MPM, Pipeline)) {
        toolchain::errs() << argv[0] << ": " << toString(std::move(Err)) << "\n";
        return 1;
      }
    }
    MPM.addPass(toolchain::PrintModulePass(Out.get()->os(), "", false, false));
    MPM.run(*M, MAM);
  }

  return 0;
}
