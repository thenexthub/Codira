//===--- MinGW.h - MinGW ToolChain Implementations --------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MINGW_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MINGW_H

#include "Cuda.h"
#include "Gnu.h"
#include "language/Core/Driver/CudaInstallationDetector.h"
#include "language/Core/Driver/LazyDetector.h"
#include "language/Core/Driver/RocmInstallationDetector.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/Support/ErrorOr.h"

namespace language::Core {
namespace driver {
namespace tools {

/// Directly call GNU Binutils assembler and linker
namespace MinGW {
class LLVM_LIBRARY_VISIBILITY Assembler : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("MinGW::Assemble", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("MinGW::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;

private:
  void AddLibGCC(const toolchain::opt::ArgList &Args,
                 toolchain::opt::ArgStringList &CmdArgs) const;
};
} // end namespace MinGW
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MinGW : public ToolChain {
public:
  MinGW(const Driver &D, const toolchain::Triple &Triple,
        const toolchain::opt::ArgList &Args);

  static void fixTripleArch(const Driver &D, toolchain::Triple &Triple,
                            const toolchain::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override;

  UnwindTableLevel
  getDefaultUnwindTableLevel(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefault() const override;
  bool isPIEDefault(const toolchain::opt::ArgList &Args) const override;
  bool isPICDefaultForced() const override;

  SanitizerMask getSupportedSanitizers() const override;

  toolchain::ExceptionHandling GetExceptionModel(
      const toolchain::opt::ArgList &Args) const override;

  void
  AddClangSystemIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                            toolchain::opt::ArgStringList &CC1Args) const override;
  void
  addClangTargetOptions(const toolchain::opt::ArgList &DriverArgs,
                        toolchain::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;
  void AddClangCXXStdlibIncludeArgs(
      const toolchain::opt::ArgList &DriverArgs,
      toolchain::opt::ArgStringList &CC1Args) const override;

  void AddCudaIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                          toolchain::opt::ArgStringList &CC1Args) const override;
  void AddHIPIncludeArgs(const toolchain::opt::ArgList &DriverArgs,
                         toolchain::opt::ArgStringList &CC1Args) const override;

  void printVerboseInfo(raw_ostream &OS) const override;

  unsigned GetDefaultDwarfVersion() const override { return 4; }

protected:
  Tool *getTool(Action::ActionClass AC) const override;
  Tool *buildLinker() const override;
  Tool *buildAssembler() const override;

private:
  LazyDetector<CudaInstallationDetector> CudaInstallation;
  LazyDetector<RocmInstallationDetector> RocmInstallation;

  std::string Base;
  std::string GccLibDir;
  language::Core::driver::toolchains::Generic_GCC::GCCVersion GccVer;
  std::string Ver;
  std::string SubdirName;
  std::string TripleDirName;
  mutable std::unique_ptr<tools::gcc::Preprocessor> Preprocessor;
  mutable std::unique_ptr<tools::gcc::Compiler> Compiler;
  void findGccLibDir(const toolchain::Triple &LiteralTriple);

  bool NativeLLVMSupport;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_MINGW_H
