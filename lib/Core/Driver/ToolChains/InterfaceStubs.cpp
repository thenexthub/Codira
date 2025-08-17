//===---  InterfaceStubs.cpp - Base InterfaceStubs Implementations C++  ---===//
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

#include "InterfaceStubs.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "toolchain/Support/Path.h"

namespace language::Core {
namespace driver {
namespace tools {
namespace ifstool {
void Merger::ConstructJob(Compilation &C, const JobAction &JA,
                          const InputInfo &Output, const InputInfoList &Inputs,
                          const toolchain::opt::ArgList &Args,
                          const char *LinkingOutput) const {
  std::string Merger = getToolChain().GetProgramPath(getShortName());
  // TODO: Use IFS library directly in the future.
  toolchain::opt::ArgStringList CmdArgs;
  CmdArgs.push_back("--input-format=IFS");
  const bool WriteBin = !Args.getLastArg(options::OPT_emit_merged_ifs);
  CmdArgs.push_back(WriteBin ? "--output-format=ELF" : "--output-format=IFS");
  CmdArgs.push_back("-o");

  // Normally we want to write to a side-car file ending in ".ifso" so for
  // example if `clang -emit-interface-stubs -shared -o libhello.so` were
  // invoked then we would like to get libhello.so and libhello.ifso. If the
  // stdout stream is given as the output file (ie `-o -`), that is the one
  // exception where we will just append to the same filestream as the normal
  // output.
  SmallString<128> OutputFilename(Output.getFilename());
  if (OutputFilename != "-") {
    if (Args.hasArg(options::OPT_shared))
      toolchain::sys::path::replace_extension(OutputFilename,
                                         (WriteBin ? "ifso" : "ifs"));
    else
      OutputFilename += (WriteBin ? ".ifso" : ".ifs");
  }

  CmdArgs.push_back(Args.MakeArgString(OutputFilename.c_str()));

  // Here we append the input files. If the input files are object files, then
  // we look for .ifs files present in the same location as the object files.
  for (const auto &Input : Inputs) {
    if (!Input.isFilename())
      continue;
    SmallString<128> InputFilename(Input.getFilename());
    if (Input.getType() == types::TY_Object)
      toolchain::sys::path::replace_extension(InputFilename, ".ifs");
    CmdArgs.push_back(Args.MakeArgString(InputFilename.c_str()));
  }

  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Args.MakeArgString(Merger), CmdArgs,
                                         Inputs, Output));
}
} // namespace ifstool
} // namespace tools
} // namespace driver
} // namespace language::Core
