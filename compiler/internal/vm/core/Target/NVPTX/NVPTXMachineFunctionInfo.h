//===-- NVPTXMachineFunctionInfo.h - NVPTX-specific Function Info  --------===//
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
//
// This class is attached to a MachineFunction instance and tracks target-
// dependent information
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXMACHINEFUNCTIONINFO_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {
class NVPTXMachineFunctionInfo : public MachineFunctionInfo {
private:
  /// Stores a mapping from index to symbol name for image handles that are
  /// replaced with image references
  SmallVector<std::string, 8> ImageHandleList;

public:
  NVPTXMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override {
    return DestMF.cloneInfo<NVPTXMachineFunctionInfo>(*this);
  }

  /// Returns the index for the symbol \p Symbol. If the symbol was previously,
  /// added, the same index is returned. Otherwise, the symbol is added and the
  /// new index is returned.
  unsigned getImageHandleSymbolIndex(StringRef Symbol) {
    // Is the symbol already present?
    for (unsigned i = 0, e = ImageHandleList.size(); i != e; ++i)
      if (ImageHandleList[i] == Symbol)
        return i;
    // Nope, insert it
    ImageHandleList.push_back(Symbol.str());
    return ImageHandleList.size()-1;
  }

  /// Check if the symbol has a mapping. Having a mapping means the handle is
  /// replaced with a reference
  bool checkImageHandleSymbol(StringRef Symbol) const {
    return toolchain::is_contained(ImageHandleList, Symbol);
  }
};
}

#endif
