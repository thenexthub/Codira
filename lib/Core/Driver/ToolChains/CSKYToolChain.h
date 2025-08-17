//===--- CSKYToolchain.h - CSKY ToolChain Implementations -----*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CSKYTOOLCHAIN_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CSKYTOOLCHAIN_H

#include "Gnu.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY CSKYToolChain : public Generic_ELF {
public:
  CSKYToolChain(const Driver &D, const toolchain::Triple &Triple,
                const toolchain::opt::ArgList &Args);

  void addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args,
                             Action::OffloadKind) const override;
  RuntimeLibType GetDefaultRuntimeLibType() const override;
  UnwindLibType GetUnwindLibType(const toolchain::opt::ArgList &Args) const override;
  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;

protected:
  Tool *buildLinker() const override;

private:
  std::string computeSysRoot() const override;
};

} // end namespace toolchains

namespace tools {
namespace CSKY {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("CSKY::Linker", "ld", TC) {}
  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace CSKY
} // end namespace tools

} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CSKYTOOLCHAIN_H
