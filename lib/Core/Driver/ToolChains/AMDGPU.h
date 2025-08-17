//===--- AMDGPU.h - AMDGPU ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPU_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPU_H

#include "Gnu.h"
#include "language/Core/Basic/TargetID.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/TargetParser/TargetParser.h"

#include <map>

namespace language::Core {
namespace driver {

namespace tools {
namespace amdgpu {

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("amdgpu::Linker", "ld.lld", TC) {}
  bool isLinkJob() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

void getAMDGPUTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                             const toolchain::opt::ArgList &Args,
                             std::vector<StringRef> &Features);

void addFullLTOPartitionOption(const Driver &D, const toolchain::opt::ArgList &Args,
                               toolchain::opt::ArgStringList &CmdArgs);
} // end namespace amdgpu
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY AMDGPUToolChain : public Generic_ELF {
protected:
  const std::map<options::ID, const StringRef> OptionsDefault;

  Tool *buildLinker() const override;
  StringRef getOptionDefault(options::ID OptID) const {
    auto opt = OptionsDefault.find(OptID);
    assert(opt != OptionsDefault.end() && "No Default for Option");
    return opt->second;
  }

public:
  AMDGPUToolChain(const Driver &D, const toolchain::Triple &Triple,
                  const toolchain::opt::ArgList &Args);
  unsigned GetDefaultDwarfVersion() const override { return 5; }

  bool IsMathErrnoDefault() const override { return false; }
  bool isCrossCompiling() const override { return true; }
  bool isPICDefault() const override { return true; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return true; }
  bool SupportsProfiling() const override { return false; }

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;

  void addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;

  /// Return whether denormals should be flushed, and treated as 0 by default
  /// for the subtarget.
  static bool getDefaultDenormsAreZeroForTarget(toolchain::AMDGPU::GPUKind GPUKind);

  toolchain::DenormalMode getDefaultDenormalModeForType(
      const toolchain::opt::ArgList &DriverArgs, const JobAction &JA,
      const toolchain::fltSemantics *FPType = nullptr) const override;

  static bool isWave64(const toolchain::opt::ArgList &DriverArgs,
                       toolchain::AMDGPU::GPUKind Kind);
  /// Needed for using lto.
  bool HasNativeLLVMSupport() const override {
    return true;
  }

  /// Needed for translating LTO options.
  const char *getDefaultLinker() const override { return "ld.lld"; }

  /// Should skip sanitize options.
  bool shouldSkipSanitizeOption(const ToolChain &TC,
                                const toolchain::opt::ArgList &DriverArgs,
                                StringRef TargetID,
                                const toolchain::opt::Arg *A) const;

  /// Uses amdgpu-arch tool to get arch of the system GPU. Will return error
  /// if unable to find one.
  virtual Expected<SmallVector<std::string>>
  getSystemGPUArchs(const toolchain::opt::ArgList &Args) const override;

protected:
  /// Check and diagnose invalid target ID specified by -mcpu.
  virtual void checkTargetID(const toolchain::opt::ArgList &DriverArgs) const;

  /// The struct type returned by getParsedTargetID.
  struct ParsedTargetIDType {
    std::optional<std::string> OptionalTargetID;
    std::optional<std::string> OptionalGPUArch;
    std::optional<toolchain::StringMap<bool>> OptionalFeatures;
  };

  /// Get target ID, GPU arch, and target ID features if the target ID is
  /// specified and valid.
  ParsedTargetIDType
  getParsedTargetID(const toolchain::opt::ArgList &DriverArgs) const;

  /// Get GPU arch from -mcpu without checking.
  StringRef getGPUArch(const toolchain::opt::ArgList &DriverArgs) const;

  /// Common warning options shared by AMDGPU HIP, OpenCL and OpenMP toolchains.
  /// Language specific warning options should go to derived classes.
  void addClangWarningOptions(toolchain::opt::ArgStringList &CC1Args) const override;
};

class LLVM_LIBRARY_VISIBILITY ROCMToolChain : public AMDGPUToolChain {
public:
  ROCMToolChain(const Driver &D, const toolchain::Triple &Triple,
                const toolchain::opt::ArgList &Args);
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;

  // Returns a list of device library names shared by different languages
  toolchain::SmallVector<BitCodeLibraryInfo, 12>
  getCommonDeviceLibNames(const toolchain::opt::ArgList &DriverArgs,
                          const std::string &GPUArch,
                          Action::OffloadKind DeviceOffloadingKind) const;

  SanitizerMask getSupportedSanitizers() const override {
    return SanitizerKind::Address;
  }

  void diagnoseUnsupportedSanitizers(const toolchain::opt::ArgList &Args) const {
    if (!Args.hasFlag(options::OPT_fgpu_sanitize, options::OPT_fno_gpu_sanitize,
                      true))
      return;
    auto &Diags = getDriver().getDiags();
    for (auto *A : Args.filtered(options::OPT_fsanitize_EQ)) {
      SanitizerMask K =
          parseSanitizerValue(A->getValue(), /*Allow Groups*/ false);
      if (K != SanitizerKind::Address)
        Diags.Report(language::Core::diag::warn_drv_unsupported_option_for_target)
            << A->getAsString(Args) << getTriple().str();
    }
  }
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AMDGPU_H
