//===--- Clang.h - Clang Tool and ToolChain Implementations ====-*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CLANG_H
#define LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CLANG_H

#include "MSVC.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/Types.h"
#include "toolchain/Frontend/Debug/Options.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
class ObjCRuntime;
namespace driver {

namespace tools {

/// Clang compiler tool.
class LLVM_LIBRARY_VISIBILITY Clang : public Tool {
  // Indicates whether this instance has integrated backend using
  // internal LLVM infrastructure.
  bool HasBackend;

public:
  static const char *getBaseInputName(const toolchain::opt::ArgList &Args,
                                      const InputInfo &Input);
  static const char *getBaseInputStem(const toolchain::opt::ArgList &Args,
                                      const InputInfoList &Inputs);
  static const char *getDependencyFileName(const toolchain::opt::ArgList &Args,
                                           const InputInfoList &Inputs);

private:
  void AddPreprocessingOptions(Compilation &C, const JobAction &JA,
                               const Driver &D, const toolchain::opt::ArgList &Args,
                               toolchain::opt::ArgStringList &CmdArgs,
                               const InputInfo &Output,
                               const InputInfoList &Inputs) const;

  void RenderTargetOptions(const toolchain::Triple &EffectiveTriple,
                           const toolchain::opt::ArgList &Args, bool KernelOrKext,
                           toolchain::opt::ArgStringList &CmdArgs) const;

  void AddAArch64TargetArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const;
  void AddARMTargetArgs(const toolchain::Triple &Triple,
                        const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs,
                        bool KernelOrKext) const;
  void AddARM64TargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;
  void AddLoongArchTargetArgs(const toolchain::opt::ArgList &Args,
                              toolchain::opt::ArgStringList &CmdArgs) const;
  void AddMIPSTargetArgs(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;
  void AddPPCTargetArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const;
  void AddR600TargetArgs(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;
  void AddRISCVTargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;
  void AddSparcTargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;
  void AddSystemZTargetArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const;
  void AddX86TargetArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const;
  void AddHexagonTargetArgs(const toolchain::opt::ArgList &Args,
                            toolchain::opt::ArgStringList &CmdArgs) const;
  void AddLanaiTargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;
  void AddWebAssemblyTargetArgs(const toolchain::opt::ArgList &Args,
                                toolchain::opt::ArgStringList &CmdArgs) const;
  void AddVETargetArgs(const toolchain::opt::ArgList &Args,
                       toolchain::opt::ArgStringList &CmdArgs) const;

  enum RewriteKind { RK_None, RK_Fragile, RK_NonFragile };

  ObjCRuntime AddObjCRuntimeArgs(const toolchain::opt::ArgList &args,
                                 const InputInfoList &inputs,
                                 toolchain::opt::ArgStringList &cmdArgs,
                                 RewriteKind rewrite) const;

  void AddClangCLArgs(const toolchain::opt::ArgList &Args, types::ID InputType,
                      toolchain::opt::ArgStringList &CmdArgs) const;

  mutable std::unique_ptr<toolchain::raw_fd_ostream> CompilationDatabase = nullptr;
  void DumpCompilationDatabase(Compilation &C, StringRef Filename,
                               StringRef Target,
                               const InputInfo &Output, const InputInfo &Input,
                               const toolchain::opt::ArgList &Args) const;

  void DumpCompilationDatabaseFragmentToDir(
      StringRef Dir, Compilation &C, StringRef Target, const InputInfo &Output,
      const InputInfo &Input, const toolchain::opt::ArgList &Args) const;

public:
  Clang(const ToolChain &TC, bool HasIntegratedBackend = true);
  ~Clang() override;

  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedAssembler() const override { return true; }
  bool hasIntegratedBackend() const override { return HasBackend; }
  bool hasIntegratedCPP() const override { return true; }
  bool canEmitIR() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

/// Clang integrated assembler tool.
class LLVM_LIBRARY_VISIBILITY ClangAs : public Tool {
public:
  ClangAs(const ToolChain &TC)
      : Tool("language::Core::as", "clang integrated assembler", TC) {}
  void AddLoongArchTargetArgs(const toolchain::opt::ArgList &Args,
                              toolchain::opt::ArgStringList &CmdArgs) const;
  void AddMIPSTargetArgs(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &CmdArgs) const;
  void AddX86TargetArgs(const toolchain::opt::ArgList &Args,
                        toolchain::opt::ArgStringList &CmdArgs) const;
  void AddRISCVTargetArgs(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &CmdArgs) const;
  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedAssembler() const override { return false; }
  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

/// Offload bundler tool.
class LLVM_LIBRARY_VISIBILITY OffloadBundler final : public Tool {
public:
  OffloadBundler(const ToolChain &TC)
      : Tool("offload bundler", "clang-offload-bundler", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
  void ConstructJobMultipleOutputs(Compilation &C, const JobAction &JA,
                                   const InputInfoList &Outputs,
                                   const InputInfoList &Inputs,
                                   const toolchain::opt::ArgList &TCArgs,
                                   const char *LinkingOutput) const override;
};

/// Offload binary tool.
class LLVM_LIBRARY_VISIBILITY OffloadPackager final : public Tool {
public:
  OffloadPackager(const ToolChain &TC)
      : Tool("Offload::Packager", "clang-offload-packager", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

/// Linker wrapper tool.
class LLVM_LIBRARY_VISIBILITY LinkerWrapper final : public Tool {
  const Tool *Linker;

public:
  LinkerWrapper(const ToolChain &TC, const Tool *Linker)
      : Tool("Offload::Linker", "linker", TC), Linker(Linker) {}

  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const toolchain::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

enum class DwarfFissionKind { None, Split, Single };

DwarfFissionKind getDebugFissionKind(const Driver &D,
                                     const toolchain::opt::ArgList &Args,
                                     toolchain::opt::Arg *&Arg);

// Calculate the output path of the module file when compiling a module unit
// with the `-fmodule-output` option or `-fmodule-output=` option specified.
// The behavior is:
// - If `-fmodule-output=` is specfied, then the module file is
//   writing to the value.
// - Otherwise if the output object file of the module unit is specified, the
// output path
//   of the module file should be the same with the output object file except
//   the corresponding suffix. This requires both `-o` and `-c` are specified.
// - Otherwise, the output path of the module file will be the same with the
//   input with the corresponding suffix.
toolchain::SmallString<256>
getCXX20NamedModuleOutputPath(const toolchain::opt::ArgList &Args,
                              const char *BaseInput);

} // end namespace tools

} // end namespace driver
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_DRIVER_TOOLCHAINS_CLANG_H
