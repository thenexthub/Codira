//===--- CachingUtils.cpp ---------------------------------------*- C++ -*-===//
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

#include "language/Frontend/CachingUtils.h"

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Frontend/CASOutputBackends.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/CompileJobCacheResult.h"
#include "language/Frontend/DiagnosticHelper.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Frontend/MakeStyleDependencies.h"
#include "language/Option/Options.h"
#include "clang/CAS/CASOptions.h"
#include "clang/CAS/IncludeTree.h"
#include "clang/Frontend/CompileJobCacheResult.h"
#include "toolchain/CAS/BuiltinUnifiedCASDatabases.h"
#include "toolchain/CAS/CASFileSystem.h"
#include "toolchain/CAS/HierarchicalTreeBuilder.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/CAS/TreeEntry.h"
#include "toolchain/MCCAS/MCCASObjectV1.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include <memory>

#define DEBUG_TYPE "cache-util"

using namespace language;
using namespace language::cas;
using namespace toolchain;
using namespace toolchain::cas;
using namespace toolchain::vfs;

namespace language {

toolchain::IntrusiveRefCntPtr<CodiraCASOutputBackend> createCodiraCachingOutputBackend(
    toolchain::cas::ObjectStore &CAS, toolchain::cas::ActionCache &Cache,
    toolchain::cas::ObjectRef BaseKey,
    const FrontendInputsAndOutputs &InputsAndOutputs,
    const FrontendOptions &Opts, FrontendOptions::ActionType Action) {
  return makeIntrusiveRefCnt<CodiraCASOutputBackend>(
      CAS, Cache, BaseKey, InputsAndOutputs, Opts, Action);
}

Error cas::CachedResultLoader::replay(CallbackTy Callback) {
  auto ResultProxy = CAS.getProxy(OutputRef);
  if (!ResultProxy)
    return ResultProxy.takeError();

  {
    language::cas::CompileJobResultSchema Schema(CAS);
    if (Schema.isRootNode(*ResultProxy)) {
      auto Result = Schema.load(OutputRef);
      if (!Result)
        return Result.takeError();

      if (auto Err = Result->forEachOutput(
              [&](language::cas::CompileJobCacheResult::Output Output) -> Error {
                return Callback(Output.Kind, Output.Object);
              }))
        return Err;

      return Error::success();
    }
  }
  {
    clang::cas::CompileJobResultSchema Schema(CAS);
    if (Schema.isRootNode(*ResultProxy)) {
      auto Result = Schema.load(OutputRef);
      if (!Result)
        return Result.takeError();
      if (auto Err = Result->forEachOutput(
          [&](clang::cas::CompileJobCacheResult::Output Output) -> Error {
            file_types::ID OutputKind = file_types::ID::TY_INVALID;
            switch (Output.Kind) {
            case clang::cas::CompileJobCacheResult::OutputKind::MainOutput:
              OutputKind = file_types::ID::TY_ClangModuleFile;
              break;
            case clang::cas::CompileJobCacheResult::OutputKind::Dependencies:
              OutputKind = file_types::ID::TY_Dependencies;
              break;
            case clang::cas::CompileJobCacheResult::OutputKind::
                SerializedDiagnostics:
              OutputKind = file_types::ID::TY_CachedDiagnostics;
              break;
            }
            assert(OutputKind != file_types::ID::TY_INVALID &&
                   "Unexpected output kind in clang cached result");
            return Callback(OutputKind, Output.Object);
          }))
        return Err;

      return Error::success();
    }
  }

  return createStringError(inconvertibleErrorCode(),
                           "unexpected output schema for cached result");
}

static Expected<std::optional<ObjectRef>>
lookupCacheKey(ObjectStore &CAS, ActionCache &Cache, ObjectRef CacheKey) {
  // Lookup the cache key for the input file.
  auto OutID = CAS.getID(CacheKey);
  auto Lookup = Cache.get(OutID);
  if (!Lookup)
    return Lookup.takeError();

  if (!*Lookup)
    return std::nullopt;

  return CAS.getReference(**Lookup);
}

namespace {
struct CacheInputEntry {
  const InputFile &Input;
  unsigned Index;
  ObjectRef OutputRef;
};
} // namespace

static bool replayCachedCompilerOutputsImpl(
    ArrayRef<CacheInputEntry> Inputs, ObjectStore &CAS, DiagnosticEngine &Diag,
    const FrontendOptions &Opts, CachingDiagnosticsProcessor &CDP,
    DiagnosticHelper *DiagHelper, OutputBackend &Backend, bool CacheRemarks,
    bool UseCASBackend) {
  bool CanReplayAllOutput = true;
  struct OutputEntry {
    std::string Path;
    CASID Key;
    file_types::ID Kind;
    const InputFile &Input;
    ObjectProxy Proxy;
  };
  SmallVector<OutputEntry> OutputProxies;
  std::optional<OutputEntry> DiagnosticsOutput;

  auto replayOutputsForInputFile = [&](const InputFile &Input,
                                       const std::string &InputPath,
                                       unsigned InputIndex,
                                       ObjectRef OutputRef,
                                       const DenseMap<file_types::ID,
                                                      std::string> &Outputs) {
    CachedResultLoader Loader(CAS, OutputRef);
    auto OutID = CAS.getID(OutputRef);
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "DEBUG: lookup cache key \'" << OutID.toString()
                            << "\' for input \'" << InputPath << "\n";);
    if (auto Err = Loader.replay([&](file_types::ID Kind,
                                     ObjectRef Ref) -> Error {
          auto OutputPath = Outputs.find(Kind);
          if (OutputPath == Outputs.end())
            return createStringError(
                inconvertibleErrorCode(),
                "unexpected output kind in the cached output");
          auto Proxy = CAS.getProxy(Ref);
          if (!Proxy)
            return Proxy.takeError();

          if (Kind == file_types::ID::TY_CachedDiagnostics) {
            assert(!DiagnosticsOutput && "more than 1 diagnotics found");
            DiagnosticsOutput.emplace(
                OutputEntry{OutputPath->second, OutID, Kind, Input, *Proxy});
          } else if (Kind == file_types::ID::TY_SymbolGraphFile &&
                     !Opts.SymbolGraphOutputDir.empty()) {
            auto Err = Proxy->forEachReference([&](ObjectRef Ref) -> Error {
              auto Proxy = CAS.getProxy(Ref);
              if (!Proxy)
                return Proxy.takeError();
              auto PathRef = Proxy->getReference(0);
              auto ContentRef = Proxy->getReference(1);
              auto Path = CAS.getProxy(PathRef);
              auto Content = CAS.getProxy(ContentRef);
              if (!Path)
                return Path.takeError();
              if (!Content)
                return Content.takeError();

              SmallString<128> OutputPath(Opts.SymbolGraphOutputDir);
              toolchain::sys::path::append(OutputPath, Path->getData());

              OutputProxies.emplace_back(OutputEntry{
                  std::string(OutputPath), OutID, Kind, Input, *Content});

              return Error::success();
            });
            if (Err)
              return Err;
          } else
            OutputProxies.emplace_back(
                OutputEntry{OutputPath->second, OutID, Kind, Input, *Proxy});
          return Error::success();
        })) {
      Diag.diagnose(SourceLoc(), diag::cache_replay_failed,
                    toString(std::move(Err)));
      CanReplayAllOutput = false;
    }
  };

  auto replayOutputFromInput = [&](const InputFile &Input,
                                   unsigned InputIndex, ObjectRef OutputRef) {
    auto InputPath = Input.getFileName();
    DenseMap<file_types::ID, std::string> Outputs;
    if (!Input.outputFilename().empty())
      Outputs.try_emplace(Opts.InputsAndOutputs.getPrincipalOutputType(),
                          Input.outputFilename());

    Input.getPrimarySpecificPaths()
        .SupplementaryOutputs.forEachSetOutputAndType(
            [&](const std::string &File, file_types::ID ID) {
              if (file_types::isProducedFromDiagnostics(ID))
                return;

              Outputs.try_emplace(ID, File);
            });

    // If this input doesn't produce any outputs, don't try to look up cache.
    // This can be a standalone emitModule action that only one input produces
    // output. The input can be skipped if it is not the first output producing
    // input, where it can have diagnostics and symbol graphs attached.
    if (Outputs.empty() &&
        InputIndex !=
            Opts.InputsAndOutputs.getIndexOfFirstOutputProducingInput())
      return;

    // Add cached diagnostic entry for lookup. Output path doesn't matter here.
    Outputs.try_emplace(file_types::ID::TY_CachedDiagnostics,
                        "<cached-diagnostics>");

    // Add symbol graph entry for lookup. Output path doesn't matter here.
    Outputs.try_emplace(file_types::ID::TY_SymbolGraphFile, "<symbol-graph>");

    return replayOutputsForInputFile(Input, InputPath, InputIndex, OutputRef,
                                     Outputs);
  };

  for (auto In : Inputs)
    replayOutputFromInput(In.Input, In.Index, In.OutputRef);

  if (!CanReplayAllOutput)
    return false;

  auto failedReplay = [DiagHelper]() {
    if (DiagHelper)
      DiagHelper->endMessage(/*retCode=*/1);
    return false;
  };

  // Replay Diagnostics first so the output failures comes after.
  // Also if the diagnostics replay failed, proceed to re-compile.
  if (DiagnosticsOutput) {
    // Only starts message if there are diagnostics.
    if (DiagHelper)
      DiagHelper->beginMessage();
    if (auto E =
            CDP.replayCachedDiagnostics(DiagnosticsOutput->Proxy.getData())) {
      Diag.diagnose(SourceLoc(), diag::error_replay_cached_diag,
                    toString(std::move(E)));
      return failedReplay();
    }
  }

  if (CacheRemarks)
    Diag.diagnose(SourceLoc(), diag::replay_output, "<cached-diagnostics>",
                  DiagnosticsOutput->Key.toString());

  // Replay the result only when everything is resolved.
  for (auto &Output : OutputProxies) {
    auto File = Backend.createFile(Output.Path);
    if (!File) {
      Diag.diagnose(SourceLoc(), diag::error_opening_output, Output.Path,
                    toString(File.takeError()));
      continue;
    }

    if (UseCASBackend && Output.Kind == file_types::ID::TY_Object) {
      auto Schema = std::make_unique<toolchain::mccasformats::v1::MCSchema>(CAS);
      if (auto E = Schema->serializeObjectFile(Output.Proxy, *File)) {
        Diag.diagnose(SourceLoc(), diag::error_mccas, toString(std::move(E)));
        return failedReplay();
      }
    } else if (Output.Kind == file_types::ID::TY_Dependencies) {
      if (emitMakeDependenciesFromSerializedBuffer(
            Output.Proxy.getData(), *File, Opts, Output.Input, Diag)) {
        Diag.diagnose(SourceLoc(), diag::cache_replay_failed,
                      "failed to emit dependency file");
        return failedReplay();
      }
    } else
      *File << Output.Proxy.getData();

    if (auto E = File->keep()) {
      Diag.diagnose(SourceLoc(), diag::error_closing_output, Output.Path,
                    toString(std::move(E)));
      continue;
    }
    if (CacheRemarks)
      Diag.diagnose(SourceLoc(), diag::replay_output, Output.Path,
                    Output.Key.toString());
  }

  if (DiagHelper)
    DiagHelper->endMessage(/*retCode=*/0);
  return true;
}

bool replayCachedCompilerOutputs(
    ObjectStore &CAS, ActionCache &Cache, ObjectRef BaseKey,
    DiagnosticEngine &Diag, const FrontendOptions &Opts,
    CachingDiagnosticsProcessor &CDP, bool CacheRemarks, bool UseCASBackend) {
  // Compute all the inputs need replay.
  toolchain::SmallVector<CacheInputEntry> Inputs;
  auto AllInputs = Opts.InputsAndOutputs.getAllInputs();
  auto lookupEntry = [&](unsigned InputIndex,
                         StringRef InputPath) -> std::optional<ObjectRef> {
    auto OutputKey =
        createCompileJobCacheKeyForOutput(CAS, BaseKey, InputIndex);

    if (!OutputKey) {
      Diag.diagnose(SourceLoc(), diag::error_cas, "output cache key creation",
                    toString(OutputKey.takeError()));
      return std::nullopt;
    }

    auto OutID = CAS.getID(*OutputKey);
    auto OutputRef = lookupCacheKey(CAS, Cache, *OutputKey);
    if (!OutputRef) {
      Diag.diagnose(SourceLoc(), diag::error_cas, "cache key lookup",
                    toString(OutputRef.takeError()));
      return std::nullopt;
    }

    if (!*OutputRef) {
      if (CacheRemarks)
        Diag.diagnose(SourceLoc(), diag::output_cache_miss, InputPath,
                      OutID.toString());
      return std::nullopt;
    }

    return *OutputRef;
  };

  // If there are primary inputs, look up only the primary input files.
  // Otherwise, prepare to do cache lookup for all inputs.
  for (unsigned Index = 0; Index < AllInputs.size(); ++Index) {
    const auto &Input = AllInputs[Index];
    if (Opts.InputsAndOutputs.hasPrimaryInputs() && !Input.isPrimary())
      continue;

    if (Input.outputFilename().empty() &&
        Input.getPrimarySpecificPaths().SupplementaryOutputs.empty() &&
        Index != Opts.InputsAndOutputs.getIndexOfFirstOutputProducingInput())
      continue;

    if (auto OutputRef = lookupEntry(Index, Input.getFileName()))
      Inputs.push_back({Input, Index, *OutputRef});
    else
      return false;
  }

  // Use on disk output backend directly here to write to disk.
  toolchain::vfs::OnDiskOutputBackend Backend;
  return replayCachedCompilerOutputsImpl(Inputs, CAS, Diag, Opts, CDP,
                                         /*DiagHelper=*/nullptr, Backend,
                                         CacheRemarks, UseCASBackend);
}

bool replayCachedCompilerOutputsForInput(
    ObjectStore &CAS, ObjectRef OutputRef, const InputFile &Input,
    unsigned InputIndex, DiagnosticEngine &Diag, DiagnosticHelper &DiagHelper,
    OutputBackend &OutBackend, const FrontendOptions &Opts,
    CachingDiagnosticsProcessor &CDP, bool CacheRemarks, bool UseCASBackend) {
  toolchain::SmallVector<CacheInputEntry> Inputs = {{Input, InputIndex, OutputRef}};
  return replayCachedCompilerOutputsImpl(Inputs, CAS, Diag, Opts, CDP,
                                         &DiagHelper, OutBackend, CacheRemarks,
                                         UseCASBackend);
}

std::unique_ptr<toolchain::MemoryBuffer>
loadCachedCompileResultFromCacheKey(ObjectStore &CAS, ActionCache &Cache,
                                    DiagnosticEngine &Diag, StringRef CacheKey,
                                    file_types::ID Kind, StringRef Filename) {
  auto failure = [&](StringRef Stage, Error Err) {
    Diag.diagnose(SourceLoc(), diag::error_cas, Stage,
                  toString(std::move(Err)));
    return nullptr;
  };
  auto ID = CAS.parseID(CacheKey);
  if (!ID) {
    Diag.diagnose(SourceLoc(), diag::error_invalid_cas_id, CacheKey,
                  toString(ID.takeError()));
    return nullptr;
  }
  auto Ref = CAS.getReference(*ID);
  if (!Ref)
    return nullptr;

  auto OutputRef = lookupCacheKey(CAS, Cache, *Ref);
  if (!OutputRef)
    return failure("lookup cache key", OutputRef.takeError());

  if (!*OutputRef)
    return nullptr;

  CachedResultLoader Loader(CAS, **OutputRef);
  std::unique_ptr<toolchain::MemoryBuffer> Buffer;
  if (auto Err =
          Loader.replay([&](file_types::ID Type, ObjectRef Ref) -> Error {
            if (Kind != Type)
              return Error::success();

            auto Proxy = CAS.getProxy(Ref);
            if (!Proxy)
              return Proxy.takeError();

            Buffer = Proxy->getMemoryBuffer(Filename);
            return Error::success();
          }))
    return failure("loading cached results", std::move(Err));

  return Buffer;
}

static toolchain::Error createCASObjectNotFoundError(const toolchain::cas::CASID &ID) {
  return createStringError(toolchain::inconvertibleErrorCode(),
                           "CASID missing from Object Store " + ID.toString());
}

Expected<IntrusiveRefCntPtr<vfs::FileSystem>>
createCASFileSystem(ObjectStore &CAS, const std::string &IncludeTree,
                    const std::string &IncludeTreeFileList) {
  assert(!IncludeTree.empty() ||
         !IncludeTreeFileList.empty() && "no root ID provided");

  if (!IncludeTree.empty()) {
    auto ID = CAS.parseID(IncludeTree);
    if (!ID)
      return ID.takeError();

    auto Ref = CAS.getReference(*ID);
    if (!Ref)
      return createCASObjectNotFoundError(*ID);
    auto IT = clang::cas::IncludeTreeRoot::get(CAS, *Ref);
    if (!IT)
      return IT.takeError();

    auto ITF = IT->getFileList();
    if (!ITF)
      return ITF.takeError();

    auto ITFS = clang::cas::createIncludeTreeFileSystem(*ITF);
    if (!ITFS)
      return ITFS.takeError();

    return *ITFS;
  }

  if (!IncludeTreeFileList.empty()) {
    auto ID = CAS.parseID(IncludeTreeFileList);
    if (!ID)
      return ID.takeError();

    auto Ref = CAS.getReference(*ID);
    if (!Ref)
      return createCASObjectNotFoundError(*ID);
    auto ITF = clang::cas::IncludeTree::FileList::get(CAS, *Ref);
    if (!ITF)
      return ITF.takeError();

    auto ITFS = clang::cas::createIncludeTreeFileSystem(*ITF);
    if (!ITFS)
      return ITFS.takeError();

    return *ITFS;
  }

  return nullptr;
}

std::vector<std::string> remapPathsFromCommandLine(
    ArrayRef<std::string> commandLine,
    toolchain::function_ref<std::string(StringRef)> RemapCallback) {
  // parse and remap options that is path and not cache invariant.
  unsigned MissingIndex;
  unsigned MissingCount;
  std::vector<const char *> Args;
  std::for_each(commandLine.begin(), commandLine.end(),
                [&](const std::string &arg) { Args.push_back(arg.c_str()); });
  std::unique_ptr<toolchain::opt::OptTable> Table = createCodiraOptTable();
  toolchain::opt::InputArgList ParsedArgs = Table->ParseArgs(
      Args, MissingIndex, MissingCount, options::FrontendOption);
  SmallVector<const char *, 16> newArgs;
  std::vector<std::string> newCommandLine;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  for (auto *Arg : ParsedArgs) {
    Arg->render(ParsedArgs, newArgs);
    const auto &Opt = Arg->getOption();
    if (Opt.matches(options::OPT_INPUT) ||
        (!Opt.hasFlag(options::CacheInvariant) &&
         Opt.hasFlag(options::ArgumentIsPath))) {
      StringRef newPath = Saver.save(RemapCallback(Arg->getValue()));
      newArgs.back() = newPath.data();
    }
  }
  std::for_each(newArgs.begin(), newArgs.end(),
                [&](const char *arg) { newCommandLine.emplace_back(arg); });

  return newCommandLine;
}

} // namespace language
