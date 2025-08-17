//===- ExpandResponseFileCompilationDataBase.cpp --------------------------===//
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
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/ConvertUTF.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace tooling {
namespace {

class ExpandResponseFilesDatabase : public CompilationDatabase {
public:
  ExpandResponseFilesDatabase(
      std::unique_ptr<CompilationDatabase> Base,
      toolchain::cl::TokenizerCallback Tokenizer,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS)
      : Base(std::move(Base)), Tokenizer(Tokenizer), FS(std::move(FS)) {
    assert(this->Base != nullptr);
    assert(this->Tokenizer != nullptr);
    assert(this->FS != nullptr);
  }

  std::vector<std::string> getAllFiles() const override {
    return Base->getAllFiles();
  }

  std::vector<CompileCommand>
  getCompileCommands(StringRef FilePath) const override {
    return expand(Base->getCompileCommands(FilePath));
  }

  std::vector<CompileCommand> getAllCompileCommands() const override {
    return expand(Base->getAllCompileCommands());
  }

private:
  std::vector<CompileCommand> expand(std::vector<CompileCommand> Cmds) const {
    for (auto &Cmd : Cmds)
      tooling::addExpandedResponseFiles(Cmd.CommandLine, Cmd.Directory,
                                        Tokenizer, *FS);
    return Cmds;
  }

private:
  std::unique_ptr<CompilationDatabase> Base;
  toolchain::cl::TokenizerCallback Tokenizer;
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS;
};

} // namespace

std::unique_ptr<CompilationDatabase>
expandResponseFiles(std::unique_ptr<CompilationDatabase> Base,
                    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS) {
  auto Tokenizer = toolchain::Triple(toolchain::sys::getProcessTriple()).isOSWindows()
                       ? toolchain::cl::TokenizeWindowsCommandLine
                       : toolchain::cl::TokenizeGNUCommandLine;
  return std::make_unique<ExpandResponseFilesDatabase>(
      std::move(Base), Tokenizer, std::move(FS));
}

} // namespace tooling
} // namespace language::Core
