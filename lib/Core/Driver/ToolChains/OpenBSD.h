//===--- OpenBSD.h - OpenBSD ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OPENBSD_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OPENBSD_H

#include "Gnu.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {

/// Directly call GNU Binutils assembler and linker
namespace openbsd {
class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC)
      : Tool("openbsd::Assembler", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("openbsd::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace openbsd
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY OpenBSD : public Generic_ELF {
public:
  OpenBSD(const Driver &D, const toolchain::Triple &Triple,
          const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override;

  bool IsMathErrnoDefault() const override { return false; }
  bool IsObjCNonFragileABIDefault() const override { return true; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return true;
  }

  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return ToolChain::RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;

  void addLibCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                             toolchain::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  std::string getCompilerRT(const toolchain::opt::ArgList &Args, StringRef Component,
                            FileType Type = ToolChain::FT_Static,
                            bool IsFortran = false) const override;

  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override;

  LangOptions::StackProtectorMode
  GetDefaultStackProtectorLevel(bool KernelOrKext) const override {
    return LangOptions::SSPStrong;
  }
  unsigned GetDefaultDwarfVersion() const override { return 2; }

  SanitizerMask getSupportedSanitizers() const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_OPENBSD_H
