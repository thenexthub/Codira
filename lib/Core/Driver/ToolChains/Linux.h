//===--- Linux.h - Linux ToolChain Implementations --------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_LINUX_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_LINUX_H

#include "Gnu.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY Linux : public Generic_ELF {
public:
  Linux(const Driver &D, const toolchain::Triple &Triple,
        const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override;

  std::string getMultiarchTriple(const Driver &D,
                                 const toolchain::Triple &TargetTriple,
                                 StringRef SysRoot) const override;

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void addLibStdCxxIncludePaths(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;
  void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;
  void AddHIPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                         toolchain::opt::ArgStringList &CC1Args) const override;
  void AddHIPRuntimeLibArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const override;
  void AddIAMCUIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;
  void addSYCLIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;
  RuntimeLibType GetDefaultRuntimeLibType() const override;
  unsigned GetDefaultDwarfVersion() const override;
  CXXStdlibType GetDefaultCXXStdlibType() const override;
  bool
  IsAArch64OutlineAtomicsDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool IsMathErrnoDefault() const override;
  SanitizerMask getSupportedSanitizers() const override;
  void addProfileRTLibs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const override;
  std::string computeSysRoot() const override;

  std::string getDynamicLinker(const toolchain::opt::ArgList &Args) const override;

  void addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const override;

  std::vector<std::string> ExtraOpts;

  const char *getDefaultLinker() const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
  Tool *buildStaticLibTool() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_LINUX_H
