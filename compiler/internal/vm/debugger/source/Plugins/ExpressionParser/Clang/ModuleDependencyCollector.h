//===-- ModuleDependencyCollector.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_MODULEDEPENDENCYCOLLECTOR_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_MODULEDEPENDENCYCOLLECTOR_H

#include "clang/Frontend/Utils.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileCollector.h"

namespace lldb_private {
class ModuleDependencyCollectorAdaptor
    : public clang::ModuleDependencyCollector {
public:
  ModuleDependencyCollectorAdaptor(
      std::shared_ptr<llvm::FileCollectorBase> file_collector)
      : clang::ModuleDependencyCollector("", llvm::vfs::getRealFileSystem()),
        m_file_collector(file_collector) {}

  void addFile(llvm::StringRef Filename,
               llvm::StringRef FileDst = {}) override {
    if (m_file_collector)
      m_file_collector->addFile(Filename);
  }

  bool insertSeen(llvm::StringRef Filename) override { return false; }
  void addFileMapping(llvm::StringRef VPath, llvm::StringRef RPath) override {}
  void writeFileMap() override {}

private:
  std::shared_ptr<llvm::FileCollectorBase> m_file_collector;
};
} // namespace lldb_private

#endif
