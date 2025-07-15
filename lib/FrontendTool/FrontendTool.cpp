//===--- FrontendTool.cpp - Codira Compiler Frontend -----------------------===//
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
/// This is the entry point to the language -frontend functionality, which
/// implements the core compiler functionality along with a number of additional
/// tools for demonstration and testing purposes.
///
/// This is separate from the rest of libFrontend to reduce the dependencies
/// required by that library.
///
//===----------------------------------------------------------------------===//

#include "language/FrontendTool/FrontendTool.h"
#include "Dependencies.h"
#include "TBD.h"
#include "language/AST/ASTDumper.h"
#include "language/AST/ASTMangler.h"
#include "language/AST/AvailabilityScope.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/FileSystem.h"
#include "language/AST/FineGrainedDependencies.h"
#include "language/AST/FineGrainedDependencyFormat.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/IRGenRequests.h"
#include "language/AST/NameLookup.h"
#include "language/AST/TBDGenRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/Edit.h"
#include "language/Basic/FileSystem.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/Platform.h"
#include "language/Basic/PrettyStackTrace.h"
#include "language/Basic/SourceManager.h"
#include "language/Basic/Statistic.h"
#include "language/Basic/SupportedFeatures.h"
#include "language/Basic/TargetInfo.h"
#include "language/Basic/UUID.h"
#include "language/Basic/Version.h"
#include "language/ConstExtract/ConstExtract.h"
#include "language/DependencyScan/ScanDependencies.h"
#include "language/Frontend/CachedDiagnostics.h"
#include "language/Frontend/CachingUtils.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/DiagnosticHelper.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/MakeStyleDependencies.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Frontend/ModuleInterfaceSupport.h"
#include "language/IRGen/TBDGen.h"
#include "language/Immediate/Immediate.h"
#include "language/Index/IndexRecord.h"
#include "language/Migrator/FixitFilter.h"
#include "language/Migrator/Migrator.h"
#include "language/Option/Options.h"
#include "language/PrintAsClang/PrintAsClang.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializationOptions.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Subsystems.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"

#include "clang/Lex/Preprocessor.h"

#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IRReader/IRReader.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/Compression.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/raw_ostream.h"

#if __has_include(<unistd.h>)
#include <unistd.h>
#elif defined(_WIN32)
#include <process.h>
#endif
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <utility>

using namespace language;

static std::string displayName(StringRef MainExecutablePath) {
  std::string Name = toolchain::sys::path::stem(MainExecutablePath).str();
  Name += " -frontend";
  return Name;
}

static void emitMakeDependenciesIfNeeded(CompilerInstance &instance) {
  instance.getInvocation()
      .getFrontendOptions()
      .InputsAndOutputs.forEachInputProducingSupplementaryOutput(
          [&](const InputFile &f) -> bool {
            return language::emitMakeDependenciesIfNeeded(instance, f);
          });
}

static void
emitLoadedModuleTraceForAllPrimariesIfNeeded(ModuleDecl *mainModule,
                                             DependencyTracker *depTracker,
                                             const FrontendOptions &opts) {
  opts.InputsAndOutputs.forEachInputProducingSupplementaryOutput(
      [&](const InputFile &input) -> bool {
        return language::emitLoadedModuleTraceIfNeeded(mainModule, depTracker,
                                                    opts, input);
      });
}

/// Writes SIL out to the given file.
static bool writeSIL(SILModule &SM, ModuleDecl *M, const SILOptions &Opts,
                     StringRef OutputFilename,
                     toolchain::vfs::OutputBackend &Backend) {
  return withOutputPath(M->getDiags(), Backend, OutputFilename,
                        [&](raw_ostream &out) -> bool {
                          SM.print(out, M, Opts);
                          return M->getASTContext().hadError();
                        });
}

static bool writeSIL(SILModule &SM, const PrimarySpecificPaths &PSPs,
                     CompilerInstance &Instance,
                     const SILOptions &Opts) {
  return writeSIL(SM, Instance.getMainModule(), Opts,
                  PSPs.OutputFilename, Instance.getOutputBackend());
}

/// Prints the Objective-C "generated header" interface for \p M to \p
/// outputPath.
/// Print the exposed "generated header" interface for \p M to \p
/// outputPath.
///
/// ...unless \p outputPath is empty, in which case it does nothing.
///
/// \returns true if there were any errors
///
/// \see language::printAsClangHeader
static bool printAsClangHeaderIfNeeded(toolchain::vfs::OutputBackend &outputBackend,
    StringRef outputPath, ModuleDecl *M, StringRef bridgingHeader,
    const FrontendOptions &frontendOpts, const IRGenOptions &irGenOpts,
    clang::HeaderSearch &clangHeaderSearchInfo) {
  if (outputPath.empty())
    return false;
  return withOutputPath(
      M->getDiags(), outputBackend, outputPath, [&](raw_ostream &out) -> bool {
        return printAsClangHeader(out, M, bridgingHeader, frontendOpts,
                                  irGenOpts, clangHeaderSearchInfo);
      });
}

/// Prints the stable module interface for \p M to \p outputPath.
///
/// ...unless \p outputPath is empty, in which case it does nothing.
///
/// \returns true if there were any errors
///
/// \see language::emitCodiraInterface
static bool
printModuleInterfaceIfNeeded(toolchain::vfs::OutputBackend &outputBackend,
                             StringRef outputPath,
                             ModuleInterfaceOptions const &Opts,
                             LangOptions const &LangOpts,
                             ModuleDecl *M) {
  if (outputPath.empty())
    return false;

  DiagnosticEngine &diags = M->getDiags();
  if (!LangOpts.isCodiraVersionAtLeast(5)) {
    assert(LangOpts.isCodiraVersionAtLeast(4));
    diags.diagnose(SourceLoc(),
                   diag::warn_unsupported_module_interface_language_version,
                   LangOpts.isCodiraVersionAtLeast(4, 2) ? "4.2" : "4");
  }
  if (M->getResilienceStrategy() != ResilienceStrategy::Resilient) {
    diags.diagnose(SourceLoc(),
                   diag::warn_unsupported_module_interface_library_evolution);
  }
  return withOutputPath(diags, outputBackend, outputPath,
                        [M, Opts](raw_ostream &out) -> bool {
                          return language::emitCodiraInterface(out, Opts, M);
                        });
}

// This is a separate function so that it shows up in stack traces.
TOOLCHAIN_ATTRIBUTE_NOINLINE
static void debugFailWithAssertion() {
  // Per the user's request, this assertion should always fail in
  // builds with assertions enabled.

  // This should not be converted to toolchain_unreachable, as those are
  // treated as optimization hints in builds where they turn into
  // __builtin_unreachable().
  assert((0) && "This is an assertion!");
}

// This is a separate function so that it shows up in stack traces.
TOOLCHAIN_ATTRIBUTE_NOINLINE
static void debugFailWithCrash() {
  TOOLCHAIN_BUILTIN_TRAP;
}

static void countStatsOfSourceFile(UnifiedStatsReporter &Stats,
                                   const CompilerInstance &Instance,
                                   SourceFile *SF) {
  auto &C = Stats.getFrontendCounters();
  auto &SM = Instance.getSourceMgr();
  C.NumDecls += SF->getTopLevelDecls().size();
  C.NumLocalTypeDecls += SF->getLocalTypeDecls().size();
  C.NumObjCMethods += SF->ObjCMethods.size();

  SmallVector<OperatorDecl *, 2> operators;
  SF->getOperatorDecls(operators);
  C.NumOperators += operators.size();

  SmallVector<PrecedenceGroupDecl *, 2> groups;
  SF->getPrecedenceGroups(groups);
  C.NumPrecedenceGroups += groups.size();

  C.NumSourceLines +=
    SM.getEntireTextForBuffer(SF->getBufferID()).count('\n');
}

static void countASTStats(UnifiedStatsReporter &Stats,
                          CompilerInstance& Instance) {
  auto &C = Stats.getFrontendCounters();
  auto &SM = Instance.getSourceMgr();
  C.NumSourceBuffers = SM.getLLVMSourceMgr().getNumBuffers();
  C.NumLinkLibraries = Instance.getLinkLibraries().size();

  auto const &AST = Instance.getASTContext();
  C.NumLoadedModules = AST.getNumLoadedModules();

  if (auto *D = Instance.getDependencyTracker()) {
    C.NumDependencies = D->getDependencies().size();
    C.NumIncrementalDependencies = D->getIncrementalDependencies().size();
    C.NumMacroPluginDependencies = D->getMacroPluginDependencies().size();
  }

  for (auto SF : Instance.getPrimarySourceFiles()) {
    auto &Ctx = SF->getASTContext();
    Ctx.evaluator.enumerateReferencesInFile(SF, [&C](const auto &ref) {
    using NodeKind = evaluator::DependencyCollector::Reference::Kind;
      switch (ref.kind) {
      case NodeKind::Empty:
      case NodeKind::Tombstone:
        toolchain_unreachable("Cannot enumerate dead dependency!");
      case NodeKind::TopLevel:
        C.NumReferencedTopLevelNames += 1;
        return;
      case NodeKind::Dynamic:
        C.NumReferencedDynamicNames += 1;
        return;
      case NodeKind::PotentialMember:
      case NodeKind::UsedMember:
        C.NumReferencedMemberNames += 1;
        return;
      }
    });
  }

  if (!Instance.getPrimarySourceFiles().empty()) {
    for (auto SF : Instance.getPrimarySourceFiles())
      countStatsOfSourceFile(Stats, Instance, SF);
  } else if (auto *M = Instance.getMainModule()) {
    // No primary source file, but a main module; this is WMO-mode
    for (auto *F : M->getFiles()) {
      if (auto *SF = dyn_cast<SourceFile>(F)) {
        countStatsOfSourceFile(Stats, Instance, SF);
      }
    }
  }
}

static void countStatsPostSILGen(UnifiedStatsReporter &Stats,
                                 const SILModule& Module) {
  auto &C = Stats.getFrontendCounters();
  // FIXME: calculate these in constant time, via the dense maps.
  C.NumSILGenFunctions += Module.getFunctionList().size();
  C.NumSILGenVtables += Module.getVTables().size();
  C.NumSILGenWitnessTables += Module.getWitnessTableList().size();
  C.NumSILGenDefaultWitnessTables += Module.getDefaultWitnessTableList().size();
  C.NumSILGenGlobalVariables += Module.getSILGlobalList().size();
}

static bool precompileBridgingHeader(const CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();
  auto clangImporter = static_cast<ClangImporter *>(
      Instance.getASTContext().getClangModuleLoader());
  auto &ImporterOpts = Invocation.getClangImporterOptions();
  auto &PCHOutDir = ImporterOpts.PrecompiledHeaderOutputDir;
  auto OutputBackend = Instance.getOutputBackend().clone();
  if (!PCHOutDir.empty()) {
    // Create or validate a persistent PCH.
    auto CodiraPCHHash = Invocation.getPCHHash();
    auto PCH = clangImporter->getOrCreatePCH(ImporterOpts, CodiraPCHHash,
                                             /*cached=*/false);
    return !PCH.has_value();
  }
  return clangImporter->emitBridgingPCH(
      opts.InputsAndOutputs.getFilenameOfFirstInput(),
      opts.InputsAndOutputs.getSingleOutputFilename(), /*cached=*/false);
}

static bool precompileClangModule(const CompilerInstance &Instance) {
  const auto &opts = Instance.getInvocation().getFrontendOptions();
  auto clangImporter = static_cast<ClangImporter *>(
      Instance.getASTContext().getClangModuleLoader());
  return clangImporter->emitPrecompiledModule(
      opts.InputsAndOutputs.getFilenameOfFirstInput(), opts.ModuleName,
      opts.InputsAndOutputs.getSingleOutputFilename());
}

static bool dumpPrecompiledClangModule(const CompilerInstance &Instance) {
  const auto &opts = Instance.getInvocation().getFrontendOptions();
  auto clangImporter = static_cast<ClangImporter *>(
      Instance.getASTContext().getClangModuleLoader());
  return clangImporter->dumpPrecompiledModule(
      opts.InputsAndOutputs.getFilenameOfFirstInput(),
      opts.InputsAndOutputs.getSingleOutputFilename());
}

static bool buildModuleFromInterface(CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const FrontendOptions &FEOpts = Invocation.getFrontendOptions();
  assert(FEOpts.InputsAndOutputs.hasSingleInput());
  StringRef InputPath = FEOpts.InputsAndOutputs.getFilenameOfFirstInput();
  StringRef PrebuiltCachePath = FEOpts.PrebuiltModuleCachePath;
  ModuleInterfaceLoaderOptions LoaderOpts(FEOpts);
  StringRef ABIPath = Instance.getPrimarySpecificPathsForAtMostOnePrimary()
                          .SupplementaryOutputs.ABIDescriptorOutputPath;
  bool IgnoreAdjacentModules = Instance.hasASTContext() &&
                               Instance.getASTContext().IgnoreAdjacentModules;

  // When building explicit module dependencies, they are
  // discovered by dependency scanner and the languagemodule is already rebuilt
  // ignoring candidate module. There is no need to serialized dependencies for
  // validation purpose because the build system (language-driver) is then
  // responsible for checking whether inputs are up-to-date.
  bool ShouldSerializeDeps = !FEOpts.ExplicitInterfaceBuild;

  // If an explicit interface build was requested, bypass the creation of a new
  // sub-instance from the interface which will build it in a separate thread,
  // and isntead directly use the current \c Instance for compilation.
  //
  // FIXME: -typecheck-module-from-interface is the exception here because
  // currently we need to ensure it still reads the flags written out
  // in the .codeinterface file itself. Instead, creation of that
  // job should incorporate those flags.
  if (FEOpts.ExplicitInterfaceBuild && !(FEOpts.isTypeCheckAction()))
    return ModuleInterfaceLoader::buildExplicitCodiraModuleFromCodiraInterface(
        Instance, Invocation.getClangModuleCachePath(),
        FEOpts.BackupModuleInterfaceDir, PrebuiltCachePath, ABIPath, InputPath,
        Invocation.getOutputFilename(), ShouldSerializeDeps,
        Invocation.getSearchPathOptions().CandidateCompiledModules);

  return ModuleInterfaceLoader::buildCodiraModuleFromCodiraInterface(
      Instance.getSourceMgr(), Instance.getDiags(),
      Invocation.getSearchPathOptions(), Invocation.getLangOptions(),
      Invocation.getClangImporterOptions(), Invocation.getCASOptions(),
      Invocation.getClangModuleCachePath(), PrebuiltCachePath,
      FEOpts.BackupModuleInterfaceDir, Invocation.getModuleName(), InputPath,
      Invocation.getOutputFilename(), ABIPath,
      FEOpts.SerializeModuleInterfaceDependencyHashes,
      FEOpts.shouldTrackSystemDependencies(), LoaderOpts,
      RequireOSSAModules_t(Invocation.getSILOptions()),
      IgnoreAdjacentModules);
}

static bool compileLLVMIR(CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &inputsAndOutputs =
      Invocation.getFrontendOptions().InputsAndOutputs;
  // Load in bitcode file.
  assert(inputsAndOutputs.hasSingleInput() &&
         "We expect a single input for bitcode input!");
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
      language::vfs::getFileOrSTDIN(Instance.getFileSystem(),
                                 inputsAndOutputs.getFilenameOfFirstInput());

  if (!FileBufOrErr) {
    Instance.getDiags().diagnose(SourceLoc(), diag::error_open_input_file,
                                 inputsAndOutputs.getFilenameOfFirstInput(),
                                 FileBufOrErr.getError().message());
    return true;
  }
  toolchain::MemoryBuffer *MainFile = FileBufOrErr.get().get();

  toolchain::SMDiagnostic Err;
  auto LLVMContext = std::make_unique<toolchain::LLVMContext>();
  std::unique_ptr<toolchain::Module> Module =
      toolchain::parseIR(MainFile->getMemBufferRef(), Err, *LLVMContext.get());
  if (!Module) {
    // TODO: Translate from the diagnostic info to the SourceManager location
    // if available.
    Instance.getDiags().diagnose(SourceLoc(), diag::error_parse_input_file,
                                 inputsAndOutputs.getFilenameOfFirstInput(),
                                 Err.getMessage());
    return true;
  }
  return performLLVM(Invocation.getIRGenOptions(), Instance.getASTContext(),
                     Module.get(), inputsAndOutputs.getSingleOutputFilename());
}

static void verifyGenericSignaturesIfNeeded(const FrontendOptions &opts,
                                            ASTContext &Context) {
  auto verifyGenericSignaturesInModule = opts.VerifyGenericSignaturesInModule;
  if (verifyGenericSignaturesInModule.empty())
    return;
  if (auto module = Context.getModuleByName(verifyGenericSignaturesInModule))
    language::validateGenericSignaturesInModule(module);
}

static bool dumpAndPrintScopeMap(const CompilerInstance &Instance,
                                 SourceFile &SF) {
  // Not const because may require reexpansion
  ASTScope &scope = SF.getScope();

  const auto &opts = Instance.getInvocation().getFrontendOptions();
  if (opts.DumpScopeMapLocations.empty()) {
    toolchain::errs() << "***Complete scope map***\n";
    scope.buildFullyExpandedTree();
    scope.print(toolchain::errs());
    return Instance.getASTContext().hadError();
  }
  // Probe each of the locations, and dump what we find.
  for (auto lineColumn : opts.DumpScopeMapLocations) {
    scope.buildFullyExpandedTree();
    scope.dumpOneScopeMapLocation(lineColumn);
  }
  return Instance.getASTContext().hadError();
}

static SourceFile &
getPrimaryOrMainSourceFile(const CompilerInstance &Instance) {
  if (SourceFile *SF = Instance.getPrimarySourceFile()) {
    return *SF;
  }
  return Instance.getMainModule()->getMainSourceFile();
}

/// Dumps the AST of all available primary source files. If corresponding output
/// files were specified, use them; otherwise, dump the AST to stdout.
static bool dumpAST(CompilerInstance &Instance,
                    ASTDumpMemberLoading memberLoading) {
  const FrontendOptions &opts = Instance.getInvocation().getFrontendOptions();
  auto dumpAST = [&](SourceFile *SF, toolchain::raw_ostream &out) {
    switch (opts.DumpASTFormat) {
    case FrontendOptions::ASTFormat::Default:
      SF->dump(out, memberLoading);
      break;
    case FrontendOptions::ASTFormat::DefaultWithDeclContext:
      language::dumpDeclContextHierarchy(out, *SF);
      SF->dump(out, memberLoading);
      break;
    case FrontendOptions::ASTFormat::JSON:
      SF->dumpJSON(out, memberLoading);
      break;
    case FrontendOptions::ASTFormat::JSONZlib:
      std::string jsonText;
      toolchain::raw_string_ostream jsonTextStream(jsonText);
      SF->dumpJSON(jsonTextStream, memberLoading);

      SmallVector<uint8_t, 0> compressed;
      toolchain::compression::zlib::compress(toolchain::arrayRefFromStringRef(jsonText),
                                        compressed);
      out << toolchain::toStringRef(compressed);
      break;
    }
  };

  auto primaryFiles = Instance.getPrimarySourceFiles();
  if (!primaryFiles.empty()) {
    for (SourceFile *sourceFile: primaryFiles) {
      auto PSPs = Instance.getPrimarySpecificPathsForSourceFile(*sourceFile);
      auto OutputFilename = PSPs.OutputFilename;
      if (withOutputPath(Instance.getASTContext().Diags,
                         Instance.getOutputBackend(), OutputFilename,
                         [&](toolchain::raw_ostream &out) -> bool {
                           dumpAST(sourceFile, out);
                           return false;
                         }))
        return true;
    }
  } else {
    // Some invocations don't have primary files. In that case, we default to
    // looking for the main file and dumping it to `stdout`.
    auto &SF = getPrimaryOrMainSourceFile(Instance);
    dumpAST(&SF, toolchain::outs());
  }
  return Instance.getASTContext().hadError();
}

static bool emitReferenceDependencies(CompilerInstance &Instance,
                                      SourceFile *const SF,
                                      StringRef outputPath) {
  const auto alsoEmitDotFile = Instance.getInvocation()
                                   .getLangOptions()
                                   .EmitFineGrainedDependencySourcefileDotFiles;

#ifndef NDEBUG
  // Before writing to the dependencies file path, preserve any previous file
  // that may have been there. No error handling -- this is just a nicety, it
  // doesn't matter if it fails.
  toolchain::sys::fs::rename(outputPath, outputPath + "~");
#endif

  using SourceFileDepGraph = fine_grained_dependencies::SourceFileDepGraph;
  return fine_grained_dependencies::withReferenceDependencies(
      SF, *Instance.getDependencyTracker(), Instance.getOutputBackend(),
      outputPath, alsoEmitDotFile, [&](SourceFileDepGraph &&g) -> bool {
        const bool hadError =
            fine_grained_dependencies::writeFineGrainedDependencyGraphToPath(
                Instance.getDiags(), Instance.getOutputBackend(), outputPath,
                g);

        // If path is stdout, cannot read it back, so check for "-"
        DEBUG_ASSERT(outputPath == "-" || g.verifyReadsWhatIsWritten(outputPath));

        if (alsoEmitDotFile)
          g.emitDotFile(Instance.getOutputBackend(), outputPath,
                        Instance.getDiags());
        return hadError;
      });
}

static void emitCodiradepsForAllPrimaryInputsIfNeeded(
    CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  if (Invocation.getFrontendOptions()
          .InputsAndOutputs.hasReferenceDependenciesFilePath() &&
      Instance.getPrimarySourceFiles().empty()) {
    Instance.getDiags().diagnose(
        SourceLoc(), diag::emit_reference_dependencies_without_primary_file);
    return;
  }

  // Do not write out languagedeps for any primaries if we've encountered an
  // error. Without this, the driver will attempt to integrate languagedeps
  // from broken language files. One way this could go wrong is if a primary that
  // fails to build in an early wave has dependents in a later wave. The
  // driver will not schedule those later dependents after this batch exits,
  // so they will have no opportunity to bring their languagedeps files up to
  // date. With this early exit, the driver sees the same priors in the
  // languagedeps files from before errors were introduced into the batch, and
  // integration therefore always hops from "known good" to "known good" states.
  //
  // FIXME: It seems more appropriate for the driver to notice the early-exit
  // and react by always enqueuing the jobs it dropped in the other waves.
  //
  // We will output a module if allowing errors, so ignore that case.
  if (Instance.getDiags().hadAnyError() &&
      !Invocation.getFrontendOptions().AllowModuleWithCompilerErrors)
    return;

  for (auto *SF : Instance.getPrimarySourceFiles()) {
    const std::string &referenceDependenciesFilePath =
        Invocation.getReferenceDependenciesFilePathForPrimary(
            SF->getFilename());
    if (referenceDependenciesFilePath.empty()) {
      continue;
    }

    emitReferenceDependencies(Instance, SF, referenceDependenciesFilePath);
  }
}

static bool emitConstValuesForWholeModuleIfNeeded(
    CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &frontendOpts = Invocation.getFrontendOptions();
  auto constExtractProtocolListPath =
      Instance.getASTContext().SearchPathOpts.ConstGatherProtocolListFilePath;
  if (constExtractProtocolListPath.empty())
    return false;
  if (!frontendOpts.InputsAndOutputs.hasConstValuesOutputPath())
    return false;
  assert(frontendOpts.InputsAndOutputs.isWholeModule() &&
         "'emitConstValuesForWholeModule' only makes sense when the whole module can be seen");
  auto ConstValuesFilePath = frontendOpts.InputsAndOutputs
    .getPrimarySpecificPathsForAtMostOnePrimary().SupplementaryOutputs
    .ConstValuesOutputPath;

  // List of protocols whose conforming nominal types
  // we should extract compile-time-known values from
  std::unordered_set<std::string> Protocols;
  bool inputParseSuccess = parseProtocolListFromFile(constExtractProtocolListPath,
                                                     Instance.getDiags(), Protocols);
  if (!inputParseSuccess)
    return true;
  auto ConstValues = gatherConstValuesForModule(Protocols,
                                                Instance.getMainModule());

  return withOutputPath(Instance.getDiags(), Instance.getOutputBackend(),
                        ConstValuesFilePath, [&](toolchain::raw_ostream &OS) {
                          writeAsJSONToFile(ConstValues, OS);
                          return false;
                        });
}

static void emitConstValuesForAllPrimaryInputsIfNeeded(
    CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  auto constExtractProtocolListPath =
      Instance.getASTContext().SearchPathOpts.ConstGatherProtocolListFilePath;
  if (constExtractProtocolListPath.empty())
    return;

  // List of protocols whose conforming nominal types
  // we should extract compile-time-known values from
  std::unordered_set<std::string> Protocols;
  bool inputParseSuccess = parseProtocolListFromFile(constExtractProtocolListPath,
                                                     Instance.getDiags(), Protocols);
  if (!inputParseSuccess)
    return;
  for (auto *SF : Instance.getPrimarySourceFiles()) {
    const std::string &ConstValuesFilePath =
        Invocation.getConstValuesFilePathForPrimary(
            SF->getFilename());
    if (ConstValuesFilePath.empty())
      continue;

    auto ConstValues = gatherConstValuesForPrimary(Protocols, SF);
    withOutputPath(Instance.getDiags(), Instance.getOutputBackend(),
                   ConstValuesFilePath, [&](toolchain::raw_ostream &OS) {
                     writeAsJSONToFile(ConstValues, OS);
                     return false;
                   });
  }
}

static bool writeModuleSemanticInfoIfNeeded(CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &frontendOpts = Invocation.getFrontendOptions();
  if (!frontendOpts.InputsAndOutputs.hasModuleSemanticInfoOutputPath())
    return false;
  std::error_code EC;
  assert(frontendOpts.InputsAndOutputs.isWholeModule() &&
         "TBDPath only makes sense when the whole module can be seen");
  auto ModuleSemanticPath = frontendOpts.InputsAndOutputs
    .getPrimarySpecificPathsForAtMostOnePrimary().SupplementaryOutputs
    .ModuleSemanticInfoOutputPath;

  return withOutputPath(Instance.getDiags(), Instance.getOutputBackend(),
                        ModuleSemanticPath, [&](toolchain::raw_ostream &OS) {
                          OS << "{}\n";
                          return false;
                        });
}

static bool writeTBDIfNeeded(CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &frontendOpts = Invocation.getFrontendOptions();
  const auto &tbdOpts = Invocation.getTBDGenOptions();
  if (!frontendOpts.InputsAndOutputs.hasTBDPath())
    return false;

  if (!frontendOpts.InputsAndOutputs.isWholeModule()) {
    Instance.getDiags().diagnose(SourceLoc(),
                                 diag::tbd_only_supported_in_whole_module);
    return false;
  }

  if (Invocation.getSILOptions().CMOMode ==
      CrossModuleOptimizationMode::Aggressive) {
    Instance.getDiags().diagnose(SourceLoc(),
                                 diag::tbd_not_supported_with_cmo);
    return false;
  }

  const std::string &TBDPath = Invocation.getTBDPathForWholeModule();

  return writeTBD(Instance.getMainModule(), TBDPath,
                  Instance.getOutputBackend(), tbdOpts);
}

static bool writeAPIDescriptor(ModuleDecl *M, StringRef OutputPath,
                               toolchain::vfs::OutputBackend &Backend) {
  return withOutputPath(M->getDiags(), Backend, OutputPath,
                        [&](raw_ostream &OS) -> bool {
                          writeAPIJSONFile(M, OS, /*PrettyPrinted=*/false);
                          return false;
                        });
}

static bool writeAPIDescriptorIfNeeded(CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &frontendOpts = Invocation.getFrontendOptions();
  if (!frontendOpts.InputsAndOutputs.hasAPIDescriptorOutputPath())
    return false;

  if (!frontendOpts.InputsAndOutputs.isWholeModule()) {
    Instance.getDiags().diagnose(
        SourceLoc(), diag::api_descriptor_only_supported_in_whole_module);
    return false;
  }

  const std::string &APIDescriptorPath =
      Invocation.getAPIDescriptorPathForWholeModule();

  return writeAPIDescriptor(Instance.getMainModule(), APIDescriptorPath,
                            Instance.getOutputBackend());
}

static bool performCompileStepsPostSILGen(CompilerInstance &Instance,
                                          std::unique_ptr<SILModule> SM,
                                          ModuleOrSourceFile MSF,
                                          const PrimarySpecificPaths &PSPs,
                                          int &ReturnValue,
                                          FrontendObserver *observer);

bool language::performCompileStepsPostSema(CompilerInstance &Instance,
                                        int &ReturnValue,
                                        FrontendObserver *observer) {
  const auto &Invocation = Instance.getInvocation();
  const FrontendOptions &opts = Invocation.getFrontendOptions();

  auto getSILOptions = [&](const PrimarySpecificPaths &PSPs,
                           const std::vector<PrimarySpecificPaths> &auxPSPs) -> SILOptions {
    SILOptions SILOpts = Invocation.getSILOptions();
    if (SILOpts.OptRecordFile.empty()) {
      // Check if the record file path was passed via supplemental outputs.
      SILOpts.OptRecordFile = SILOpts.OptRecordFormat ==
        toolchain::remarks::Format::YAML ?
          PSPs.SupplementaryOutputs.YAMLOptRecordPath :
          PSPs.SupplementaryOutputs.BitstreamOptRecordPath;
    }
    if (!auxPSPs.empty()) {
      assert(SILOpts.AuxOptRecordFiles.empty());
      for (const auto &auxFile: auxPSPs) {
        SILOpts.AuxOptRecordFiles.push_back(
          SILOpts.OptRecordFormat == toolchain::remarks::Format::YAML ?
          auxFile.SupplementaryOutputs.YAMLOptRecordPath :
          auxFile.SupplementaryOutputs.BitstreamOptRecordPath);
      }
    }
    return SILOpts;
  };

  auto *mod = Instance.getMainModule();
  if (!opts.InputsAndOutputs.hasPrimaryInputs()) {
    // If there are no primary inputs the compiler is in WMO mode and builds one
    // SILModule for the entire module.
    const PrimarySpecificPaths PSPs =
        Instance.getPrimarySpecificPathsForWholeModuleOptimizationMode();

    std::vector<PrimarySpecificPaths> auxPSPs;
    for (unsigned i = 1; i < opts.InputsAndOutputs.inputCount(); ++i) {
      auto &auxPSP =
        opts.InputsAndOutputs.getPrimarySpecificPathsForRemaining(i);
      auxPSPs.push_back(auxPSP);
    }

    SILOptions SILOpts = getSILOptions(PSPs, auxPSPs);
    IRGenOptions irgenOpts = Invocation.getIRGenOptions();
    auto SM = performASTLowering(mod, Instance.getSILTypes(), SILOpts,
                                 &irgenOpts);
    return performCompileStepsPostSILGen(Instance, std::move(SM), mod, PSPs,
                                         ReturnValue, observer);
  }


  std::vector<PrimarySpecificPaths> emptyAuxPSPs;
  // If there are primary source files, build a separate SILModule for
  // each source file, and run the remaining SILOpt-Serialize-IRGen-LLVM
  // once for each such input.
  if (!Instance.getPrimarySourceFiles().empty()) {
    bool result = false;
    for (auto *PrimaryFile : Instance.getPrimarySourceFiles()) {
      const PrimarySpecificPaths PSPs =
          Instance.getPrimarySpecificPathsForSourceFile(*PrimaryFile);
      SILOptions SILOpts = getSILOptions(PSPs, emptyAuxPSPs);
    IRGenOptions irgenOpts = Invocation.getIRGenOptions();
      auto SM = performASTLowering(*PrimaryFile, Instance.getSILTypes(),
                                   SILOpts, &irgenOpts);
      result |= performCompileStepsPostSILGen(Instance, std::move(SM),
                                              PrimaryFile, PSPs, ReturnValue,
                                              observer);
    }

    return result;
  }

  // If there are primary inputs but no primary _source files_, there might be
  // a primary serialized input.
  bool result = false;
  for (FileUnit *fileUnit : mod->getFiles()) {
    if (auto SASTF = dyn_cast<SerializedASTFile>(fileUnit))
      if (opts.InputsAndOutputs.isInputPrimary(SASTF->getFilename())) {
        const PrimarySpecificPaths &PSPs =
            Instance.getPrimarySpecificPathsForPrimary(SASTF->getFilename());
        SILOptions SILOpts = getSILOptions(PSPs, emptyAuxPSPs);
        auto SM = performASTLowering(*SASTF, Instance.getSILTypes(), SILOpts);
        result |= performCompileStepsPostSILGen(Instance, std::move(SM), mod,
                                                PSPs, ReturnValue, observer);
      }
  }

  return result;
}

static void emitIndexDataForSourceFile(SourceFile *PrimarySourceFile,
                                       const CompilerInstance &Instance);

/// Emits index data for all primary inputs, or the main module.
static void emitIndexData(const CompilerInstance &Instance) {
  if (Instance.getPrimarySourceFiles().empty()) {
    emitIndexDataForSourceFile(nullptr, Instance);
  } else {
    for (SourceFile *SF : Instance.getPrimarySourceFiles())
      emitIndexDataForSourceFile(SF, Instance);
  }
}

/// Emits all "one-per-module" supplementary outputs that don't depend on
/// anything past type-checking.
static bool emitAnyWholeModulePostTypeCheckSupplementaryOutputs(
    CompilerInstance &Instance) {
  const auto &Context = Instance.getASTContext();
  const auto &Invocation = Instance.getInvocation();
  const FrontendOptions &opts = Invocation.getFrontendOptions();

  // FIXME: Whole-module outputs with a non-whole-module action ought to
  // be disallowed, but the driver implements -index-file mode by generating a
  // regular whole-module frontend command line and modifying it to index just
  // one file (by making it a primary) instead of all of them. If that
  // invocation also has flags to emit whole-module supplementary outputs, the
  // compiler can crash trying to access information for non-type-checked
  // declarations in the non-primary files. For now, prevent those crashes by
  // guarding the emission of whole-module supplementary outputs.
  if (!opts.InputsAndOutputs.isWholeModule())
    return false;

  // Record whether we failed to emit any of these outputs, but keep going; one
  // failure does not mean skipping the rest.
  bool hadAnyError = false;

  if ((!Context.hadError() || opts.AllowModuleWithCompilerErrors) &&
      opts.InputsAndOutputs.hasClangHeaderOutputPath()) {
    std::string BridgingHeaderPathForPrint = Instance.getBridgingHeaderPath();
    if (!BridgingHeaderPathForPrint.empty()) {
      if (opts.BridgingHeaderDirForPrint.has_value()) {
        // User specified preferred directory for including, use that dir.
        toolchain::SmallString<32> Buffer(*opts.BridgingHeaderDirForPrint);
        toolchain::sys::path::append(Buffer,
          toolchain::sys::path::filename(BridgingHeaderPathForPrint));
        BridgingHeaderPathForPrint = (std::string)Buffer;
      }
    }
    hadAnyError |= printAsClangHeaderIfNeeded(
        Instance.getOutputBackend(),
        Invocation.getClangHeaderOutputPathForAtMostOnePrimary(),
        Instance.getMainModule(), BridgingHeaderPathForPrint, opts,
        Invocation.getIRGenOptions(),
        Context.getClangModuleLoader()
            ->getClangPreprocessor()
            .getHeaderSearchInfo());
  }

  // Only want the header if there's been any errors, ie. there's not much
  // point outputting a languageinterface for an invalid module
  if (Context.hadError())
    return hadAnyError;

  if (opts.InputsAndOutputs.hasModuleInterfaceOutputPath()) {
    hadAnyError |= printModuleInterfaceIfNeeded(
        Instance.getOutputBackend(),
        Invocation.getModuleInterfaceOutputPathForWholeModule(),
        Invocation.getModuleInterfaceOptions(),
        Invocation.getLangOptions(),
        Instance.getMainModule());
  }

  if (opts.InputsAndOutputs.hasPrivateModuleInterfaceOutputPath()) {
    // Copy the settings from the module interface to add SPI printing.
    ModuleInterfaceOptions privOpts = Invocation.getModuleInterfaceOptions();
    privOpts.setInterfaceMode(PrintOptions::InterfaceMode::Private);

    hadAnyError |= printModuleInterfaceIfNeeded(
        Instance.getOutputBackend(),
        Invocation.getPrivateModuleInterfaceOutputPathForWholeModule(),
        privOpts,
        Invocation.getLangOptions(),
        Instance.getMainModule());
  }
  if (opts.InputsAndOutputs.hasPackageModuleInterfaceOutputPath()) {
    // Copy the settings from the module interface to add package decl printing.
    ModuleInterfaceOptions pkgOpts = Invocation.getModuleInterfaceOptions();
    pkgOpts.setInterfaceMode(PrintOptions::InterfaceMode::Package);

    hadAnyError |= printModuleInterfaceIfNeeded(
        Instance.getOutputBackend(),
        Invocation.getPackageModuleInterfaceOutputPathForWholeModule(),
        pkgOpts,
        Invocation.getLangOptions(),
        Instance.getMainModule());
  }

  {
    hadAnyError |= writeTBDIfNeeded(Instance);
  }

  {
    hadAnyError |= writeAPIDescriptorIfNeeded(Instance);
  }

  {
    hadAnyError |= writeModuleSemanticInfoIfNeeded(Instance);
  }

  {
    hadAnyError |= emitConstValuesForWholeModuleIfNeeded(Instance);
  }
  return hadAnyError;
}

static void dumpAPIIfNeeded(const CompilerInstance &Instance) {
  using namespace toolchain::sys;
  const auto &Invocation = Instance.getInvocation();
  StringRef OutDir = Invocation.getFrontendOptions().DumpAPIPath;
  if (OutDir.empty())
    return;

  auto getOutPath = [&](SourceFile *SF) -> std::string {
    SmallString<256> Path = OutDir;
    StringRef Filename = SF->getFilename();
    path::append(Path, path::filename(Filename));
    return std::string(Path.str());
  };

  std::unordered_set<std::string> Filenames;

  auto dumpFile = [&](SourceFile *SF) -> bool {
    SmallString<512> TempBuf;
    toolchain::raw_svector_ostream TempOS(TempBuf);

    PrintOptions PO = PrintOptions::printInterface(
        Invocation.getFrontendOptions().PrintFullConvention);
    PO.PrintOriginalSourceText = true;
    PO.Indent = 2;
    PO.PrintAccess = false;
    PO.SkipUnderscoredSystemProtocols = true;
    SF->print(TempOS, PO);
    if (TempOS.str().trim().empty())
      return false; // nothing to show.

    std::string OutPath = getOutPath(SF);
    bool WasInserted = Filenames.insert(OutPath).second;
    if (!WasInserted) {
      toolchain::errs() << "multiple source files ended up with the same dump API "
                      "filename to write to: " << OutPath << '\n';
      return true;
    }

    std::error_code EC;
    toolchain::raw_fd_ostream OS(OutPath, EC, fs::FA_Read | fs::FA_Write);
    if (EC) {
      toolchain::errs() << "error opening file '" << OutPath << "': "
                   << EC.message() << '\n';
      return true;
    }

    OS << TempOS.str();
    return false;
  };

  std::error_code EC = fs::create_directories(OutDir);
  if (EC) {
    toolchain::errs() << "error creating directory '" << OutDir << "': "
                 << EC.message() << '\n';
    return;
  }

  for (auto *FU : Instance.getMainModule()->getFiles()) {
    if (auto *SF = dyn_cast<SourceFile>(FU))
      if (dumpFile(SF))
        return;
  }
}

static bool shouldEmitIndexData(const CompilerInvocation &Invocation) {
  const auto &opts = Invocation.getFrontendOptions();
  auto action = opts.RequestedAction;

  if (action == FrontendOptions::ActionType::CompileModuleFromInterface &&
      opts.ExplicitInterfaceBuild) {
    return true;
  }

  // FIXME: This predicate matches the status quo, but there's no reason
  // indexing cannot run for actions that do not require stdlib e.g. to better
  // facilitate tests.
  return FrontendOptions::doesActionRequireCodiraStandardLibrary(action);
}

/// Perform any actions that must have access to the ASTContext, and need to be
/// delayed until the Codira compile pipeline has finished. This may be called
/// before or after LLVM depending on when the ASTContext gets freed.
static void performEndOfPipelineActions(CompilerInstance &Instance) {
  assert(Instance.hasASTContext());
  auto &ctx = Instance.getASTContext();
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();

  // If we were asked to print Clang stats, do so.
  if (opts.PrintClangStats && ctx.getClangModuleLoader())
    ctx.getClangModuleLoader()->printStatistics();

  // Report AST stats if needed.
  if (auto *stats = ctx.Stats)
    countASTStats(*stats, Instance);

  if (opts.DumpClangLookupTables && ctx.getClangModuleLoader())
    ctx.getClangModuleLoader()->dumpCodiraLookupTables();

  // Report mangling stats if there was no error.
  if (!ctx.hadError())
    Mangle::printManglingStats();

  // Make sure we didn't load a module during a parse-only invocation, unless
  // it's -emit-imported-modules, which can load modules.
  auto action = opts.RequestedAction;
  if (FrontendOptions::shouldActionOnlyParse(action) &&
      !ctx.getLoadedModules().empty() &&
      action != FrontendOptions::ActionType::EmitImportedModules) {
    assert(ctx.getNumLoadedModules() == 1 &&
           "Loaded a module during parse-only");
    assert(ctx.getLoadedModules().begin()->second == Instance.getMainModule());
  }

  if (!opts.AllowModuleWithCompilerErrors) {
    // Verify the AST for all the modules we've loaded.
    ctx.verifyAllLoadedModules();

    // Verify generic signatures if we've been asked to.
    verifyGenericSignaturesIfNeeded(Invocation.getFrontendOptions(), ctx);
  }

  // Emit any additional outputs that we only need for a successful compilation.
  // We don't want to unnecessarily delay getting any errors back to the user.
  if (!ctx.hadError()) {
    emitLoadedModuleTraceForAllPrimariesIfNeeded(
        Instance.getMainModule(), Instance.getDependencyTracker(), opts);

    dumpAPIIfNeeded(Instance);
    language::emitFineModuleTraceIfNeeded(Instance, opts);
  }

  // Contains the hadError checks internally, we still want to output the
  // Objective-C header when there's errors and currently allowing them
  emitAnyWholeModulePostTypeCheckSupplementaryOutputs(Instance);

  // Verify reference dependencies of the current compilation job. Note this
  // must be run *before* verifying diagnostics so that the former can be tested
  // via the latter.
  if (opts.EnableIncrementalDependencyVerifier) {
    if (!Instance.getPrimarySourceFiles().empty()) {
      language::verifyDependencies(Instance.getSourceMgr(),
                                Instance.getPrimarySourceFiles());
    } else {
      language::verifyDependencies(Instance.getSourceMgr(),
                                Instance.getMainModule()->getFiles());
    }
  }

  if (Invocation.getLangOptions()
          .EnableExperimentalEagerClangModuleDiagnostics) {

    // A consumer meant to import all visible declarations.
    class EagerConsumer : public VisibleDeclConsumer {
    public:
      virtual void
      foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                DynamicLookupInfo dynamicLookupInfo = {}) override {
        if (auto *IDC = dyn_cast<IterableDeclContext>(VD)) {
          (void)IDC->getMembers();
        }
      }
    };

    EagerConsumer consumer;
    for (auto module : ctx.getLoadedModules()) {
      // None of the passed parameter have an effect, we just need to trigger
      // imports.
      module.second->lookupVisibleDecls(/*Access Path*/ {}, consumer,
                                        NLKind::QualifiedLookup);
    }
  }

  if (shouldEmitIndexData(Invocation)) {
    emitIndexData(Instance);
  }

  // Emit Codiradeps for every file in the batch.
  emitCodiradepsForAllPrimaryInputsIfNeeded(Instance);

  // Emit Make-style dependencies.
  emitMakeDependenciesIfNeeded(Instance);

  // Emit extracted constant values for every file in the batch
  emitConstValuesForAllPrimaryInputsIfNeeded(Instance);
}

static bool printCodiraVersion(const CompilerInvocation &Invocation) {
  toolchain::outs() << version::getCodiraFullVersion(
                      version::Version::getCurrentLanguageVersion())
               << '\n';
  toolchain::outs() << "Target: " << Invocation.getLangOptions().Target.str()
               << '\n';
  if (!toolchain::cl::getCompilerBuildConfig().empty())
    toolchain::cl::printBuildConfig(toolchain::outs());
  return false;
}

static void printSingleFrontendOpt(toolchain::opt::OptTable &table, options::ID id,
                                   toolchain::raw_ostream &OS) {
  if (table.getOption(id).hasFlag(options::FrontendOption) ||
      table.getOption(id).hasFlag(options::AutolinkExtractOption) ||
      table.getOption(id).hasFlag(options::ModuleWrapOption) ||
      table.getOption(id).hasFlag(options::CodiraSymbolGraphExtractOption) ||
      table.getOption(id).hasFlag(options::CodiraSynthesizeInterfaceOption) ||
      table.getOption(id).hasFlag(options::CodiraAPIDigesterOption)) {
    auto name = StringRef(table.getOptionName(id));
    if (!name.empty()) {
      OS << "    \"" << name << "\",\n";
    }
  }
}

static bool printCodiraArguments(CompilerInstance &instance) {
  ASTContext &context = instance.getASTContext();
  const CompilerInvocation &invocation = instance.getInvocation();
  const FrontendOptions &opts = invocation.getFrontendOptions();
  std::string path = opts.InputsAndOutputs.getSingleOutputFilename();
  std::error_code EC;
  toolchain::raw_fd_ostream out(path, EC, toolchain::sys::fs::OF_None);

  if (out.has_error() || EC) {
    context.Diags.diagnose(SourceLoc(), diag::error_opening_output, path,
                           EC.message());
    out.clear_error();
    return true;
  }
  std::unique_ptr<toolchain::opt::OptTable> table = createCodiraOptTable();

  out << "{\n";
  LANGUAGE_DEFER {
    out << "}\n";
  };
  out << "  \"SupportedArguments\": [\n";
#define OPTION(...)                                                            \
  printSingleFrontendOpt(*table,                                               \
                         language::options::TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__), out);
#include "language/Option/Options.inc"
#undef OPTION
  out << "    \"LastOption\"\n";
  out << "  ],\n";
  out << "  \"SupportedFeatures\": [\n";
  // Print supported feature names here.
  out << "    \"LastFeature\"\n";
  out << "  ]\n";
  return false;
}

static bool
withSemanticAnalysis(CompilerInstance &Instance, FrontendObserver *observer,
                     toolchain::function_ref<bool(CompilerInstance &)> cont,
                     bool runDespiteErrors = false) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();
  assert(!FrontendOptions::shouldActionOnlyParse(opts.RequestedAction) &&
         "Action may only parse, but has requested semantic analysis!");

  Instance.performSema();
  if (observer)
    observer->performedSemanticAnalysis(Instance);

  switch (opts.CrashMode) {
  case FrontendOptions::DebugCrashMode::AssertAfterParse:
    debugFailWithAssertion();
    return true;
  case FrontendOptions::DebugCrashMode::CrashAfterParse:
    debugFailWithCrash();
    return true;
  case FrontendOptions::DebugCrashMode::None:
    break;
  }

  (void)migrator::updateCodeAndEmitRemapIfNeeded(&Instance);

  bool hadError = Instance.getASTContext().hadError()
                      && !opts.AllowModuleWithCompilerErrors;
  if (hadError && !runDespiteErrors)
    return true;

  return cont(Instance) || hadError;
}

static bool performScanDependencies(CompilerInstance &Instance) {
  if (Instance.getInvocation().getFrontendOptions().ImportPrescan)
    return dependencies::prescanDependencies(Instance);
  else
    return dependencies::scanDependencies(Instance);
}

static bool performParseOnly(ModuleDecl &MainModule) {
  // A -parse invocation only cares about the side effects of parsing, so
  // force the parsing of all the source files.
  for (auto *file : MainModule.getFiles()) {
    if (auto *SF = dyn_cast<SourceFile>(file))
      (void)SF->getTopLevelDecls();
  }
  return MainModule.getASTContext().hadError();
}

static bool performAction(CompilerInstance &Instance,
                          int &ReturnValue,
                          FrontendObserver *observer) {
  const auto &opts = Instance.getInvocation().getFrontendOptions();
  switch (Instance.getInvocation().getFrontendOptions().RequestedAction) {
  // MARK: Trivial Actions
  case FrontendOptions::ActionType::NoneAction:
    return Instance.getASTContext().hadError();
  case FrontendOptions::ActionType::PrintVersion:
    return printCodiraVersion(Instance.getInvocation());
  case FrontendOptions::ActionType::PrintArguments:
    return printCodiraArguments(Instance);
  case FrontendOptions::ActionType::REPL:
    toolchain::report_fatal_error("Compiler-internal integrated REPL has been "
                             "removed; use the LLDB-enhanced REPL instead.");

  // MARK: Actions for Clang and Clang Modules
  case FrontendOptions::ActionType::EmitPCH:
    return precompileBridgingHeader(Instance);
  case FrontendOptions::ActionType::EmitPCM:
    return precompileClangModule(Instance);
  case FrontendOptions::ActionType::DumpPCM:
    return dumpPrecompiledClangModule(Instance);

  // MARK: Module Interface Actions
  case FrontendOptions::ActionType::CompileModuleFromInterface:
  case FrontendOptions::ActionType::TypecheckModuleFromInterface:
    return buildModuleFromInterface(Instance);

  // MARK: Actions that Dump
  case FrontendOptions::ActionType::DumpParse:
    return dumpAST(Instance, ASTDumpMemberLoading::Parsed);
  case FrontendOptions::ActionType::DumpAST:
    return withSemanticAnalysis(
        Instance, observer,
        [](CompilerInstance &Instance) {
          return dumpAST(Instance, ASTDumpMemberLoading::TypeChecked);
        },
        /*runDespiteErrors=*/true);
  case FrontendOptions::ActionType::PrintAST:
    return withSemanticAnalysis(
        Instance, observer, [](CompilerInstance &Instance) {
          getPrimaryOrMainSourceFile(Instance).print(
              toolchain::outs(), PrintOptions::printEverything());
          return Instance.getASTContext().hadError();
        });
  case FrontendOptions::ActionType::PrintASTDecl:
    return withSemanticAnalysis(
        Instance, observer, [](CompilerInstance &Instance) {
          getPrimaryOrMainSourceFile(Instance).print(
              toolchain::outs(), PrintOptions::printDeclarations());
          return Instance.getASTContext().hadError();
        });
  case FrontendOptions::ActionType::DumpScopeMaps:
    return withSemanticAnalysis(
        Instance, observer, [](CompilerInstance &Instance) {
          return dumpAndPrintScopeMap(Instance,
                                      getPrimaryOrMainSourceFile(Instance));
        }, /*runDespiteErrors=*/true);
  case FrontendOptions::ActionType::DumpAvailabilityScopes:
    return withSemanticAnalysis(
        Instance, observer, [](CompilerInstance &Instance) {
          getPrimaryOrMainSourceFile(Instance).getAvailabilityScope()->dump(
              toolchain::errs(), Instance.getASTContext().SourceMgr);
          return Instance.getASTContext().hadError();
        }, /*runDespiteErrors=*/true);
  case FrontendOptions::ActionType::DumpInterfaceHash:
    getPrimaryOrMainSourceFile(Instance).dumpInterfaceHash(toolchain::errs());
    return Instance.getASTContext().hadError();
  case FrontendOptions::ActionType::EmitImportedModules:
    return emitImportedModules(Instance.getMainModule(), opts,
                               Instance.getOutputBackend());

  // MARK: Dependency Scanning Actions
  case FrontendOptions::ActionType::ScanDependencies:
    return performScanDependencies(Instance);

  // MARK: General Compilation Actions
  case FrontendOptions::ActionType::Parse:
    return performParseOnly(*Instance.getMainModule());
  case FrontendOptions::ActionType::ResolveImports:
    return Instance.performParseAndResolveImportsOnly();
  case FrontendOptions::ActionType::Typecheck:
    return withSemanticAnalysis(Instance, observer,
                                [](CompilerInstance &Instance) {
                                  return Instance.getASTContext().hadError();
                                });
  case FrontendOptions::ActionType::Immediate: {
    const auto &Ctx = Instance.getASTContext();
    if (Ctx.LangOpts.hasFeature(Feature::LazyImmediate)) {
      ReturnValue = RunImmediatelyFromAST(Instance);
      return Ctx.hadError();
    }
    return withSemanticAnalysis(
        Instance, observer, [&](CompilerInstance &Instance) {
          assert(FrontendOptions::doesActionGenerateSIL(opts.RequestedAction) &&
                 "All actions not requiring SILGen must have been handled!");
          return performCompileStepsPostSema(Instance, ReturnValue, observer);
        });
  }
  case FrontendOptions::ActionType::EmitSILGen:
  case FrontendOptions::ActionType::EmitSIBGen:
  case FrontendOptions::ActionType::EmitSIL:
  case FrontendOptions::ActionType::EmitLoweredSIL:
  case FrontendOptions::ActionType::EmitSIB:
  case FrontendOptions::ActionType::EmitModuleOnly:
  case FrontendOptions::ActionType::MergeModules:
  case FrontendOptions::ActionType::EmitAssembly:
  case FrontendOptions::ActionType::EmitIRGen:
  case FrontendOptions::ActionType::EmitIR:
  case FrontendOptions::ActionType::EmitBC:
  case FrontendOptions::ActionType::EmitObject:
  case FrontendOptions::ActionType::DumpTypeInfo:
    return withSemanticAnalysis(
        Instance, observer, [&](CompilerInstance &Instance) {
          assert(FrontendOptions::doesActionGenerateSIL(opts.RequestedAction) &&
                 "All actions not requiring SILGen must have been handled!");
          return performCompileStepsPostSema(Instance, ReturnValue, observer);
        });
  }

  assert(false && "Unhandled case in performCompile!");
  return Instance.getASTContext().hadError();
}

/// Try replay the compiler result from cache.
///
/// Return true if all the outputs are fetched from cache. Otherwise, return
/// false and will not replay any output.
static bool tryReplayCompilerResults(CompilerInstance &Instance) {
  if (!Instance.supportCaching() ||
      Instance.getInvocation().getCASOptions().CacheSkipReplay)
    return false;

  assert(Instance.getCompilerBaseKey() &&
         "Instance is not setup correctly for replay");

  auto *CDP = Instance.getCachingDiagnosticsProcessor();
  assert(CDP && "CachingDiagnosticsProcessor needs to be setup for replay");

  // Don't capture diagnostics from replay.
  CDP->endDiagnosticCapture();

  bool replayed = replayCachedCompilerOutputs(
      Instance.getObjectStore(), Instance.getActionCache(),
      *Instance.getCompilerBaseKey(), Instance.getDiags(),
      Instance.getInvocation().getFrontendOptions(), *CDP,
      Instance.getInvocation().getCASOptions().EnableCachingRemarks,
      Instance.getInvocation().getIRGenOptions().UseCASBackend);

  // If we didn't replay successfully, re-start capture.
  if (!replayed)
    CDP->startDiagnosticCapture();

  return replayed;
}

/// Performs the compile requested by the user.
/// \param Instance Will be reset after performIRGeneration when the verifier
///                 mode is NoVerify and there were no errors.
/// \returns true on error
static bool performCompile(CompilerInstance &Instance,
                           int &ReturnValue,
                           FrontendObserver *observer) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();
  const FrontendOptions::ActionType Action = opts.RequestedAction;

  if (tryReplayCompilerResults(Instance))
    return false;

  // To compile LLVM IR, just pass it off unmodified.
  if (opts.InputsAndOutputs.shouldTreatAsLLVM())
    return compileLLVMIR(Instance);

  assert([&]() -> bool {
    if (FrontendOptions::shouldActionOnlyParse(Action)) {
      // Parsing gets triggered lazily, but let's make sure we have the right
      // input kind.
      return toolchain::all_of(
          opts.InputsAndOutputs.getAllInputs(), [](const InputFile &IF) {
            const auto kind = IF.getType();
            return kind == file_types::TY_Codira ||
                   kind == file_types::TY_CodiraModuleInterfaceFile;
          });
    }
    return true;
  }() && "Only supports parsing .code files");

  bool hadError = performAction(Instance, ReturnValue, observer);
  auto canIgnoreErrorForExit = [&Instance, &opts]() {
    return opts.AllowModuleWithCompilerErrors ||
      (opts.isTypeCheckAction() && Instance.downgradeInterfaceVerificationErrors());
  };

  // We might have freed the ASTContext already, but in that case we would
  // have already performed these actions.
  if (Instance.hasASTContext() &&
      FrontendOptions::doesActionPerformEndOfPipelineActions(Action)) {
    performEndOfPipelineActions(Instance);
    if (!canIgnoreErrorForExit())
      hadError |= Instance.getASTContext().hadError();
  }
  return hadError;
}

static bool serializeSIB(SILModule *SM, const PrimarySpecificPaths &PSPs,
                         const ASTContext &Context, ModuleOrSourceFile MSF) {
  const std::string &moduleOutputPath =
      PSPs.SupplementaryOutputs.ModuleOutputPath;
  assert(!moduleOutputPath.empty() && "must have an output path");

  SerializationOptions serializationOpts;
  serializationOpts.OutputPath = moduleOutputPath;
  serializationOpts.SerializeAllSIL = true;
  serializationOpts.IsSIB = true;
  serializationOpts.IsOSSA = Context.SILOpts.EnableOSSAModules;

  symbolgraphgen::SymbolGraphOptions symbolGraphOptions;

  serialize(MSF, serializationOpts, symbolGraphOptions, SM);
  return Context.hadError();
}

static bool serializeModuleSummary(SILModule *SM,
                                   const PrimarySpecificPaths &PSPs,
                                   const ASTContext &Context) {
  auto summaryOutputPath = PSPs.SupplementaryOutputs.ModuleSummaryOutputPath;
  return withOutputPath(Context.Diags, Context.getOutputBackend(),
                           summaryOutputPath, [&](toolchain::raw_ostream &out) {
                             out << "Some stuff";
                             return false;
                           });
}

static GeneratedModule
generateIR(const IRGenOptions &IRGenOpts, const TBDGenOptions &TBDOpts,
           std::unique_ptr<SILModule> SM,
           const PrimarySpecificPaths &PSPs,
           StringRef OutputFilename, ModuleOrSourceFile MSF,
           toolchain::GlobalVariable *&HashGlobal,
           ArrayRef<std::string> parallelOutputFilenames) {
  if (auto *SF = MSF.dyn_cast<SourceFile *>()) {
    return performIRGeneration(SF, IRGenOpts, TBDOpts,
                               std::move(SM), OutputFilename, PSPs,
                               SF->getPrivateDiscriminator().str(),
                               &HashGlobal);
  } else {
    return performIRGeneration(MSF.get<ModuleDecl *>(), IRGenOpts, TBDOpts,
                               std::move(SM), OutputFilename, PSPs,
                               parallelOutputFilenames, &HashGlobal);
  }
}

static bool processCommandLineAndRunImmediately(CompilerInstance &Instance,
                                                std::unique_ptr<SILModule> &&SM,
                                                ModuleOrSourceFile MSF,
                                                FrontendObserver *observer,
                                                int &ReturnValue) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();
  assert(!MSF.is<SourceFile *>() && "-i doesn't work in -primary-file mode");
  const IRGenOptions &IRGenOpts = Invocation.getIRGenOptions();
  const ProcessCmdLine &CmdLine =
      ProcessCmdLine(opts.ImmediateArgv.begin(), opts.ImmediateArgv.end());

  PrettyStackTraceStringAction trace(
      "running user code",
      MSF.is<SourceFile *>() ? MSF.get<SourceFile *>()->getFilename()
                     : MSF.get<ModuleDecl *>()->getModuleFilename());

  ReturnValue =
      RunImmediately(Instance, CmdLine, IRGenOpts, Invocation.getSILOptions(),
                     std::move(SM));
  return Instance.getASTContext().hadError();
}

static bool validateTBDIfNeeded(const CompilerInvocation &Invocation,
                                ModuleOrSourceFile MSF,
                                const toolchain::Module &IRModule) {
  const auto mode = Invocation.getFrontendOptions().ValidateTBDAgainstIR;
  const bool canPerformTBDValidation = [&]() {
    // If the user has requested we skip validation, honor it.
    if (mode == FrontendOptions::TBDValidationMode::None) {
      return false;
    }

    // Embedded Codira does not support TBD.
    if (Invocation.getLangOptions().hasFeature(Feature::Embedded)) {
      return false;
    }

    // Cross-module optimization does not support TBD.
    if (Invocation.getSILOptions().CMOMode == CrossModuleOptimizationMode::Aggressive ||
        Invocation.getSILOptions().CMOMode == CrossModuleOptimizationMode::Everything) {
      return false;
    }

    // If we can't validate the given input file, bail early. This covers cases
    // like passing raw SIL as a primary file.
    const auto &IO = Invocation.getFrontendOptions().InputsAndOutputs;
    // FIXME: This would be a good test of the interface format.
    if (IO.shouldTreatAsModuleInterface() || IO.shouldTreatAsSIL() ||
        IO.shouldTreatAsLLVM() || IO.shouldTreatAsObjCHeader()) {
      return false;
    }

    // Modules with SIB files cannot be validated. This is because SIB files
    // may have serialized hand-crafted SIL definitions that are invisible to
    // TBDGen as it is an AST-only traversal.
    if (auto *mod = MSF.dyn_cast<ModuleDecl *>()) {
      bool hasSIB = toolchain::any_of(mod->getFiles(), [](const FileUnit *File) -> bool {
        auto SASTF = dyn_cast<SerializedASTFile>(File);
        return SASTF && SASTF->isSIB();
      });

      if (hasSIB) {
        return false;
      }
    }

    // "Default" mode's behavior varies if using a debug compiler.
    if (mode == FrontendOptions::TBDValidationMode::Default) {
#ifndef NDEBUG
      // With a debug compiler, we do some validation by default.
      return true;
#else
      // Otherwise, the default is to do nothing.
      return false;
#endif
    }


    return true;
  }();

  if (!canPerformTBDValidation) {
    return false;
  }

  const bool diagnoseExtraSymbolsInTBD = [mode]() {
    switch (mode) {
    case FrontendOptions::TBDValidationMode::None:
      toolchain_unreachable("Handled Above!");
    case FrontendOptions::TBDValidationMode::Default:
    case FrontendOptions::TBDValidationMode::MissingFromTBD:
      return false;
    case FrontendOptions::TBDValidationMode::All:
      return true;
    }
    toolchain_unreachable("invalid mode");
  }();

  TBDGenOptions Opts = Invocation.getTBDGenOptions();
  // Ignore embedded symbols from external modules for validation to remove
  // noise from e.g. statically-linked libraries.
  Opts.embedSymbolsFromModules.clear();
  if (auto *SF = MSF.dyn_cast<SourceFile *>()) {
    return validateTBD(SF, IRModule, Opts, diagnoseExtraSymbolsInTBD);
  } else {
    return validateTBD(MSF.get<ModuleDecl *>(), IRModule, Opts,
                       diagnoseExtraSymbolsInTBD);
  }
}

static void freeASTContextIfPossible(CompilerInstance &Instance) {
  // If the stats reporter is installed, we need the ASTContext to live through
  // the entire compilation process.
  if (Instance.getASTContext().Stats) {
    return;
  }

  // If this instance is used for multiple compilations, we need the ASTContext
  // to live.
  if (Instance.getInvocation()
          .getFrontendOptions()
          .ReuseFrontendForMultipleCompilations) {
    return;
  }

  const auto &opts = Instance.getInvocation().getFrontendOptions();

  // If there are multiple primary inputs it is too soon to free
  // the ASTContext, etc.. OTOH, if this compilation generates code for > 1
  // primary input, then freeing it after processing the last primary is
  // unlikely to reduce the peak heap size. So, only optimize the
  // single-primary-case (or WMO).
  if (opts.InputsAndOutputs.hasMultiplePrimaryInputs()) {
    return;
  }

  // Make sure to perform the end of pipeline actions now, because they need
  // access to the ASTContext.
  performEndOfPipelineActions(Instance);

  Instance.freeASTContext();
}

static bool generateCode(CompilerInstance &Instance, StringRef OutputFilename,
                         toolchain::Module *IRModule,
                         toolchain::GlobalVariable *HashGlobal) {
  const auto &opts = Instance.getInvocation().getIRGenOptions();
  std::unique_ptr<toolchain::TargetMachine> TargetMachine =
      createTargetMachine(opts, Instance.getASTContext());

  TargetMachine->Options.MCOptions.CAS = Instance.getSharedCASInstance();

  if (Instance.getInvocation().getCASOptions().EnableCaching &&
      opts.UseCASBackend)
    TargetMachine->Options.MCOptions.ResultCallBack =
        [&](const toolchain::cas::CASID &ID) -> toolchain::Error {
      if (auto Err = Instance.getCASOutputBackend().storeMCCASObjectID(
              OutputFilename, ID))
        return Err;

      return toolchain::Error::success();
    };

  // Free up some compiler resources now that we have an IRModule.
  freeASTContextIfPossible(Instance);

  // If we emitted any errors while performing the end-of-pipeline actions, bail.
  if (Instance.getDiags().hadAnyError())
    return true;

  // Now that we have a single IR Module, hand it over to performLLVM.
  return performLLVM(opts, Instance.getDiags(), nullptr, HashGlobal, IRModule,
                     TargetMachine.get(), OutputFilename,
                     Instance.getOutputBackend(),
                     Instance.getStatsReporter());
}

static bool performCompileStepsPostSILGen(CompilerInstance &Instance,
                                          std::unique_ptr<SILModule> SM,
                                          ModuleOrSourceFile MSF,
                                          const PrimarySpecificPaths &PSPs,
                                          int &ReturnValue,
                                          FrontendObserver *observer) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();
  FrontendOptions::ActionType Action = opts.RequestedAction;
  const ASTContext &Context = Instance.getASTContext();
  const IRGenOptions &IRGenOpts = Invocation.getIRGenOptions();

  std::optional<BufferIndirectlyCausingDiagnosticRAII> ricd;
  if (auto *SF = MSF.dyn_cast<SourceFile *>())
    ricd.emplace(*SF);

  if (observer)
    observer->performedSILGeneration(*SM);

  // Cancellation check after SILGen.
  if (Instance.isCancellationRequested())
    return true;

  auto *Stats = Instance.getASTContext().Stats;
  if (Stats)
    countStatsPostSILGen(*Stats, *SM);

  // We've been told to emit SIL after SILGen, so write it now.
  if (Action == FrontendOptions::ActionType::EmitSILGen) {
    return writeSIL(*SM, PSPs, Instance, Invocation.getSILOptions());
  }

  // In lazy typechecking mode, SILGen may have triggered requests which
  // resulted in errors. We don't want to proceed with optimization or
  // serialization if there were errors since the SIL may be incomplete or
  // invalid.
  if (Context.TypeCheckerOpts.EnableLazyTypecheck && Context.hadError())
    return true;

  if (Action == FrontendOptions::ActionType::EmitSIBGen) {
    serializeSIB(SM.get(), PSPs, Context, MSF);
    return Context.hadError();
  }

  SM->installSILRemarkStreamer();

  // This is the action to be used to serialize SILModule.
  // It may be invoked multiple times, but it will perform
  // serialization only once. The serialization may either happen
  // after high-level optimizations or after all optimizations are
  // done, depending on the compiler setting.

  auto SerializeSILModuleAction = [&]() {
    const SupplementaryOutputPaths &outs = PSPs.SupplementaryOutputs;
    if (outs.ModuleOutputPath.empty())
      return;

    SerializationOptions serializationOpts =
        Invocation.computeSerializationOptions(outs, Instance.getMainModule());

    // Infer if this is an emit-module job part of an incremental build,
    // vs a partial emit-module job (with primary files) or other kinds.
    // We may want to rely on a flag instead to differentiate them.
    const bool isEmitModuleSeparately =
        Action == FrontendOptions::ActionType::EmitModuleOnly &&
        MSF.is<ModuleDecl *>() &&
        Instance.getInvocation()
            .getTypeCheckerOptions()
            .SkipFunctionBodies == FunctionBodySkipping::NonInlinableWithoutTypes;
    const bool canEmitIncrementalInfoIntoModule =
        !serializationOpts.DisableCrossModuleIncrementalInfo &&
        (Action == FrontendOptions::ActionType::MergeModules ||
         isEmitModuleSeparately);
    if (canEmitIncrementalInfoIntoModule) {
      const auto alsoEmitDotFile =
          Instance.getInvocation()
              .getLangOptions()
              .EmitFineGrainedDependencySourcefileDotFiles;

      using SourceFileDepGraph = fine_grained_dependencies::SourceFileDepGraph;
      auto *Mod = MSF.get<ModuleDecl *>();
      fine_grained_dependencies::withReferenceDependencies(
          Mod, *Instance.getDependencyTracker(),
          Instance.getOutputBackend(), Mod->getModuleFilename(),
          alsoEmitDotFile, [&](SourceFileDepGraph &&g) {
            serialize(MSF, serializationOpts, Invocation.getSymbolGraphOptions(), SM.get(), &g);
            return false;
          });
    } else {
      serialize(MSF, serializationOpts, Invocation.getSymbolGraphOptions(), SM.get());
    }
  };

  // Set the serialization action, so that the SIL module
  // can be serialized at any moment, e.g. during the optimization pipeline.
  SM->setSerializeSILAction(SerializeSILModuleAction);

  // Perform optimizations and mandatory/diagnostic passes.
  if (Instance.performSILProcessing(SM.get()))
    return true;

  if (observer)
    observer->performedSILProcessing(*SM);

  // Cancellation check after SILOptimization.
  if (Instance.isCancellationRequested())
    return true;

  if (PSPs.haveModuleSummaryOutputPath()) {
    if (serializeModuleSummary(SM.get(), PSPs, Context)) {
      return true;
    }
  }

  if (Action == FrontendOptions::ActionType::EmitSIB)
    return serializeSIB(SM.get(), PSPs, Context, MSF);

  if (PSPs.haveModuleOrModuleDocOutputPaths()) {
    if (Action == FrontendOptions::ActionType::MergeModules ||
        Action == FrontendOptions::ActionType::EmitModuleOnly) {
      return Context.hadError() && !opts.AllowModuleWithCompilerErrors;
    }
  }

  assert(Action >= FrontendOptions::ActionType::EmitSIL &&
         "All actions not requiring SILPasses must have been handled!");

  // We've been told to write canonical SIL, so write it now.
  if (Action == FrontendOptions::ActionType::EmitSIL)
    return writeSIL(*SM, PSPs, Instance, Invocation.getSILOptions());

  assert(Action >= FrontendOptions::ActionType::Immediate &&
         "All actions not requiring IRGen must have been handled!");
  assert(Action != FrontendOptions::ActionType::REPL &&
         "REPL mode must be handled immediately after Instance->performSema()");

  // Check if we had any errors; if we did, don't proceed to IRGen.
  if (Context.hadError())
    return !opts.AllowModuleWithCompilerErrors;

  runSILLoweringPasses(*SM);

  // If we are asked to emit lowered SIL, dump it now and return.
  if (Action == FrontendOptions::ActionType::EmitLoweredSIL)
    return writeSIL(*SM, PSPs, Instance, Invocation.getSILOptions());

  // Cancellation check after SILLowering.
  if (Instance.isCancellationRequested())
    return true;

  // TODO: at this point we need to flush any the _tracing and profiling_
  // in the UnifiedStatsReporter, because the those subsystems of the USR
  // retain _pointers into_ the SILModule, and the SILModule's lifecycle is
  // not presently such that it will outlive the USR (indeed, as it's
  // destroyed on a separate thread, this fact isn't even _deterministic_
  // after this point). If future plans require the USR tracing or
  // profiling entities after this point, more rearranging will be required.
  if (Stats)
    Stats->flushTracesAndProfiles();

  if (Action == FrontendOptions::ActionType::DumpTypeInfo)
    return performDumpTypeInfo(IRGenOpts, *SM);

  if (Action == FrontendOptions::ActionType::Immediate)
    return processCommandLineAndRunImmediately(
        Instance, std::move(SM), MSF, observer, ReturnValue);

  StringRef OutputFilename = PSPs.OutputFilename;
  std::vector<std::string> ParallelOutputFilenames =
      opts.InputsAndOutputs.copyOutputFilenames();
  toolchain::GlobalVariable *HashGlobal;
  auto IRModule =
      generateIR(IRGenOpts, Invocation.getTBDGenOptions(), std::move(SM), PSPs,
                 OutputFilename, MSF, HashGlobal, ParallelOutputFilenames);

  // Cancellation check after IRGen.
  if (Instance.isCancellationRequested())
    return true;

  // If no IRModule is available, bail. This can either happen if IR generation
  // fails, or if parallelIRGen happened correctly (in which case it would have
  // already performed LLVM).
  if (!IRModule)
    return Instance.getDiags().hadAnyError();

  if (validateTBDIfNeeded(Invocation, MSF, *IRModule.getModule()))
    return true;

  if (IRGenOpts.UseSingleModuleLLVMEmission) {
    // Pretend the other files that drivers/build systems expect exist by
    // creating empty files.
    if (writeEmptyOutputFilesFor(Context, ParallelOutputFilenames, IRGenOpts))
      return true;
  }

  return generateCode(Instance, OutputFilename, IRModule.getModule(),
                      HashGlobal);
}

static void emitIndexDataForSourceFile(SourceFile *PrimarySourceFile,
                                       const CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
  const auto &opts = Invocation.getFrontendOptions();

  if (opts.IndexStorePath.empty())
    return;

  // FIXME: provide index unit token(s) explicitly and only use output file
  // paths as a fallback.

  bool isDebugCompilation;
  switch (Invocation.getSILOptions().OptMode) {
    case OptimizationMode::NotSet:
    case OptimizationMode::NoOptimization:
      isDebugCompilation = true;
      break;
    case OptimizationMode::ForSpeed:
    case OptimizationMode::ForSize:
      isDebugCompilation = false;
      break;
  }

  if (PrimarySourceFile) {
    const PrimarySpecificPaths &PSPs =
        opts.InputsAndOutputs.getPrimarySpecificPathsForPrimary(
            PrimarySourceFile->getFilename());
    StringRef OutputFile = PSPs.IndexUnitOutputFilename;
    if (OutputFile.empty())
      OutputFile = PSPs.OutputFilename;
    (void) index::indexAndRecord(PrimarySourceFile, OutputFile,
                                 opts.IndexStorePath,
                                 !opts.IndexIgnoreClangModules,
                                 opts.IndexSystemModules,
                                 opts.IndexIgnoreStdlib,
                                 opts.IndexIncludeLocals,
                                 isDebugCompilation,
                                 opts.DisableImplicitModules,
                                 Invocation.getTargetTriple(),
                                 *Instance.getDependencyTracker(),
                                 Invocation.getIRGenOptions().FilePrefixMap);
  } else {
    std::string moduleToken =
        Invocation.getModuleOutputPathForAtMostOnePrimary();
    if (moduleToken.empty())
      moduleToken = opts.InputsAndOutputs.getSingleIndexUnitOutputFilename();

    (void) index::indexAndRecord(Instance.getMainModule(),
                                 opts.InputsAndOutputs
                                   .copyIndexUnitOutputFilenames(),
                                 moduleToken, opts.IndexStorePath,
                                 !opts.IndexIgnoreClangModules,
                                 opts.IndexSystemModules,
                                 opts.IndexIgnoreStdlib,
                                 opts.IndexIncludeLocals,
                                 isDebugCompilation,
                                 opts.DisableImplicitModules,
                                 Invocation.getTargetTriple(),
                                 *Instance.getDependencyTracker(),
                                 Invocation.getIRGenOptions().FilePrefixMap);
  }
}

/// A PrettyStackTraceEntry to print frontend information useful for debugging.
class PrettyStackTraceFrontend : public toolchain::PrettyStackTraceEntry {
  const CompilerInvocation &Invocation;

public:
  PrettyStackTraceFrontend(const CompilerInvocation &invocation)
      : Invocation(invocation) {}

  void print(toolchain::raw_ostream &os) const override {
    auto effective = Invocation.getLangOptions().EffectiveLanguageVersion;
    if (effective != version::Version::getCurrentLanguageVersion()) {
      os << "Compiling with effective version " << effective;
    } else {
      os << "Compiling with the current language version";
    }
    if (Invocation.getFrontendOptions().AllowModuleWithCompilerErrors) {
      os << " while allowing modules with compiler errors";
    }
    os << "\n";
  };
};

int language::performFrontend(ArrayRef<const char *> Args,
                           const char *Argv0, void *MainAddr,
                           FrontendObserver *observer) {
  INITIALIZE_LLVM();
  toolchain::setBugReportMsg(LANGUAGE_CRASH_BUG_REPORT_MESSAGE "\n");
  toolchain::EnablePrettyStackTraceOnSigInfoForThisThread();

  std::unique_ptr<CompilerInstance> Instance =
    std::make_unique<CompilerInstance>();

  CompilerInvocation Invocation;

  DiagnosticHelper DH = DiagnosticHelper::create(*Instance, Invocation, Args);

  // Hopefully we won't trigger any LLVM-level fatal errors, but if we do try
  // to route them through our usual textual diagnostics before crashing.
  //
  // Unfortunately it's not really safe to do anything else, since very
  // low-level operations in LLVM can trigger fatal errors.
  toolchain::ScopedFatalErrorHandler handler(
      [](void *rawCallback, const char *reason, bool shouldCrash) {
        auto *helper = static_cast<DiagnosticHelper *>(rawCallback);
        helper->diagnoseFatalError(reason, shouldCrash);
      },
      &DH);

  struct FinishDiagProcessingCheckRAII {
    bool CalledFinishDiagProcessing = false;
    ~FinishDiagProcessingCheckRAII() {
      assert(CalledFinishDiagProcessing && "returned from the function "
        "without calling finishDiagProcessing");
    }
  } FinishDiagProcessingCheckRAII;

  auto finishDiagProcessing = [&](int retValue, bool verifierEnabled) -> int {
    FinishDiagProcessingCheckRAII.CalledFinishDiagProcessing = true;
    DH.setSuppressOutput(false);
    if (auto *CDP = Instance->getCachingDiagnosticsProcessor()) {
      // Don't cache if build failed.
      if (retValue)
        CDP->endDiagnosticCapture();
    }
    bool diagnosticsError = Instance->getDiags().finishProcessing();
    // If the verifier is enabled and did not encounter any verification errors,
    // return 0 even if the compile failed. This behavior isn't ideal, but large
    // parts of the test suite are reliant on it.
    if (verifierEnabled && !diagnosticsError) {
      return 0;
    }
    return retValue ? retValue : diagnosticsError;
  };

  if (Args.empty()) {
    Instance->getDiags().diagnose(SourceLoc(), diag::error_no_frontend_args);
    return finishDiagProcessing(1, /*verifierEnabled*/ false);
  }

  SmallString<128> workingDirectory;
  toolchain::sys::fs::current_path(workingDirectory);

  std::string MainExecutablePath =
      toolchain::sys::fs::getMainExecutable(Argv0, MainAddr);

  // Parse arguments.
  SmallVector<std::unique_ptr<toolchain::MemoryBuffer>, 4> configurationFileBuffers;
  if (Invocation.parseArgs(Args, Instance->getDiags(),
                           &configurationFileBuffers, workingDirectory,
                           MainExecutablePath)) {
    return finishDiagProcessing(1, /*verifierEnabled*/ false);
  }

  // Don't ask clients to report bugs when running a script in immediate mode.
  // When a script asserts the compiler reports the error with the same
  // stacktrace as a compiler crash. From here we can't tell which is which,
  // for now let's not explicitly ask for bug reports.
  if (Invocation.getFrontendOptions().RequestedAction ==
      FrontendOptions::ActionType::Immediate) {
    toolchain::setBugReportMsg(nullptr);
  }

  PrettyStackTraceFrontend frontendTrace(Invocation);

  // Make an array of PrettyStackTrace objects to dump the configuration files
  // we used to parse the arguments. These are RAII objects, so they and the
  // buffers they refer to must be kept alive in order to be useful. (That is,
  // we want them to be alive for the entire rest of performFrontend.)
  //
  // This can't be a SmallVector or similar because PrettyStackTraces can't be
  // moved (or copied)...and it can't be an array of non-optionals because
  // PrettyStackTraces can't be default-constructed. So we end up with a
  // dynamically-sized array of optional PrettyStackTraces, which get
  // initialized by iterating over the buffers we collected above.
  auto configurationFileStackTraces =
      std::make_unique<std::optional<PrettyStackTraceFileContents>[]>(
          configurationFileBuffers.size());

  // If the compile is a whole module job, then the contents of the filelist
  // is every file in the module, which is not very interesting and could be
  // hundreds or thousands of lines. Skip dumping this output in that case.
  if (!Invocation.getFrontendOptions().InputsAndOutputs.isWholeModule()) {
    for_each(configurationFileBuffers.begin(), configurationFileBuffers.end(),
             configurationFileStackTraces.get(),
             [](const std::unique_ptr<toolchain::MemoryBuffer> &buffer,
                std::optional<PrettyStackTraceFileContents> &trace) {
               trace.emplace(*buffer);
             });
  }

  // The compiler invocation is now fully configured; notify our observer.
  if (observer) {
    observer->parsedArgs(Invocation);
  }

  if (Invocation.getFrontendOptions().PrintHelp ||
      Invocation.getFrontendOptions().PrintHelpHidden) {
    unsigned IncludedFlagsBitmask = options::FrontendOption;
    unsigned ExcludedFlagsBitmask =
      Invocation.getFrontendOptions().PrintHelpHidden ? 0 :
                                                        toolchain::opt::HelpHidden;
    std::unique_ptr<toolchain::opt::OptTable> Options(createCodiraOptTable());
    Options->printHelp(toolchain::outs(), displayName(MainExecutablePath).c_str(),
                       "Codira frontend", IncludedFlagsBitmask,
                       ExcludedFlagsBitmask, /*ShowAllAliases*/false);
    return finishDiagProcessing(0, /*verifierEnabled*/ false);
  }

  if (Invocation.getFrontendOptions().PrintTargetInfo) {
    language::targetinfo::printTargetInfo(Invocation, toolchain::outs());
    return finishDiagProcessing(0, /*verifierEnabled*/ false);
  }

  if (Invocation.getFrontendOptions().PrintSupportedFeatures) {
    language::features::printSupportedFeatures(toolchain::outs());
    return finishDiagProcessing(0, /*verifierEnabled*/ false);
  }

  if (Invocation.getFrontendOptions().RequestedAction ==
      FrontendOptions::ActionType::NoneAction) {
    Instance->getDiags().diagnose(SourceLoc(),
                                  diag::error_missing_frontend_action);
    return finishDiagProcessing(1, /*verifierEnabled*/ false);
  }

  if (Invocation.getFrontendOptions().PrintStats) {
    toolchain::EnableStatistics();
  }

  DH.beginMessage();

  const DiagnosticOptions &diagOpts = Invocation.getDiagnosticOptions();
  bool verifierEnabled = diagOpts.VerifyMode != DiagnosticOptions::NoVerify;

  std::string InstanceSetupError;
  if (Instance->setup(Invocation, InstanceSetupError, Args)) {
    int ReturnCode = 1;
    DH.endMessage(ReturnCode);

    return finishDiagProcessing(ReturnCode, /*verifierEnabled*/ false);
  }

  // The compiler instance has been configured; notify our observer.
  if (observer) {
    observer->configuredCompiler(*Instance);
  }

  if (verifierEnabled) {
    // Suppress printed diagnostic output during the compile if the verifier is
    // enabled.
    DH.setSuppressOutput(true);
  }

  CompilerInstance::HashingBackendPtrTy HashBackend = nullptr;
  if (Invocation.getFrontendOptions().DeterministicCheck) {
    // Setup a verfication instance to run.
    std::unique_ptr<CompilerInstance> VerifyInstance =
        std::make_unique<CompilerInstance>();
    // Add a null diagnostic consumer to the diagnostic engine so the
    // compilation will exercise all the diagnose code path but not emitting
    // anything.
    NullDiagnosticConsumer DC;
    VerifyInstance->getDiags().addConsumer(DC);
    std::string InstanceSetupError;
    // This should not fail because it passed already.
    (void)VerifyInstance->setup(Invocation, InstanceSetupError, Args);

    // Run the first time without observer and discard return value;
    int ReturnValueTest = 0;
    (void)performCompile(*VerifyInstance, ReturnValueTest,
                         /*observer*/ nullptr);
    // Get the hashing output backend and free the compiler instance.
    HashBackend = VerifyInstance->getHashingBackend();
  }

  int ReturnValue = 0;
  bool HadError = performCompile(*Instance, ReturnValue, observer);

  if (verifierEnabled) {
    DiagnosticEngine &diags = Instance->getDiags();
    if (diags.hasFatalErrorOccurred() &&
        !Invocation.getDiagnosticOptions().ShowDiagnosticsAfterFatalError) {
      diags.resetHadAnyError();
      DH.setSuppressOutput(false);
      diags.diagnose(SourceLoc(), diag::verify_encountered_fatal);
      HadError = true;
    }
  }

  if (Invocation.getFrontendOptions().DeterministicCheck) {
    // Collect all output files.
    auto ReHashBackend = Instance->getHashingBackend();
    std::set<std::string> AllOutputs;
    toolchain::for_each(HashBackend->outputFiles(), [&](StringRef F) {
      AllOutputs.insert(F.str());
    });
    toolchain::for_each(ReHashBackend->outputFiles(), [&](StringRef F) {
      AllOutputs.insert(F.str());
    });

    DiagnosticEngine &diags = Instance->getDiags();
    for (auto &Filename : AllOutputs) {
      auto O1 = HashBackend->getHashValueForFile(Filename);
      if (!O1) {
        diags.diagnose(SourceLoc(), diag::error_output_missing, Filename,
                       /*SecondRun=*/false);
        HadError = true;
        continue;
      }
      auto O2 = ReHashBackend->getHashValueForFile(Filename);
      if (!O2) {
        diags.diagnose(SourceLoc(), diag::error_output_missing, Filename,
                       /*SecondRun=*/true);
        HadError = true;
        continue;
      }
      if (*O1 != *O2) {
        diags.diagnose(SourceLoc(), diag::error_nondeterministic_output,
                       Filename, *O1, *O2);
        HadError = true;
        continue;
      }
      diags.diagnose(SourceLoc(), diag::matching_output_produced, Filename,
                     *O1);
    }
  }

  auto r = finishDiagProcessing(HadError ? 1 : ReturnValue, verifierEnabled);
  if (auto *StatsReporter = Instance->getStatsReporter())
    StatsReporter->noteCurrentProcessExitStatus(r);

  DH.endMessage(r);
  return r;
}

void FrontendObserver::parsedArgs(CompilerInvocation &invocation) {}
void FrontendObserver::configuredCompiler(CompilerInstance &instance) {}
void FrontendObserver::performedSemanticAnalysis(CompilerInstance &instance) {}
void FrontendObserver::performedSILGeneration(SILModule &module) {}
void FrontendObserver::performedSILProcessing(SILModule &module) {}
