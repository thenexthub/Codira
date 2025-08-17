//===- ExtractAPI/APIIgnoresList.cpp -------*- C++ -*-===//
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
/// \file
/// This file implements APIIgnoresList that allows users to specifiy a file
/// containing symbols to ignore during API extraction.
///
//===----------------------------------------------------------------------===//

#include "language/Core/ExtractAPI/APIIgnoresList.h"
#include "language/Core/Basic/FileManager.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/Error.h"

using namespace language::Core;
using namespace language::Core::extractapi;
using namespace toolchain;

char IgnoresFileNotFound::ID;

void IgnoresFileNotFound::log(toolchain::raw_ostream &os) const {
  os << "Could not find API ignores file " << Path;
}

std::error_code IgnoresFileNotFound::convertToErrorCode() const {
  return toolchain::inconvertibleErrorCode();
}

Expected<APIIgnoresList>
APIIgnoresList::create(const FilePathList &IgnoresFilePathList,
                       FileManager &FM) {
  SmallVector<StringRef, 32> Lines;
  BufferList symbolBufferList;

  for (const auto &CurrentIgnoresFilePath : IgnoresFilePathList) {
    auto BufferOrErr = FM.getBufferForFile(CurrentIgnoresFilePath);

    if (!BufferOrErr)
      return make_error<IgnoresFileNotFound>(CurrentIgnoresFilePath);

    auto Buffer = std::move(BufferOrErr.get());
    Buffer->getBuffer().split(Lines, '\n', /*MaxSplit*/ -1,
                              /*KeepEmpty*/ false);
    symbolBufferList.push_back(std::move(Buffer));
  }

  // Symbol names don't have spaces in them, let's just remove these in case
  // the input is slighlty malformed.
  transform(Lines, Lines.begin(), [](StringRef Line) { return Line.trim(); });
  sort(Lines);
  return APIIgnoresList(std::move(Lines), std::move(symbolBufferList));
}

bool APIIgnoresList::shouldIgnore(StringRef SymbolName) const {
  auto It = lower_bound(SymbolsToIgnore, SymbolName);
  return (It != SymbolsToIgnore.end()) && (*It == SymbolName);
}
