//===--- PatternMatching.h --------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef INDEXSTOREDB_SUPPORT_PATTERNMATCHING_H
#define INDEXSTOREDB_SUPPORT_PATTERNMATCHING_H

#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_Support/Visibility.h>

namespace IndexStoreDB {

INDEXSTOREDB_EXPORT
bool matchesPattern(StringRef Input,
                    StringRef Pattern,
                    bool AnchorStart,
                    bool AnchorEnd,
                    bool Subsequence,
                    bool IgnoreCase);

} // namespace IndexStoreDB

#endif
