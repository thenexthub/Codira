//===--- CASOutputBackends.cpp ----------------------------------*- C++ -*-===//
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

#include "language/Frontend/CASOutputBackends.h"

#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"
#include "language/Frontend/CachingUtils.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/CompileJobCacheResult.h"
#include "clang/Frontend/CompileJobCacheResult.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/CAS/HierarchicalTreeBuilder.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/Mutex.h"
#include <optional>

#define DEBUG_TYPE "language-cas-backend"

using namespace language;
using namespace toolchain;
using namespace toolchain::cas;
using namespace toolchain::vfs;

namespace {
class CodiraCASOutputFile final : public OutputFileImpl {
public:
  Error keep() override { return OnKeep(Path, Bytes); }
  Error discard() override { return Error::success(); }
  raw_pwrite_stream &getOS() override { return OS; }

  using OnKeepType = toolchain::unique_function<Error(StringRef, StringRef)>;
  CodiraCASOutputFile(StringRef Path, OnKeepType OnKeep)
      : Path(Path.str()), OS(Bytes), OnKeep(std::move(OnKeep)) {}

private:
  std::string Path;
  SmallString<16> Bytes;
  raw_svector_ostream OS;
  OnKeepType OnKeep;
};
} // namespace

namespace language::cas {
void CodiraCASOutputBackend::anchor() {}

class CodiraCASOutputBackend::Implementation {
public:
  Implementation(ObjectStore &CAS, ActionCache &Cache, ObjectRef BaseKey,
                 const FrontendInputsAndOutputs &InputsAndOutputs,
                 const FrontendOptions &Opts,
                 FrontendOptions::ActionType Action)
      : CAS(CAS), Cache(Cache), BaseKey(BaseKey),
        InputsAndOutputs(InputsAndOutputs), Opts(Opts), Action(Action) {
    initBackend(InputsAndOutputs);
  }

  toolchain::Expected<std::unique_ptr<toolchain::vfs::OutputFileImpl>>
  createFileImpl(toolchain::StringRef ResolvedPath,
                 std::optional<toolchain::vfs::OutputConfig> Config) {
    auto ProducingInput = OutputToInputMap.find(ResolvedPath);
    if (ProducingInput == OutputToInputMap.end() &&
        ResolvedPath.starts_with(Opts.SymbolGraphOutputDir)) {
      return std::make_unique<CodiraCASOutputFile>(
          ResolvedPath, [this](StringRef Path, StringRef Bytes) -> Error {
            bool Shortened = Path.consume_front(Opts.SymbolGraphOutputDir);
            assert(Shortened && "symbol graph path outside output dir");
            (void)Shortened;
            std::optional<ObjectRef> PathRef;
            if (Error E =
                    CAS.storeFromString(std::nullopt, Path).moveInto(PathRef))
              return E;
            std::optional<ObjectRef> BytesRef;
            if (Error E =
                    CAS.storeFromString(std::nullopt, Bytes).moveInto(BytesRef))
              return E;
            auto Ref = CAS.store({*PathRef, *BytesRef}, {});
            if (!Ref)
              return Ref.takeError();
            SymbolGraphOutputRefs.push_back(*Ref);
            return Error::success();
          });
    }
    assert(ProducingInput != OutputToInputMap.end() && "Unknown output file");

    unsigned InputIndex = ProducingInput->second.first;
    auto OutputType = ProducingInput->second.second;

    // Uncached output kind.
    if (!isStoredDirectly(OutputType))
      return std::make_unique<toolchain::vfs::NullOutputFileImpl>();

    return std::make_unique<CodiraCASOutputFile>(
        ResolvedPath,
        [this, InputIndex, OutputType](StringRef Path,
                                       StringRef Bytes) -> Error {
          return storeImpl(Path, Bytes, InputIndex, OutputType);
        });
  }

  void initBackend(const FrontendInputsAndOutputs &InputsAndOutputs);

  Error storeImpl(StringRef Path, StringRef Bytes, unsigned InputIndex,
                  file_types::ID OutputKind);

  // Return true if all the outputs are produced for the given index.
  bool addProducedOutput(unsigned InputIndex, file_types::ID OutputKind,
                         ObjectRef BytesRef);
  Error finalizeCacheKeysFor(unsigned InputIndex);

private:
  friend class CodiraCASOutputBackend;
  ObjectStore &CAS;
  ActionCache &Cache;
  ObjectRef BaseKey;
  const FrontendInputsAndOutputs &InputsAndOutputs;
  const FrontendOptions &Opts;
  FrontendOptions::ActionType Action;

  // Lock for updating output file status.
  toolchain::sys::SmartMutex<true> OutputLock;

  // Map from output path to the input index and output kind.
  StringMap<std::pair<unsigned, file_types::ID>> OutputToInputMap;

  // A vector of output refs where the index is the input index.
  SmallVector<DenseMap<file_types::ID, ObjectRef>> OutputRefs;

  SmallVector<ObjectRef> SymbolGraphOutputRefs;
};

CodiraCASOutputBackend::CodiraCASOutputBackend(
    ObjectStore &CAS, ActionCache &Cache, ObjectRef BaseKey,
    const FrontendInputsAndOutputs &InputsAndOutputs,
    const FrontendOptions &Opts, FrontendOptions::ActionType Action)
    : Impl(*new CodiraCASOutputBackend::Implementation(
          CAS, Cache, BaseKey, InputsAndOutputs, Opts, Action)) {}

CodiraCASOutputBackend::~CodiraCASOutputBackend() { delete &Impl; }

bool CodiraCASOutputBackend::isStoredDirectly(file_types::ID Kind) {
  return !file_types::isProducedFromDiagnostics(Kind) &&
         Kind != file_types::TY_Dependencies;
}

IntrusiveRefCntPtr<OutputBackend> CodiraCASOutputBackend::cloneImpl() const {
  return makeIntrusiveRefCnt<CodiraCASOutputBackend>(
      Impl.CAS, Impl.Cache, Impl.BaseKey, Impl.InputsAndOutputs, Impl.Opts,
      Impl.Action);
}

Expected<std::unique_ptr<OutputFileImpl>>
CodiraCASOutputBackend::createFileImpl(StringRef ResolvedPath,
                                      std::optional<OutputConfig> Config) {
  return Impl.createFileImpl(ResolvedPath, Config);
}

file_types::ID CodiraCASOutputBackend::getOutputFileType(StringRef Path) const {
  return file_types::lookupTypeForExtension(toolchain::sys::path::extension(Path));
}

Error CodiraCASOutputBackend::storeImpl(StringRef Path, StringRef Bytes,
                                       unsigned InputIndex,
                                       file_types::ID OutputKind) {
  return Impl.storeImpl(Path, Bytes, InputIndex, OutputKind);
}

Error CodiraCASOutputBackend::storeCachedDiagnostics(unsigned InputIndex,
                                                    StringRef Bytes) {
  return storeImpl("<cached-diagnostics>", Bytes, InputIndex,
                   file_types::ID::TY_CachedDiagnostics);
}

Error CodiraCASOutputBackend::storeMakeDependenciesFile(StringRef OutputFilename,
                                                       toolchain::StringRef Bytes) {
  auto Input = Impl.OutputToInputMap.find(OutputFilename);
  if (Input == Impl.OutputToInputMap.end())
    return toolchain::createStringError("InputIndex for output file not found!");
  auto InputIndex = Input->second.first;
  assert(Input->second.second == file_types::TY_Dependencies &&
         "wrong output type");
  return storeImpl(OutputFilename, Bytes, InputIndex,
                   file_types::TY_Dependencies);
}

Error CodiraCASOutputBackend::storeMCCASObjectID(StringRef OutputFilename,
                                                toolchain::cas::CASID ID) {
  auto Input = Impl.OutputToInputMap.find(OutputFilename);
  if (Input == Impl.OutputToInputMap.end())
    return toolchain::createStringError("InputIndex for output file not found!");
  auto InputIndex = Input->second.first;
  auto MCRef = Impl.CAS.getReference(ID);
  if (!MCRef)
    return createStringError("Invalid CASID: " + ID.toString() +
                             ". No associated ObjectRef found!");

  if (Impl.addProducedOutput(InputIndex, file_types::TY_Object, *MCRef))
    return Impl.finalizeCacheKeysFor(InputIndex);

  return Error::success();
}

void CodiraCASOutputBackend::Implementation::initBackend(
    const FrontendInputsAndOutputs &InputsAndOutputs) {
  // FIXME: The output to input map might not be enough for example all the
  // outputs can be written to `-`, but the backend cannot distinguish which
  // input it actually comes from. Maybe the solution is just not to cache
  // any commands write output to `-`.
  file_types::ID mainOutputType = InputsAndOutputs.getPrincipalOutputType();
  auto addInput = [&](const InputFile &Input, unsigned Index) {
    // Ignore the outputFilename for typecheck action since it is not producing
    // an output file for that.
    if (!Input.outputFilename().empty() &&
        Action != FrontendOptions::ActionType::Typecheck)
      OutputToInputMap.insert(
          {Input.outputFilename(), {Index, mainOutputType}});
    Input.getPrimarySpecificPaths()
        .SupplementaryOutputs.forEachSetOutputAndType(
            [&](const std::string &Out, file_types::ID ID) {
              // FIXME: Opt Remarks are not setup correctly to be cached and
              // didn't go through the output backend. Do not cache them.
              if (ID == file_types::ID::TY_YAMLOptRecord ||
                  ID == file_types::ID::TY_BitstreamOptRecord)
                return;

              if (!file_types::isProducedFromDiagnostics(ID))
                OutputToInputMap.insert({Out, {Index, ID}});
            });
  };

  for (unsigned idx = 0; idx < InputsAndOutputs.getAllInputs().size(); ++idx)
    addInput(InputsAndOutputs.getAllInputs()[idx], idx);

  // FIXME: Cached diagnostics is associated with the first output producing
  // input file.
  OutputToInputMap.insert(
      {"<cached-diagnostics>",
       {InputsAndOutputs.getIndexOfFirstOutputProducingInput(),
        file_types::TY_CachedDiagnostics}});

  // Resize the output refs to hold all inputs.
  OutputRefs.resize(InputsAndOutputs.getAllInputs().size());
}

Error CodiraCASOutputBackend::Implementation::storeImpl(
    StringRef Path, StringRef Bytes, unsigned InputIndex,
    file_types::ID OutputKind) {
  std::optional<ObjectRef> BytesRef;
  if (Error E = CAS.storeFromString(std::nullopt, Bytes).moveInto(BytesRef))
    return E;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "DEBUG: producing CAS output of type \'"
                          << file_types::getTypeName(OutputKind)
                          << "\' for input \'" << InputIndex << "\': \'"
                          << CAS.getID(*BytesRef).toString() << "\'\n";);

  if (addProducedOutput(InputIndex, OutputKind, *BytesRef))
    return finalizeCacheKeysFor(InputIndex);
  return Error::success();
}

bool CodiraCASOutputBackend::Implementation::addProducedOutput(
    unsigned InputIndex, file_types::ID OutputKind,
    ObjectRef BytesRef) {
  toolchain::sys::SmartScopedLock<true> LockOutput(OutputLock);
  auto &ProducedOutputs = OutputRefs[InputIndex];
  ProducedOutputs.insert({OutputKind, BytesRef});

  return toolchain::all_of(OutputToInputMap, [&](auto &E) {
    return (E.second.first != InputIndex ||
            ProducedOutputs.count(E.second.second));
  });
}

Error CodiraCASOutputBackend::Implementation::finalizeCacheKeysFor(
    unsigned InputIndex) {
  const auto &ProducedOutputs = OutputRefs[InputIndex];
  assert(!ProducedOutputs.empty() && "Expect outputs for this input");

  std::vector<std::pair<file_types::ID, ObjectRef>> OutputsForInput;
  toolchain::for_each(ProducedOutputs, [&OutputsForInput](auto E) {
    OutputsForInput.emplace_back(E.first, E.second);
  });
  if (InputIndex == InputsAndOutputs.getIndexOfFirstOutputProducingInput() &&
      !SymbolGraphOutputRefs.empty()) {
    auto SGRef = CAS.store(SymbolGraphOutputRefs, {});
    if (!SGRef)
      return SGRef.takeError();
    OutputsForInput.emplace_back(file_types::ID::TY_SymbolGraphFile, *SGRef);
  }
  // Sort to a stable ordering for deterministic output cache object.
  toolchain::sort(OutputsForInput,
             [](auto &LHS, auto &RHS) { return LHS.first < RHS.first; });

  std::optional<ObjectRef> Result;
  // Use a clang compatible result CAS object schema when emiting PCM.
  if (Action == FrontendOptions::ActionType::EmitPCM) {
    clang::cas::CompileJobCacheResult::Builder Builder;

    for (auto &Outs : OutputsForInput) {
      if (Outs.first == file_types::ID::TY_ClangModuleFile)
        Builder.addOutput(
            clang::cas::CompileJobCacheResult::OutputKind::MainOutput,
            Outs.second);
      else if (Outs.first == file_types::ID::TY_CachedDiagnostics)
        Builder.addOutput(clang::cas::CompileJobCacheResult::OutputKind::
                              SerializedDiagnostics,
                          Outs.second);
      else if (Outs.first == file_types::ID::TY_Dependencies)
        Builder.addOutput(
            clang::cas::CompileJobCacheResult::OutputKind::Dependencies,
            Outs.second);
      else
        toolchain_unreachable("Unexpected output when compiling clang module");
    }

    if (auto Err = Builder.build(CAS).moveInto(Result))
      return Err;

  } else {
    language::cas::CompileJobCacheResult::Builder Builder;

    for (auto &Outs : OutputsForInput)
      Builder.addOutput(Outs.first, Outs.second);

    if (auto Err = Builder.build(CAS).moveInto(Result))
      return Err;
  }

  auto CacheKey = createCompileJobCacheKeyForOutput(CAS, BaseKey, InputIndex);
  if (!CacheKey)
    return CacheKey.takeError();

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "DEBUG: writing cache entry for input \'"
                          << InputIndex << "\': \'"
                          << CAS.getID(*CacheKey).toString() << "\' => \'"
                          << CAS.getID(*Result).toString() << "\'\n";);

  if (auto E = Cache.put(CAS.getID(*CacheKey), CAS.getID(*Result))) {
    // If `LANGUAGE_STRICT_CAS_ERRORS` environment is set, do not attempt to
    // recover from error.
    if (::getenv("LANGUAGE_STRICT_CAS_ERRORS"))
      return E;
    // Failed to update the action cache, this can happen when there is a
    // non-deterministic output from compiler. Check that the cache entry is
    // not missing, if so, then assume this is a cache poisoning that might
    // be benign and the build might continue without problem.
    auto Check = Cache.get(CAS.getID(*CacheKey));
    if (!Check) {
      // A real CAS error, return the original error.
      consumeError(Check.takeError());
      return E;
    }
    if (!*Check)
      return E;
    // Emit an error message to stderr but don't fail the job. Ideally this
    // should be sent to a DiagnosticEngine but CASBackend lives longer than
    // diagnostic engine and needs to capture all diagnostic outputs before
    // sending this request.
    toolchain::errs() << "error: failed to update cache: " << toString(std::move(E))
                 << "\n";
  }

  return Error::success();
}

} // end namespace language::cas
