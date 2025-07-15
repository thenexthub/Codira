//===------------ CodiraCaching.cpp - Codira Compiler -----------------------===//
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
// Implementation of the caching APIs in libCodiraScan
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/SourceManager.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "language/DependencyScan/StringUtils.h"
#include "language/Driver/FrontendUtil.h"
#include "language/Frontend/CachedDiagnostics.h"
#include "language/Frontend/CachingUtils.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/CompileJobCacheResult.h"
#include "language/Frontend/DiagnosticHelper.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/MakeStyleDependencies.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "clang/CAS/CASOptions.h"
#include "clang/Frontend/CompileJobCacheResult.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/BuiltinUnifiedCASDatabases.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Endian.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>
#include <variant>

namespace {
/// Helper class to manage CAS/Caching from libCodiraScan C APIs.
class CodiraScanCAS {
public:
  toolchain::cas::ObjectStore &getCAS() const { return *CAS; }
  toolchain::cas::ActionCache &getCache() const { return *Cache; }

  // Construct CodiraScanCAS.
  static toolchain::Expected<CodiraScanCAS *>
  createCodiraScanCAS(toolchain::StringRef Path);

  static toolchain::Expected<CodiraScanCAS *>
  createCodiraScanCAS(clang::CASOptions &CASOpts);

private:
  CodiraScanCAS(std::shared_ptr<toolchain::cas::ObjectStore> CAS,
               std::shared_ptr<toolchain::cas::ActionCache> Cache)
      : CAS(CAS), Cache(Cache) {}

  std::shared_ptr<toolchain::cas::ObjectStore> CAS;
  std::shared_ptr<toolchain::cas::ActionCache> Cache;
};

struct CodiraCachedCompilationHandle {
  CodiraCachedCompilationHandle(toolchain::cas::ObjectRef Key,
                               toolchain::cas::ObjectRef Output,
                               clang::cas::CompileJobCacheResult &&Result,
                               unsigned InputIndex, CodiraScanCAS &CAS)
      : Key(Key), Output(Output), InputIndex(InputIndex), Result(Result),
        DB(CAS) {}
  CodiraCachedCompilationHandle(toolchain::cas::ObjectRef Key,
                               toolchain::cas::ObjectRef Output,
                               language::cas::CompileJobCacheResult &&Result,
                               unsigned InputIndex, CodiraScanCAS &CAS)
      : Key(Key), Output(Output), InputIndex(InputIndex), Result(Result),
        DB(CAS) {}

  toolchain::cas::ObjectRef Key;
  toolchain::cas::ObjectRef Output;
  unsigned InputIndex;
  std::variant<language::cas::CompileJobCacheResult,
               clang::cas::CompileJobCacheResult>
      Result;
  CodiraScanCAS &DB;
};

struct CodiraCachedOutputHandle {
  toolchain::cas::ObjectRef Ref;
  language::file_types::ID Kind;
  toolchain::cas::ObjectStore &CAS;
  std::optional<toolchain::cas::ObjectProxy> LoadedProxy;

  CodiraCachedOutputHandle(toolchain::cas::ObjectRef Ref, language::file_types::ID Kind,
                          toolchain::cas::ObjectStore &CAS)
      : Ref(Ref), Kind(Kind), CAS(CAS) {}
};

struct CodiraScanReplayInstance {
  language::CompilerInvocation Invocation;
  toolchain::BumpPtrAllocator StringAlloc;
  toolchain::SmallVector<const char *> Args;
};

struct CodiraCachedReplayResult {
  toolchain::SmallVector<char> outMsg;
  toolchain::SmallVector<char> errMsg;
  toolchain::raw_svector_ostream outOS;
  toolchain::raw_svector_ostream errOS;

  CodiraCachedReplayResult() : outOS(outMsg), errOS(errMsg) {}
};
} // namespace

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(clang::CASOptions, languagescan_cas_options_t)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CodiraScanCAS, languagescan_cas_t)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CodiraCachedCompilationHandle,
                                   languagescan_cached_compilation_t)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CodiraCachedOutputHandle,
                                   languagescan_cached_output_t)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CodiraScanReplayInstance,
                                   languagescan_cache_replay_instance_t)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CodiraCachedReplayResult,
                                   languagescan_cache_replay_result_t)

//=== CAS Functions ----------------------------------------------------------//

languagescan_cas_options_t languagescan_cas_options_create() {
  clang::CASOptions *CASOpts = new clang::CASOptions();
  return wrap(CASOpts);
}

void languagescan_cas_options_dispose(languagescan_cas_options_t options) {
  delete unwrap(options);
}

void languagescan_cas_options_set_ondisk_path(languagescan_cas_options_t options,
                                           const char *path) {
  unwrap(options)->CASPath = path;
}

void languagescan_cas_options_set_plugin_path(languagescan_cas_options_t options,
                                           const char *path) {
  unwrap(options)->PluginPath = path;
}

bool languagescan_cas_options_set_plugin_option(languagescan_cas_options_t options,
                                             const char *name,
                                             const char *value,
                                             languagescan_string_ref_t *error) {
  unwrap(options)->PluginOptions.emplace_back(name, value);
  return false;
}

languagescan_cas_t
languagescan_cas_create_from_options(languagescan_cas_options_t options,
                                  languagescan_string_ref_t *error) {
  clang::CASOptions *opts = unwrap(options);
  auto cas = CodiraScanCAS::createCodiraScanCAS(*opts);
  if (!cas) {
    *error =
        language::c_string_utils::create_clone(toString(cas.takeError()).c_str());
    return nullptr;
  }
  return wrap(*cas);
}

void languagescan_cas_dispose(languagescan_cas_t cas) { delete unwrap(cas); }

languagescan_string_ref_t languagescan_cas_store(languagescan_cas_t cas, uint8_t *data,
                                           unsigned size,
                                           languagescan_string_ref_t *error) {
  auto failure = [&](toolchain::Error E) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return language::c_string_utils::create_null();
  };

  auto &CAS = unwrap(cas)->getCAS();
  toolchain::StringRef StrContent((char *)data, size);
  auto Result = CAS.storeFromString({}, StrContent);
  if (!Result)
    return failure(Result.takeError());

  *error = language::c_string_utils::create_null();
  return language::c_string_utils::create_clone(
      CAS.getID(*Result).toString().c_str());
}

int64_t languagescan_cas_get_ondisk_size(languagescan_cas_t cas,
                                      languagescan_string_ref_t *error) {
  auto &CAS = unwrap(cas)->getCAS();
  std::optional<uint64_t> Size;
  if (auto E = CAS.getStorageSize().moveInto(Size)) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return -2;
  }

  *error = language::c_string_utils::create_null();
  return Size ? *Size : -1;
}

bool
languagescan_cas_set_ondisk_size_limit(languagescan_cas_t cas, int64_t size_limit,
    languagescan_string_ref_t *error) {
  if (size_limit < 0) {
    *error = language::c_string_utils::create_clone(
        "invalid size limit passing to languagescan_cas_set_ondisk_size_limit");
    return true;
  }
  auto &CAS = unwrap(cas)->getCAS();
  std::optional<uint64_t> SizeLimit;
  if (size_limit > 0)
    SizeLimit = size_limit;
  if (auto E = CAS.setSizeLimit(SizeLimit)) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return true;
  }
  *error = language::c_string_utils::create_null();
  return false;
}

bool languagescan_cas_prune_ondisk_data(languagescan_cas_t cas,
                                     languagescan_string_ref_t *error) {
  auto &CAS = unwrap(cas)->getCAS();
  if (auto E = CAS.pruneStorageData()) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return true;
  }
  *error = language::c_string_utils::create_null();
  return false;
}

/// Expand the invocation if there is responseFile into Args that are passed in
/// the parameter. Return language-frontend arguments in an ArrayRef, which has the
/// first "-frontend" option dropped if needed.
static toolchain::ArrayRef<const char *>
expandCodiraInvocation(int argc, const char **argv, toolchain::StringSaver &Saver,
                      toolchain::SmallVectorImpl<const char *> &ArgsStorage) {
  ArgsStorage.reserve(argc);
  for (int i = 0; i < argc; ++i)
    ArgsStorage.push_back(Saver.save(argv[i]).data());
  language::driver::ExpandResponseFilesWithRetry(Saver, ArgsStorage);

  // Drop the `-frontend` option if it is passed.
  toolchain::ArrayRef<const char*> FrontendArgs(ArgsStorage);
  if (!FrontendArgs.empty() &&
      toolchain::StringRef(FrontendArgs.front()) == "-frontend")
    FrontendArgs = FrontendArgs.drop_front();
  return FrontendArgs;
}

static toolchain::Expected<std::string>
computeCacheKey(toolchain::cas::ObjectStore &CAS, toolchain::ArrayRef<const char *> Args,
                toolchain::StringRef InputPath) {
  auto BaseKey = language::createCompileJobBaseCacheKey(CAS, Args);
  if (!BaseKey)
    return BaseKey.takeError();

  // Parse the arguments to figure out the index for the input.
  language::CompilerInvocation Invocation;
  language::SourceManager SourceMgr;
  language::DiagnosticEngine Diags(SourceMgr);

  if (Invocation.parseArgs(Args, Diags, nullptr, {}))
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "Argument parsing failed");

  auto computeKey = [&](unsigned Index) -> toolchain::Expected<std::string> {
    auto Key = language::createCompileJobCacheKeyForOutput(CAS, *BaseKey, Index);
    if (!Key)
      return Key.takeError();
    return CAS.getID(*Key).toString();
  };
  auto AllInputs =
      Invocation.getFrontendOptions().InputsAndOutputs.getAllInputs();
  // First pass, check for path equal.
  for (unsigned Idx = 0; Idx < AllInputs.size(); ++Idx) {
    if (AllInputs[Idx].getFileName() == InputPath)
      return computeKey(Idx);
  }

  // If not found, slow second iteration with real_path.
  toolchain::SmallString<256> InputRealPath;
  toolchain::sys::fs::real_path(InputPath, InputRealPath, true);
  for (unsigned Idx = 0; Idx < AllInputs.size(); ++Idx) {
    toolchain::SmallString<256> TestRealPath;
    toolchain::sys::fs::real_path(AllInputs[Idx].getFileName(), TestRealPath, true);
    if (InputRealPath == TestRealPath)
      return computeKey(Idx);
  }

  return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                 "requested input not found from invocation");
}

static toolchain::Expected<std::string>
computeCacheKeyFromIndex(toolchain::cas::ObjectStore &CAS,
                         toolchain::ArrayRef<const char *> Args,
                         unsigned InputIndex) {
  auto BaseKey = language::createCompileJobBaseCacheKey(CAS, Args);
  if (!BaseKey)
    return BaseKey.takeError();

  auto Key =
      language::createCompileJobCacheKeyForOutput(CAS, *BaseKey, InputIndex);
  if (!Key)
    return Key.takeError();
  return CAS.getID(*Key).toString();
}

languagescan_string_ref_t
languagescan_cache_compute_key(languagescan_cas_t cas, int argc, const char **argv,
                            const char *input, languagescan_string_ref_t *error) {
  toolchain::SmallVector<const char *> ArgsStorage;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  auto Args = expandCodiraInvocation(argc, argv, Saver, ArgsStorage);

  auto ID = computeCacheKey(unwrap(cas)->getCAS(), Args, input);
  if (!ID) {
    *error =
        language::c_string_utils::create_clone(toString(ID.takeError()).c_str());
    return language::c_string_utils::create_null();
  }
  *error = language::c_string_utils::create_null();
  return language::c_string_utils::create_clone(ID->c_str());
}

languagescan_string_ref_t
languagescan_cache_compute_key_from_input_index(languagescan_cas_t cas, int argc,
                                             const char **argv,
                                             unsigned input_index,
                                             languagescan_string_ref_t *error) {
  toolchain::SmallVector<const char *> ArgsStorage;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  auto Args = expandCodiraInvocation(argc, argv, Saver, ArgsStorage);

  auto ID =
      computeCacheKeyFromIndex(unwrap(cas)->getCAS(), Args, input_index);
  if (!ID) {
    *error =
        language::c_string_utils::create_clone(toString(ID.takeError()).c_str());
    return language::c_string_utils::create_null();
  }
  *error = language::c_string_utils::create_null();
  return language::c_string_utils::create_clone(ID->c_str());
}

// Create a non-owning string ref that is used in call backs.
static languagescan_string_ref_t createNonOwningString(toolchain::StringRef Str) {
  if (Str.empty())
    return language::c_string_utils::create_null();

  languagescan_string_ref_t Result;
  Result.data = Str.data();
  Result.length = Str.size();
  return Result;
}

static toolchain::Error createUnsupportedSchemaError() {
  return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                 "unsupported compile result schema found");
}

static toolchain::Expected<CodiraCachedCompilationHandle *>
createCachedCompilation(CodiraScanCAS &CAS, const toolchain::cas::CASID &ID,
                        const toolchain::cas::CASID &Key) {
  auto Ref = CAS.getCAS().getReference(ID);
  if (!Ref)
    return nullptr;

  auto Proxy = CAS.getCAS().getProxy(*Ref);
  if (!Proxy)
    return Proxy.takeError();

  // Load input file name from the key CAS object. Input file name is the data
  // blob in the root node.
  auto KeyProxy = CAS.getCAS().getProxy(Key);
  if (!KeyProxy)
    return KeyProxy.takeError();
  auto Input = KeyProxy->getData();

  unsigned Index =
      toolchain::support::endian::read<uint32_t, toolchain::endianness::little,
                                  toolchain::support::unaligned>(Input.data());
  {
    language::cas::CompileJobResultSchema Schema(CAS.getCAS());
    if (Schema.isRootNode(*Proxy)) {
      auto Result = Schema.load(Proxy->getRef());
      if (!Result)
        return Result.takeError();
      return new CodiraCachedCompilationHandle(KeyProxy->getRef(), *Ref,
                                              std::move(*Result), Index, CAS);
    }
  }
  {
    clang::cas::CompileJobResultSchema Schema(CAS.getCAS());
    if (Schema.isRootNode(*Proxy)) {
      auto Result = Schema.load(Proxy->getRef());
      if (!Result)
        return Result.takeError();
      return new CodiraCachedCompilationHandle(KeyProxy->getRef(), *Ref,
                                              std::move(*Result), Index, CAS);
    }
  }
  return createUnsupportedSchemaError();
}

languagescan_cached_compilation_t
languagescan_cache_query(languagescan_cas_t cas, const char *key, bool globally,
                      languagescan_string_ref_t *error) {
  auto failure = [&](toolchain::Error E) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return nullptr;
  };
  auto notfound = [&]() {
    *error = language::c_string_utils::create_null();
    return nullptr;
  };

  auto &CAS = unwrap(cas)->getCAS();
  auto &Cache = unwrap(cas)->getCache();
  auto ID = CAS.parseID(key);
  if (!ID)
    return failure(ID.takeError());

  auto Result = Cache.get(*ID, globally);
  if (!Result)
    return failure(Result.takeError());
  if (!*Result)
    return notfound();

  auto Cached = createCachedCompilation(*unwrap(cas), **Result, *ID);
  if (!Cached)
    return failure(Cached.takeError());

  if (!*Cached)
    return notfound();

  *error = language::c_string_utils::create_null();
  return wrap(*Cached);
}

void languagescan_cache_query_async(
    languagescan_cas_t cas, const char *key, bool globally, void *ctx,
    void (*callback)(void *ctx, languagescan_cached_compilation_t,
                     languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *token) {
  if (token)
    *token = nullptr;
  auto &CAS = unwrap(cas)->getCAS();
  auto &Cache = unwrap(cas)->getCache();
  auto failure = [ctx, callback](toolchain::Error E) {
    std::string ErrMsg = toString(std::move(E));
    return callback(ctx, nullptr, createNonOwningString(ErrMsg));
  };
  auto notfound = [ctx, callback]() {
    return callback(ctx, nullptr, language::c_string_utils::create_null());
  };
  auto success = [ctx, callback](languagescan_cached_compilation_t comp) {
    return callback(ctx, comp, language::c_string_utils::create_null());
  };

  auto ID = CAS.parseID(key);
  if (!ID)
    return failure(ID.takeError());

  auto KeyID = *ID;
  Cache.getAsync(*ID, globally,
                 [failure, notfound, success, cas, KeyID](
                     toolchain::Expected<std::optional<toolchain::cas::CASID>> Result) {
                   if (!Result)
                     return failure(Result.takeError());
                   if (!*Result)
                     return notfound();

                   auto Cached =
                       createCachedCompilation(*unwrap(cas), **Result, KeyID);
                   if (!Cached)
                     return failure(Cached.takeError());

                   if (!*Cached)
                     return notfound();

                   return success(wrap(*Cached));
                 });
}

unsigned languagescan_cached_compilation_get_num_outputs(
    languagescan_cached_compilation_t id) {
  auto *Comp = unwrap(id);
  if (auto *Result =
          std::get_if<language::cas::CompileJobCacheResult>(&Comp->Result))
    return Result->getNumOutputs();

  auto *Result = std::get_if<clang::cas::CompileJobCacheResult>(&Comp->Result);
  assert(Result && "unexpected variant");
  return Result->getNumOutputs();
}

languagescan_cached_output_t
languagescan_cached_compilation_get_output(languagescan_cached_compilation_t id,
                                        unsigned idx) {
  auto *Comp = unwrap(id);
  if (auto *Result =
      std::get_if<language::cas::CompileJobCacheResult>(&Comp->Result))
    return wrap(new CodiraCachedOutputHandle(Result->getOutput(idx).Object,
                                            Result->getOutput(idx).Kind,
                                            Comp->DB.getCAS()));

  auto *Result = std::get_if<clang::cas::CompileJobCacheResult>(&Comp->Result);
  assert(Result && "unexpected variant");
  language::file_types::ID Kind = language::file_types::TY_INVALID;
  switch (Result->getOutput(idx).Kind) {
  case clang::cas::CompileJobCacheResult::OutputKind::MainOutput:
    Kind = language::file_types::TY_ClangModuleFile;
    break;
  case clang::cas::CompileJobCacheResult::OutputKind::Dependencies:
    Kind = language::file_types::TY_Dependencies;
    break;
  case clang::cas::CompileJobCacheResult::OutputKind::SerializedDiagnostics:
    Kind = language::file_types::TY_CachedDiagnostics;
    break;
  }
  assert(Kind != language::file_types::TY_INVALID && "Invalid kind");
  return wrap(new CodiraCachedOutputHandle(Result->getOutput(idx).Object, Kind,
                                          Comp->DB.getCAS()));
}

bool languagescan_cached_compilation_is_uncacheable(
    languagescan_cached_compilation_t id) {
  // Currently, all compilations are cacheable.
  return false;
}

void languagescan_cached_compilation_dispose(languagescan_cached_compilation_t id) {
  delete unwrap(id);
}

bool languagescan_cached_output_load(languagescan_cached_output_t output,
                                  languagescan_string_ref_t *error) {
  auto *Out = unwrap(output);
  auto failure = [&](toolchain::Error E) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(E)).c_str());
    return false;
  };
  auto notfound = [&]() {
    *error = language::c_string_utils::create_null();
    return false;
  };
  auto success = [&]() {
    *error = language::c_string_utils::create_null();
    return true;
  };
  // If proxy exists, there is nothing to be done.
  if (Out->LoadedProxy)
    return success();

  if (auto Err = Out->CAS.getProxyIfExists(Out->Ref).moveInto(Out->LoadedProxy))
    return failure(std::move(Err));

  if (!Out->LoadedProxy)
    return notfound();

  toolchain::DenseSet<toolchain::cas::ObjectRef> Visited;
  toolchain::SmallVector<toolchain::cas::ObjectRef> WorkList;
  auto addToWorkList = [&](toolchain::cas::ObjectRef Item) {
    if (Visited.insert(Item).second)
      WorkList.push_back(Item);
  };

  addToWorkList(Out->LoadedProxy->getRef());
  while (!WorkList.empty()) {
    auto Current = WorkList.pop_back_val();
    auto Proxy = Out->CAS.getProxyIfExists(Current);
    if (!Proxy)
      return failure(Proxy.takeError());

    if (!*Proxy)
      return notfound();

    if (auto Err = (*Proxy)->forEachReference(
            [&](toolchain::cas::ObjectRef Ref) -> toolchain::Error {
              addToWorkList(Ref);
              return toolchain::Error::success();
            }))
      return failure(std::move(Err));
  }
  return success();
}

namespace {
/// Asynchronously visits the graph of the object node to ensure it's fully
/// materialized.
class AsyncObjectLoader
    : public std::enable_shared_from_this<AsyncObjectLoader> {
  void *Ctx;
  void (*Callback)(void *Ctx, bool, languagescan_string_ref_t);
  toolchain::cas::ObjectStore &CAS;

  // The output handle to update after load if set.
  CodiraCachedOutputHandle *OutputHandle;

  toolchain::SmallDenseSet<toolchain::cas::ObjectRef> ObjectsSeen;
  unsigned NumPending = 0;
  std::optional<toolchain::cas::ObjectProxy> RootObj;
  std::atomic<bool> MissingNode{false};
  /// The first error that occurred.
  std::optional<toolchain::Error> ErrOccurred;
  std::mutex Mutex;

public:
  AsyncObjectLoader(void *Ctx,
                    void (*Callback)(void *Ctx, bool, languagescan_string_ref_t),
                    toolchain::cas::ObjectStore &CAS,
                    CodiraCachedOutputHandle *Handle = nullptr)
      : Ctx(Ctx), Callback(Callback), CAS(CAS), OutputHandle(Handle) {}

  void visit(toolchain::cas::ObjectRef Ref, bool IsRootNode) {
    {
      std::lock_guard<std::mutex> Guard(Mutex);
      if (!ObjectsSeen.insert(Ref).second)
        return;
      ++NumPending;
    }
    auto This = shared_from_this();
    CAS.getProxyAsync(
        Ref, [This, IsRootNode](
                 toolchain::Expected<std::optional<toolchain::cas::ObjectProxy>> Obj) {
          LANGUAGE_DEFER { This->finishedNode(); };
          if (!Obj) {
            This->encounteredError(Obj.takeError());
            return;
          }
          if (!*Obj) {
            This->MissingNode = true;
            return;
          }
          if (IsRootNode)
            This->RootObj = *Obj;
          if (auto Err = (*Obj)->forEachReference(
                  [&This](toolchain::cas::ObjectRef Sub) -> toolchain::Error {
                    This->visit(Sub, /*IsRootNode*/ false);
                    return toolchain::Error::success();
                  })) {
            This->encounteredError(std::move(Err));
            return;
          }
        });
  }

  void finishedNode() {
    {
      std::lock_guard<std::mutex> Guard(Mutex);
      assert(NumPending);
      --NumPending;
      if (NumPending != 0)
        return;
    }
    if (ErrOccurred) {
      std::string ErrMsg = toString(std::move(*ErrOccurred));
      return Callback(Ctx, false, createNonOwningString(ErrMsg));
    }
    if (MissingNode)
      return Callback(Ctx, false, language::c_string_utils::create_null());

    if (OutputHandle)
      OutputHandle->LoadedProxy = RootObj;
    return Callback(Ctx, true, language::c_string_utils::create_null());
  }

  /// Only keeps the first error that occurred.
  void encounteredError(toolchain::Error &&E) {
    std::lock_guard<std::mutex> Guard(Mutex);
    if (ErrOccurred) {
      toolchain::consumeError(std::move(E));
      return;
    }
    ErrOccurred = std::move(E);
  }
};
} // namespace

void languagescan_cached_output_load_async(
    languagescan_cached_output_t output, void *ctx,
    void (*callback)(void *ctx, bool success, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *token) {
  if (token)
    *token = nullptr;

  auto *Out = unwrap(output);
  if (Out->LoadedProxy)
    return callback(ctx, true, language::c_string_utils::create_null());

  auto Loader =
      std::make_shared<AsyncObjectLoader>(ctx, callback, Out->CAS, Out);
  Loader->visit(Out->Ref, /*IsRootNode*/ true);
}

void languagescan_cache_download_cas_object_async(
    languagescan_cas_t cas, const char *id, void *ctx,
    void (*callback)(void *ctx, bool success, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *token) {
  if (token)
    *token = nullptr;

  auto failure = [ctx, callback](toolchain::Error E) {
    std::string ErrMsg = toString(std::move(E));
    return callback(ctx, false, createNonOwningString(ErrMsg));
  };

  auto &CAS = unwrap(cas)->getCAS();
  auto ID = CAS.parseID(id);
  if (!ID)
    return failure(ID.takeError());

  auto Ref = CAS.getReference(*ID);
  if (!Ref)
    return callback(ctx, false, language::c_string_utils::create_null());

  auto Loader =
      std::make_shared<AsyncObjectLoader>(ctx, callback, CAS);
  Loader->visit(*Ref, /*IsRootNode*/ true);
}

bool languagescan_cached_output_is_materialized(languagescan_cached_output_t output) {
  auto *Out = unwrap(output);
  // Already loaded.
  if (Out->LoadedProxy)
    return true;

  auto Loaded = Out->CAS.isMaterialized(Out->Ref);
  if (!Loaded) {
    // There is an error loading, output is not materialized. Discard error and
    // return false.
    consumeError(Loaded.takeError());
    return false;
  }
  return *Loaded;
}

languagescan_string_ref_t
languagescan_cached_output_get_casid(languagescan_cached_output_t output) {
  auto *Out = unwrap(output);
  auto ID = Out->CAS.getID(Out->Ref);
  return language::c_string_utils::create_clone(ID.toString().c_str());
}

void languagescan_cached_compilation_make_global_async(
    languagescan_cached_compilation_t comp, void *ctx,
    void (*callback)(void *ctx, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *token) {
  if (token)
    *token = nullptr;
  auto failure = [ctx, callback](toolchain::Error E) {
    std::string ErrMsg = toString(std::move(E));
    return callback(ctx, createNonOwningString(ErrMsg));
  };
  auto success = [ctx, callback]() {
    return callback(ctx, language::c_string_utils::create_null());
  };

  auto *Compilation = unwrap(comp);
  auto &CAS = Compilation->DB.getCAS();
  auto &Cache = Compilation->DB.getCache();

  auto ID = CAS.getID(Compilation->Key);
  auto Result = CAS.getID(Compilation->Output);

  Cache.putAsync(ID, Result, /*globally=*/true,
                 [failure, success](toolchain::Error E) {
                   if (E)
                     return failure(std::move(E));

                   return success();
                 });
}

languagescan_string_ref_t
languagescan_cached_output_get_name(languagescan_cached_output_t output) {
  auto *Out = unwrap(output);
  return language::c_string_utils::create_clone(
      language::file_types::getTypeName(Out->Kind).str().c_str());
}

languagescan_cache_replay_instance_t
languagescan_cache_replay_instance_create(int argc, const char **argv,
                                       languagescan_string_ref_t *error) {
  auto *Instance = new CodiraScanReplayInstance();
  toolchain::SmallVector<const char *> Compilation;
  toolchain::StringSaver Saver(Instance->StringAlloc);
  auto Args = expandCodiraInvocation(argc, argv, Saver, Instance->Args);

  // Capture the diagnostics when creating invocation.
  std::string err_msg;
  toolchain::raw_string_ostream OS(err_msg);
  language::PrintingDiagnosticConsumer Diags(OS);
  language::SourceManager SrcMgr;
  language::DiagnosticEngine DE(SrcMgr);
  DE.addConsumer(Diags);

  if (Instance->Invocation.parseArgs(Args, DE, nullptr, {})) {
    delete Instance;
    *error = language::c_string_utils::create_clone(err_msg.c_str());
    return nullptr;
  }

  if (!Instance->Invocation.getCASOptions().EnableCaching) {
    delete Instance;
    *error = language::c_string_utils::create_clone(
        "caching is not enabled from command-line");
    return nullptr;
  }

  // Clear the LLVMArgs as `toolchain::cl::ParseCommandLineOptions` is not
  // thread-safe to be called in libCodiraScan. The replay instance should not be
  // used to do compilation so clearing `-Xtoolchain` should not affect replay
  // result.
  Instance->Invocation.getFrontendOptions().LLVMArgs.clear();

  return wrap(Instance);
}

void languagescan_cache_replay_instance_dispose(
    languagescan_cache_replay_instance_t instance) {
  delete unwrap(instance);
}

namespace {
class StreamOutputFileImpl final
    : public toolchain::RTTIExtends<StreamOutputFileImpl,
                               toolchain::vfs::OutputFileImpl> {

public:
  StreamOutputFileImpl(toolchain::raw_pwrite_stream &OS) : OS(OS) {}
  toolchain::Error keep() final { return toolchain::Error::success(); }
  toolchain::Error discard() final { return toolchain::Error::success(); }
  toolchain::raw_pwrite_stream &getOS() final { return OS; }

private:
  toolchain::raw_pwrite_stream &OS;
};

// The replay output backend. Currently, it redirects "-" to the
// stdout provided.
struct ReplayOutputBackend : public toolchain::vfs::ProxyOutputBackend {
  toolchain::Expected<std::unique_ptr<toolchain::vfs::OutputFileImpl>>
  createFileImpl(toolchain::StringRef Path,
                 std::optional<toolchain::vfs::OutputConfig> Config) override {
    if (Path == "-")
      return std::make_unique<StreamOutputFileImpl>(StdOut);
    return toolchain::vfs::ProxyOutputBackend::createFileImpl(Path, Config);
  }

  toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend>
  cloneImpl() const override {
    return toolchain::makeIntrusiveRefCnt<ReplayOutputBackend>(
        getUnderlyingBackend().clone(), StdOut);
  }

  ReplayOutputBackend(
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> UnderlyingBackend,
      toolchain::raw_pwrite_stream &StdOut)
      : ProxyOutputBackend(UnderlyingBackend), StdOut(StdOut) {}

  toolchain::raw_pwrite_stream &StdOut;
};
} // namespace

static toolchain::Error replayCompilation(CodiraScanReplayInstance &Instance,
                                     toolchain::cas::ObjectStore &CAS,
                                     CodiraCachedCompilationHandle &Comp,
                                     toolchain::raw_pwrite_stream &Out,
                                     toolchain::raw_pwrite_stream &Err) {
  using namespace language;
  using namespace toolchain;
  CompilerInstance Inst;
  CompilerInvocation &Invocation = Instance.Invocation;

  // Find the input file from the invocation.
  auto &InputsAndOutputs =
      Instance.Invocation.getFrontendOptions().InputsAndOutputs;
  auto AllInputs = InputsAndOutputs.getAllInputs();
  if (Comp.InputIndex >= AllInputs.size())
    return createStringError(inconvertibleErrorCode(),
                             "InputFile index too large for compilation");
  const auto &Input = AllInputs[Comp.InputIndex];

  // Setup DiagnosticsConsumers.
  DiagnosticHelper DH = DiagnosticHelper::create(
      Inst, Invocation, Instance.Args, Err, /*QuasiPID=*/true);

  std::string InstanceSetupError;
  if (Inst.setupForReplay(Instance.Invocation, InstanceSetupError,
                          Instance.Args))
    return createStringError(inconvertibleErrorCode(), InstanceSetupError);

  auto *CDP = Inst.getCachingDiagnosticsProcessor();
  assert(CDP && "CachingDiagnosticsProcessor needs to be setup for replay");
  // No diags are captured in replay instance.
  CDP->endDiagnosticCapture();
  // Replay settings.
  bool Remarks = Instance.Invocation.getCASOptions().EnableCachingRemarks;
  bool UseCASBackend = Invocation.getIRGenOptions().UseCASBackend;

  // OutputBackend for replay.
  ReplayOutputBackend Backend(
      makeIntrusiveRefCnt<toolchain::vfs::OnDiskOutputBackend>(), Out);

  if (!replayCachedCompilerOutputsForInput(
          CAS, Comp.Output, Input, Comp.InputIndex, Inst.getDiags(), DH,
          Backend, Instance.Invocation.getFrontendOptions(), *CDP, Remarks,
          UseCASBackend)) {
    Inst.getDiags().diagnose(SourceLoc(), diag::cache_replay_failed,
                             "failed to load all outputs");
  }

  Inst.getDiags().finishProcessing();
  return toolchain::Error::success();
}

languagescan_cache_replay_result_t
languagescan_cache_replay_compilation(languagescan_cache_replay_instance_t instance,
                                   languagescan_cached_compilation_t comp,
                                   languagescan_string_ref_t *error) {
  CodiraScanReplayInstance &Instance = *unwrap(instance);
  CodiraCachedCompilationHandle &Comp = *unwrap(comp);
  CodiraCachedReplayResult *Result = new CodiraCachedReplayResult();

  if (auto err = replayCompilation(Instance, Comp.DB.getCAS(), Comp,
                                   Result->outOS, Result->errOS)) {
    *error =
        language::c_string_utils::create_clone(toString(std::move(err)).c_str());
    delete Result;
    return nullptr;
  }
  *error = language::c_string_utils::create_null();
  return wrap(Result);
}

languagescan_string_ref_t languagescan_cache_replay_result_get_stdout(
    languagescan_cache_replay_result_t result) {
  return createNonOwningString(unwrap(result)->outOS.str());
}

languagescan_string_ref_t languagescan_cache_replay_result_get_stderr(
    languagescan_cache_replay_result_t result) {
  return createNonOwningString(unwrap(result)->errOS.str());
}

void languagescan_cached_output_dispose(languagescan_cached_output_t out) {
  delete unwrap(out);
}

void languagescan_cache_replay_result_dispose(
    languagescan_cache_replay_result_t result) {
  delete unwrap(result);
}

// FIXME: implement cancellation.
void languagescan_cache_action_cancel(languagescan_cache_cancellation_token_t) {}
void languagescan_cache_cancellation_token_dispose(
    languagescan_cache_cancellation_token_t) {}

toolchain::Expected<CodiraScanCAS *>
CodiraScanCAS::createCodiraScanCAS(toolchain::StringRef Path) {
  clang::CASOptions Opts;
  Opts.CASPath = Path;

  return createCodiraScanCAS(Opts);
}

toolchain::Expected<CodiraScanCAS *>
CodiraScanCAS::createCodiraScanCAS(clang::CASOptions &CASOpts) {
  auto DB = CASOpts.getOrCreateDatabases();
  if (!DB)
    return DB.takeError();

  return new CodiraScanCAS(std::move(DB->first), std::move(DB->second));
}
