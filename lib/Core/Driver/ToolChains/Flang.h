//===--- Flang.h - Flang Tool and ToolChain Implementations ====-*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FLANG_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FLANG_H

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/Action.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/ToolChain.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/Compiler.h"

namespace language::Core {
namespace driver {

namespace tools {

/// Flang compiler tool.
class LLVM_LIBRARY_VISIBILITY Flang : public Tool {
private:
  /// Extract fortran dialect options from the driver arguments and add them to
  /// the list of arguments for the generated command/job.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addFortranDialectOptions(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract preprocessing options from the driver arguments and add them to
  /// the preprocessor command arguments.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addPreprocessingOptions(const toolchain::opt::ArgList &Args,
                               toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract PIC options from the driver arguments and add them to
  /// the command arguments.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addPicOptions(const toolchain::opt::ArgList &Args,
                     toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract target options from the driver arguments and add them to
  /// the command arguments.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addTargetOptions(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for AArch64 target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddAArch64TargetArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for AMDGPU target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddAMDGPUTargetArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for LoongArch64 target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddLoongArch64TargetArgs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for RISC-V target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddRISCVTargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for X86_64 target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddX86_64TargetArgs(const toolchain::opt::ArgList &Args,
                           toolchain::opt::ArgStringList &CmdArgs) const;

  /// Add specific options for PPC target.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void AddPPCTargetArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract offload options from the driver arguments and add them to
  /// the command arguments.
  /// \param [in] C The current compilation for the driver invocation
  /// \param [in] Inputs The input infomration on the current file inputs
  /// \param [in] JA The job action
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addOffloadOptions(Compilation &C, const InputInfoList &Inputs,
                         const JobAction &JA, const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract options for code generation from the driver arguments and add them
  /// to the command arguments.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addCodegenOptions(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;

  /// Extract other compilation options from the driver arguments and add them
  /// to the command arguments.
  ///
  /// \param [in] Args The list of input driver arguments
  /// \param [out] CmdArgs The list of output command arguments
  void addOtherOptions(const toolchain::opt::ArgList &Args,
                       toolchain::opt::ArgStringList &CmdArgs) const;

public:
  Flang(const ToolChain &TC);
  ~Flang() override;

  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedAssembler() const override { return true; }
  bool hasIntegratedCPP() const override { return true; }
  bool canEmitIR() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace tools

} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_FLANG_H
