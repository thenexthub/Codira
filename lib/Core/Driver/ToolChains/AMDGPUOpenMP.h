//===- AMDGPUOpenMP.h - AMDGPUOpenMP ToolChain Implementation -*- C++ -*---===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPUOPENMP_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPUOPENMP_H

#include "AMDGPU.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {

namespace toolchains {
class AMDGPUOpenMPToolChain;
}

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY AMDGPUOpenMPToolChain final
    : public ROCMToolChain {
public:
  AMDGPUOpenMPToolChain(const Driver &D, const toolchain::Triple &Triple,
                        const ToolChain &HostTC,
                        const toolchain::opt::ArgList &Args);

  const toolchain::Triple *getAuxTriple() const override {
    return &HostTC.getTriple();
  }

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;
  void addClangWarningOptions(toolchain::opt::ArgStringList &CC1Args) const override;
  CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &Args,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void AddIAMCUIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;

  SanitizerMask getSupportedSanitizers() const override;

  VersionTuple
  computeMSVCVersion(const Driver *D,
                     const toolchain::opt::ArgList &Args) const override;

  toolchain::SmallVector<BitCodeLibraryInfo, 12>
  getDeviceLibs(const toolchain::opt::ArgList &Args,
                const Action::OffloadKind DeviceOffloadKind) const override;

  const ToolChain &HostTC;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPUOPENMP_H
