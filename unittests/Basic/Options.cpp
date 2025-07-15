//===--- Options.cpp ------------------------------------------------------===//
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

#include "language/Option/Options.h"
#include "toolchain/Option/Option.h"
#include "gtest/gtest.h"

using namespace language;
using namespace language::options;

TEST(Options, outputPathCacheInvariant) {
  auto optTable = createCodiraOptTable();
  // 0 is OP_INVALD
  for (auto id = 1; id < LastOption; ++id) {
    auto opt = optTable->getOption(id);
    // Only check the flag accepted by language-frontend.
    if (!opt.hasFlag(CodiraFlags::FrontendOption))
      continue;

    // The following two options are only accepted by migrator.
    if (id == OPT_emit_migrated_file_path || id == OPT_emit_remap_file_path)
      continue;

    auto name = opt.getName();
    // Check that if a flag matches the convention `-emit-*-path{=}`, it should
    // be a path to an output file and should be marked as cache invariant.
    if (name.starts_with("emit") &&
        (name.ends_with("-path") || name.ends_with("-path=")))
      ASSERT_TRUE(opt.hasFlag(CodiraFlags::CacheInvariant));
  }
}
