//===------------------ COFF.cpp - COFF format utilities ------------------===//
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

#include "vm/core/ExecutionEngine/Orc/COFF.h"
#include "vm/core/Object/Binary.h"

#define DEBUG_TYPE "orc"

namespace vm::core::orc {

Expected<bool> COFFImportFileScanner::operator()(object::Archive &A,
                                                 MemoryBufferRef MemberBuf,
                                                 size_t Index) const {
  // Try to build a binary for the member.
  auto Bin = object::createBinary(MemberBuf);
  if (!Bin) {
    // If we can't then consume the error and return false (i.e. not loadable).
    consumeError(Bin.takeError());
    return false;
  }

  // If this is a COFF import file then handle it and return false (not
  // loadable).
  if ((*Bin)->isCOFFImportFile()) {
    ImportedDynamicLibraries.insert((*Bin)->getFileName().str());
    return false;
  }

  // Otherwise the member is loadable (at least as far as COFFImportFileScanner
  // is concerned), so return true;
  return true;
}

} // namespace vm::core::orc
