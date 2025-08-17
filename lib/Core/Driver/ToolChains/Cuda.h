//===--- Cuda.h - Cuda ToolChain Implementations ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CUDA_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CUDA_H

#include "language/Core/Basic/Cuda.h"
#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/CudaInstallationDetector.h"
#include "language/Core/Driver/Multilib.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/VersionTuple.h"
#include <bitset>
#include <set>
#include <vector>

namespace language::Core {
namespace driver {
namespace tools {
namespace NVPTX {

// Run ptxas, the NVPTX assembler.
class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("NVPTX::Assembler", "ptxas", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

// Runs fatbinary, which combines GPU object files ("cubin" files) and/or PTX
// assembly into a single output file.
class LLVM_LIBRARY_VISIBILITY FatBinary : public Tool {
public:
  FatBinary(const ToolChain &TC) : Tool("NVPTX::Linker", "fatbinary", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

// Runs nvlink, which links GPU object files ("cubin" files) into a single file.
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("NVPTX::Linker", "nvlink", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

void getNVPTXTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                            const toolchain::opt::ArgList &Args,
                            std::vector<StringRef> &Features);

} // end namespace NVPTX
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY NVPTXToolChain : public ToolChain {
public:
  NVPTXToolChain(const Driver &D, const toolchain::Triple &Triple,
                 const toolchain::Triple &HostTriple,
                 const toolchain::opt::ArgList &Args);

  NVPTXToolChain(const Driver &D, const toolchain::Triple &Triple,
                 const toolchain::opt::ArgList &Args);

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;

  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;

  // Never try to use the integrated assembler with CUDA; always fork out to
  // ptxas.
  bool useIntegratedAs() const override { return false; }
  bool isCrossCompiling() const override { return true; }
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool HasNativeLLVMSupport() const override { return true; }
  bool isPICDefaultForced() const override { return false; }
  bool SupportsProfiling() const override { return false; }

  bool IsMathErrnoDefault() const override { return false; }

  bool supportsDebugInfoOption(const toolchain::opt::Arg *A) const override;
  void adjustDebugInfoKind(toolchain::codegenoptions::DebugInfoKind &DebugInfoKind,
                           const toolchain::opt::ArgList &Args) const override;

  // NVPTX supports only DWARF2.
  unsigned GetDefaultDwarfVersion() const override { return 2; }
  unsigned getMaxDwarfVersion() const override { return 2; }

  /// Uses nvptx-arch tool to get arch of the system GPU. Will return error
  /// if unable to find one.
  virtual Expected<SmallVector<std::string>>
  getSystemGPUArchs(const toolchain::opt::ArgList &Args) const override;

  CudaInstallationDetector CudaInstallation;

protected:
  Tool *buildAssembler() const override; // ptxas.
  Tool *buildLinker() const override;    // nvlink.
};

class LLVM_LIBRARY_VISIBILITY CudaToolChain : public NVPTXToolChain {
public:
  CudaToolChain(const Driver &D, const toolchain::Triple &Triple,
                const ToolChain &HostTC, const toolchain::opt::ArgList &Args);

  const toolchain::Triple *getAuxTriple() const override {
    return &HostTC.getTriple();
  }

  bool HasNativeLLVMSupport() const override { return false; }

  std::string getInputFilename(const InputInfo &Input) const override;

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;

  toolchain::DenormalMode getDefaultDenormalModeForType(
      const toolchain::opt::ArgList &DriverArgs, const JobAction &JA,
      const toolchain::fltSemantics *FPType = nullptr) const override;

  void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;

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

  SanitizerMask getSupportedSanitizers() const override;

  VersionTuple
  computeMSVCVersion(const Driver *D,
                     const toolchain::opt::ArgList &Args) const override;

  const ToolChain &HostTC;

protected:
  Tool *buildAssembler() const override; // ptxas
  Tool *buildLinker() const override;    // fatbinary (ok, not really a linker)
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CUDA_H
