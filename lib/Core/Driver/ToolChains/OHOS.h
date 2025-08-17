//===--- OHOS.h - OHOS ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OHOS_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OHOS_H

#include "Linux.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY OHOS : public Generic_ELF {
public:
  OHOS(const Driver &D, const toolchain::Triple &Triple,
          const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override { return true; }

  bool IsMathErrnoDefault() const override { return false; }

  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return ToolChain::RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }
  // Not add -funwind-tables by default
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override { return true; }
  bool isPICDefaultForced() const override { return false; }
  UnwindLibType GetUnwindLibType(const toolchain::opt::ArgList &Args) const override;
  UnwindLibType GetDefaultUnwindLibType() const override { return UNW_CompilerRT; }

  RuntimeLibType
  GetRuntimeLibType(const toolchain::opt::ArgList &Args) const override;
  CXXStdlibType
  GetCXXStdlibType(const toolchain::opt::ArgList &Args) const override;

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void
  AddClangCXXStdlibIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                               toolchain::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  std::string computeSysRoot() const override;
  std::string getDynamicLinker(const toolchain::opt::ArgList &Args) const override;

  std::string getCompilerRT(const toolchain::opt::ArgList &Args, StringRef Component,
                            FileType Type = ToolChain::FT_Static,
                            bool IsFortran = false) const override;

  const char *getDefaultLinker() const override {
    return "ld.lld";
  }

  Tool *buildLinker() const override {
    return new tools::gnutools::Linker(*this);
  }
  Tool *buildAssembler() const override {
    return new tools::gnutools::Assembler(*this);
  }

  path_list getRuntimePaths() const;

protected:
  std::string getMultiarchTriple(const toolchain::Triple &T) const;
  std::string getMultiarchTriple(const Driver &D,
                                 const toolchain::Triple &TargetTriple,
                                 StringRef SysRoot) const override;
  void addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const override;
  SanitizerMask getSupportedSanitizers() const override;
  void addProfileRTLibs(const toolchain::opt::ArgList &Args,
                             toolchain::opt::ArgStringList &CmdArgs) const override;
  path_list getArchSpecificLibPaths() const override;

private:
  Multilib SelectedMultilib;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OHOS_H
