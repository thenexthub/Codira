//===------- LookupAndRecordAddrs.h - Symbol lookup support utility -------===//
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

#include "vm/core/ExecutionEngine/Orc/LookupAndRecordAddrs.h"

#include <future>

namespace vm::core {
namespace orc {

void lookupAndRecordAddrs(
    unique_function<void(Error)> OnRecorded, ExecutionSession &ES, LookupKind K,
    const JITDylibSearchOrder &SearchOrder,
    std::vector<std::pair<SymbolStringPtr, ExecutorAddr *>> Pairs,
    SymbolLookupFlags LookupFlags) {

  SymbolLookupSet Symbols;
  for (auto &KV : Pairs)
    Symbols.add(KV.first, LookupFlags);

  ES.lookup(
      K, SearchOrder, std::move(Symbols), SymbolState::Ready,
      [Pairs = std::move(Pairs),
       OnRec = std::move(OnRecorded)](Expected<SymbolMap> Result) mutable {
        if (!Result)
          return OnRec(Result.takeError());
        for (auto &KV : Pairs) {
          auto I = Result->find(KV.first);
          *KV.second =
              I != Result->end() ? I->second.getAddress() : orc::ExecutorAddr();
        }
        OnRec(Error::success());
      },
      NoDependenciesToRegister);
}

Error lookupAndRecordAddrs(
    ExecutionSession &ES, LookupKind K, const JITDylibSearchOrder &SearchOrder,
    std::vector<std::pair<SymbolStringPtr, ExecutorAddr *>> Pairs,
    SymbolLookupFlags LookupFlags) {

  std::promise<MSVCPError> ResultP;
  auto ResultF = ResultP.get_future();
  lookupAndRecordAddrs([&](Error Err) { ResultP.set_value(std::move(Err)); },
                       ES, K, SearchOrder, std::move(Pairs), LookupFlags);
  return ResultF.get();
}

Error lookupAndRecordAddrs(
    ExecutorProcessControl &EPC, tpctypes::DylibHandle H,
    std::vector<std::pair<SymbolStringPtr, ExecutorAddr *>> Pairs,
    SymbolLookupFlags LookupFlags) {

  SymbolLookupSet Symbols;
  for (auto &KV : Pairs)
    Symbols.add(KV.first, LookupFlags);

  DylibManager::LookupRequest LR(H, Symbols);
  auto Result = EPC.getDylibMgr().lookupSymbols(LR);
  if (!Result)
    return Result.takeError();

  if (Result->size() != 1)
    return make_error<StringError>("Error in lookup result",
                                   inconvertibleErrorCode());
  if (Result->front().size() != Pairs.size())
    return make_error<StringError>("Error in lookup result elements",
                                   inconvertibleErrorCode());

  for (unsigned I = 0; I != Pairs.size(); ++I) {
    if (Result->front()[I])
      *Pairs[I].second = Result->front()[I]->getAddress();
  }
  return Error::success();
}

} // End namespace orc.
} // End namespace vm::core.
