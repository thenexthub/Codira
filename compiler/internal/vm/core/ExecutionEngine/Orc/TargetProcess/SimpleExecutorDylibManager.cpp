//===--- SimpleExecutorDylibManager.cpp - Executor-side dylib management --===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ExecutionEngine/Orc/TargetProcess/SimpleExecutorDylibManager.h"

#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"

#include "vm/core/Support/MSVCErrorWorkarounds.h"

#include <future>

#define DEBUG_TYPE "orc"

namespace vm::core {
namespace orc {
namespace rt_bootstrap {

SimpleExecutorDylibManager::~SimpleExecutorDylibManager() {
  assert(Dylibs.empty() && "shutdown not called?");
}

Expected<tpctypes::DylibHandle>
SimpleExecutorDylibManager::open(const std::string &Path, uint64_t Mode) {
  if (Mode != 0)
    return make_error<StringError>("open: non-zero mode bits not yet supported",
                                   inconvertibleErrorCode());

  const char *PathCStr = Path.empty() ? nullptr : Path.c_str();
  std::string ErrMsg;

  auto DL = sys::DynamicLibrary::getPermanentLibrary(PathCStr, &ErrMsg);
  if (!DL.isValid())
    return make_error<StringError>(std::move(ErrMsg), inconvertibleErrorCode());

  std::lock_guard<std::mutex> Lock(M);
  auto H = ExecutorAddr::fromPtr(DL.getOSSpecificHandle());
  Resolvers.push_back(std::make_unique<DylibSymbolResolver>(H));
  Dylibs.insert(DL.getOSSpecificHandle());
  return ExecutorAddr::fromPtr(Resolvers.back().get());
}

Error SimpleExecutorDylibManager::shutdown() {

  DylibSet DS;
  {
    std::lock_guard<std::mutex> Lock(M);
    std::swap(DS, Dylibs);
  }

  // There is no removal of dylibs at the moment, so nothing to do here.
  return Error::success();
}

void SimpleExecutorDylibManager::addBootstrapSymbols(
    StringMap<ExecutorAddr> &M) {
  M[rt::SimpleExecutorDylibManagerInstanceName] = ExecutorAddr::fromPtr(this);
  M[rt::SimpleExecutorDylibManagerOpenWrapperName] =
      ExecutorAddr::fromPtr(&openWrapper);
  M[rt::SimpleExecutorDylibManagerResolveWrapperName] =
      ExecutorAddr::fromPtr(&resolveWrapper);
}

toolchain::orc::shared::CWrapperFunctionBuffer
SimpleExecutorDylibManager::openWrapper(const char *ArgData, size_t ArgSize) {
  return shared::
      WrapperFunction<rt::SPSSimpleExecutorDylibManagerOpenSignature>::handle(
             ArgData, ArgSize,
             shared::makeMethodWrapperHandler(
                 &SimpleExecutorDylibManager::open))
          .release();
}

toolchain::orc::shared::CWrapperFunctionBuffer
SimpleExecutorDylibManager::resolveWrapper(const char *ArgData,
                                           size_t ArgSize) {
  using ResolveResult = ExecutorResolver::ResolveResult;
  return shared::WrapperFunction<
             rt::SPSSimpleExecutorDylibManagerResolveSignature>::
      handle(ArgData, ArgSize,
             [](ExecutorAddr Obj, RemoteSymbolLookupSet L) -> ResolveResult {
               using TmpResult =
                   MSVCPExpected<std::vector<std::optional<ExecutorSymbolDef>>>;
               std::promise<TmpResult> P;
               auto F = P.get_future();
               Obj.toPtr<ExecutorResolver *>()->resolveAsync(
                   std::move(L),
                   [&](TmpResult R) { P.set_value(std::move(R)); });
               return F.get();
             })
          .release();
}

} // namespace rt_bootstrap
} // end namespace orc
} // end namespace vm::core
