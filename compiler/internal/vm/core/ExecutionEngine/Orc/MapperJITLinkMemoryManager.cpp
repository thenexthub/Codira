//=== MapperJITLinkMemoryManager.cpp - Memory management with MemoryMapper ===//
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

#include "vm/core/ExecutionEngine/Orc/MapperJITLinkMemoryManager.h"

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/Support/Process.h"

using namespace vm::core::jitlink;

namespace vm::core {
namespace orc {

class MapperJITLinkMemoryManager::InFlightAlloc
    : public JITLinkMemoryManager::InFlightAlloc {
public:
  InFlightAlloc(MapperJITLinkMemoryManager &Parent, LinkGraph &G,
                ExecutorAddr AllocAddr,
                std::vector<MemoryMapper::AllocInfo::SegInfo> Segs)
      : Parent(Parent), G(G), AllocAddr(AllocAddr), Segs(std::move(Segs)) {}

  void finalize(OnFinalizedFunction OnFinalize) override {
    MemoryMapper::AllocInfo AI;
    AI.MappingBase = AllocAddr;

    std::swap(AI.Segments, Segs);
    std::swap(AI.Actions, G.allocActions());

    Parent.Mapper->initialize(AI, [OnFinalize = std::move(OnFinalize)](
                                      Expected<ExecutorAddr> Result) mutable {
      if (!Result) {
        OnFinalize(Result.takeError());
        return;
      }

      OnFinalize(FinalizedAlloc(*Result));
    });
  }

  void abandon(OnAbandonedFunction OnFinalize) override {
    Parent.Mapper->deinitialize({AllocAddr}, std::move(OnFinalize));
  }

private:
  MapperJITLinkMemoryManager &Parent;
  LinkGraph &G;
  ExecutorAddr AllocAddr;
  std::vector<MemoryMapper::AllocInfo::SegInfo> Segs;
};

MapperJITLinkMemoryManager::MapperJITLinkMemoryManager(
    size_t ReservationGranularity, std::unique_ptr<MemoryMapper> Mapper)
    : ReservationUnits(ReservationGranularity), AvailableMemory(AMAllocator),
      Mapper(std::move(Mapper)) {}

void MapperJITLinkMemoryManager::allocate(const JITLinkDylib *JD, LinkGraph &G,
                                          OnAllocatedFunction OnAllocated) {
  BasicLayout BL(G);

  // find required address space
  auto SegsSizes = BL.getContiguousPageBasedLayoutSizes(Mapper->getPageSize());
  if (!SegsSizes) {
    OnAllocated(SegsSizes.takeError());
    return;
  }

  auto TotalSize = SegsSizes->total();

  auto CompleteAllocation = [this, &G, BL = std::move(BL),
                             OnAllocated = std::move(OnAllocated)](
                                Expected<ExecutorAddrRange> Result) mutable {
    if (!Result) {
      Mutex.unlock();
      return OnAllocated(Result.takeError());
    }

    auto NextSegAddr = Result->Start;

    std::vector<MemoryMapper::AllocInfo::SegInfo> SegInfos;

    for (auto &KV : BL.segments()) {
      auto &AG = KV.first;
      auto &Seg = KV.second;

      auto TotalSize = Seg.ContentSize + Seg.ZeroFillSize;

      Seg.Addr = NextSegAddr;
      Seg.WorkingMem = Mapper->prepare(G, NextSegAddr, TotalSize);

      NextSegAddr += alignTo(TotalSize, Mapper->getPageSize());

      MemoryMapper::AllocInfo::SegInfo SI;
      SI.Offset = Seg.Addr - Result->Start;
      SI.ContentSize = Seg.ContentSize;
      SI.ZeroFillSize = Seg.ZeroFillSize;
      SI.AG = AG;
      SI.WorkingMem = Seg.WorkingMem;

      SegInfos.push_back(SI);
    }

    UsedMemory.insert({Result->Start, NextSegAddr - Result->Start});

    if (NextSegAddr < Result->End) {
      // Save the remaining memory for reuse in next allocation(s)
      AvailableMemory.insert(NextSegAddr, Result->End - 1, true);
    }
    Mutex.unlock();

    if (auto Err = BL.apply()) {
      OnAllocated(std::move(Err));
      return;
    }

    OnAllocated(std::make_unique<InFlightAlloc>(*this, G, Result->Start,
                                                std::move(SegInfos)));
  };

  Mutex.lock();

  // find an already reserved range that is large enough
  ExecutorAddrRange SelectedRange{};

  for (AvailableMemoryMap::iterator It = AvailableMemory.begin();
       It != AvailableMemory.end(); It++) {
    if (It.stop() - It.start() + 1 >= TotalSize) {
      SelectedRange = ExecutorAddrRange(It.start(), It.stop() + 1);
      It.erase();
      break;
    }
  }

  if (SelectedRange.empty()) { // no already reserved range was found
    auto TotalAllocation = alignTo(TotalSize, ReservationUnits);
    Mapper->reserve(TotalAllocation, std::move(CompleteAllocation));
  } else {
    CompleteAllocation(SelectedRange);
  }
}

void MapperJITLinkMemoryManager::deallocate(
    std::vector<FinalizedAlloc> Allocs, OnDeallocatedFunction OnDeallocated) {
  std::vector<ExecutorAddr> Bases;
  Bases.reserve(Allocs.size());
  for (auto &FA : Allocs) {
    ExecutorAddr Addr = FA.getAddress();
    Bases.push_back(Addr);
  }

  Mapper->deinitialize(Bases, [this, Allocs = std::move(Allocs),
                               OnDeallocated = std::move(OnDeallocated)](
                                  toolchain::Error Err) mutable {
    // TODO: How should we treat memory that we fail to deinitialize?
    // We're currently bailing out and treating it as "burned" -- should we
    // require that a failure to deinitialize still reset the memory so that
    // we can reclaim it?
    if (Err) {
      for (auto &FA : Allocs)
        FA.release();
      OnDeallocated(std::move(Err));
      return;
    }

    {
      std::lock_guard<std::mutex> Lock(Mutex);

      for (auto &FA : Allocs) {
        ExecutorAddr Addr = FA.getAddress();
        ExecutorAddrDiff Size = UsedMemory[Addr];

        UsedMemory.erase(Addr);
        AvailableMemory.insert(Addr, Addr + Size - 1, true);

        FA.release();
      }
    }

    OnDeallocated(Error::success());
  });
}

} // end namespace orc
} // end namespace vm::core
