//===--- AMDGPUMachineModuleInfo.h ------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUMACHINEMODULEINFO_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUMACHINEMODULEINFO_H

#include "vm/core/CodeGen/MachineModuleInfoImpls.h"
#include "vm/core/IR/LLVMContext.h"

namespace vm::core {

class AMDGPUMachineModuleInfo final : public MachineModuleInfoELF {
private:

  // All supported memory/synchronization scopes can be found here:
  //   http://toolchain.org/docs/AMDGPUUsage.html#memory-scopes

  /// Agent synchronization scope ID (cross address space).
  SyncScope::ID AgentSSID;
  /// Workgroup synchronization scope ID (cross address space).
  SyncScope::ID WorkgroupSSID;
  /// Wavefront synchronization scope ID (cross address space).
  SyncScope::ID WavefrontSSID;
  /// Cluster synchronization scope ID (cross address space).
  SyncScope::ID ClusterSSID;
  /// System synchronization scope ID (single address space).
  SyncScope::ID SystemOneAddressSpaceSSID;
  /// Agent synchronization scope ID (single address space).
  SyncScope::ID AgentOneAddressSpaceSSID;
  /// Workgroup synchronization scope ID (single address space).
  SyncScope::ID WorkgroupOneAddressSpaceSSID;
  /// Wavefront synchronization scope ID (single address space).
  SyncScope::ID WavefrontOneAddressSpaceSSID;
  /// Single thread synchronization scope ID (single address space).
  SyncScope::ID SingleThreadOneAddressSpaceSSID;
  /// Cluster synchronization scope ID (single address space).
  SyncScope::ID ClusterOneAddressSpaceSSID;

  /// In AMDGPU target synchronization scopes are inclusive, meaning a
  /// larger synchronization scope is inclusive of a smaller synchronization
  /// scope.
  ///
  /// \returns \p SSID's inclusion ordering, or "std::nullopt" if \p SSID is not
  /// supported by the AMDGPU target.
  std::optional<uint8_t>
  getSyncScopeInclusionOrdering(SyncScope::ID SSID) const {
    if (SSID == SyncScope::SingleThread ||
        SSID == getSingleThreadOneAddressSpaceSSID())
      return 0;
    else if (SSID == getWavefrontSSID() ||
             SSID == getWavefrontOneAddressSpaceSSID())
      return 1;
    else if (SSID == getWorkgroupSSID() ||
             SSID == getWorkgroupOneAddressSpaceSSID())
      return 2;
    else if (SSID == getClusterSSID() ||
             SSID == getClusterOneAddressSpaceSSID())
      return 3;
    else if (SSID == getAgentSSID() ||
             SSID == getAgentOneAddressSpaceSSID())
      return 4;
    else if (SSID == SyncScope::System ||
             SSID == getSystemOneAddressSpaceSSID())
      return 5;

    return std::nullopt;
  }

  /// \returns True if \p SSID is restricted to single address space, false
  /// otherwise
  bool isOneAddressSpace(SyncScope::ID SSID) const {
    return SSID == getClusterOneAddressSpaceSSID() ||
           SSID == getSingleThreadOneAddressSpaceSSID() ||
           SSID == getWavefrontOneAddressSpaceSSID() ||
           SSID == getWorkgroupOneAddressSpaceSSID() ||
           SSID == getAgentOneAddressSpaceSSID() ||
           SSID == getSystemOneAddressSpaceSSID();
  }

public:
  AMDGPUMachineModuleInfo(const MachineModuleInfo &MMI);

  /// \returns Agent synchronization scope ID (cross address space).
  SyncScope::ID getAgentSSID() const {
    return AgentSSID;
  }
  /// \returns Workgroup synchronization scope ID (cross address space).
  SyncScope::ID getWorkgroupSSID() const {
    return WorkgroupSSID;
  }
  /// \returns Wavefront synchronization scope ID (cross address space).
  SyncScope::ID getWavefrontSSID() const {
    return WavefrontSSID;
  }
  /// \returns Cluster synchronization scope ID (cross address space).
  SyncScope::ID getClusterSSID() const { return ClusterSSID; }
  /// \returns System synchronization scope ID (single address space).
  SyncScope::ID getSystemOneAddressSpaceSSID() const {
    return SystemOneAddressSpaceSSID;
  }
  /// \returns Agent synchronization scope ID (single address space).
  SyncScope::ID getAgentOneAddressSpaceSSID() const {
    return AgentOneAddressSpaceSSID;
  }
  /// \returns Workgroup synchronization scope ID (single address space).
  SyncScope::ID getWorkgroupOneAddressSpaceSSID() const {
    return WorkgroupOneAddressSpaceSSID;
  }
  /// \returns Wavefront synchronization scope ID (single address space).
  SyncScope::ID getWavefrontOneAddressSpaceSSID() const {
    return WavefrontOneAddressSpaceSSID;
  }
  /// \returns Single thread synchronization scope ID (single address space).
  SyncScope::ID getSingleThreadOneAddressSpaceSSID() const {
    return SingleThreadOneAddressSpaceSSID;
  }
  /// \returns Single thread synchronization scope ID (single address space).
  SyncScope::ID getClusterOneAddressSpaceSSID() const {
    return ClusterOneAddressSpaceSSID;
  }

  /// In AMDGPU target synchronization scopes are inclusive, meaning a
  /// larger synchronization scope is inclusive of a smaller synchronization
  /// scope.
  ///
  /// \returns True if synchronization scope \p A is larger than or equal to
  /// synchronization scope \p B, false if synchronization scope \p A is smaller
  /// than synchronization scope \p B, or "std::nullopt" if either
  /// synchronization scope \p A or \p B is not supported by the AMDGPU target.
  std::optional<bool> isSyncScopeInclusion(SyncScope::ID A,
                                           SyncScope::ID B) const {
    const auto &AIO = getSyncScopeInclusionOrdering(A);
    const auto &BIO = getSyncScopeInclusionOrdering(B);
    if (!AIO || !BIO)
      return std::nullopt;

    bool IsAOneAddressSpace = isOneAddressSpace(A);
    bool IsBOneAddressSpace = isOneAddressSpace(B);

    return *AIO >= *BIO &&
           (IsAOneAddressSpace == IsBOneAddressSpace || !IsAOneAddressSpace);
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUMACHINEMODULEINFO_H
