//===--- Solaris.h - Solaris ToolChain Implementations ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_SOLARIS_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_SOLARIS_H

#include "Gnu.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {

/// Directly call Solaris assembler and linker
namespace solaris {
class LLVM_LIBRARY_VISIBILITY Assembler final : public gnutools::Assembler {
public:
  Assembler(const ToolChain &TC) : gnutools::Assembler(TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

bool isLinkerGnuLd(const ToolChain &TC, const toolchain::opt::ArgList &Args);

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("solaris::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }
  std::string getLinkerPath(const toolchain::opt::ArgList &Args) const;

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace solaris
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY Solaris : public Generic_ELF {
public:
  Solaris(const Driver &D, const toolchain::Triple &Triple,
          const toolchain::opt::ArgList &Args);

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;

  void
  addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                           toolchain::opt::ArgStringList &CC1Args) const override;

  SanitizerMask getSupportedSanitizers() const override;

  const char *getDefaultLinker() const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_SOLARIS_H
