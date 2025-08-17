//===--- Hexagon.h - Hexagon ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HEXAGON_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HEXAGON_H

#include "Linux.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace hexagon {
// For Hexagon, we do not need to instantiate tools for PreProcess, PreCompile
// and Compile.
// We simply use "clang -cc1" for those actions.
class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC)
      : Tool("hexagon::Assembler", "hexagon-as", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void RenderExtraToolArgs(const JobAction &JA,
                           toolchain::opt::ArgStringList &CmdArgs) const;
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("hexagon::Linker", "hexagon-ld", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void RenderExtraToolArgs(const JobAction &JA,
                           toolchain::opt::ArgStringList &CmdArgs) const;
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

void getHexagonTargetFeatures(const Driver &D, const toolchain::Triple &Triple,
                              const toolchain::opt::ArgList &Args,
                              std::vector<StringRef> &Features);

} // end namespace hexagon.
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY HexagonToolChain : public Linux {
protected:
  GCCVersion GCCLibAndIncVersion;
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

  unsigned getOptimizationLevel(const toolchain::opt::ArgList &DriverArgs) const;

public:
  HexagonToolChain(const Driver &D, const toolchain::Triple &Triple,
                   const toolchain::opt::ArgList &Args);
  ~HexagonToolChain() override;

  void addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void addLibStdCxxIncludePaths(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void addLibCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args) const override;

  const char *getDefaultLinker() const override {
    return getTriple().isMusl() ? "ld.lld" : "hexagon-link";
  }

  CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;

  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  StringRef GetGCCLibAndIncVersion() const { return GCCLibAndIncVersion.Text; }

  std::string getHexagonTargetDir(
      const std::string &InstalledDir,
      const SmallVectorImpl<std::string> &PrefixDirs) const;
  void getHexagonLibraryPaths(const toolchain::opt::ArgList &Args,
      ToolChain::path_list &LibPaths) const;

  std::string getCompilerRTPath() const override;

  static bool isAutoHVXEnabled(const toolchain::opt::ArgList &Args);
  static StringRef GetDefaultCPU();
  static StringRef GetTargetCPUVersion(const toolchain::opt::ArgList &Args);

  static std::optional<unsigned>
  getSmallDataThreshold(const toolchain::opt::ArgList &Args);
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HEXAGON_H
