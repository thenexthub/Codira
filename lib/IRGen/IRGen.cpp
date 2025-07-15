//===--- IRGen.cpp - Codira LLVM IR Generation -----------------------------===//
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
//
//  This file implements the entrypoints into IR generation.
//
//===----------------------------------------------------------------------===//

#include "../Serialization/ModuleFormat.h"
#include "GenValueWitness.h"
#include "IRGenModule.h"
#include "language/ABI/MetadataValues.h"
#include "language/ABI/ObjectFile.h"
#include "language/AST/DiagnosticsIRGen.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/IRGenRequests.h"
#include "language/AST/LinkLibrary.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/SILGenRequests.h"
#include "language/AST/SILOptimizerRequests.h"
#include "language/AST/TBDGenRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/MD5Stream.h"
#include "language/Basic/Platform.h"
#include "language/Basic/STLExtras.h"
#include "language/Basic/Statistic.h"
#include "language/Basic/Version.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/ClangImporter/ClangModule.h"
#include "language/IRGen/IRGenPublic.h"
#include "language/IRGen/IRGenSILPasses.h"
#include "language/IRGen/TBDGen.h"
#include "language/LLVMPasses/Passes.h"
#include "language/LLVMPasses/PassesFwd.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILRemarkStreamer.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/PassPipeline.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Subsystems.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "toolchain/ADT/ScopeExit.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Analysis/AliasAnalysis.h"
#include "toolchain/Bitcode/BitcodeWriter.h"
#include "toolchain/Bitcode/BitcodeWriterPass.h"
#include "toolchain/CodeGen/BasicTTIImpl.h"
#include "toolchain/CodeGen/TargetSubtargetInfo.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/IR/LegacyPassManager.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/PassManager.h"
#include "toolchain/IR/ValueSymbolTable.h"
#include "toolchain/IR/Verifier.h"
#include "toolchain/IRPrinter/IRPrintingPasses.h"
#include "toolchain/Linker/Linker.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Object/ObjectFile.h"
#include "toolchain/Passes/PassBuilder.h"
#include "toolchain/Passes/PassPlugin.h"
#include "toolchain/Passes/StandardInstrumentations.h"
#include "toolchain/Remarks/Remark.h"
#include "toolchain/Remarks/RemarkStreamer.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/FormattedStream.h"
#include "toolchain/Support/Mutex.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/VirtualOutputConfig.h"
#include "toolchain/Target/TargetMachine.h"
#include "toolchain/TargetParser/SubtargetFeature.h"
#include "toolchain/Transforms/IPO.h"
#include "toolchain/Transforms/IPO/AlwaysInliner.h"
#include "toolchain/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "toolchain/Transforms/Instrumentation.h"
#include "toolchain/Transforms/Instrumentation/AddressSanitizer.h"
#include "toolchain/Transforms/Instrumentation/InstrProfiling.h"
#include "toolchain/Transforms/Instrumentation/SanitizerCoverage.h"
#include "toolchain/Transforms/Instrumentation/ThreadSanitizer.h"
#include "toolchain/Transforms/ObjCARC.h"
#include "toolchain/Transforms/Scalar.h"
#include "toolchain/Transforms/Scalar/DCE.h"

#include "toolchain/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "toolchain/IR/DiagnosticInfo.h"
#include "toolchain/IR/LLVMRemarkStreamer.h"
#include "toolchain/Support/ToolOutputFile.h"

#include <thread>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace language;
using namespace irgen;
using namespace toolchain;

#define DEBUG_TYPE "irgen"

static cl::opt<bool> DisableObjCARCContract(
    "disable-objc-arc-contract", cl::Hidden,
    cl::desc("Disable running objc arc contract for testing purposes"));

// This option is for performance benchmarking: to ensure a consistent
// performance data, modules are aligned to the page size.
// Warning: this blows up the text segment size. So use this option only for
// performance benchmarking.
static cl::opt<bool> AlignModuleToPageSize(
    "align-module-to-page-size", cl::Hidden,
    cl::desc("Align the text section of all LLVM modules to the page size"));

std::tuple<toolchain::TargetOptions, std::string, std::vector<std::string>,
           std::string>
language::getIRTargetOptions(const IRGenOptions &Opts, ASTContext &Ctx) {
  // Things that maybe we should collect from the command line:
  //   - relocation model
  //   - code model
  // FIXME: We should do this entirely through Clang, for consistency.
  TargetOptions TargetOpts;

  // Explicitly request debugger tuning for LLDB which is the default
  // on Darwin platforms but not on others.
  TargetOpts.DebuggerTuning = toolchain::DebuggerKind::LLDB;
  TargetOpts.FunctionSections = Opts.FunctionSections;

  // Set option to UseCASBackend if CAS was enabled on the command line.
  TargetOpts.UseCASBackend = Opts.UseCASBackend;

  // Set option to select the CASBackendMode.
  TargetOpts.MCOptions.CASObjMode = Opts.CASObjMode;

  auto *Clang = static_cast<ClangImporter *>(Ctx.getClangModuleLoader());

  // Set UseInitArray appropriately.
  TargetOpts.UseInitArray = Clang->getCodeGenOpts().UseInitArray;

  // Set emulated TLS in inlined C/C++ functions based on what clang is doing,
  // ie either setting the default based on the OS or -Xcc -f{no-,}emulated-tls
  // command-line flags.
  TargetOpts.EmulatedTLS = Clang->getCodeGenOpts().EmulatedTLS;

  // WebAssembly doesn't support atomics yet, see
  // https://github.com/apple/language/issues/54533 for more details.
  if (Clang->getTargetInfo().getTriple().isOSBinFormatWasm())
    TargetOpts.ThreadModel = toolchain::ThreadModel::Single;

  if (Opts.EnableGlobalISel) {
    TargetOpts.EnableGlobalISel = true;
    TargetOpts.GlobalISelAbort = GlobalISelAbortMode::DisableWithDiag;
  }

  switch (Opts.CodiraAsyncFramePointer) {
  case CodiraAsyncFramePointerKind::Never:
    TargetOpts.CodiraAsyncFramePointer = CodiraAsyncFramePointerMode::Never;
    break;
  case CodiraAsyncFramePointerKind::Auto:
    TargetOpts.CodiraAsyncFramePointer = CodiraAsyncFramePointerMode::DeploymentBased;
    break;
  case CodiraAsyncFramePointerKind::Always:
    TargetOpts.CodiraAsyncFramePointer = CodiraAsyncFramePointerMode::Always;
    break;
  }

  clang::TargetOptions &ClangOpts = Clang->getTargetInfo().getTargetOpts();
  return std::make_tuple(TargetOpts, ClangOpts.CPU, ClangOpts.Features, ClangOpts.Triple);
}

void setModuleFlags(IRGenModule &IGM) {

  auto *Module = IGM.getModule();

  // These module flags don't affect code generation; they just let us
  // error during LTO if the user tries to combine files across ABIs.
  Module->addModuleFlag(toolchain::Module::Error, "Codira Version",
                        IRGenModule::languageVersion);

  if (IGM.getOptions().VirtualFunctionElimination ||
      IGM.getOptions().WitnessMethodElimination) {
    Module->addModuleFlag(toolchain::Module::Error, "Virtual Function Elim", 1);
  }
}

static void align(toolchain::Module *Module) {
  // For performance benchmarking: Align the module to the page size by
  // aligning the first function of the module.
    unsigned pageSize =
#if HAVE_UNISTD_H
      sysconf(_SC_PAGESIZE));
#else
      4096; // Use a default value
#endif
    for (auto I = Module->begin(), E = Module->end(); I != E; ++I) {
      if (!I->isDeclaration()) {
        I->setAlignment(toolchain::MaybeAlign(pageSize));
        break;
      }
    }
}

static void populatePGOOptions(std::optional<PGOOptions> &Out,
                               const IRGenOptions &Opts) {
  if (!Opts.UseSampleProfile.empty()) {
    Out = PGOOptions(
      /*ProfileFile=*/ Opts.UseSampleProfile,
      /*CSProfileGenFile=*/ "",
      /*ProfileRemappingFile=*/ "",
      /*MemoryProfile=*/ "",
      /*FS=*/ toolchain::vfs::getRealFileSystem(), // TODO: is this fine?
      /*Action=*/ PGOOptions::SampleUse,
      /*CSPGOAction=*/ PGOOptions::NoCSAction,
      /*ColdType=*/ PGOOptions::ColdFuncOpt::Default,
      /*DebugInfoForProfiling=*/ Opts.DebugInfoForProfiling
    );
    return;
  }

  if (Opts.DebugInfoForProfiling) {
    Out = PGOOptions(
        /*ProfileFile=*/ "",
        /*CSProfileGenFile=*/ "",
        /*ProfileRemappingFile=*/ "",
        /*MemoryProfile=*/ "",
        /*FS=*/ nullptr,
        /*Action=*/ PGOOptions::NoAction,
        /*CSPGOAction=*/ PGOOptions::NoCSAction,
        /*ColdType=*/ PGOOptions::ColdFuncOpt::Default,
        /*DebugInfoForProfiling=*/ true
    );
    return;
  }
}

template <typename... ArgTypes>
void diagnoseSync(
    DiagnosticEngine &Diags, toolchain::sys::Mutex *DiagMutex, SourceLoc Loc,
    Diag<ArgTypes...> ID,
    typename language::detail::PassArgument<ArgTypes>::type... Args) {
  std::optional<toolchain::sys::ScopedLock> Lock;
  if (DiagMutex)
    Lock.emplace(*DiagMutex);

  Diags.diagnose(Loc, ID, std::move(Args)...);
}

void language::performLLVMOptimizations(const IRGenOptions &Opts,
                                     DiagnosticEngine &Diags,
                                     toolchain::sys::Mutex *DiagMutex,
                                     toolchain::Module *Module,
                                     toolchain::TargetMachine *TargetMachine,
                                     toolchain::raw_pwrite_stream *out) {
  std::optional<PGOOptions> PGOOpt;
  populatePGOOptions(PGOOpt, Opts);

  PipelineTuningOptions PTO;

  bool RunCodiraSpecificLLVMOptzns =
      !Opts.DisableCodiraSpecificLLVMOptzns && !Opts.DisableLLVMOptzns;

  bool DoHotColdSplit = false;
  PTO.CallGraphProfile = false;

  toolchain::OptimizationLevel level = toolchain::OptimizationLevel::O0;
  if (Opts.shouldOptimize() && !Opts.DisableLLVMOptzns) {
    // For historical reasons, loop interleaving is set to mirror setting for
    // loop unrolling.
    PTO.LoopInterleaving = true;
    PTO.LoopVectorization = true;
    PTO.SLPVectorization = true;
    PTO.MergeFunctions = !Opts.DisableLLVMMergeFunctions;
    // Splitting trades code size to enhance memory locality, avoid in -Osize.
    DoHotColdSplit = Opts.EnableHotColdSplit && !Opts.optimizeForSize();
    level = toolchain::OptimizationLevel::Os;
  } else {
    level = toolchain::OptimizationLevel::O0;
  }

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  bool DebugPassStructure = false;
  PassInstrumentationCallbacks PIC;
  PrintPassOptions PrintPassOpts;
  PrintPassOpts.Indent = DebugPassStructure;
  PrintPassOpts.SkipAnalyses = DebugPassStructure;
  StandardInstrumentations SI(Module->getContext(), DebugPassStructure,
                              Opts.VerifyEach, PrintPassOpts);
  SI.registerCallbacks(PIC, &MAM);

  PassBuilder PB(TargetMachine, PTO, PGOOpt, &PIC);

  // Attempt to load pass plugins and register their callbacks with PB.
  for (const auto &PluginFile : Opts.LLVMPassPlugins) {
    Expected<PassPlugin> PassPlugin = PassPlugin::Load(PluginFile);
    if (PassPlugin) {
      PassPlugin->registerPassBuilderCallbacks(PB);
    } else {
      diagnoseSync(Diags, DiagMutex, SourceLoc(),
                   diag::unable_to_load_pass_plugin, PluginFile,
                   toString(PassPlugin.takeError()));
    }
  }

  // Register the AA manager first so that our version is the one used.
  FAM.registerPass([&] {
    auto AA = PB.buildDefaultAAPipeline();
    if (RunCodiraSpecificLLVMOptzns)
      AA.registerFunctionAnalysis<CodiraAA>();
    return AA;
  });
  FAM.registerPass([&] { return CodiraAA(); });

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
  ModulePassManager MPM;

  PB.setEnableHotColdSplitting(DoHotColdSplit);

  if (RunCodiraSpecificLLVMOptzns) {
    PB.registerScalarOptimizerLateEPCallback(
        [](FunctionPassManager &FPM, OptimizationLevel Level) {
          if (Level != OptimizationLevel::O0)
            FPM.addPass(CodiraARCOptPass());
        });
    PB.registerOptimizerLastEPCallback([&](ModulePassManager &MPM,
                                          OptimizationLevel Level) {
      if (Level != OptimizationLevel::O0)
        MPM.addPass(createModuleToFunctionPassAdaptor(CodiraARCContractPass()));
      if (Level == OptimizationLevel::O0)
        MPM.addPass(AlwaysInlinerPass());
      if (Opts.EmitAsyncFramePushPopMetadata)
        MPM.addPass(AsyncEntryReturnMetadataPass());
    });
  }

  // PassBuilder adds coroutine passes per default.
  //

  if (Opts.Sanitizers & SanitizerKind::Address) {
    PB.registerOptimizerLastEPCallback([&](ModulePassManager &MPM,
                                           OptimizationLevel Level) {
      AddressSanitizerOptions ASOpts;
      ASOpts.CompileKernel = false;
      ASOpts.Recover = bool(Opts.SanitizersWithRecoveryInstrumentation &
                            SanitizerKind::Address);
      ASOpts.UseAfterScope = false;
      ASOpts.UseAfterReturn = toolchain::AsanDetectStackUseAfterReturnMode::Runtime;
      if (Opts.SanitizerUseStableABI) {
        ASOpts.MaxInlinePoisoningSize = 0;
        ASOpts.InstrumentationWithCallsThreshold = 0;
        ASOpts.InsertVersionCheck = false;
      }
      MPM.addPass(AddressSanitizerPass(
          ASOpts, /*UseGlobalGC=*/true, Opts.SanitizeAddressUseODRIndicator,
          /*DestructorKind=*/toolchain::AsanDtorKind::Global));
    });
  }

  if (Opts.Sanitizers & SanitizerKind::Thread) {
    PB.registerOptimizerLastEPCallback(
        [&](ModulePassManager &MPM, OptimizationLevel Level) {
          MPM.addPass(ModuleThreadSanitizerPass());
          MPM.addPass(createModuleToFunctionPassAdaptor(ThreadSanitizerPass()));
        });
  }

  if (Opts.SanitizeCoverage.CoverageType !=
      toolchain::SanitizerCoverageOptions::SCK_None) {
    PB.registerOptimizerLastEPCallback([&](ModulePassManager &MPM,
                                           OptimizationLevel Level) {
      std::vector<std::string> allowlistFiles;
      std::vector<std::string> ignorelistFiles;
      MPM.addPass(SanitizerCoveragePass(Opts.SanitizeCoverage,
                                        allowlistFiles, ignorelistFiles));
    });
  }

  if (RunCodiraSpecificLLVMOptzns && !Opts.DisableLLVMMergeFunctions) {
    PB.registerOptimizerLastEPCallback(
        [&](ModulePassManager &MPM, OptimizationLevel Level) {
          if (Level != OptimizationLevel::O0) {
            const PointerAuthSchema &schema = Opts.PointerAuth.FunctionPointers;
            unsigned key = (schema.isEnabled() ? schema.getKey() : 0);
            MPM.addPass(CodiraMergeFunctionsPass(schema.isEnabled(), key));
          }
        });
  }

  if (Opts.GenerateProfile) {
    InstrProfOptions options;
    options.Atomic = bool(Opts.Sanitizers & SanitizerKind::Thread);
    PB.registerPipelineStartEPCallback(
        [options](ModulePassManager &MPM, OptimizationLevel level) {
           MPM.addPass(InstrProfilingLoweringPass(options, false));
        });
  }
  if (Opts.shouldOptimize()) {
    PB.registerPipelineStartEPCallback(
        [](ModulePassManager &MPM, OptimizationLevel level) {
          // Run this before SROA to avoid un-neccessary expansion of dead
          // loads.
          MPM.addPass(createModuleToFunctionPassAdaptor(DCEPass()));
        });
  }
  bool isThinLTO = Opts.LLVMLTOKind  == IRGenLLVMLTOKind::Thin;
  bool isFullLTO = Opts.LLVMLTOKind  == IRGenLLVMLTOKind::Full;
  if (!Opts.shouldOptimize() || Opts.DisableLLVMOptzns) {
    MPM = PB.buildO0DefaultPipeline(level, isFullLTO || isThinLTO);
  } else if (isThinLTO) {
    MPM = PB.buildThinLTOPreLinkDefaultPipeline(level);
  } else if (isFullLTO) {
    MPM = PB.buildLTOPreLinkDefaultPipeline(level);
  } else {
    MPM = PB.buildPerModuleDefaultPipeline(level);
  }

  // Make sure we do ARC contraction under optimization.  We don't
  // rely on any other LLVM ARC transformations, but we do need ARC
  // contraction to add the objc_retainAutoreleasedReturnValue
  // assembly markers and remove clang.arc.used.
  if (Opts.shouldOptimize() && !DisableObjCARCContract &&
      !Opts.DisableLLVMOptzns)
    MPM.addPass(createModuleToFunctionPassAdaptor(ObjCARCContractPass()));

  if (Opts.Verify) {
    // Run verification before we run the pipeline.
    ModulePassManager VerifyPM;
    VerifyPM.addPass(VerifierPass());
    VerifyPM.run(*Module, MAM);
    // PB.registerPipelineStartEPCallback(
    //       [](ModulePassManager &MPM, OptimizationLevel Level) {
    //         MPM.addPass(VerifierPass());
    //       });

    // Run verification after we ran the pipeline;
    MPM.addPass(VerifierPass());
  }

  if (Opts.PrintInlineTree)
    MPM.addPass(InlineTreePrinterPass());

  // Add bitcode/ll output passes to pass manager.

  switch (Opts.OutputKind) {
  case IRGenOutputKind::LLVMAssemblyBeforeOptimization:
    toolchain_unreachable("Should be handled earlier.");
  case IRGenOutputKind::NativeAssembly:
  case IRGenOutputKind::ObjectFile:
  case IRGenOutputKind::Module:
    break;
  case IRGenOutputKind::LLVMAssemblyAfterOptimization:
    MPM.addPass(PrintModulePass(*out, "", /*ShouldPreserveUseListOrder=*/false,
                                /*EmitSummaryIndex=*/false));
    break;
  case IRGenOutputKind::LLVMBitcode: {
    // Emit a module summary by default for Regular LTO except ld64-based ones
    // (which use the legacy LTO API).
    bool EmitRegularLTOSummary =
        TargetMachine->getTargetTriple().getVendor() != toolchain::Triple::Apple;

    if (Opts.LLVMLTOKind == IRGenLLVMLTOKind::Thin) {
      MPM.addPass(ThinLTOBitcodeWriterPass(*out, nullptr));
    } else {
      if (EmitRegularLTOSummary) {
        Module->addModuleFlag(toolchain::Module::Error, "ThinLTO", uint32_t(0));
        // Assume other sources are compiled with -fsplit-lto-unit (it's enabled
        // by default when -flto is specified on platforms that support regular
        // lto summary.)
        Module->addModuleFlag(toolchain::Module::Error, "EnableSplitLTOUnit",
                              uint32_t(1));
      }
      MPM.addPass(BitcodeWriterPass(
          *out, /*ShouldPreserveUseListOrder*/ false, EmitRegularLTOSummary));
    }
    break;
  }
  }

  MPM.run(*Module, MAM);

  if (AlignModuleToPageSize) {
    align(Module);
  }
}

/// Computes the MD5 hash of the toolchain \p Module including the compiler version
/// and options which influence the compilation.
static MD5::MD5Result getHashOfModule(const IRGenOptions &Opts,
                                      const toolchain::Module *Module) {
  // Calculate the hash of the whole toolchain module.
  MD5Stream HashStream;
  toolchain::WriteBitcodeToFile(*Module, HashStream);

  // Update the hash with the compiler version. We want to recompile if the
  // toolchain pipeline of the compiler changed.
  HashStream << version::getCodiraFullVersion();

  // Add all options which influence the toolchain compilation but are not yet
  // reflected in the toolchain module itself.
  Opts.writeLLVMCodeGenOptionsTo(HashStream);

  MD5::MD5Result result;
  HashStream.final(result);
  return result;
}

/// Returns false if the hash of the current module \p HashData matches the
/// hash which is stored in an existing output object file.
static bool needsRecompile(StringRef OutputFilename, ArrayRef<uint8_t> HashData,
                           toolchain::GlobalVariable *HashGlobal,
                           toolchain::sys::Mutex *DiagMutex) {
  if (OutputFilename.empty())
    return true;

  auto BinaryOwner = object::createBinary(OutputFilename);
  if (!BinaryOwner) {
    consumeError(BinaryOwner.takeError());
    return true;
  }
  auto *ObjectFile = dyn_cast<object::ObjectFile>(BinaryOwner->getBinary());
  if (!ObjectFile)
    return true;

  StringRef HashSectionName = HashGlobal->getSection();
  // Strip the segment name. For mach-o the GlobalVariable's section name format
  // is <segment>,<section>.
  size_t Comma = HashSectionName.find_last_of(',');
  if (Comma != StringRef::npos)
    HashSectionName = HashSectionName.substr(Comma + 1);

  // Search for the section which holds the hash.
  for (auto &Section : ObjectFile->sections()) {
    toolchain::Expected<StringRef> SectionNameOrErr = Section.getName();
    if (!SectionNameOrErr) {
      toolchain::consumeError(SectionNameOrErr.takeError());
      continue;
    }

    StringRef SectionName = *SectionNameOrErr;
    if (SectionName == HashSectionName) {
      toolchain::Expected<toolchain::StringRef> SectionData = Section.getContents();
      if (!SectionData) {
        return true;
      }
      ArrayRef<uint8_t> PrevHashData(
          reinterpret_cast<const uint8_t *>(SectionData->data()),
          SectionData->size());
      TOOLCHAIN_DEBUG(if (PrevHashData.size() == sizeof(MD5::MD5Result)) {
        if (DiagMutex) DiagMutex->lock();
        SmallString<32> HashStr;
        MD5::stringifyResult(
            *reinterpret_cast<MD5::MD5Result *>(
                const_cast<unsigned char *>(PrevHashData.data())),
            HashStr);
        toolchain::dbgs() << OutputFilename << ": prev MD5=" << HashStr <<
          (HashData == PrevHashData ? " skipping\n" : " recompiling\n");
        if (DiagMutex) DiagMutex->unlock();
      });
      if (HashData == PrevHashData)
        return false;

      return true;
    }
  }
  return true;
}

static void countStatsPostIRGen(UnifiedStatsReporter &Stats,
                                const toolchain::Module& Module) {
  auto &C = Stats.getFrontendCounters();
  // FIXME: calculate these in constant time if possible.
  C.NumIRGlobals += Module.global_size();
  C.NumIRFunctions += Module.getFunctionList().size();
  C.NumIRAliases += Module.alias_size();
  C.NumIRIFuncs += Module.ifunc_size();
  C.NumIRNamedMetaData += Module.named_metadata_size();
  C.NumIRValueSymbols += Module.getValueSymbolTable().size();
  C.NumIRComdatSymbols += Module.getComdatSymbolTable().size();
  for (auto const &Func : Module) {
    for (auto const &BB : Func) {
      ++C.NumIRBasicBlocks;
      C.NumIRInsts += BB.size();
    }
  }
}

namespace {
  class CodiraDiagnosticHandler final : public toolchain::DiagnosticHandler {

  public:
    CodiraDiagnosticHandler(const IRGenOptions &Opts) : IRGenOpts(Opts) {}

    bool handleDiagnostics(const toolchain::DiagnosticInfo &DI) override {
      return true;
    }

    bool isAnalysisRemarkEnabled(StringRef PassName) const override {
      return IRGenOpts.AnnotateCondFailMessage &&
        PassName == "annotation-remarks";
    }
    bool isMissedOptRemarkEnabled(StringRef PassName) const override {
      return IRGenOpts.AnnotateCondFailMessage &&
        PassName == "annotation-remarks";
    }
    bool isPassedOptRemarkEnabled(StringRef PassName) const override {
      return IRGenOpts.AnnotateCondFailMessage &&
        PassName == "annotation-remarks";
    }

    bool isAnyRemarkEnabled() const override {
      return IRGenOpts.AnnotateCondFailMessage;
    }

  private:
    const IRGenOptions &IRGenOpts;
  };
}

/// Run the LLVM passes. In multi-threaded compilation this will be done for
/// multiple LLVM modules in parallel.
bool language::performLLVM(const IRGenOptions &Opts,
                        DiagnosticEngine &Diags,
                        toolchain::sys::Mutex *DiagMutex,
                        toolchain::GlobalVariable *HashGlobal,
                        toolchain::Module *Module,
                        toolchain::TargetMachine *TargetMachine,
                        StringRef OutputFilename,
                        toolchain::vfs::OutputBackend &Backend,
                        UnifiedStatsReporter *Stats) {

  if (Opts.UseIncrementalLLVMCodeGen && HashGlobal) {
    // Check if we can skip the toolchain part of the compilation if we have an
    // existing object file which was generated from the same toolchain IR.
    auto hash = getHashOfModule(Opts, Module);

    TOOLCHAIN_DEBUG(
      if (DiagMutex) DiagMutex->lock();
      SmallString<32> ResultStr;
      MD5::stringifyResult(hash, ResultStr);
      toolchain::dbgs() << OutputFilename << ": MD5=" << ResultStr << '\n';
      if (DiagMutex) DiagMutex->unlock();
    );

    ArrayRef<uint8_t> HashData(reinterpret_cast<uint8_t *>(&hash),
                               sizeof(hash));
    if (Opts.OutputKind == IRGenOutputKind::ObjectFile &&
        !Opts.PrintInlineTree && !Opts.AlwaysCompile &&
        !needsRecompile(OutputFilename, HashData, HashGlobal, DiagMutex)) {
      // The toolchain IR did not change. We don't need to re-create the object file.
      return false;
    }

    // Store the hash in the global variable so that it is written into the
    // object file.
    auto *HashConstant = ConstantDataArray::get(Module->getContext(), HashData);
    HashGlobal->setInitializer(HashConstant);
  }

  std::optional<toolchain::vfs::OutputFile> OutputFile;
  LANGUAGE_DEFER {
    if (!OutputFile)
      return;
    if (auto E = OutputFile->keep()) {
      diagnoseSync(Diags, DiagMutex, SourceLoc(), diag::error_closing_output,
                   OutputFilename, toString(std::move(E)));
    }
  };
  if (!OutputFilename.empty()) {
    // Try to open the output file.  Clobbering an existing file is fine.
    // Open in binary mode if we're doing binary output.
    toolchain::vfs::OutputConfig Config;
    if (auto E =
            Backend.createFile(OutputFilename, Config).moveInto(OutputFile)) {
      diagnoseSync(Diags, DiagMutex, SourceLoc(), diag::error_opening_output,
                   OutputFilename, toString(std::move(E)));
      return true;
    }

    if (Opts.OutputKind == IRGenOutputKind::LLVMAssemblyBeforeOptimization) {
      Module->print(*OutputFile, nullptr);
      return false;
    }
  } else {
    assert(Opts.OutputKind == IRGenOutputKind::Module && "no output specified");
  }

  auto &Ctxt = Module->getContext();
  std::unique_ptr<toolchain::DiagnosticHandler> OldDiagnosticHandler =
          Ctxt.getDiagnosticHandler();
  Ctxt.setDiagnosticHandler(std::make_unique<CodiraDiagnosticHandler>(Opts));

  performLLVMOptimizations(Opts, Diags, DiagMutex, Module, TargetMachine,
                           OutputFile ? &OutputFile->getOS() : nullptr);

  if (Stats) {
    if (DiagMutex)
      DiagMutex->lock();
    countStatsPostIRGen(*Stats, *Module);
    if (DiagMutex)
      DiagMutex->unlock();
  }

  if (OutputFilename.empty())
    return false;

  std::unique_ptr<raw_fd_ostream> CASIDFile;
  if (Opts.UseCASBackend && Opts.EmitCASIDFile &&
      Opts.CASObjMode != toolchain::CASBackendMode::CASID &&
      Opts.OutputKind == IRGenOutputKind::ObjectFile && OutputFilename != "-") {
    std::string OutputFilenameCASID = std::string(OutputFilename);
    OutputFilenameCASID.append(".casid");
    std::error_code EC;
    CASIDFile = std::make_unique<raw_fd_ostream>(OutputFilenameCASID, EC);
    if (EC) {
      diagnoseSync(Diags, DiagMutex, SourceLoc(), diag::error_opening_output,
                   OutputFilename, std::move(EC.message()));
      return true;
    }
  }

  auto res = compileAndWriteLLVM(Module, TargetMachine, Opts, Stats, Diags,
                                 *OutputFile, DiagMutex,
                                 CASIDFile ? CASIDFile.get() : nullptr);

  Ctxt.setDiagnosticHandler(std::move(OldDiagnosticHandler));

  return res;
}

bool language::compileAndWriteLLVM(
    toolchain::Module *module, toolchain::TargetMachine *targetMachine,
    const IRGenOptions &opts, UnifiedStatsReporter *stats,
    DiagnosticEngine &diags, toolchain::raw_pwrite_stream &out,
    toolchain::sys::Mutex *diagMutex, toolchain::raw_pwrite_stream *casid) {

  // Set up the final code emission pass. Bitcode/LLVM IR is emitted as part of
  // the optimization pass pipeline.
  switch (opts.OutputKind) {
  case IRGenOutputKind::LLVMAssemblyBeforeOptimization:
    toolchain_unreachable("Should be handled earlier.");
  case IRGenOutputKind::Module:
    break;
  case IRGenOutputKind::LLVMAssemblyAfterOptimization:
    break;
  case IRGenOutputKind::LLVMBitcode: {
    break;
  }
  case IRGenOutputKind::NativeAssembly:
  case IRGenOutputKind::ObjectFile: {
    legacy::PassManager EmitPasses;
    CodeGenFileType FileType;
    FileType =
        (opts.OutputKind == IRGenOutputKind::NativeAssembly ? CodeGenFileType::AssemblyFile
                                                            : CodeGenFileType::ObjectFile);
    EmitPasses.add(createTargetTransformInfoWrapperPass(
        targetMachine->getTargetIRAnalysis()));

    bool fail = targetMachine->addPassesToEmitFile(
        EmitPasses, out, nullptr, FileType, !opts.Verify, nullptr, casid);
    if (fail) {
      diagnoseSync(diags, diagMutex, SourceLoc(),
                   diag::error_codegen_init_fail);
      return true;
    }

    EmitPasses.run(*module);
    break;
  }
  }

  if (stats) {
    if (diagMutex)
      diagMutex->lock();
    stats->getFrontendCounters().NumLLVMBytesOutput += out.tell();
    if (diagMutex)
      diagMutex->unlock();
  }
  return false;
}

static void setPointerAuthOptions(PointerAuthOptions &opts,
                                  const clang::PointerAuthOptions &clangOpts,
                                  const IRGenOptions &irgenOpts) {
  // Intentionally do a slice-assignment to copy over the clang options.
  static_cast<clang::PointerAuthOptions&>(opts) = clangOpts;

  assert(clangOpts.FunctionPointers);
  if (clangOpts.FunctionPointers.getKind() != PointerAuthSchema::Kind::ARM8_3)
    return;

  using Discrimination = PointerAuthSchema::Discrimination;

  // A key suitable for code pointers that might be used anywhere in the ABI.
  auto codeKey = clangOpts.FunctionPointers.getARM8_3Key();

  // A key suitable for data pointers that might be used anywhere in the ABI.
  // Using a data key for data pointers and vice-versa is important for
  // ABI future-proofing.
  auto dataKey = PointerAuthSchema::ARM8_3Key::ASDA;

  // A key suitable for code pointers that are only used in private
  // situations.  Do not use this key for any sort of signature that
  // might end up on a global constant initializer.
  auto nonABICodeKey = PointerAuthSchema::ARM8_3Key::ASIB;

  // A key suitable for data pointers that are only used in private
  // situations.  Do not use this key for any sort of signature that
  // might end up on a global constant initializer.
  auto nonABIDataKey = PointerAuthSchema::ARM8_3Key::ASDB;

  // If you change anything here, be sure to update <ptrauth.h>.
  opts.CodiraFunctionPointers =
    PointerAuthSchema(codeKey, /*address*/ false, Discrimination::Type);
  opts.KeyPaths =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.ValueWitnesses =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.ValueWitnessTable =
    PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Constant,
                      SpecialPointerAuthDiscriminators::ValueWitnessTable);
  opts.ProtocolWitnesses =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.ProtocolAssociatedTypeAccessFunctions =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.ProtocolAssociatedTypeWitnessTableAccessFunctions =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.CodiraClassMethods =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.CodiraClassMethodPointers =
    PointerAuthSchema(codeKey, /*address*/ false, Discrimination::Decl);
  opts.HeapDestructors =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);

  // Partial-apply captures are not ABI and can use a more aggressive key.
  opts.PartialApplyCapture =
    PointerAuthSchema(nonABICodeKey, /*address*/ true, Discrimination::Decl);

  opts.TypeDescriptors =
    PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);
  opts.TypeDescriptorsAsArguments =
    PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.TypeLayoutString =
    PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.CodiraDynamicReplacements =
    PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Decl);
  opts.CodiraDynamicReplacementKeys =
    PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.ProtocolConformanceDescriptors =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);
  opts.ProtocolConformanceDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.ProtocolDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.OpaqueTypeDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.ContextDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.OpaqueTypeDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.ContextDescriptorsAsArguments =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  // Coroutine resumption functions are never stored globally in the ABI,
  // so we can do some things that aren't normally okay to do.  However,
  // we can't use ASIB because that would break ARM64 interoperation.
  // The address used in the discrimination is not the address where the
  // function pointer is signed, but the address of the coroutine buffer.
  opts.YieldManyResumeFunctions =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Type);
  opts.YieldOnceResumeFunctions =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Type);
  opts.YieldOnce2ResumeFunctions =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Type);

  opts.ResilientClassStubInitCallbacks =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Constant,
      SpecialPointerAuthDiscriminators::ResilientClassStubInitCallback);

  opts.AsyncCodiraFunctionPointers =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Type);

  opts.AsyncCodiraClassMethods =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.AsyncProtocolWitnesses =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.AsyncCodiraClassMethodPointers =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.AsyncCodiraDynamicReplacements =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.AsyncPartialApplyCapture =
      PointerAuthSchema(nonABIDataKey, /*address*/ true, Discrimination::Decl);

  opts.AsyncContextParent =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Constant,
                        SpecialPointerAuthDiscriminators::AsyncContextParent);

  opts.AsyncContextResume =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Constant,
                        SpecialPointerAuthDiscriminators::AsyncContextResume);

  opts.TaskResumeFunction =
      PointerAuthSchema(codeKey, /*address*/ true, Discrimination::Constant,
                        SpecialPointerAuthDiscriminators::TaskResumeFunction);

  opts.TaskResumeContext =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Constant,
                        SpecialPointerAuthDiscriminators::TaskResumeContext);

  opts.AsyncContextExtendedFrameEntry = PointerAuthSchema(
      dataKey, /*address*/ true, Discrimination::Constant,
      SpecialPointerAuthDiscriminators::CodiraAsyncContextExtendedFrameEntry);

  opts.ExtendedExistentialTypeShape =
      PointerAuthSchema(dataKey, /*address*/ false,
                        Discrimination::Constant,
                        SpecialPointerAuthDiscriminators
                          ::ExtendedExistentialTypeShape);

  opts.NonUniqueExtendedExistentialTypeShape =
      PointerAuthSchema(dataKey, /*address*/ false,
                        Discrimination::Constant,
                        SpecialPointerAuthDiscriminators
                          ::NonUniqueExtendedExistentialTypeShape);

  opts.ClangTypeTaskContinuationFunction = PointerAuthSchema(
      codeKey, /*address*/ false, Discrimination::Constant,
      SpecialPointerAuthDiscriminators::ClangTypeTaskContinuationFunction);

  opts.GetExtraInhabitantTagFunction = PointerAuthSchema(
      codeKey, /*address*/ false, Discrimination::Constant,
      SpecialPointerAuthDiscriminators::GetExtraInhabitantTagFunction);

  opts.StoreExtraInhabitantTagFunction = PointerAuthSchema(
      codeKey, /*address*/ false, Discrimination::Constant,
      SpecialPointerAuthDiscriminators::StoreExtraInhabitantTagFunction);

  if (irgenOpts.UseRelativeProtocolWitnessTables)
    opts.RelativeProtocolWitnessTable = PointerAuthSchema(
        dataKey, /*address*/ false, Discrimination::Constant,
        SpecialPointerAuthDiscriminators::RelativeProtocolWitnessTable);

  opts.CoroCodiraFunctionPointers =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Type);

  opts.CoroCodiraClassMethods =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.CoroProtocolWitnesses =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.CoroCodiraClassMethodPointers =
      PointerAuthSchema(dataKey, /*address*/ false, Discrimination::Decl);

  opts.CoroCodiraDynamicReplacements =
      PointerAuthSchema(dataKey, /*address*/ true, Discrimination::Decl);

  opts.CoroPartialApplyCapture =
      PointerAuthSchema(nonABIDataKey, /*address*/ true, Discrimination::Decl);
}

std::unique_ptr<toolchain::TargetMachine>
language::createTargetMachine(const IRGenOptions &Opts, ASTContext &Ctx) {
  CodeGenOptLevel OptLevel = Opts.shouldOptimize()
                                   ? CodeGenOptLevel::Default // -Os
                                   : CodeGenOptLevel::None;

  // Set up TargetOptions and create the target features string.
  TargetOptions TargetOpts;
  std::string CPU;
  std::string EffectiveClangTriple;
  std::vector<std::string> targetFeaturesArray;
  std::tie(TargetOpts, CPU, targetFeaturesArray, EffectiveClangTriple)
    = getIRTargetOptions(Opts, Ctx);
  const toolchain::Triple &EffectiveTriple = toolchain::Triple(EffectiveClangTriple);
  std::string targetFeatures;
  if (!targetFeaturesArray.empty()) {
    toolchain::SubtargetFeatures features;
    for (const std::string &feature : targetFeaturesArray)
      if (!shouldRemoveTargetFeature(feature)) {
        features.AddFeature(feature);
      }
    targetFeatures = features.getString();
  }

  // Set up pointer-authentication.
  if (auto loader = Ctx.getClangModuleLoader()) {
    auto &clangInstance = loader->getClangInstance();
    if (clangInstance.getLangOpts().PointerAuthCalls) {
      // FIXME: This is gross. This needs to be done in the Frontend
      // after the module loaders are set up, and where these options are
      // formally not const.
      setPointerAuthOptions(const_cast<IRGenOptions &>(Opts).PointerAuth,
                            clangInstance.getCodeGenOpts().PointerAuth, Opts);
    }
  }

  std::string Error;
  const Target *Target =
      TargetRegistry::lookupTarget(EffectiveTriple.str(), Error);
  if (!Target) {
    Ctx.Diags.diagnose(SourceLoc(), diag::no_toolchain_target, EffectiveTriple.str(),
                       Error);
    return nullptr;
  }


  // On Cygwin 64 bit, dlls are loaded above the max address for 32 bits.
  // This means that the default CodeModel causes generated code to segfault
  // when run.
  std::optional<CodeModel::Model> cmodel = std::nullopt;
  if (EffectiveTriple.isArch64Bit() && EffectiveTriple.isWindowsCygwinEnvironment())
    cmodel = CodeModel::Large;

  // Create a target machine.
  toolchain::TargetMachine *TargetMachine = Target->createTargetMachine(
      EffectiveTriple.str(), CPU, targetFeatures, TargetOpts, Reloc::PIC_,
      cmodel, OptLevel);
  if (!TargetMachine) {
    Ctx.Diags.diagnose(SourceLoc(), diag::no_toolchain_target,
                       EffectiveTriple.str(), "no LLVM target machine");
    return nullptr;
  }
  return std::unique_ptr<toolchain::TargetMachine>(TargetMachine);
}

IRGenerator::IRGenerator(const IRGenOptions &options, SILModule &module)
  : Opts(options), SIL(module), QueueIndex(0) {
}

std::unique_ptr<toolchain::TargetMachine> IRGenerator::createTargetMachine() {
  return ::createTargetMachine(Opts, SIL.getASTContext());
}

// With -embed-bitcode, save a copy of the toolchain IR as data in the
// __LLVM,__bitcode section and save the command-line options in the
// __LLVM,__language_cmdline section.
static void embedBitcode(toolchain::Module *M, const IRGenOptions &Opts)
{
  if (Opts.EmbedMode == IRGenEmbedMode::None)
    return;

  // Save toolchain.compiler.used and remove it.
  SmallVector<toolchain::Constant*, 2> UsedArray;
  SmallVector<toolchain::GlobalValue*, 4> UsedGlobals;
  auto *UsedElementType =
    toolchain::Type::getInt8Ty(M->getContext())->getPointerTo(0);
  toolchain::GlobalVariable *Used =
    collectUsedGlobalVariables(*M, UsedGlobals, true);
  for (auto *GV : UsedGlobals) {
    if (GV->getName() != "toolchain.embedded.module" &&
        GV->getName() != "toolchain.cmdline")
      UsedArray.push_back(
          ConstantExpr::getPointerBitCastOrAddrSpaceCast(GV, UsedElementType));
  }
  if (Used)
    Used->eraseFromParent();

  // Embed the bitcode for the toolchain module.
  std::string Data;
  toolchain::raw_string_ostream OS(Data);
  if (Opts.EmbedMode == IRGenEmbedMode::EmbedBitcode)
    toolchain::WriteBitcodeToFile(*M, OS);

  ArrayRef<uint8_t> ModuleData(
      reinterpret_cast<const uint8_t *>(OS.str().data()), OS.str().size());
  toolchain::Constant *ModuleConstant =
    toolchain::ConstantDataArray::get(M->getContext(), ModuleData);
  toolchain::GlobalVariable *GV = new toolchain::GlobalVariable(*M,
                                       ModuleConstant->getType(), true,
                                       toolchain::GlobalValue::PrivateLinkage,
                                       ModuleConstant);
  UsedArray.push_back(
    toolchain::ConstantExpr::getPointerBitCastOrAddrSpaceCast(GV, UsedElementType));
  GV->setSection("__LLVM,__bitcode");
  if (toolchain::GlobalVariable *Old =
      M->getGlobalVariable("toolchain.embedded.module", true)) {
    GV->takeName(Old);
    Old->replaceAllUsesWith(GV);
    delete Old;
  } else {
    GV->setName("toolchain.embedded.module");
  }

  // Embed command-line options.
  ArrayRef<uint8_t>
      CmdData(reinterpret_cast<const uint8_t *>(Opts.CmdArgs.data()),
              Opts.CmdArgs.size());
  toolchain::Constant *CmdConstant =
    toolchain::ConstantDataArray::get(M->getContext(), CmdData);
  GV = new toolchain::GlobalVariable(*M, CmdConstant->getType(), true,
                                toolchain::GlobalValue::PrivateLinkage,
                                CmdConstant);
  GV->setSection("__LLVM,__language_cmdline");
  UsedArray.push_back(
    toolchain::ConstantExpr::getPointerBitCastOrAddrSpaceCast(GV, UsedElementType));
  if (toolchain::GlobalVariable *Old = M->getGlobalVariable("toolchain.cmdline", true)) {
    GV->takeName(Old);
    Old->replaceAllUsesWith(GV);
    delete Old;
  } else {
    GV->setName("toolchain.cmdline");
  }

  if (UsedArray.empty())
    return;

  // Recreate toolchain.compiler.used.
  auto *ATy = toolchain::ArrayType::get(UsedElementType, UsedArray.size());
  auto *NewUsed = new GlobalVariable(
           *M, ATy, false, toolchain::GlobalValue::AppendingLinkage,
           toolchain::ConstantArray::get(ATy, UsedArray), "toolchain.compiler.used");
  NewUsed->setSection("toolchain.metadata");
}

static void initLLVMModule(IRGenModule &IGM, SILModule &SIL, std::optional<unsigned> idx = {}) {
  auto *Module = IGM.getModule();
  assert(Module && "Expected toolchain:Module for IR generation!");
  
  Module->setTargetTriple(IGM.Triple.str());

  if (IGM.Context.LangOpts.SDKVersion) {
    if (Module->getSDKVersion().empty())
      Module->setSDKVersion(*IGM.Context.LangOpts.SDKVersion);
    else
      assert(Module->getSDKVersion() == *IGM.Context.LangOpts.SDKVersion);
  }

  if (!IGM.VariantTriple.str().empty()) {
    if (Module->getDarwinTargetVariantTriple().empty()) {
      Module->setDarwinTargetVariantTriple(IGM.VariantTriple.str());
    } else {
      assert(Module->getDarwinTargetVariantTriple() == IGM.VariantTriple.str());
    }
  }

  if (IGM.Context.LangOpts.VariantSDKVersion) {
    if (Module->getDarwinTargetVariantSDKVersion().empty())
      Module->setDarwinTargetVariantSDKVersion(*IGM.Context.LangOpts.VariantSDKVersion);
    else
      assert(Module->getDarwinTargetVariantSDKVersion() ==
               *IGM.Context.LangOpts.VariantSDKVersion);
  }

  // Set the module's string representation.
  Module->setDataLayout(IGM.DataLayout.getStringRepresentation());

  auto *MDNode = IGM.getModule()->getOrInsertNamedMetadata("language.module.flags");
  auto &Context = IGM.getModule()->getContext();
  auto *Value = SIL.getCodiraModule()->isStdlibModule()
              ? toolchain::ConstantInt::getTrue(Context)
              : toolchain::ConstantInt::getFalse(Context);
  MDNode->addOperand(toolchain::MDTuple::get(Context,
                                        {toolchain::MDString::get(Context,
                                                             "standard-library"),
                                         toolchain::ConstantAsMetadata::get(Value)}));

  if (auto *SILstreamer = SIL.getSILRemarkStreamer()) {
    auto remarkStream = SILstreamer->releaseStream();
    if (remarkStream) {
      // Install RemarkStreamer into LLVM and keep the remarks file alive. This is
      // required even if no LLVM remarks are enabled, because the AsmPrinter
      // serializes meta information about the remarks into the object file.
      IGM.RemarkStream = std::move(remarkStream);
      SILstreamer->intoLLVMContext(Context);
      auto &RS = *IGM.getLLVMContext().getMainRemarkStreamer();
      if (IGM.getOptions().AnnotateCondFailMessage) {
        Context.setLLVMRemarkStreamer(
           std::make_unique<toolchain::LLVMRemarkStreamer>(RS));
      } else {
        // Don't filter for now.
        Context.setLLVMRemarkStreamer(
            std::make_unique<toolchain::LLVMRemarkStreamer>(RS));
      }
    } else {
      assert(idx && "Not generating multiple output files?");

      // Construct toolchainremarkstreamer objects for LLVM remarks originating in
      // the LLVM backend and install it in the remaining LLVMModule(s).
      auto &SILOpts = SIL.getOptions();
      assert(SILOpts.AuxOptRecordFiles.size() > (*idx - 1));

      const auto &filename = SILOpts.AuxOptRecordFiles[*idx - 1];
      auto &diagEngine = SIL.getASTContext().Diags;
      std::error_code errorCode;
      auto file = std::make_unique<toolchain::raw_fd_ostream>(filename, errorCode,
                                                         toolchain::sys::fs::OF_None);
      if (errorCode) {
        diagEngine.diagnose(SourceLoc(), diag::cannot_open_file, filename,
                            errorCode.message());
        return;
      }

      const auto format = SILOpts.OptRecordFormat;
      toolchain::Expected<std::unique_ptr<toolchain::remarks::RemarkSerializer>>
        remarkSerializerOrErr = toolchain::remarks::createRemarkSerializer(
          format, toolchain::remarks::SerializerMode::Separate, *file);
      if (toolchain::Error err = remarkSerializerOrErr.takeError()) {
        diagEngine.diagnose(SourceLoc(), diag::error_creating_remark_serializer,
                            toString(std::move(err)));
        return;
      }

      auto auxRS = std::make_unique<toolchain::remarks::RemarkStreamer>(
        std::move(*remarkSerializerOrErr), filename);
      const auto passes = SILOpts.OptRecordPasses;
      if (!passes.empty()) {
        if (toolchain::Error err = auxRS->setFilter(passes)) {
          diagEngine.diagnose(SourceLoc(), diag::error_creating_remark_serializer,
                          toString(std::move(err)));
          return ;
        }
      }

      Context.setMainRemarkStreamer(std::move(auxRS));
      Context.setLLVMRemarkStreamer(
        std::make_unique<toolchain::LLVMRemarkStreamer>(
          *Context.getMainRemarkStreamer()));
      IGM.RemarkStream = std::move(file);
    }
  }
}

std::pair<IRGenerator *, IRGenModule *>
language::irgen::createIRGenModule(SILModule *SILMod, StringRef OutputFilename,
                                StringRef MainInputFilenameForDebugInfo,
                                StringRef PrivateDiscriminator,
                                IRGenOptions &Opts) {
  IRGenerator *irgen = new IRGenerator(Opts, *SILMod);
  auto targetMachine = irgen->createTargetMachine();
  if (!targetMachine) {
    delete irgen;
    return std::make_pair(nullptr, nullptr);
  }
  // Create the IR emitter.
  IRGenModule *IGM = new IRGenModule(
      *irgen, std::move(targetMachine), nullptr, "", OutputFilename,
      MainInputFilenameForDebugInfo, PrivateDiscriminator);

  initLLVMModule(*IGM, *SILMod);

  return std::pair<IRGenerator *, IRGenModule *>(irgen, IGM);
}

void language::irgen::deleteIRGenModule(
    std::pair<IRGenerator *, IRGenModule *> &IRGenPair) {
  delete IRGenPair.second;
  delete IRGenPair.first;
}

/// Run the IRGen preparation SIL pipeline. Passes have access to the
/// IRGenModule.
static void runIRGenPreparePasses(SILModule &Module,
                                  irgen::IRGenModule &IRModule) {
  auto &opts = Module.getOptions();
  auto plan = SILPassPipelinePlan::getIRGenPreparePassPipeline(opts);
  executePassPipelinePlan(&Module, plan, /*isMandatory*/ true, &IRModule);
}

namespace {
using IREntitiesToEmit = SmallVector<LinkEntity, 1>;

struct SymbolSourcesToEmit {
  SILRefsToEmit silRefsToEmit;
  IREntitiesToEmit irEntitiesToEmit;
};

static std::optional<SymbolSourcesToEmit>
getSymbolSourcesToEmit(const IRGenDescriptor &desc) {
  if (!desc.SymbolsToEmit)
    return std::nullopt;

  assert(!desc.SILMod && "Already emitted SIL?");

  // First retrieve the symbol source map to figure out what we need to build,
  // making sure to include non-public symbols.
  auto &ctx = desc.getParentModule()->getASTContext();
  auto tbdDesc = desc.getTBDGenDescriptor();
  tbdDesc.getOptions().PublicOrPackageSymbolsOnly = false;
  const auto *symbolMap =
      evaluateOrFatal(ctx.evaluator, SymbolSourceMapRequest{std::move(tbdDesc)});

  // Then split up the symbols so they can be emitted by the appropriate part
  // of the pipeline.
  SILRefsToEmit silRefsToEmit;
  IREntitiesToEmit irEntitiesToEmit;
  for (const auto &symbol : *desc.SymbolsToEmit) {
    auto itr = symbolMap->find(symbol);
    assert(itr != symbolMap->end() && "Couldn't find symbol");
    const auto &source = itr->getValue();
    switch (source.kind) {
    case SymbolSource::Kind::SIL:
      silRefsToEmit.push_back(source.getSILDeclRef());
      break;
    case SymbolSource::Kind::IR:
      irEntitiesToEmit.push_back(source.getIRLinkEntity());
      break;
    case SymbolSource::Kind::LinkerDirective:
    case SymbolSource::Kind::Unknown:
    case SymbolSource::Kind::Global:
      toolchain_unreachable("Not supported");
    }
  }
  return SymbolSourcesToEmit{silRefsToEmit, irEntitiesToEmit};
}
} // end of anonymous namespace

/// Generates LLVM IR, runs the LLVM passes and produces the output file.
/// All this is done in a single thread.
GeneratedModule IRGenRequest::evaluate(Evaluator &evaluator,
                                       IRGenDescriptor desc) const {
  const auto &Opts = desc.Opts;
  const auto &PSPs = desc.PSPs;
  auto *M = desc.getParentModule();
  auto &Ctx = M->getASTContext();
  assert(!Ctx.hadError());

  auto symsToEmit = getSymbolSourcesToEmit(desc);
  assert(!symsToEmit || symsToEmit->irEntitiesToEmit.empty() &&
         "IR symbol emission not implemented yet");

  // If we've been provided a SILModule, use it. Otherwise request the lowered
  // SIL for the file or module.
  auto SILMod = std::unique_ptr<SILModule>(desc.SILMod);
  if (!SILMod) {
    auto loweringDesc = ASTLoweringDescriptor{desc.Ctx, desc.Conv, desc.SILOpts,
                                              nullptr, std::nullopt};
    SILMod = evaluateOrFatal(Ctx.evaluator, LoweredSILRequest{loweringDesc});

    // If there was an error, bail.
    if (Ctx.hadError())
      return GeneratedModule::null();
  }

  auto filesToEmit = desc.getFilesToEmit();
  auto *primaryFile =
      dyn_cast_or_null<SourceFile>(desc.Ctx.dyn_cast<FileUnit *>());

  IRGenerator irgen(Opts, *SILMod);

  auto targetMachine = irgen.createTargetMachine();
  if (!targetMachine) return GeneratedModule::null();

  // Create the IR emitter.
  IRGenModule IGM(irgen, std::move(targetMachine), primaryFile, desc.ModuleName,
                  PSPs.OutputFilename, PSPs.MainInputFilenameForDebugInfo,
                  desc.PrivateDiscriminator);

  initLLVMModule(IGM, *SILMod);

  // Run SIL level IRGen preparation passes.
  runIRGenPreparePasses(*SILMod, IGM);

  (void)layoutStringsEnabled(IGM, /*diagnose*/ true);

  {
    FrontendStatsTracer tracer(Ctx.Stats, "IRGen");

    // Emit the module contents.
    irgen.emitGlobalTopLevel(desc.getLinkerDirectives());

    for (auto *file : filesToEmit) {
      if (auto *nextSF = dyn_cast<SourceFile>(file)) {
        IGM.emitSourceFile(*nextSF);
        if (auto *synthSFU = file->getSynthesizedFile()) {
          IGM.emitSynthesizedFileUnit(*synthSFU);
        }
      }
    }

    IGM.addLinkLibraries();

    // Okay, emit any definitions that we suddenly need.
    irgen.emitLazyDefinitions();

    // Register our info with the runtime if needed.
    if (Opts.UseJIT) {
      IGM.emitBuiltinReflectionMetadata();
      IGM.emitRuntimeRegistration();
    } else {
      // Emit protocol conformances into a section we can recognize at runtime.
      // In JIT mode these are manually registered above.
      IGM.emitCodiraProtocols(/*asContiguousArray*/ false);
      IGM.emitProtocolConformances(/*asContiguousArray*/ false);
      IGM.emitTypeMetadataRecords(/*asContiguousArray*/ false);
      IGM.emitAccessibleFunctions();
      IGM.emitBuiltinReflectionMetadata();
      IGM.emitReflectionMetadataVersion();
      irgen.emitEagerClassInitialization();
      irgen.emitObjCActorsNeedingSuperclassSwizzle();
      irgen.emitDynamicReplacements();
    }

    // Emit coverage mapping info. This needs to happen after we've emitted
    // any lazy definitions, as we need to know whether or not we emitted a
    // profiler increment for a given coverage map.
    irgen.emitCoverageMapping();

    // Emit symbols for eliminated dead methods.
    IGM.emitVTableStubs();

    // Verify type layout if we were asked to.
    if (!Opts.VerifyTypeLayoutNames.empty())
      IGM.emitTypeVerifier();

    std::for_each(Opts.LinkLibraries.begin(), Opts.LinkLibraries.end(),
                  [&](LinkLibrary linkLib) {
      IGM.addLinkLibrary(linkLib);
    });

    if (!IGM.finalize())
      return GeneratedModule::null();

    setModuleFlags(IGM);
  }

  // Bail out if there are any errors.
  if (Ctx.hadError()) return GeneratedModule::null();

  // Free the memory occupied by the SILModule.
  // Execute this task in parallel to the embedding of bitcode.
  auto SILModuleRelease = [&SILMod]() {
    SILMod.reset(nullptr);
  };
  auto Thread = std::thread(SILModuleRelease);
  // Wait for the thread to terminate.
  LANGUAGE_DEFER { Thread.join(); };

  embedBitcode(IGM.getModule(), Opts);

  // TODO: Turn the module hash into an actual output.
  if (auto **outModuleHash = desc.outModuleHash) {
    *outModuleHash = IGM.ModuleHash;
  }
  return std::move(IGM).intoGeneratedModule();
}

namespace {
struct LLVMCodeGenThreads {

  struct Thread {
    LLVMCodeGenThreads &parent;
    unsigned threadIndex;
#ifdef __APPLE__
    pthread_t threadId;
#else
    std::thread thread;
#endif

    Thread(LLVMCodeGenThreads &parent, unsigned threadIndex)
        : parent(parent), threadIndex(threadIndex)
    {}

    /// Run toolchain codegen.
    void run() {
      auto *diagMutex = parent.diagMutex;
      while (IRGenModule *IGM = parent.irgen->fetchFromQueue()) {
        TOOLCHAIN_DEBUG(diagMutex->lock();
                   dbgs() << "thread " << threadIndex << ": fetched "
                          << IGM->OutputFilename << "\n";
                   diagMutex->unlock(););
        embedBitcode(IGM->getModule(), parent.irgen->Opts);
        performLLVM(parent.irgen->Opts, IGM->Context.Diags, diagMutex,
                    IGM->ModuleHash, IGM->getModule(), IGM->TargetMachine.get(),
                    IGM->OutputFilename, IGM->Context.getOutputBackend(),
                    IGM->Context.Stats);
        if (IGM->Context.Diags.hadAnyError())
          return;
      }
      TOOLCHAIN_DEBUG(diagMutex->lock();
                 dbgs() << "thread " << threadIndex << ": done\n";
                 diagMutex->unlock(););
      return;
    }
  };

  IRGenerator *irgen;
  toolchain::sys::Mutex *diagMutex;
  std::vector<Thread> threads;

  LLVMCodeGenThreads(IRGenerator *irgen, toolchain::sys::Mutex *diagMutex,
                     unsigned numThreads)
      : irgen(irgen), diagMutex(diagMutex) {
    threads.reserve(numThreads);
    for (unsigned idx = 0; idx < numThreads; ++idx) {
      // the 0-th thread is executed by the main thread.
      threads.push_back(Thread(*this, idx + 1));
    }
  }

  static void *runThread(void *arg) {
    auto *thread = reinterpret_cast<Thread *>(arg);
    thread->run();
    return nullptr;
  }

  void startThreads() {
#ifdef __APPLE__
    // Increase the thread stack size on macosx to 8MB (default is 512KB). This
    // matches the main thread.
    pthread_attr_t stackSizeAttribute;
    int err = pthread_attr_init(&stackSizeAttribute);
    assert(!err);
    err = pthread_attr_setstacksize(&stackSizeAttribute, 8 * 1024 * 1024);
    assert(!err);

    for (auto &thread : threads) {
      pthread_create(&thread.threadId, &stackSizeAttribute,
                     LLVMCodeGenThreads::runThread, &thread);
    }

    pthread_attr_destroy(&stackSizeAttribute);
#else
    for (auto &thread : threads) {
      thread.thread = std::thread(runThread, &thread);
    }
#endif

  }

  void runMainThread() {
    Thread mainThread(*this, 0);
    mainThread.run();
  }

  void join() {
#ifdef __APPLE__
    for (auto &thread : threads)
      pthread_join(thread.threadId, 0);
#else
    for (auto &thread: threads) {
      thread.thread.join();
    }
#endif
  }
};
}

/// Generates LLVM IR, runs the LLVM passes and produces the output files.
/// All this is done in multiple threads.
static void performParallelIRGeneration(IRGenDescriptor desc) {
  const auto &Opts = desc.Opts;
  auto outputFilenames = desc.parallelOutputFilenames;
  auto SILMod = std::unique_ptr<SILModule>(desc.SILMod);
  auto *M = desc.getParentModule();

  IRGenerator irgen(Opts, *SILMod);

  // Enter a cleanup to delete all the IGMs and their associated LLVMContexts
  // that have been associated with the IRGenerator.
  struct IGMDeleter {
    IRGenerator &IRGen;
    IGMDeleter(IRGenerator &irgen) : IRGen(irgen) {}
    ~IGMDeleter() {
      for (auto it = IRGen.begin(); it != IRGen.end(); ++it) {
        IRGenModule *IGM = it->second;
        delete IGM;
      }
    }
  } _igmDeleter(irgen);

  auto OutputIter = outputFilenames.begin();
  bool IGMcreated = false;

  auto &Ctx = M->getASTContext();
  // Create an IRGenModule for each source file.
  bool DidRunSILCodeGenPreparePasses = false;
  unsigned idx = 0;
  for (auto *File : M->getFiles()) {
    auto nextSF = dyn_cast<SourceFile>(File);
    if (!nextSF)
      continue;
    
    // There must be an output filename for each source file.
    // We ignore additional output filenames.
    if (OutputIter == outputFilenames.end()) {
      Ctx.Diags.diagnose(SourceLoc(), diag::too_few_output_filenames);
      return;
    }

    auto targetMachine = irgen.createTargetMachine();
    if (!targetMachine) continue;

    // Create the IR emitter.
    auto outputName = *OutputIter++;
    IRGenModule *IGM = new IRGenModule(
        irgen, std::move(targetMachine), nextSF, desc.ModuleName, outputName,
        nextSF->getFilename(), nextSF->getPrivateDiscriminator().str());

    initLLVMModule(*IGM, *SILMod, idx++);
    if (!DidRunSILCodeGenPreparePasses) {
      // Run SIL level IRGen preparation passes on the module the first time
      // around.
      runIRGenPreparePasses(*SILMod, *IGM);
      DidRunSILCodeGenPreparePasses = true;
    }

    (void)layoutStringsEnabled(*IGM, /*diagnose*/ true);

    // Only need to do this once.
    if (!IGMcreated)
      IGM->addLinkLibraries();
    IGMcreated = true;
  }

  if (!IGMcreated) {
    // TODO: Check this already at argument parsing.
    Ctx.Diags.diagnose(SourceLoc(), diag::no_input_files_for_mt);
    return;
  }

  {
    FrontendStatsTracer tracer(Ctx.Stats, "IRGen");

    // Emit the module contents.
    irgen.emitGlobalTopLevel(desc.getLinkerDirectives());

    for (auto *File : M->getFiles()) {
      if (auto *SF = dyn_cast<SourceFile>(File)) {
        {
          CurrentIGMPtr IGM = irgen.getGenModule(SF);
          IGM->emitSourceFile(*SF);
        }

        if (auto *synthSFU = File->getSynthesizedFile()) {
          CurrentIGMPtr IGM = irgen.getGenModule(synthSFU);
          IGM->emitSynthesizedFileUnit(*synthSFU);
        }
      }
    }

    // Okay, emit any definitions that we suddenly need.
    irgen.emitLazyDefinitions();

    irgen.emitCodiraProtocols();

    irgen.emitDynamicReplacements();

    irgen.emitProtocolConformances();

    irgen.emitTypeMetadataRecords();

    irgen.emitAccessibleFunctions();

    irgen.emitReflectionMetadataVersion();

    irgen.emitEagerClassInitialization();
    irgen.emitObjCActorsNeedingSuperclassSwizzle();

    // Emit reflection metadata for builtin and imported types.
    irgen.emitBuiltinReflectionMetadata();

    // Emit coverage mapping info. This needs to happen after we've emitted
    // any lazy definitions, as we need to know whether or not we emitted a
    // profiler increment for a given coverage map.
    irgen.emitCoverageMapping();

    IRGenModule *PrimaryGM = irgen.getPrimaryIGM();

    // Emit symbols for eliminated dead methods.
    PrimaryGM->emitVTableStubs();

    // Verify type layout if we were asked to.
    if (!Opts.VerifyTypeLayoutNames.empty())
      PrimaryGM->emitTypeVerifier();
    
    std::for_each(Opts.LinkLibraries.begin(), Opts.LinkLibraries.end(),
                  [&](LinkLibrary linkLib) {
                    PrimaryGM->addLinkLibrary(linkLib);
                  });

    toolchain::DenseSet<StringRef> referencedGlobals;

    for (auto it = irgen.begin(); it != irgen.end(); ++it) {
      IRGenModule *IGM = it->second;
      toolchain::Module *M = IGM->getModule();
      auto collectReference = [&](toolchain::GlobalValue &G) {
        if (G.isDeclaration()
            && (G.getLinkage() == GlobalValue::WeakODRLinkage ||
                G.getLinkage() == GlobalValue::LinkOnceODRLinkage ||
                G.getLinkage() == GlobalValue::ExternalLinkage)) {
          referencedGlobals.insert(G.getName());
          G.setLinkage(GlobalValue::ExternalLinkage);
        }
      };
      for (toolchain::GlobalVariable &G : M->globals()) {
        collectReference(G);
      }
      for (toolchain::Function &F : M->getFunctionList()) {
        collectReference(F);
      }
      for (toolchain::GlobalAlias &A : M->aliases()) {
        collectReference(A);
      }
    }

    for (auto it = irgen.begin(); it != irgen.end(); ++it) {
      IRGenModule *IGM = it->second;
      toolchain::Module *M = IGM->getModule();

      // Update the linkage of shared functions/globals.
      // If a shared function/global is referenced from another file it must have
      // weak instead of linkonce linkage. Otherwise LLVM would remove the
      // definition (if it's not referenced in the same file).
      auto updateLinkage = [&](toolchain::GlobalValue &G) {
        if (!G.isDeclaration()
            && (G.getLinkage() == GlobalValue::WeakODRLinkage ||
                G.getLinkage() == GlobalValue::LinkOnceODRLinkage)
            && referencedGlobals.count(G.getName()) != 0) {
          G.setLinkage(GlobalValue::WeakODRLinkage);
        }
      };
      for (toolchain::GlobalVariable &G : M->globals()) {
        updateLinkage(G);
      }
      for (toolchain::Function &F : M->getFunctionList()) {
        updateLinkage(F);
      }
      for (toolchain::GlobalAlias &A : M->aliases()) {
        updateLinkage(A);
      }

      if (!IGM->finalize())
        return;

      setModuleFlags(*IGM);
    }
  }

  // Bail out if there are any errors.
  if (Ctx.hadError()) return;

  FrontendStatsTracer tracer(Ctx.Stats, "LLVM pipeline");

  toolchain::sys::Mutex DiagMutex;

  // Start all the threads and do the LLVM compilation.

  LLVMCodeGenThreads codeGenThreads(&irgen, &DiagMutex, Opts.NumThreads - 1);
  codeGenThreads.startThreads();

  // Free the memory occupied by the SILModule.
  // Execute this task in parallel to the LLVM compilation.
  auto SILModuleRelease = [&SILMod]() {
    SILMod.reset(nullptr);
  };
  auto releaseModuleThread = std::thread(SILModuleRelease);

  codeGenThreads.runMainThread();

  // Wait for all threads.
  releaseModuleThread.join();
  codeGenThreads.join();
}

GeneratedModule language::performIRGeneration(
    language::ModuleDecl *M, const IRGenOptions &Opts,
    const TBDGenOptions &TBDOpts, std::unique_ptr<SILModule> SILMod,
    StringRef ModuleName, const PrimarySpecificPaths &PSPs,
    ArrayRef<std::string> parallelOutputFilenames,
    toolchain::GlobalVariable **outModuleHash) {
  // Get a pointer to the SILModule to avoid a potential use-after-move.
  const auto *SILModPtr = SILMod.get();
  const auto &SILOpts = SILModPtr->getOptions();
  auto desc = IRGenDescriptor::forWholeModule(
      M, Opts, TBDOpts, SILOpts, SILModPtr->Types, std::move(SILMod),
      ModuleName, PSPs, /*symsToEmit*/ std::nullopt, parallelOutputFilenames,
      outModuleHash);

  if (Opts.shouldPerformIRGenerationInParallel() &&
      !parallelOutputFilenames.empty() &&
      !Opts.UseSingleModuleLLVMEmission) {
    ::performParallelIRGeneration(desc);
    // TODO: Parallel LLVM compilation cannot be used if a (single) module is
    // needed as return value.
    return GeneratedModule::null();
  }
  return evaluateOrFatal(M->getASTContext().evaluator, IRGenRequest{desc});
}

GeneratedModule language::
performIRGeneration(FileUnit *file, const IRGenOptions &Opts,
                    const TBDGenOptions &TBDOpts,
                    std::unique_ptr<SILModule> SILMod,
                    StringRef ModuleName, const PrimarySpecificPaths &PSPs,
                    StringRef PrivateDiscriminator,
                    toolchain::GlobalVariable **outModuleHash) {
  // Get a pointer to the SILModule to avoid a potential use-after-move.
  const auto *SILModPtr = SILMod.get();
  const auto &SILOpts = SILModPtr->getOptions();
  auto desc = IRGenDescriptor::forFile(
      file, Opts, TBDOpts, SILOpts, SILModPtr->Types, std::move(SILMod),
      ModuleName, PSPs, PrivateDiscriminator,
      /*symsToEmit*/ std::nullopt, outModuleHash);
  return evaluateOrFatal(file->getASTContext().evaluator, IRGenRequest{desc});
}

void language::createCodiraModuleObjectFile(SILModule &SILMod, StringRef Buffer,
                                        StringRef OutputPath) {
  auto &Ctx = SILMod.getASTContext();
  assert(!Ctx.hadError());

  IRGenOptions Opts;
  // This tool doesn't pass  the necessary runtime library path to
  // TypeConverter, because this feature isn't needed.
  Opts.DisableLegacyTypeInfo = true;

  Opts.OutputKind = IRGenOutputKind::ObjectFile;
  IRGenerator irgen(Opts, SILMod);

  auto targetMachine = irgen.createTargetMachine();
  if (!targetMachine) return;

  IRGenModule IGM(irgen, std::move(targetMachine), nullptr,
                  OutputPath, OutputPath, "", "");
  initLLVMModule(IGM, SILMod);
  auto *Ty = toolchain::ArrayType::get(IGM.Int8Ty, Buffer.size());
  auto *Data =
      toolchain::ConstantDataArray::getString(IGM.getLLVMContext(),
                                         Buffer, /*AddNull=*/false);
  auto &M = *IGM.getModule();
  auto *ASTSym = new toolchain::GlobalVariable(M, Ty, /*constant*/ true,
                                          toolchain::GlobalVariable::InternalLinkage,
                                          Data, "__Codira_AST");

  std::string Section;
  switch (IGM.TargetInfo.OutputObjectFormat) {
  case toolchain::Triple::DXContainer:
  case toolchain::Triple::GOFF:
  case toolchain::Triple::SPIRV:
  case toolchain::Triple::UnknownObjectFormat:
    toolchain_unreachable("unknown object format");
  case toolchain::Triple::XCOFF:
  case toolchain::Triple::COFF: {
    CodiraObjectFileFormatCOFF COFF;
    Section = COFF.getSectionName(ReflectionSectionKind::languageast);
    break;
  }
  case toolchain::Triple::ELF:
  case toolchain::Triple::Wasm: {
    CodiraObjectFileFormatELF ELF;
    Section = ELF.getSectionName(ReflectionSectionKind::languageast);
    break;
  }
  case toolchain::Triple::MachO: {
    CodiraObjectFileFormatMachO MachO;
    Section = std::string(*MachO.getSegmentName()) + "," +
              MachO.getSectionName(ReflectionSectionKind::languageast).str();
    break;
  }
  }
  IGM.addUsedGlobal(ASTSym);
  ASTSym->setSection(Section);
  ASTSym->setAlignment(toolchain::MaybeAlign(serialization::LANGUAGEMODULE_ALIGNMENT));
  IGM.finalize();
  ::performLLVM(Opts, Ctx.Diags, nullptr, nullptr, IGM.getModule(),
                IGM.TargetMachine.get(),
                OutputPath, Ctx.getOutputBackend(), Ctx.Stats);
}

bool language::performLLVM(const IRGenOptions &Opts, ASTContext &Ctx,
                        toolchain::Module *Module, StringRef OutputFilename) {
  // Build TargetMachine.
  auto TargetMachine = createTargetMachine(Opts, Ctx);
  if (!TargetMachine)
    return true;

  auto *Clang = static_cast<ClangImporter *>(Ctx.getClangModuleLoader());
  // Use clang's datalayout.
  Module->setDataLayout(Clang->getTargetInfo().getDataLayoutString());

  embedBitcode(Module, Opts);
  if (::performLLVM(Opts, Ctx.Diags, nullptr, nullptr, Module,
                    TargetMachine.get(), OutputFilename, Ctx.getOutputBackend(),
                    Ctx.Stats))
    return true;
  return false;
}

GeneratedModule OptimizedIRRequest::evaluate(Evaluator &evaluator,
                                             IRGenDescriptor desc) const {
  auto *parentMod = desc.getParentModule();
  auto &ctx = parentMod->getASTContext();

  // Resolve imports for all the source files.
  for (auto *file : parentMod->getFiles()) {
    if (auto *SF = dyn_cast<SourceFile>(file))
      performImportResolution(*SF);
  }

  bindExtensions(*parentMod);

  if (ctx.hadError())
    return GeneratedModule::null();

  auto irMod = evaluateOrFatal(ctx.evaluator, IRGenRequest{desc});
  if (!irMod)
    return irMod;

  performLLVMOptimizations(desc.Opts, ctx.Diags, nullptr, irMod.getModule(),
                           irMod.getTargetMachine(), desc.out);
  return irMod;
}

StringRef SymbolObjectCodeRequest::evaluate(Evaluator &evaluator,
                                            IRGenDescriptor desc) const {
  return "";
#if 0
  auto &ctx = desc.getParentModule()->getASTContext();
  auto mod = cantFail(evaluator(OptimizedIRRequest{desc}));
  auto *targetMachine = mod.getTargetMachine();

  // Add the passes to emit the LLVM module as object code.
  // TODO: Use compileAndWriteLLVM.
  legacy::PassManager emitPasses;
  emitPasses.add(createTargetTransformInfoWrapperPass(
      targetMachine->getTargetIRAnalysis()));

  SmallString<0> output;
  raw_svector_ostream os(output);
  targetMachine->addPassesToEmitFile(emitPasses, os, nullptr, CGFT_ObjectFile);
  emitPasses.run(*mod.getModule());
  os << '\0';
  return ctx.AllocateCopy(output.str());
#endif
}
