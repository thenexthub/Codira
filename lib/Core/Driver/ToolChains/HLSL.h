//===--- HLSL.h - HLSL ToolChain Implementations ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HLSL_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HLSL_H

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {

namespace tools {

namespace hlsl {
class LLVM_LIBRARY_VISIBILITY Validator : public Tool {
public:
  Validator(const ToolChain &TC) : Tool("hlsl::Validator", "dxv", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY MetalConverter : public Tool {
public:
  MetalConverter(const ToolChain &TC)
      : Tool("hlsl::MetalConverter", "metal-shaderconverter", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // namespace hlsl
} // namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY HLSLToolChain : public ToolChain {
public:
  HLSLToolChain(const Driver &D, const toolchain::Triple &Triple,
                const toolchain::opt::ArgList &Args);
  Tool *getTool(Action::ActionClass AC) const override;

  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }

  toolchain::opt::DerivedArgList *
  TranslateArgs(const toolchain::opt::DerivedArgList &Args, StringRef BoundArch,
                Action::OffloadKind DeviceOffloadKind) const override;
  static std::optional<std::string> parseTargetProfile(StringRef TargetProfile);
  bool requiresValidation(toolchain::opt::DerivedArgList &Args) const;
  bool requiresBinaryTranslation(toolchain::opt::DerivedArgList &Args) const;
  bool isLastJob(toolchain::opt::DerivedArgList &Args, Action::ActionClass AC) const;

  // Set default DWARF version to 4 for DXIL uses version 4.
  unsigned GetDefaultDwarfVersion() const override { return 4; }

private:
  mutable std::unique_ptr<tools::hlsl::Validator> Validator;
  mutable std::unique_ptr<tools::hlsl::MetalConverter> MetalConverter;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_HLSL_H
