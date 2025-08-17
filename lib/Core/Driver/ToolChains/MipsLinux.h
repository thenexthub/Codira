//===--- Mips.h - Mips ToolChain Implementations ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MIPS_LINUX_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MIPS_LINUX_H

#include "Linux.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MipsLLVMToolChain : public Linux {
protected:
  Tool *buildLinker() const override;

public:
  MipsLLVMToolChain(const Driver &D, const toolchain::Triple &Triple,
                    const toolchain::opt::ArgList &Args);

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;

  CXXStdlibType GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;

  void addLibCxxIncludePaths(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  std::string getCompilerRT(const toolchain::opt::ArgList &Args, StringRef Component,
                            FileType Type = ToolChain::FT_Static,
                            bool IsFortran = false) const override;

  std::string computeSysRoot() const override;

  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return GCCInstallation.isValid() ? RuntimeLibType::RLT_Libgcc
                                     : RuntimeLibType::RLT_CompilerRT;
  }

  const char *getDefaultLinker() const override {
    return "ld.lld";
  }

private:
  std::string LibSuffix;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MIPS_LINUX_H
