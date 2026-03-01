//===- LLDBTableGenUtils.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILS_TABLEGEN_LLDBTABLEGENUTILS_H
#define LLDB_UTILS_TABLEGEN_LLDBTABLEGENUTILS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include <map>
#include <string>
#include <vector>

namespace llvm {
class RecordKeeper;
class Record;
} // namespace llvm

namespace lldb_private {

/// Map of names to their associated records. This map also ensures that our
/// records are sorted in a deterministic way.
typedef std::map<std::string, std::vector<const llvm::Record *>> RecordsByName;

/// Return records grouped by name.
RecordsByName getRecordsByName(llvm::ArrayRef<const llvm::Record *> Records,
                               llvm::StringRef);

} // namespace lldb_private

#endif
