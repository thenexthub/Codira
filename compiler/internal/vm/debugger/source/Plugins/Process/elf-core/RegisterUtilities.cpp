//===-- RegisterUtilities.cpp ---------------------------------------------===//
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

#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "llvm/ADT/STLExtras.h"
#include <optional>

using namespace lldb_private;

static std::optional<uint32_t>
getNoteType(const llvm::Triple &Triple,
            llvm::ArrayRef<RegsetDesc> RegsetDescs) {
  for (const auto &Entry : RegsetDescs) {
    if (Entry.OS != Triple.getOS())
      continue;
    if (Entry.Arch != llvm::Triple::UnknownArch &&
        Entry.Arch != Triple.getArch())
      continue;
    return Entry.Note;
  }
  return std::nullopt;
}

DataExtractor lldb_private::getRegset(llvm::ArrayRef<CoreNote> Notes,
                                      const llvm::Triple &Triple,
                                      llvm::ArrayRef<RegsetDesc> RegsetDescs) {
  auto TypeOr = getNoteType(Triple, RegsetDescs);
  if (!TypeOr)
    return DataExtractor();
  uint32_t Type = *TypeOr;
  auto Iter = llvm::find_if(
      Notes, [Type](const CoreNote &Note) { return Note.info.n_type == Type; });
  return Iter == Notes.end() ? DataExtractor() : DataExtractor(Iter->data);
}
