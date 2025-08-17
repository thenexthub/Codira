//===--- SPIRV.cpp - SPIR-V Tool Implementations ----------------*- C++ -*-===//
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
#include "SPIRV.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/InputInfo.h"
#include "language/Core/Driver/Options.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core::driver::tools;
using namespace toolchain::opt;

void SPIRV::constructTranslateCommand(Compilation &C, const Tool &T,
                                      const JobAction &JA,
                                      const InputInfo &Output,
                                      const InputInfo &Input,
                                      const toolchain::opt::ArgStringList &Args) {
  toolchain::opt::ArgStringList CmdArgs(Args);
  CmdArgs.push_back(Input.getFilename());

  assert(Input.getType() != types::TY_PP_Asm && "Unexpected input type");

  if (Output.getType() == types::TY_PP_Asm)
    CmdArgs.push_back("--spirv-tools-dis");

  CmdArgs.append({"-o", Output.getFilename()});

  // Try to find "toolchain-spirv-<LLVM_VERSION_MAJOR>". Otherwise, fall back to
  // plain "toolchain-spirv".
  using namespace std::string_literals;
  auto VersionedTool = "toolchain-spirv-"s + std::to_string(LLVM_VERSION_MAJOR);
  std::string ExeCand = T.getToolChain().GetProgramPath(VersionedTool.c_str());
  if (!toolchain::sys::fs::can_execute(ExeCand))
    ExeCand = T.getToolChain().GetProgramPath("toolchain-spirv");

  const char *Exec = C.getArgs().MakeArgString(ExeCand);
  C.addCommand(std::make_unique<Command>(JA, T, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Input, Output));
}

void SPIRV::constructAssembleCommand(Compilation &C, const Tool &T,
                                     const JobAction &JA,
                                     const InputInfo &Output,
                                     const InputInfo &Input,
                                     const toolchain::opt::ArgStringList &Args) {
  toolchain::opt::ArgStringList CmdArgs(Args);
  CmdArgs.push_back(Input.getFilename());

  assert(Input.getType() == types::TY_PP_Asm && "Unexpected input type");

  CmdArgs.append({"-o", Output.getFilename()});

  // Try to find "spirv-as-<LLVM_VERSION_MAJOR>". Otherwise, fall back to
  // plain "spirv-as".
  using namespace std::string_literals;
  auto VersionedTool = "spirv-as-"s + std::to_string(LLVM_VERSION_MAJOR);
  std::string ExeCand = T.getToolChain().GetProgramPath(VersionedTool.c_str());
  if (!toolchain::sys::fs::can_execute(ExeCand))
    ExeCand = T.getToolChain().GetProgramPath("spirv-as");

  const char *Exec = C.getArgs().MakeArgString(ExeCand);
  C.addCommand(std::make_unique<Command>(JA, T, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Input, Output));
}

void SPIRV::Translator::ConstructJob(Compilation &C, const JobAction &JA,
                                     const InputInfo &Output,
                                     const InputInfoList &Inputs,
                                     const ArgList &Args,
                                     const char *LinkingOutput) const {
  claimNoWarnArgs(Args);
  if (Inputs.size() != 1)
    toolchain_unreachable("Invalid number of input files.");
  constructTranslateCommand(C, *this, JA, Output, Inputs[0], {});
}

void SPIRV::Assembler::ConstructJob(Compilation &C, const JobAction &JA,
                                    const InputInfo &Output,
                                    const InputInfoList &Inputs,
                                    const ArgList &Args,
                                    const char *AssembleOutput) const {
  claimNoWarnArgs(Args);
  if (Inputs.size() != 1)
    toolchain_unreachable("Invalid number of input files.");
  constructAssembleCommand(C, *this, JA, Output, Inputs[0], {});
}

language::Core::driver::Tool *SPIRVToolChain::getAssembler() const {
  if (!Assembler)
    Assembler = std::make_unique<SPIRV::Assembler>(*this);
  return Assembler.get();
}

language::Core::driver::Tool *SPIRVToolChain::SelectTool(const JobAction &JA) const {
  Action::ActionClass AC = JA.getKind();
  return SPIRVToolChain::getTool(AC);
}

language::Core::driver::Tool *SPIRVToolChain::getTool(Action::ActionClass AC) const {
  switch (AC) {
  default:
    break;
  case Action::AssembleJobClass:
    return SPIRVToolChain::getAssembler();
  }
  return ToolChain::getTool(AC);
}
language::Core::driver::Tool *SPIRVToolChain::buildLinker() const {
  return new tools::SPIRV::Linker(*this);
}

void SPIRV::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const ToolChain &ToolChain = getToolChain();
  std::string Linker = ToolChain.GetProgramPath(getShortName());
  ArgStringList CmdArgs;
  AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  // Use of --sycl-link will call the clang-sycl-linker instead of
  // the default linker (spirv-link).
  if (Args.hasArg(options::OPT_sycl_link))
    Linker = ToolChain.GetProgramPath("clang-sycl-linker");
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Args.MakeArgString(Linker), CmdArgs,
                                         Inputs, Output));
}

SPIRVToolChain::SPIRVToolChain(const Driver &D, const toolchain::Triple &Triple,
                               const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // TODO: Revisit need/use of --sycl-link option once SYCL toolchain is
  // available and SYCL linking support is moved there.
  NativeLLVMSupport = Args.hasArg(options::OPT_sycl_link);
}

bool SPIRVToolChain::HasNativeLLVMSupport() const { return NativeLLVMSupport; }
