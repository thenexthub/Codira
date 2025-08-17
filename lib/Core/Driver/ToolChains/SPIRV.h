//===--- SPIRV.h - SPIR-V Tool Implementations ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_SPIRV_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_SPIRV_H

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace SPIRV {

void constructTranslateCommand(Compilation &C, const Tool &T,
                               const JobAction &JA, const InputInfo &Output,
                               const InputInfo &Input,
                               const toolchain::opt::ArgStringList &Args);

void constructAssembleCommand(Compilation &C, const Tool &T,
                              const JobAction &JA, const InputInfo &Output,
                              const InputInfo &Input,
                              const toolchain::opt::ArgStringList &Args);

class LLVM_LIBRARY_VISIBILITY Translator : public Tool {
public:
  Translator(const ToolChain &TC)
      : Tool("SPIR-V::Translator", "toolchain-spirv", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool hasIntegratedAssembler() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("SPIR-V::Linker", "spirv-link", TC) {}
  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("SPIR-V::Assembler", "spirv-as", TC) {}
  bool hasIntegratedAssembler() const override { return false; }
  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *AssembleOutput) const override;
};

} // namespace SPIRV
} // namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY SPIRVToolChain : public ToolChain {
  mutable std::unique_ptr<Tool> Assembler;

public:
  SPIRVToolChain(const Driver &D, const toolchain::Triple &Triple,
                 const toolchain::opt::ArgList &Args);

  bool useIntegratedAs() const override { return true; }

  bool IsIntegratedBackendDefault() const override { return true; }
  bool IsNonIntegratedBackendSupported() const override { return true; }
  bool IsMathErrnoDefault() const override { return false; }
  bool isCrossCompiling() const override { return true; }
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }
  bool SupportsProfiling() const override { return false; }
  bool HasNativeLLVMSupport() const override;

  language::Core::driver::Tool *SelectTool(const JobAction &JA) const override;

protected:
  language::Core::driver::Tool *getTool(Action::ActionClass AC) const override;
  Tool *buildLinker() const override;

private:
  language::Core::driver::Tool *getAssembler() const;

  bool NativeLLVMSupport;
};

} // namespace toolchains
} // namespace driver
} // namespace language::Core
#endif
