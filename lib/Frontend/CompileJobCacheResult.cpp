//===--- CompileJobCacheResult.cpp - compile cache result schema ----------===//
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
// This file contains the cache schema for language cached compile job result.
//
//===----------------------------------------------------------------------===//

#include "language/Frontend/CompileJobCacheResult.h"
#include "language/Basic/FileTypes.h"

using namespace language;
using namespace language::cas;
using namespace toolchain;
using namespace toolchain::cas;

Error CompileJobCacheResult::forEachOutput(
    toolchain::function_ref<Error(Output)> Callback) const {
  size_t Count = getNumOutputs();
  for (size_t I = 0; I < Count; ++I) {
    file_types::ID Kind = getOutputKind(I);
    ObjectRef Object = getOutputObject(I);
    if (auto Err = Callback({Object, Kind}))
      return Err;
  }
  return Error::success();
}

Error CompileJobCacheResult::forEachLoadedOutput(
    toolchain::function_ref<Error(Output, std::optional<ObjectProxy>)> Callback) {
  // Kick-off materialization for all outputs concurrently.
  SmallVector<std::future<AsyncProxyValue>, 4> FutureOutputs;
  size_t Count = getNumOutputs();
  for (size_t I = 0; I < Count; ++I) {
    ObjectRef Ref = getOutputObject(I);
    FutureOutputs.push_back(getCAS().getProxyFuture(Ref));
  }

  // Make sure all the outputs have materialized.
  std::optional<Error> OccurredError;
  SmallVector<std::optional<ObjectProxy>, 4> Outputs;
  for (auto &FutureOutput : FutureOutputs) {
    auto Obj = FutureOutput.get().take();
    if (!Obj) {
      if (!OccurredError)
        OccurredError = Obj.takeError();
      else
        OccurredError =
            toolchain::joinErrors(std::move(*OccurredError), Obj.takeError());
      continue;
    }
    Outputs.push_back(*Obj);
  }
  if (OccurredError)
    return std::move(*OccurredError);

  // Pass the loaded outputs.
  for (size_t I = 0; I < Count; ++I) {
    if (auto Err = Callback({getOutputObject(I), getOutputKind(I)}, Outputs[I]))
      return Err;
  }
  return Error::success();
}

CompileJobCacheResult::Output CompileJobCacheResult::getOutput(size_t I) const {
  return Output{getOutputObject(I), getOutputKind(I)};
}

std::optional<CompileJobCacheResult::Output>
CompileJobCacheResult::getOutput(file_types::ID Kind) const {
  size_t Count = getNumOutputs();
  for (size_t I = 0; I < Count; ++I) {
    file_types::ID K = getOutputKind(I);
    if (Kind == K)
      return Output{getOutputObject(I), Kind};
  }
  return {};
}

Error CompileJobCacheResult::print(toolchain::raw_ostream &OS) {
  return forEachOutput([&](Output O) -> Error {
    OS << file_types::getTypeName(O.Kind) << "    " << getCAS().getID(O.Object)
       << '\n';
    return Error::success();
  });
}

size_t CompileJobCacheResult::getNumOutputs() const { return getData().size(); }
ObjectRef CompileJobCacheResult::getOutputObject(size_t I) const {
  return getReference(I);
}

file_types::ID
CompileJobCacheResult::getOutputKind(size_t I) const {
  return static_cast<file_types::ID>(getData()[I]);
}

CompileJobCacheResult::CompileJobCacheResult(const ObjectProxy &Obj)
    : ObjectProxy(Obj) {}

struct CompileJobCacheResult::Builder::PrivateImpl {
  SmallVector<ObjectRef> Objects;
  SmallVector<file_types::ID> Kinds;

  struct KindMap {
    file_types::ID Kind;
    std::string Path;
  };
  SmallVector<KindMap> KindMaps;
};

CompileJobCacheResult::Builder::Builder() : Impl(*new PrivateImpl) {}
CompileJobCacheResult::Builder::~Builder() { delete &Impl; }

void CompileJobCacheResult::Builder::addKindMap(file_types::ID Kind,
                                                StringRef Path) {
  Impl.KindMaps.push_back({Kind, std::string(Path)});
}
void CompileJobCacheResult::Builder::addOutput(file_types::ID Kind,
                                               ObjectRef Object) {
  Impl.Kinds.push_back(Kind);
  Impl.Objects.push_back(Object);
}
Error CompileJobCacheResult::Builder::addOutput(StringRef Path,
                                                ObjectRef Object) {
  Impl.Objects.push_back(Object);
  for (auto &KM : Impl.KindMaps) {
    if (KM.Path == Path) {
      Impl.Kinds.push_back(KM.Kind);
      return Error::success();
    }
  }
  return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                 "cached output file has unknown path '" +
                                     Path + "'");
}

Expected<ObjectRef> CompileJobCacheResult::Builder::build(ObjectStore &CAS) {
  CompileJobResultSchema Schema(CAS);
  // The resulting Refs contents are:
  // Object 0...N, SchemaKind
  SmallVector<ObjectRef> Refs;
  std::swap(Impl.Objects, Refs);
  Refs.push_back(Schema.getKindRef());
  return CAS.store(Refs, {(char *)Impl.Kinds.begin(), Impl.Kinds.size()});
}

static constexpr toolchain::StringLiteral CompileJobResultSchemaName =
    "language::cas::schema::compile_job_result::v1";

char CompileJobResultSchema::ID = 0;

CompileJobResultSchema::CompileJobResultSchema(ObjectStore &CAS)
    : CompileJobResultSchema::RTTIExtends(CAS),
      KindRef(
          toolchain::cantFail(CAS.storeFromString({}, CompileJobResultSchemaName))) {
}

Expected<CompileJobCacheResult>
CompileJobResultSchema::load(ObjectRef Ref) const {
  auto Proxy = CAS.getProxy(Ref);
  if (!Proxy)
    return Proxy.takeError();
  if (!isNode(*Proxy))
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "not a compile job result");
  return CompileJobCacheResult(*Proxy);
}

bool CompileJobResultSchema::isRootNode(const ObjectProxy &Node) const {
  return isNode(Node);
}

bool CompileJobResultSchema::isNode(const ObjectProxy &Node) const {
  size_t N = Node.getNumReferences();
  return N && Node.getReference(N - 1) == getKindRef();
}
