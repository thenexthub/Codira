//===- Archive.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_ARCHIVE_H
#define LLVM_LIB_OBJCOPY_ARCHIVE_H

#include "vm/core/Object/ArchiveWriter.h"
#include "vm/core/Support/Error.h"
#include <vector>

namespace vm::core {
namespace objcopy {

class MultiFormatConfig;

/// Applies the transformations described by \p Config to
/// each member in archive \p Ar.
/// \returns Vector of transformed archive members.
Expected<std::vector<NewArchiveMember>>
createNewArchiveMembers(const MultiFormatConfig &Config,
                        const object::Archive &Ar);

} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_ARCHIVE_H
