//===--- TargetExecutionUtils.cpp - Execution utils for target processes --===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ExecutionEngine/Orc/TargetProcess/TargetExecutionUtils.h"

#include <vector>

namespace vm::core {
namespace orc {

int runAsMain(int (*Main)(int, char *[]), ArrayRef<std::string> Args,
              std::optional<StringRef> ProgramName) {
  std::vector<std::unique_ptr<char[]>> ArgVStorage;
  std::vector<char *> ArgV;

  ArgVStorage.reserve(Args.size() + (ProgramName ? 1 : 0));
  ArgV.reserve(Args.size() + 1 + (ProgramName ? 1 : 0));

  if (ProgramName) {
    ArgVStorage.push_back(std::make_unique<char[]>(ProgramName->size() + 1));
    toolchain::copy(*ProgramName, &ArgVStorage.back()[0]);
    ArgVStorage.back()[ProgramName->size()] = '\0';
    ArgV.push_back(ArgVStorage.back().get());
  }

  for (const auto &Arg : Args) {
    ArgVStorage.push_back(std::make_unique<char[]>(Arg.size() + 1));
    toolchain::copy(Arg, &ArgVStorage.back()[0]);
    ArgVStorage.back()[Arg.size()] = '\0';
    ArgV.push_back(ArgVStorage.back().get());
  }
  ArgV.push_back(nullptr);

  return Main(Args.size() + !!ProgramName, ArgV.data());
}

int runAsVoidFunction(int (*Func)(void)) { return Func(); }

int runAsIntFunction(int (*Func)(int), int Arg) { return Func(Arg); }

} // End namespace orc.
} // End namespace vm::core.
