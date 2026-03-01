//===---- SimpleRemoteMemoryMapper.cpp - Remote memory mapper ----*- C++ -*-==//
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

#include "vm/core/ExecutionEngine/Orc/SimpleRemoteMemoryMapper.h"

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"

namespace vm::core::orc {

SimpleRemoteMemoryMapper::SimpleRemoteMemoryMapper(ExecutorProcessControl &EPC,
                                                   SymbolAddrs SAs)
    : EPC(EPC), SAs(SAs) {}

void SimpleRemoteMemoryMapper::reserve(size_t NumBytes,
                                       OnReservedFunction OnReserved) {
  EPC.callSPSWrapperAsync<rt::SPSSimpleRemoteMemoryMapReserveSignature>(
      SAs.Reserve,
      [NumBytes, OnReserved = std::move(OnReserved)](
          Error SerializationErr, Expected<ExecutorAddr> Result) mutable {
        if (SerializationErr) {
          cantFail(Result.takeError());
          return OnReserved(std::move(SerializationErr));
        }

        if (Result)
          OnReserved(ExecutorAddrRange(*Result, NumBytes));
        else
          OnReserved(Result.takeError());
      },
      SAs.Instance, static_cast<uint64_t>(NumBytes));
}

char *SimpleRemoteMemoryMapper::prepare(jitlink::LinkGraph &G,
                                        ExecutorAddr Addr, size_t ContentSize) {
  return G.allocateBuffer(ContentSize).data();
}

void SimpleRemoteMemoryMapper::initialize(MemoryMapper::AllocInfo &AI,
                                          OnInitializedFunction OnInitialized) {

  tpctypes::FinalizeRequest FR;

  std::swap(FR.Actions, AI.Actions);
  FR.Segments.reserve(AI.Segments.size());

  for (auto Seg : AI.Segments)
    FR.Segments.push_back({Seg.AG, AI.MappingBase + Seg.Offset,
                           Seg.ContentSize + Seg.ZeroFillSize,
                           ArrayRef<char>(Seg.WorkingMem, Seg.ContentSize)});

  EPC.callSPSWrapperAsync<rt::SPSSimpleRemoteMemoryMapInitializeSignature>(
      SAs.Initialize,
      [OnInitialized = std::move(OnInitialized)](
          Error SerializationErr, Expected<ExecutorAddr> Result) mutable {
        if (SerializationErr) {
          cantFail(Result.takeError());
          return OnInitialized(std::move(SerializationErr));
        }

        OnInitialized(std::move(Result));
      },
      SAs.Instance, std::move(FR));
}

void SimpleRemoteMemoryMapper::deinitialize(
    ArrayRef<ExecutorAddr> Allocations,
    MemoryMapper::OnDeinitializedFunction OnDeinitialized) {
  EPC.callSPSWrapperAsync<rt::SPSSimpleRemoteMemoryMapDeinitializeSignature>(
      SAs.Deinitialize,
      [OnDeinitialized = std::move(OnDeinitialized)](Error SerializationErr,
                                                     Error Result) mutable {
        if (SerializationErr) {
          cantFail(std::move(Result));
          return OnDeinitialized(std::move(SerializationErr));
        }

        OnDeinitialized(std::move(Result));
      },
      SAs.Instance, Allocations);
}

void SimpleRemoteMemoryMapper::release(ArrayRef<ExecutorAddr> Bases,
                                       OnReleasedFunction OnReleased) {
  EPC.callSPSWrapperAsync<rt::SPSSimpleRemoteMemoryMapReleaseSignature>(
      SAs.Release,
      [OnReleased = std::move(OnReleased)](Error SerializationErr,
                                           Error Result) mutable {
        if (SerializationErr) {
          cantFail(std::move(Result));
          return OnReleased(std::move(SerializationErr));
        }

        return OnReleased(std::move(Result));
      },
      SAs.Instance, Bases);
}

} // namespace vm::core::orc
