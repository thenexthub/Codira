//===--- FileTypes.cpp - for language/Basic/FileTypes.h ----------------------===//
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

#include "language/Basic/FileTypes.h"
#include "gtest/gtest.h"

namespace {
using namespace language;
using namespace language::file_types;

static const std::vector<std::pair<std::string, ID>> ExtIDs = {
#define TYPE(NAME, ID, EXTENSION, FLAGS) {EXTENSION, TY_##ID},
#include "language/Basic/FileTypes.def"
};

TEST(FileSystem, lookupTypeFromFilename) {
  for (auto &Entry: ExtIDs) {
    // no extension, skip.
    if (Entry.first.empty())
      continue;
    // raw-sil, raw-sib, lowered-sil, and raw-toolchain-ir do not have unique
    // extensions.
    if (Entry.second == TY_RawSIL || Entry.second == TY_RawSIB ||
        Entry.second == TY_LoweredSIL || Entry.second == TY_RawTOOLCHAIN_IR)
      continue;

    std::string Filename = "Myfile." + Entry.first;
    ID Type = lookupTypeFromFilename(Filename);
    ASSERT_EQ(getTypeName(Type), getTypeName(Entry.second));
  }

  ASSERT_EQ(lookupTypeFromFilename(""), TY_INVALID);
  ASSERT_EQ(lookupTypeFromFilename("."), TY_INVALID);
  ASSERT_EQ(lookupTypeFromFilename(".."), TY_INVALID);
}

} // namespace
