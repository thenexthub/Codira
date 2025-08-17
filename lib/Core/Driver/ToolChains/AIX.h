//===--- AIX.h - AIX ToolChain Implementations ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AIX_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AIX_H

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"

namespace language::Core {
namespace driver {
namespace tools {

/// Directly call system default assembler and linker.
namespace aix {

class LLVM_LIBRARY_VISIBILITY Assembler final : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("aix::Assembler", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("aix::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace aix

} // end namespace tools
} // end namespace driver
} // end namespace language::Core

namespace language::Core {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY AIX : public ToolChain {
public:
  AIX(const Driver &D, const toolchain::Triple &Triple,
      const toolchain::opt::ArgList &Args);

  bool parseInlineAsmUsingAsmParser() const override {
    return ParseInlineAsmUsingAsmParser;
  }
  bool isPICDefault() const override { return true; }
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return true; }
  bool HasNativeLLVMSupport() const override { return true; }

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;

  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void AddCXXStdlibLibArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const override;

  void addClangTargetOptions(
      const toolchain::opt::ArgList &Args, toolchain::opt::ArgStringList &CC1Args,
      Action::OffloadKind DeviceOffloadingKind) const override;

  void addProfileRTLibs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const override;

  CXXStdlibType GetDefaultCXXStdlibType() const override;

  RuntimeLibType GetDefaultRuntimeLibType() const override;

  // Set default DWARF version to 3 for now as latest AIX OS supports version 3.
  unsigned GetDefaultDwarfVersion() const override { return 3; }

  toolchain::DebuggerKind getDefaultDebuggerTuning() const override {
    return toolchain::DebuggerKind::DBX;
  }

  path_list getArchSpecificLibPaths() const override { return path_list(); };

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

private:
  toolchain::StringRef GetHeaderSysroot(const toolchain::opt::ArgList &DriverArgs) const;
  bool ParseInlineAsmUsingAsmParser;
  void AddOpenMPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_AIX_H
