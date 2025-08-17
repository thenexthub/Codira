//===--- PS4CPU.h - PS4CPU ToolChain Implementations ------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_PS4CPU_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_PS4CPU_H

#include "Gnu.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {

namespace PScpu {
// Functions/classes in this namespace support both PS4 and PS5.

void addProfileRTArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                      toolchain::opt::ArgStringList &CmdArgs);

void addSanitizerArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                      toolchain::opt::ArgStringList &CmdArgs);

class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("PScpu::Assembler", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // namespace PScpu

namespace PS4cpu {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("PS4cpu::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // namespace PS4cpu

namespace PS5cpu {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("PS5cpu::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // namespace PS5cpu

} // namespace tools

namespace toolchains {

// Common Toolchain base class for PS4 and PS5.
class LLVM_LIBRARY_VISIBILITY PS4PS5Base : public Generic_ELF {
public:
  PS4PS5Base(const Driver &D, const toolchain::Triple &Triple,
             const toolchain::opt::ArgList &Args, StringRef Platform,
             const char *EnvVar);

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  // No support for finding a C++ standard library yet.
  void addLibCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args) const override {
  }
  void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override {}

  bool IsMathErrnoDefault() const override { return false; }
  bool IsObjCNonFragileABIDefault() const override { return true; }
  bool HasNativeLLVMSupport() const override { return true; }
  bool isPICDefault() const override { return true; }

  LangOptions::StackProtectorMode
  GetDefaultStackProtectorLevel(bool KernelOrKext) const override {
    return LangOptions::SSPStrong;
  }

  toolchain::DebuggerKind getDefaultDebuggerTuning() const override {
    return toolchain::DebuggerKind::SCE;
  }

  SanitizerMask getSupportedSanitizers() const override;

  void addClangTargetOptions(
      const toolchain::opt::ArgList &DriverArgs, toolchain::opt::ArgStringList &CC1Args,
      Action::OffloadKind DeviceOffloadingKind) const override;

  toolchain::DenormalMode getDefaultDenormalModeForType(
      const toolchain::opt::ArgList &DriverArgs, const JobAction &JA,
      const toolchain::fltSemantics *FPType) const override {
    // DAZ and FTZ are on by default.
    return toolchain::DenormalMode::getPreserveSign();
  }

  // Helper methods for PS4/PS5.
  virtual const char *getLinkerBaseName() const = 0;
  virtual std::string qualifyPSCmdName(StringRef CmdName) const = 0;
  virtual void addSanitizerArgs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs,
                                const char *Prefix,
                                const char *Suffix) const = 0;
  virtual const char *getProfileRTLibName() const = 0;

  StringRef getSDKLibraryRootDir() const { return SDKLibraryRootDir; }

private:
  // We compute the SDK locations in the ctor, and use them later.
  std::string SDKHeaderRootDir;
  std::string SDKLibraryRootDir;
};

// PS4-specific Toolchain class.
class LLVM_LIBRARY_VISIBILITY PS4CPU : public PS4PS5Base {
public:
  PS4CPU(const Driver &D, const toolchain::Triple &Triple,
         const toolchain::opt::ArgList &Args);

  unsigned GetDefaultDwarfVersion() const override { return 4; }

  // PS4 toolchain uses legacy thin LTO API, which is not
  // capable of unit splitting.
  bool canSplitThinLTOUnit() const override { return false; }

  const char *getLinkerBaseName() const override { return "ld"; }
  std::string qualifyPSCmdName(StringRef CmdName) const override {
    return Twine("orbis-", CmdName).str();
  }
  void addSanitizerArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs, const char *Prefix,
                        const char *Suffix) const override;
  const char *getProfileRTLibName() const override {
    return "libclang_rt.profile-x86_64.a";
  }

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

// PS5-specific Toolchain class.
class LLVM_LIBRARY_VISIBILITY PS5CPU : public PS4PS5Base {
public:
  PS5CPU(const Driver &D, const toolchain::Triple &Triple,
         const toolchain::opt::ArgList &Args);

  unsigned GetDefaultDwarfVersion() const override { return 5; }

  SanitizerMask getSupportedSanitizers() const override;

  const char *getLinkerBaseName() const override { return "lld"; }
  std::string qualifyPSCmdName(StringRef CmdName) const override {
    return Twine("prospero-", CmdName).str();
  }
  void addSanitizerArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs, const char *Prefix,
                        const char *Suffix) const override;
  const char *getProfileRTLibName() const override {
    return "libclang_rt.profile_nosubmission.a";
  }

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_PS4CPU_H
