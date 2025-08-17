//===--- Fuchsia.h - Fuchsia ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FUCHSIA_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FUCHSIA_H

#include "Gnu.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace fuchsia {
class LLVM_LIBRARY_VISIBILITY StaticLibTool : public Tool {
public:
  StaticLibTool(const ToolChain &TC)
      : Tool("fuchsia::StaticLibTool", "toolchain-ar", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("fuchsia::Linker", "ld.lld", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace fuchsia
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY Fuchsia : public ToolChain {
public:
  Fuchsia(const Driver &D, const toolchain::Triple &Triple,
          const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override { return true; }
  bool IsMathErrnoDefault() const override { return false; }
  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return ToolChain::RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }
  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override {
    return UnwindTableLevel::Asynchronous;
  }
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return true;
  }
  bool isPICDefaultForced() const override { return false; }
  toolchain::DebuggerKind getDefaultDebuggerTuning() const override {
    return toolchain::DebuggerKind::GDB;
  }

  LangOptions::StackProtectorMode
  GetDefaultStackProtectorLevel(bool KernelOrKext) const override {
    return LangOptions::SSPStrong;
  }

  std::string ComputeEffectiveClangTriple(const toolchain::opt::ArgList &Args,
                                          types::ID InputType) const override;

  SanitizerMask getSupportedSanitizers() const override;
  SanitizerMask getDefaultSanitizers() const override;

  RuntimeLibType
  GetRuntimeLibType(const toolchain::opt::ArgList &Args) const override;
  CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;

  bool IsAArch64OutlineAtomicsDefault(
      const toolchain::opt::ArgList &Args) const override {
    return true;
  }

  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  const char *getDefaultLinker() const override { return "ld.lld"; }

protected:
  Tool *buildLinker() const override;
  Tool *buildStaticLibTool() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FUCHSIA_H
