//===- AMDGPUMCResourceInfo.h ----- MC Resource Info --------------*- C++ -*-=//
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
/// \file
/// \brief MC infrastructure to propagate the function level resource usage
/// info.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUMCRESOURCEINFO_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUMCRESOURCEINFO_H

#include "AMDGPUResourceUsageAnalysis.h"
#include "MCTargetDesc/AMDGPUMCExpr.h"

namespace vm::core {

class MCContext;
class MCSymbol;
class StringRef;
class MachineFunction;

class MCResourceInfo {
public:
  enum ResourceInfoKind {
    RIK_NumVGPR,
    RIK_NumAGPR,
    RIK_NumSGPR,
    RIK_NumNamedBarrier,
    RIK_PrivateSegSize,
    RIK_UsesVCC,
    RIK_UsesFlatScratch,
    RIK_HasDynSizedStack,
    RIK_HasRecursion,
    RIK_HasIndirectCall
  };

private:
  int32_t MaxVGPR = 0;
  int32_t MaxAGPR = 0;
  int32_t MaxSGPR = 0;
  int32_t MaxNamedBarrier = 0;

  // Whether the MCResourceInfo has been finalized through finalize(MCContext
  // &). Should only be called once, at the end of AsmPrinting to assign MaxXGPR
  // symbols to their final value.
  bool Finalized = false;

  void assignResourceInfoExpr(int64_t localValue, ResourceInfoKind RIK,
                              AMDGPUMCExpr::VariantKind Kind,
                              const MachineFunction &MF,
                              const SmallVectorImpl<const Function *> &Callees,
                              MCContext &OutContext);

  // Assigns expression for Max S/V/A-GPRs to the referenced symbols.
  void assignMaxRegs(MCContext &OutContext);

  // Take flattened max of cyclic function calls' knowns. For example, for
  // a cycle A->B->C->D->A, take max(A, B, C, D) for A and have B, C, D have the
  // propgated value from A.
  const MCExpr *flattenedCycleMax(MCSymbol *RecSym, ResourceInfoKind RIK,
                                  MCContext &OutContext);

public:
  MCResourceInfo() = default;
  void addMaxVGPRCandidate(int32_t candidate) {
    MaxVGPR = std::max(MaxVGPR, candidate);
  }
  void addMaxAGPRCandidate(int32_t candidate) {
    MaxAGPR = std::max(MaxAGPR, candidate);
  }
  void addMaxSGPRCandidate(int32_t candidate) {
    MaxSGPR = std::max(MaxSGPR, candidate);
  }
  void addMaxNamedBarrierCandidate(int32_t candidate) {
    MaxNamedBarrier = std::max(MaxNamedBarrier, candidate);
  }

  MCSymbol *getSymbol(StringRef FuncName, ResourceInfoKind RIK,
                      MCContext &OutContext, bool IsLocal);
  const MCExpr *getSymRefExpr(StringRef FuncName, ResourceInfoKind RIK,
                              MCContext &Ctx, bool IsLocal);

  void reset();

  // Resolves the final symbols that requires the inter-function resource info
  // to be resolved.
  void finalize(MCContext &OutContext);

  MCSymbol *getMaxVGPRSymbol(MCContext &OutContext);
  MCSymbol *getMaxAGPRSymbol(MCContext &OutContext);
  MCSymbol *getMaxSGPRSymbol(MCContext &OutContext);
  MCSymbol *getMaxNamedBarrierSymbol(MCContext &OutContext);

  /// AMDGPUResourceUsageAnalysis gathers resource usage on a per-function
  /// granularity. However, some resource info has to be assigned the call
  /// transitive maximum or accumulative. For example, if A calls B and B's VGPR
  /// usage exceeds A's, A should be assigned B's VGPR usage. Furthermore,
  /// functions with indirect calls should be assigned the module level maximum.
  void gatherResourceInfo(
      const MachineFunction &MF,
      const AMDGPUResourceUsageAnalysisWrapperPass::FunctionResourceInfo &FRI,
      MCContext &OutContext);

  const MCExpr *createTotalNumVGPRs(const MachineFunction &MF, MCContext &Ctx);
  const MCExpr *createTotalNumSGPRs(const MachineFunction &MF, bool hasXnack,
                                    MCContext &Ctx);
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUMCRESOURCEINFO_H
