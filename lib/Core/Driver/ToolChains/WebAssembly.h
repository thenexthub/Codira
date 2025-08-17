//===--- WebAssembly.h - WebAssembly ToolChain Implementations --*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H

#include "Gnu.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace wasm {

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  explicit Linker(const ToolChain &TC) : Tool("wasm::Linker", "linker", TC) {}
  bool isLinkJob() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }
  std::string getLinkerPath(const toolchain::opt::ArgList &Args) const;
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace wasm
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY WebAssembly final : public ToolChain {
public:
  WebAssembly(const Driver &D, const toolchain::Triple &Triple,
              const toolchain::opt::ArgList &Args);

private:
  bool IsMathErrnoDefault() const override;
  bool IsObjCNonFragileABIDefault() const override;
  bool UseObjCMixedDispatch() const override;
  bool isPICDefault() const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefaultForced() const override;
  bool hasBlocksRuntime() const override;
  bool SupportsProfiling() const override;
  bool HasNativeLLVMSupport() const override;
  unsigned GetDefaultDwarfVersion() const override { return 4; }
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;
  RuntimeLibType GetDefaultRuntimeLibType() const override;
  CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;
  SanitizerMask getSupportedSanitizers() const override;

  const char *getDefaultLinker() const override;

  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }

  Tool *buildLinker() const override;

  std::string getMultiarchTriple(const Driver &D,
                                 const toolchain::Triple &TargetTriple,
                                 StringRef SysRoot) const override;

  void addLibCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args) const;
  void addLibStdCXXIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                                toolchain::opt::ArgStringList &CC1Args) const;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_WEBASSEMBLY_H
