//===- GuessTargetAndModeCompilationDatabase.cpp --------------------------===//
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

#include "language/Core/Tooling/CompilationDatabase.h"
#include "language/Core/Tooling/Tooling.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Program.h"
#include <memory>

namespace language::Core {
namespace tooling {

namespace {
class LocationAdderDatabase : public CompilationDatabase {
public:
  LocationAdderDatabase(std::unique_ptr<CompilationDatabase> Base)
      : Base(std::move(Base)) {
    assert(this->Base != nullptr);
  }

  std::vector<std::string> getAllFiles() const override {
    return Base->getAllFiles();
  }

  std::vector<CompileCommand> getAllCompileCommands() const override {
    return addLocation(Base->getAllCompileCommands());
  }

  std::vector<CompileCommand>
  getCompileCommands(StringRef FilePath) const override {
    return addLocation(Base->getCompileCommands(FilePath));
  }

private:
  std::vector<CompileCommand>
  addLocation(std::vector<CompileCommand> Cmds) const {
    for (auto &Cmd : Cmds) {
      if (Cmd.CommandLine.empty())
        continue;
      std::string &Driver = Cmd.CommandLine.front();
      // If the driver name already is absolute, we don't need to do anything.
      if (toolchain::sys::path::is_absolute(Driver))
        continue;
      // If the name is a relative path, like bin/clang, we assume it's
      // possible to resolve it and don't do anything about it either.
      if (toolchain::any_of(Driver,
                       [](char C) { return toolchain::sys::path::is_separator(C); }))
        continue;
      auto Absolute = toolchain::sys::findProgramByName(Driver);
      // If we found it in path, update the entry in Cmd.CommandLine
      if (Absolute && toolchain::sys::path::is_absolute(*Absolute))
        Driver = std::move(*Absolute);
    }
    return Cmds;
  }
  std::unique_ptr<CompilationDatabase> Base;
};
} // namespace

std::unique_ptr<CompilationDatabase>
inferToolLocation(std::unique_ptr<CompilationDatabase> Base) {
  return std::make_unique<LocationAdderDatabase>(std::move(Base));
}

} // namespace tooling
} // namespace language::Core
