//===- ExtractAPI/APIIgnoresList.h ---------------*- C++ -*-===//
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
///
/// \file This file defines APIIgnoresList which is a type that allows querying
/// files containing symbols to ignore when extracting API information.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_API_IGNORES_LIST_H
#define LANGUAGE_CORE_API_IGNORES_LIST_H

#include "language/Core/Basic/FileManager.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/raw_ostream.h"

#include <memory>
#include <system_error>

namespace toolchain {
class MemoryBuffer;
} // namespace toolchain

namespace language::Core {
namespace extractapi {

struct IgnoresFileNotFound : public toolchain::ErrorInfo<IgnoresFileNotFound> {
  std::string Path;
  static char ID;

  explicit IgnoresFileNotFound(StringRef Path) : Path(Path) {}

  virtual void log(toolchain::raw_ostream &os) const override;

  virtual std::error_code convertToErrorCode() const override;
};

/// A type that provides access to a new line separated list of symbol names to
/// ignore when extracting API information.
struct APIIgnoresList {
  using FilePathList = std::vector<std::string>;

  /// The API to use for generating from the files at \p IgnoresFilePathList.
  ///
  /// \returns an initialized APIIgnoresList or an Error.
  static toolchain::Expected<APIIgnoresList>
  create(const FilePathList &IgnoresFilePathList, FileManager &FM);

  APIIgnoresList() = default;

  /// Check if \p SymbolName is specified in the APIIgnoresList and if it should
  /// therefore be ignored.
  bool shouldIgnore(toolchain::StringRef SymbolName) const;

private:
  using SymbolNameList = toolchain::SmallVector<toolchain::StringRef, 32>;
  using BufferList = toolchain::SmallVector<std::unique_ptr<toolchain::MemoryBuffer>>;

  APIIgnoresList(SymbolNameList SymbolsToIgnore, BufferList Buffers)
      : SymbolsToIgnore(std::move(SymbolsToIgnore)),
        Buffers(std::move(Buffers)) {}

  SymbolNameList SymbolsToIgnore;
  BufferList Buffers;
};

} // namespace extractapi
} // namespace language::Core

#endif // LANGUAGE_CORE_API_IGNORES_LIST_H
