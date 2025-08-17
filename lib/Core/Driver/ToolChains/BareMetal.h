//===--- BareMetal.h - Bare Metal Tool and ToolChain ------------*- C++-*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_BAREMETAL_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_BAREMETAL_H

#include "ToolChains/Gnu.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

#include <string>

namespace language::Core {
namespace driver {

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY BareMetal : public Generic_ELF {
public:
  BareMetal(const Driver &D, const toolchain::Triple &Triple,
            const toolchain::opt::ArgList &Args);
  ~BareMetal() override = default;

  static bool handlesTarget(const toolchain::Triple &Triple);

  void findMultilibs(const Driver &D, const toolchain::Triple &Triple,
                     const toolchain::opt::ArgList &Args);

protected:
  Tool *buildLinker() const override;
  Tool *buildStaticLibTool() const override;

public:
  bool initGCCInstallation(const toolchain::Triple &Triple,
                           const toolchain::opt::ArgList &Args);
  bool hasValidGCCInstallation() const { return IsGCCInstallationValid; }
  bool isBareMetal() const override { return true; }
  bool isCrossCompiling() const override { return true; }
  bool HasNativeLLVMSupport() const override { return true; }
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }
  bool SupportsProfiling() const override { return false; }

  StringRef getOSLibName() const override { return "baremetal"; }

  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override {
    return UnwindTableLevel::None;
  }

  CXXStdlibType GetDefaultCXXStdlibType() const override;

  RuntimeLibType GetDefaultRuntimeLibType() const override;

  UnwindLibType GetUnwindLibType(const toolchain::opt::ArgList &Args) const override;

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;
  std::string computeSysRoot() const override;
  std::string getCompilerRTPath() const override;
  SanitizerMask getSupportedSanitizers() const override;

  SmallVector<std::string>
  getMultilibMacroDefinesStr(toolchain::opt::ArgList &Args) const override;

private:
  using OrderedMultilibs =
      toolchain::iterator_range<toolchain::SmallVector<Multilib>::const_reverse_iterator>;
  OrderedMultilibs getOrderedMultilibs() const;

  std::string SysRoot;

  bool IsGCCInstallationValid;

  SmallVector<std::string> MultilibMacroDefines;
};

} // namespace toolchains

namespace tools {
namespace baremetal {

class LLVM_LIBRARY_VISIBILITY StaticLibTool : public Tool {
public:
  StaticLibTool(const ToolChain &TC)
      : Tool("baremetal::StaticLibTool", "toolchain-ar", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("baremetal::Linker", "linker", TC) {}
  bool isLinkJob() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // namespace baremetal
} // namespace tools

} // namespace driver
} // namespace language::Core

#endif
