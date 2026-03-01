//===- CBuffer.cpp - HLSL constant buffer handling ------------------------===//
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

#include "vm/core/Frontend/HLSL/CBuffer.h"
#include "vm/core/Frontend/HLSL/HLSLResource.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;
using namespace vm::core::hlsl;

static SmallVector<size_t>
getMemberOffsets(const DataLayout &DL, GlobalVariable *Handle,
                 toolchain::function_ref<bool(Type *)> IsPadding) {
  SmallVector<size_t> Offsets;

  auto *HandleTy = cast<TargetExtType>(Handle->getValueType());
  assert((HandleTy->getName().ends_with(".CBuffer") ||
          HandleTy->getName() == "spirv.VulkanBuffer") &&
         "Not a cbuffer type");
  assert(HandleTy->getNumTypeParameters() == 1 && "Expected layout type");
  auto *LayoutTy = cast<StructType>(HandleTy->getTypeParameter(0));

  const StructLayout *SL = DL.getStructLayout(LayoutTy);
  for (int I = 0, E = LayoutTy->getNumElements(); I < E; ++I)
    if (!IsPadding(LayoutTy->getElementType(I)))
      Offsets.push_back(SL->getElementOffset(I));

  return Offsets;
}

std::optional<CBufferMetadata>
CBufferMetadata::get(Module &M, toolchain::function_ref<bool(Type *)> IsPadding) {
  NamedMDNode *CBufMD = M.getNamedMetadata("hlsl.cbs");
  if (!CBufMD)
    return std::nullopt;

  std::optional<CBufferMetadata> Result({CBufMD});

  for (const MDNode *MD : CBufMD->operands()) {
    assert(MD->getNumOperands() && "Invalid cbuffer metadata");

    // For an unused cbuffer, the handle may have been optimized out
    Metadata *OpMD = MD->getOperand(0);
    if (!OpMD)
      continue;

    auto *Handle =
        cast<GlobalVariable>(cast<ValueAsMetadata>(OpMD)->getValue());
    CBufferMapping &Mapping = Result->Mappings.emplace_back(Handle);

    SmallVector<size_t> MemberOffsets =
        getMemberOffsets(M.getDataLayout(), Handle, IsPadding);

    for (int I = 1, E = MD->getNumOperands(); I < E; ++I) {
      Metadata *OpMD = MD->getOperand(I);
      // Some members may be null if they've been optimized out.
      if (!OpMD)
        continue;
      auto *V = cast<GlobalVariable>(cast<ValueAsMetadata>(OpMD)->getValue());
      Mapping.Members.emplace_back(V, MemberOffsets[I - 1]);
    }
  }

  return Result;
}

void CBufferMetadata::eraseFromModule() {
  // Remove the cbs named metadata
  MD->eraseFromParent();
}
