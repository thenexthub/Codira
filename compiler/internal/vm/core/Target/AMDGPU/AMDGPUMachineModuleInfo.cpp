//===--- AMDGPUMachineModuleInfo.cpp ----------------------------*- C++ -*-===//
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
/// AMDGPU Machine Module Info.
///
//
//===----------------------------------------------------------------------===//

#include "AMDGPUMachineModuleInfo.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

AMDGPUMachineModuleInfo::AMDGPUMachineModuleInfo(const MachineModuleInfo &MMI)
    : MachineModuleInfoELF(MMI) {
  LLVMContext &CTX = MMI.getModule()->getContext();
  AgentSSID = CTX.getOrInsertSyncScopeID("agent");
  WorkgroupSSID = CTX.getOrInsertSyncScopeID("workgroup");
  WavefrontSSID = CTX.getOrInsertSyncScopeID("wavefront");
  ClusterSSID = CTX.getOrInsertSyncScopeID("cluster");
  SystemOneAddressSpaceSSID =
      CTX.getOrInsertSyncScopeID("one-as");
  AgentOneAddressSpaceSSID =
      CTX.getOrInsertSyncScopeID("agent-one-as");
  WorkgroupOneAddressSpaceSSID =
      CTX.getOrInsertSyncScopeID("workgroup-one-as");
  WavefrontOneAddressSpaceSSID =
      CTX.getOrInsertSyncScopeID("wavefront-one-as");
  SingleThreadOneAddressSpaceSSID =
      CTX.getOrInsertSyncScopeID("singlethread-one-as");
  ClusterOneAddressSpaceSSID = CTX.getOrInsertSyncScopeID("cluster-one-as");
}
