//===- AMDGPUUnifyDivergentExitNodes.h ------------------------------------===//
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
// This is a variant of the UnifyFunctionExitNodes pass. Rather than ensuring
// there is at most one ret and one unreachable instruction, it ensures there is
// at most one divergent exiting block.
//
// StructurizeCFG can't deal with multi-exit regions formed by branches to
// multiple return nodes. It is not desirable to structurize regions with
// uniform branches, so unifying those to the same return block as divergent
// branches inhibits use of scalar branching. It still can't deal with the case
// where one branch goes to return, and one unreachable. Replace unreachable in
// this case with a return.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUUNIFYDIVERGENTEXITNODES_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUUNIFYDIVERGENTEXITNODES_H

#include "AMDGPU.h"

namespace vm::core {
class AMDGPUUnifyDivergentExitNodesPass
    : public PassInfoMixin<AMDGPUUnifyDivergentExitNodesPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUUNIFYDIVERGENTEXITNODES_H
