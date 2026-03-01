//===-- WebAssemblySortRegion.h - WebAssembly Sort SortRegion ----*- C++-*-===//
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
///
/// \file
/// \brief This file implements regions used in CFGSort and CFGStackify.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSORTREGION_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSORTREGION_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/iterator_range.h"

namespace vm::core {

class MachineBasicBlock;
class MachineLoop;
class MachineLoopInfo;
class WebAssemblyException;
class WebAssemblyExceptionInfo;

namespace WebAssembly {

// Wrapper for loops and exceptions
class SortRegion {
public:
  virtual ~SortRegion() = default;
  virtual MachineBasicBlock *getHeader() const = 0;
  virtual bool contains(const MachineBasicBlock *MBB) const = 0;
  virtual unsigned getNumBlocks() const = 0;
  using block_iterator = ArrayRef<MachineBasicBlock *>::const_iterator;
  virtual iterator_range<block_iterator> blocks() const = 0;
  virtual bool isLoop() const = 0;
};

template <typename T> class ConcreteSortRegion : public SortRegion {
  const T *Unit;

public:
  ConcreteSortRegion(const T *Unit) : Unit(Unit) {}
  MachineBasicBlock *getHeader() const override { return Unit->getHeader(); }
  bool contains(const MachineBasicBlock *MBB) const override {
    return Unit->contains(MBB);
  }
  unsigned getNumBlocks() const override { return Unit->getNumBlocks(); }
  iterator_range<block_iterator> blocks() const override {
    return Unit->blocks();
  }
  bool isLoop() const override { return false; }
};

// This class has information of nested SortRegions; this is analogous to what
// LoopInfo is for loops.
class SortRegionInfo {
  friend class ConcreteSortRegion<MachineLoopInfo>;
  friend class ConcreteSortRegion<WebAssemblyException>;

  const MachineLoopInfo &MLI;
  const WebAssemblyExceptionInfo &WEI;
  DenseMap<const MachineLoop *, std::unique_ptr<SortRegion>> LoopMap;
  DenseMap<const WebAssemblyException *, std::unique_ptr<SortRegion>>
      ExceptionMap;

public:
  SortRegionInfo(const MachineLoopInfo &MLI,
                 const WebAssemblyExceptionInfo &WEI)
      : MLI(MLI), WEI(WEI) {}

  // Returns a smallest loop or exception that contains MBB
  const SortRegion *getRegionFor(const MachineBasicBlock *MBB);

  // Return the "bottom" block among all blocks dominated by the region
  // (MachineLoop or WebAssemblyException) header. This works when the entity is
  // discontiguous.
  MachineBasicBlock *getBottom(const SortRegion *R);
  MachineBasicBlock *getBottom(const MachineLoop *ML);
  MachineBasicBlock *getBottom(const WebAssemblyException *WE);
};

} // end namespace WebAssembly

} // end namespace vm::core

#endif
