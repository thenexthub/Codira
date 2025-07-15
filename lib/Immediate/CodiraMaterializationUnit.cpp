//===--- CodiraMaterializationUnit.cpp - JIT Codira ASTs ----------*- C++ -*-===//
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
// Defines the `CodiraMaterializationUnit` class, which allows you to JIT
// individual Codira AST declarations.
//
//===----------------------------------------------------------------------===//

#include <memory>
#include <optional>
#include <string>

#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ExecutionEngine/JITLink/JITLink.h"
#include "toolchain/ExecutionEngine/JITSymbol.h"
#include "toolchain/ExecutionEngine/Orc/Core.h"
#include "toolchain/ExecutionEngine/Orc/EPCIndirectionUtils.h"
#include "toolchain/ExecutionEngine/Orc/ExecutionUtils.h"
#include "toolchain/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "toolchain/ExecutionEngine/Orc/LLJIT.h"
#include "toolchain/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "toolchain/ExecutionEngine/Orc/ObjectTransformLayer.h"
#include "toolchain/ExecutionEngine/Orc/Shared/ExecutorAddress.h"
#include "toolchain/ExecutionEngine/Orc/Shared/MemoryFlags.h"
#include "toolchain/ExecutionEngine/Orc/TargetProcess/TargetExecutionUtils.h"
#include "toolchain/Support/Error.h"

#include "language/AST/IRGenRequests.h"
#include "language/AST/SILGenRequests.h"
#include "language/AST/TBDGenRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Frontend/Frontend.h"
#include "language/Immediate/CodiraMaterializationUnit.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Subsystems.h"

#define DEBUG_TYPE "language-immediate"

using namespace language;

/// The suffix appended to function bodies when creating lazy reexports
static const std::string ManglingSuffix = "$impl";

/// Mangle a function for a lazy reexport
static std::string mangle(const StringRef Unmangled) {
  return Unmangled.str() + ManglingSuffix;
}

/// Whether a function name is mangled to be a lazy reexport
static bool isMangled(const StringRef Symbol) {
  return Symbol.ends_with(ManglingSuffix);
}

/// Demangle a lazy reexport
static StringRef demangle(const StringRef Mangled) {
  return isMangled(Mangled) ? Mangled.drop_back(ManglingSuffix.size())
                            : Mangled;
}

toolchain::Expected<std::unique_ptr<CodiraJIT>>
CodiraJIT::Create(CompilerInstance &CI) {

  auto J = CreateLLJIT(CI);
  if (!J)
    return J.takeError();

  // Create generator to resolve symbols defined in current process
  auto &ES = (*J)->getExecutionSession();
  auto EPCIU =
      toolchain::orc::EPCIndirectionUtils::Create(ES.getExecutorProcessControl());
  if (!EPCIU)
    return EPCIU.takeError();

  (*EPCIU)->createLazyCallThroughManager(
      ES, toolchain::orc::ExecutorAddr::fromPtr(&handleLazyCompilationFailure));

  if (auto Err = setUpInProcessLCTMReentryViaEPCIU(**EPCIU))
    return std::move(Err);
  return std::unique_ptr<CodiraJIT>(
      new CodiraJIT(std::move(*J), std::move(*EPCIU)));
}

CodiraJIT::~CodiraJIT() {
  if (auto Err = EPCIU->cleanup())
    J->getExecutionSession().reportError(std::move(Err));
}

toolchain::Expected<int> CodiraJIT::runMain(toolchain::ArrayRef<std::string> Args) {
  if (auto Err = J->initialize(J->getMainJITDylib())) {
    return std::move(Err);
  }

  auto MainSym = J->lookup("main");
  if (!MainSym) {
    return MainSym.takeError();
  }

  using MainFnTy = int (*)(int, char *[]);
  MainFnTy JITMain = MainSym->toPtr<MainFnTy>();

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Running main\n");
  int Result = toolchain::orc::runAsMain(JITMain, Args);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Running static destructors\n");
  if (auto Err = J->deinitialize(J->getMainJITDylib())) {
    return std::move(Err);
  }

  return Result;
}

toolchain::orc::JITDylib &CodiraJIT::getMainJITDylib() {
  return J->getMainJITDylib();
}

std::string CodiraJIT::mangle(StringRef Name) { return J->mangle(Name); }

toolchain::orc::SymbolStringPtr CodiraJIT::mangleAndIntern(StringRef Name) {
  return J->mangleAndIntern(Name);
}

toolchain::orc::SymbolStringPtr CodiraJIT::intern(StringRef Name) {
  return J->getExecutionSession().intern(Name);
}

toolchain::orc::IRCompileLayer &CodiraJIT::getIRCompileLayer() {
  return J->getIRCompileLayer();
}

toolchain::orc::ObjectTransformLayer &CodiraJIT::getObjTransformLayer() {
  return J->getObjTransformLayer();
}

toolchain::Expected<std::unique_ptr<toolchain::orc::LLJIT>>
CodiraJIT::CreateLLJIT(CompilerInstance &CI) {
  toolchain::TargetOptions TargetOpt;
  std::string CPU;
  std::string Triple;
  std::vector<std::string> Features;
  const auto &Invocation = CI.getInvocation();
  const auto &IRGenOpts = Invocation.getIRGenOptions();
  auto &Ctx = CI.getASTContext();
  std::tie(TargetOpt, CPU, Features, Triple) =
      getIRTargetOptions(IRGenOpts, Ctx);
  auto JTMB = toolchain::orc::JITTargetMachineBuilder(toolchain::Triple(Triple))
                  .setRelocationModel(toolchain::Reloc::PIC_)
                  .setOptions(std::move(TargetOpt))
                  .setCPU(std::move(CPU))
                  .addFeatures(Features)
                  .setCodeGenOptLevel(toolchain::CodeGenOptLevel::Default);
  auto J = toolchain::orc::LLJITBuilder()
               .setJITTargetMachineBuilder(std::move(JTMB))
               .create();
  if (!J)
    return J.takeError();
  auto G = toolchain::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
      (*J)->getDataLayout().getGlobalPrefix());
  if (!G)
    return G.takeError();
  (*J)->getMainJITDylib().addGenerator(std::move(*G));
  return J;
}

static toolchain::Error
renameFunctionBodies(toolchain::orc::MaterializationResponsibility &MR,
                     toolchain::jitlink::LinkGraph &G) {

  using namespace toolchain;
  using namespace toolchain::orc;

  toolchain::StringSet<> ToRename;
  for (auto &KV : MR.getSymbols()) {
    StringRef Name = *KV.first;
    if (isMangled(Name))
      // All mangled functions we are responsible for
      // materializing must be mangled at the object levels
      ToRename.insert(demangle(Name));
  }
  for (auto &Sec : G.sections()) {
    // Skip non-executable sections.
    if ((Sec.getMemProt() & MemProt::Exec) == MemProt::None)
      continue;

    for (auto *Sym : Sec.symbols()) {
      // Skip all anonymous and non-callables.
      if (!Sym->hasName() || !Sym->isCallable())
        continue;

      if (ToRename.count(Sym->getName())) {
        // FIXME: Get rid of the temporary when Codira's toolchain-project is
        // updated to LLVM 17.
        auto NewName = G.allocateCString(Twine(mangle(Sym->getName())));
        Sym->setName({NewName.data(), NewName.size() - 1});
      }
    }
  }

  return toolchain::Error::success();
}

void CodiraJIT::Plugin::modifyPassConfig(
    toolchain::orc::MaterializationResponsibility &MR, toolchain::jitlink::LinkGraph &G,
    toolchain::jitlink::PassConfiguration &PassConfig) {
  PassConfig.PrePrunePasses.push_back(
      [&](toolchain::jitlink::LinkGraph &G) { return renameFunctionBodies(MR, G); });
}

toolchain::Error
CodiraJIT::Plugin::notifyFailed(toolchain::orc::MaterializationResponsibility &MR) {
  return toolchain::Error::success();
}

toolchain::Error
CodiraJIT::Plugin::notifyRemovingResources(toolchain::orc::JITDylib &JD,
                                          toolchain::orc::ResourceKey K) {
  return toolchain::Error::success();
}

void CodiraJIT::Plugin::notifyTransferringResources(
    toolchain::orc::JITDylib &JD, toolchain::orc::ResourceKey DstKey,
    toolchain::orc::ResourceKey SrcKey) {}

void CodiraJIT::handleLazyCompilationFailure() {
  toolchain::errs() << "Lazy compilation error\n";
  exit(1);
}

CodiraJIT::CodiraJIT(std::unique_ptr<toolchain::orc::LLJIT> J,
                   std::unique_ptr<toolchain::orc::EPCIndirectionUtils> EPCIU)
    : J(std::move(J)), EPCIU(std::move(EPCIU)),
      LCTM(this->EPCIU->getLazyCallThroughManager()),
      ISM(this->EPCIU->createIndirectStubsManager()) {}

void CodiraJIT::addRenamer() {
  static_cast<toolchain::orc::ObjectLinkingLayer &>(this->J->getObjLinkingLayer())
      .addPlugin(std::make_unique<Plugin>());
}

/// IRGen the provided `SILModule` with the specified options.
/// Returns `std::nullopt` if a compiler error is encountered
static std::optional<GeneratedModule>
generateModule(const CompilerInstance &CI, std::unique_ptr<SILModule> SM) {
  // TODO: Use OptimizedIRRequest for this.
  const auto &Context = CI.getASTContext();
  auto *languageModule = CI.getMainModule();
  const auto PSPs = CI.getPrimarySpecificPathsForAtMostOnePrimary();
  const auto &Invocation = CI.getInvocation();
  const auto &TBDOpts = Invocation.getTBDGenOptions();
  const auto &IRGenOpts = Invocation.getIRGenOptions();

  // Lower the SIL module to LLVM IR
  auto GenModule = performIRGeneration(
      languageModule, IRGenOpts, TBDOpts, std::move(SM),
      languageModule->getName().str(), PSPs, ArrayRef<std::string>());

  if (Context.hadError()) {
    return std::nullopt;
  }

  assert(GenModule && "Emitted no diagnostics but IR generation failed?");
  auto *Module = GenModule.getModule();

  // Run LLVM passes on the resulting module
  performLLVM(IRGenOpts, Context.Diags, /*diagMutex*/ nullptr,
              /*hash*/ nullptr, Module, GenModule.getTargetMachine(),
              CI.getPrimarySpecificPathsForAtMostOnePrimary().OutputFilename,
              CI.getOutputBackend(), Context.Stats);

  if (Context.hadError()) {
    return std::nullopt;
  }

  return GenModule;
}

/// Log a compilation error to standard error
static void logError(toolchain::Error Err) {
  logAllUnhandledErrors(std::move(Err), toolchain::errs(), "");
}

std::unique_ptr<LazyCodiraMaterializationUnit>
LazyCodiraMaterializationUnit::Create(CodiraJIT &JIT, CompilerInstance &CI) {
  auto *M = CI.getMainModule();
  TBDGenOptions Opts;
  Opts.PublicOrPackageSymbolsOnly = false;
  auto TBDDesc = TBDGenDescriptor::forModule(M, std::move(Opts));
  SymbolSourceMapRequest SourceReq{TBDDesc};
  const auto *Sources = evaluateOrFatal(
      M->getASTContext().evaluator,
      std::move(SourceReq));
  toolchain::orc::SymbolFlagsMap PublicInterface;
  for (const auto &Entry : *Sources) {
    const auto &Source = Entry.getValue();
    const auto &SymbolName = Entry.getKey();
    auto Flags = Source.getJITSymbolFlags();
    if (Flags.isCallable()) {
      // Only create lazy reexports for callable symbols
      auto MangledName = mangle(SymbolName);
      PublicInterface[JIT.intern(MangledName)] = Flags;
    } else {
      PublicInterface[JIT.intern(SymbolName)] = Flags;
    }
  }
  return std::unique_ptr<LazyCodiraMaterializationUnit>(
      new LazyCodiraMaterializationUnit(JIT, CI, std::move(Sources),
                                       std::move(PublicInterface)));
}

StringRef LazyCodiraMaterializationUnit::getName() const {
  return "CodiraMaterializationUnit";
}

LazyCodiraMaterializationUnit::LazyCodiraMaterializationUnit(
    CodiraJIT &JIT, CompilerInstance &CI, const SymbolSourceMap *Sources,
    toolchain::orc::SymbolFlagsMap Symbols)
    : MaterializationUnit({std::move(Symbols), nullptr}), Sources(Sources),
      JIT(JIT), CI(CI) {}

void LazyCodiraMaterializationUnit::materialize(
    std::unique_ptr<toolchain::orc::MaterializationResponsibility> MR) {
  SymbolSources Entities;
  const auto &RS = MR->getRequestedSymbols();
  for (auto &Sym : RS) {
    auto Name = demangle(*Sym);
    auto itr = Sources->find(Name);
    assert(itr != Sources->end() && "Requested symbol doesn't have source?");
    const auto &Source = itr->getValue();
    Source.typecheck();
    if (CI.getASTContext().hadError()) {
      // If encounter type error, bail out
      MR->failMaterialization();
      return;
    }
    Entities.push_back(Source);
  }
  auto SM = performASTLowering(CI, std::move(Entities));

  // Promote linkages of SIL entities
  // defining requested symbols so they are
  // emitted during IRGen.
  SM->promoteLinkages();

  runSILDiagnosticPasses(*SM);
  runSILLoweringPasses(*SM);
  auto GM = generateModule(CI, std::move(SM));
  if (!GM) {
    MR->failMaterialization();
    return;
  }
  auto *Module = GM->getModule();

  // All renamings defined by `MR`, e.g. "foo" -> "foo$impl"
  toolchain::StringMap<toolchain::orc::SymbolStringPtr> Renamings;
  for (auto &[Sym, Flags] : MR->getSymbols()) {
    Renamings[demangle(*Sym)] = Sym;
  }
  // Now we must register all other public symbols defined by
  // the module with the JIT
  toolchain::orc::SymbolFlagsMap LazilyDiscoveredSymbols;

  // All symbols defined by the compiled module in `MR`
  toolchain::DenseSet<toolchain::orc::SymbolStringPtr> DefinedSymbols;

  // Register all global values, including global
  // variables and functions
  for (auto &GV : Module->global_values()) {
    auto Name = JIT.mangle(GV.getName());
    auto itr = Renamings.find(Name);
    if (GV.hasAppendingLinkage() || GV.isDeclaration()) {
      continue;
    }
    if (itr == Renamings.end()) {
      if (GV.hasLocalLinkage()) {
        continue;
      }
      LazilyDiscoveredSymbols[JIT.intern(Name)] =
          toolchain::JITSymbolFlags::fromGlobalValue(GV);
      // Ignore all symbols that will not appear in symbol table
    } else {
      // Promote linkage of requested symbols that will
      // not appear in symbol table otherwise
      if (GV.hasLocalLinkage()) {
        GV.setLinkage(toolchain::GlobalValue::ExternalLinkage);
        GV.setVisibility(toolchain::GlobalValue::HiddenVisibility);
      }
      DefinedSymbols.insert(itr->getValue());
    }
  }

  toolchain::orc::SymbolFlagsMap UnrequestedSymbols;
  for (auto &[Sym, Flags] : MR->getSymbols()) {
    if (!DefinedSymbols.contains(Sym)) {
      UnrequestedSymbols[Sym] = Flags;
    }
  }
  std::unique_ptr<MaterializationUnit> UnrequestedMU(
      new LazyCodiraMaterializationUnit(JIT, CI, Sources,
                                       std::move(UnrequestedSymbols)));
  if (auto Err = MR->replace(std::move(UnrequestedMU))) {
    logError(std::move(Err));
    MR->failMaterialization();
    return;
  }
  if (auto Err = MR->defineMaterializing(std::move(LazilyDiscoveredSymbols))) {
    logError(std::move(Err));
    MR->failMaterialization();
    return;
  }
  auto TSM = std::move(*GM).intoThreadSafeContext();
  JIT.getIRCompileLayer().emit(std::move(MR), std::move(TSM));
}

toolchain::Error
CodiraJIT::addCodira(toolchain::orc::JITDylib &JD,
                   std::unique_ptr<toolchain::orc::MaterializationUnit> MU) {
  // Create stub map.
  toolchain::orc::SymbolAliasMap Stubs;
  for (auto &[Name, Flags] : MU->getSymbols()) {
    if (isMangled(*Name)) {
      // Create a stub for mangled functions
      auto OriginalName = demangle(*Name);
      Stubs.insert(
          {J->getExecutionSession().intern(OriginalName), {Name, Flags}});
    }
  }
  assert(ISM.get() && "No ISM?");

  if (!Stubs.empty())
    if (auto Err = JD.define(lazyReexports(LCTM, *ISM, JD, std::move(Stubs))))
      return Err;

  return JD.define(std::move(MU));
}

void LazyCodiraMaterializationUnit::discard(
    const toolchain::orc::JITDylib &JD, const toolchain::orc::SymbolStringPtr &Sym) {}

EagerCodiraMaterializationUnit::EagerCodiraMaterializationUnit(
    CodiraJIT &JIT, const CompilerInstance &CI, const IRGenOptions &IRGenOpts,
    std::unique_ptr<SILModule> SM)
    : MaterializationUnit(getInterface(JIT, CI)), JIT(JIT), CI(CI),
      IRGenOpts(IRGenOpts), SM(std::move(SM)) {}

StringRef EagerCodiraMaterializationUnit::getName() const {
  return "EagerCodiraMaterializationUnit";
}

void EagerCodiraMaterializationUnit::materialize(
    std::unique_ptr<toolchain::orc::MaterializationResponsibility> R) {

  auto GenModule = generateModule(CI, std::move(SM));

  if (!GenModule) {
    R->failMaterialization();
    return;
  }

  auto *Module = GenModule->getModule();

  // Dump IR if requested
  dumpJIT(*Module);

  // Now we must register all other public symbols defined by
  // the module with the JIT
  toolchain::orc::SymbolFlagsMap Symbols;
  // Register all global values, including global
  // variables and functions
  for (const auto &GV : Module->global_values()) {
    // Ignore all symbols that will not appear in symbol table
    if (GV.hasLocalLinkage() || GV.isDeclaration() || GV.hasAppendingLinkage())
      continue;
    auto Name = GV.getName();
    // The entry point is already registered up front with the
    // interface, so ignore it as well
    if (Name == CI.getASTContext().getEntryPointFunctionName())
      continue;
    auto MangledName = JIT.mangleAndIntern(Name);
    // Register this symbol with the proper flags
    Symbols[MangledName] = toolchain::JITSymbolFlags::fromGlobalValue(GV);
  }
  // Register the symbols we have discovered with the JIT
  if (auto Err = R->defineMaterializing(Symbols)) {
    logError(std::move(Err));
  }
  auto TSM = std::move(*GenModule).intoThreadSafeContext();
  JIT.getIRCompileLayer().emit(std::move(R), std::move(TSM));
}

toolchain::orc::MaterializationUnit::Interface
EagerCodiraMaterializationUnit::getInterface(CodiraJIT &JIT,
                                            const CompilerInstance &CI) {
  const auto &EntryPoint = CI.getASTContext().getEntryPointFunctionName();
  auto MangledEntryPoint = JIT.mangleAndIntern(EntryPoint);
  auto Flags = toolchain::JITSymbolFlags::Callable | toolchain::JITSymbolFlags::Exported;
  toolchain::orc::SymbolFlagsMap Symbols{{MangledEntryPoint, Flags}};
  return {std::move(Symbols), nullptr};
}

void EagerCodiraMaterializationUnit::discard(
    const toolchain::orc::JITDylib &JD, const toolchain::orc::SymbolStringPtr &Sym) {}

static void DumpLLVMIR(const toolchain::Module &M) {
  std::string path = (M.getName() + ".ll").str();
  for (size_t count = 0; toolchain::sys::fs::exists(path);)
    path = (M.getName() + toolchain::utostr(count++) + ".ll").str();

  std::error_code error;
  toolchain::raw_fd_ostream stream(path, error);
  if (error)
    return;
  M.print(stream, /*AssemblyAnnotationWriter=*/nullptr);
}

void EagerCodiraMaterializationUnit::dumpJIT(const toolchain::Module &M) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Module to be executed:\n"; M.dump());
  switch (IRGenOpts.DumpJIT) {
  case JITDebugArtifact::None:
    break;
  case JITDebugArtifact::LLVMIR:
    DumpLLVMIR(M);
    break;
  case JITDebugArtifact::Object:
    JIT.getObjTransformLayer().setTransform(toolchain::orc::DumpObjects());
    break;
  }
}
