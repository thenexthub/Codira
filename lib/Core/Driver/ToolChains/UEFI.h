//===--- UEFI.h - UEFI ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_UEFI_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_UEFI_H

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core::driver {
namespace tools {
namespace uefi {
class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("uefi::Linker", "lld-link", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace uefi
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY UEFI : public ToolChain {
public:
  UEFI(const Driver &D, const toolchain::Triple &Triple,
       const toolchain::opt::ArgList &Args);

protected:
  Tool *buildLinker() const override;

public:
  bool HasNativeLLVMSupport() const override { return true; }
  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override {
    return UnwindTableLevel::Asynchronous;
  }
  bool isPICDefault() const override { return true; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return true; }

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;

  toolchain::codegenoptions::DebugInfoFormat getDefaultDebugFormat() const override {
    return toolchain::codegenoptions::DIF_CodeView;
  }
};

} // namespace toolchains
} // namespace language::Core::driver

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_UEFI_H
