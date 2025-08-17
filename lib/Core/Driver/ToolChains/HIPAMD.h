//===--- HIPAMD.h - HIP ToolChain Implementations ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPAMD_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPAMD_H

#include "AMDGPU.h"
#include "language/Core/Driver/SyclInstallationDetector.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {

namespace tools {

namespace AMDGCN {
// Runs toolchain-link/opt/llc/lld, which links multiple LLVM bitcode, together with
// device library, then compiles it to ISA in a shared object.
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("AMDGCN::Linker", "amdgcn-link", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;

private:
  void constructLldCommand(Compilation &C, const JobAction &JA,
                           const InputInfoList &Inputs, const InputInfo &Output,
                           const toolchain::opt::ArgList &Args) const;
  void constructLlvmLinkCommand(Compilation &C, const JobAction &JA,
                                const InputInfoList &Inputs,
                                const InputInfo &Output,
                                const toolchain::opt::ArgList &Args) const;
  void constructLinkAndEmitSpirvCommand(Compilation &C, const JobAction &JA,
                                        const InputInfoList &Inputs,
                                        const InputInfo &Output,
                                        const toolchain::opt::ArgList &Args) const;
};

} // end namespace AMDGCN
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY HIPAMDToolChain final : public ROCMToolChain {
public:
  HIPAMDToolChain(const Driver &D, const toolchain::Triple &Triple,
                  const ToolChain &HostTC, const toolchain::opt::ArgList &Args);

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
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &Args,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void AddIAMCUIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;
  void AddHIPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                         toolchain::opt::ArgStringList &CC1Args) const override;
  toolchain::SmallVector<BitCodeLibraryInfo, 12>
  getDeviceLibs(const toolchain::opt::ArgList &Args,
                Action::OffloadKind DeviceOffloadKind) const override;

  SanitizerMask getSupportedSanitizers() const override;

  VersionTuple
  computeMSVCVersion(const Driver *D,
                     const toolchain::opt::ArgList &Args) const override;

  unsigned GetDefaultDwarfVersion() const override { return 5; }

  const ToolChain &HostTC;
  void checkTargetID(const toolchain::opt::ArgList &DriverArgs) const override;

protected:
  Tool *buildLinker() const override;
};

class LLVM_LIBRARY_VISIBILITY SPIRVAMDToolChain final : public ROCMToolChain {
public:
  SPIRVAMDToolChain(const Driver &D, const toolchain::Triple &Triple,
                    const toolchain::opt::ArgList &Args);

protected:
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HIPAMD_H
