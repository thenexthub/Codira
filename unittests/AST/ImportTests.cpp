//===--- ImportTests.cpp - Tests for representation of imports ------------===//
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

#include "TestContext.h"
#include "language/AST/Import.h"
#include "gtest/gtest.h"

using namespace language;
using namespace language::unittest;

namespace {
/// Helper class used to create ImportPath and hold all strings for identifiers
class ImportPathContext {
  TestContext Ctx;

public:
  ImportPathContext() = default;

  /// Helper routine for building ImportPath
  /// Build()
  /// @see ImportPathBuilder
  inline ImportPath Build(StringRef Name) noexcept {
    return ImportPath::Builder(Ctx.Ctx, Name, '.').copyTo(Ctx.Ctx);
  }
};

} // namespace

TEST(ImportPath, Comparison) {
  ImportPathContext ctx;

  /// Simple soundness check:
  EXPECT_FALSE(ctx.Build("A.B.C") < ctx.Build("A.B.C"));

  /// Check order chain:
  /// A < A.A < A.A.A < A.A.B < A.B < A.B.A < AA < B < B.A
  EXPECT_LT(ctx.Build("A"), ctx.Build("A.A"));
  EXPECT_LT(ctx.Build("A.A"), ctx.Build("A.A.A"));
  EXPECT_LT(ctx.Build("A.A.A"), ctx.Build("A.A.B"));
  EXPECT_LT(ctx.Build("A.A.B"), ctx.Build("A.B"));
  EXPECT_LT(ctx.Build("A.B"), ctx.Build("A.B.A"));
  EXPECT_LT(ctx.Build("A.B.A"), ctx.Build("AA"));
  EXPECT_LT(ctx.Build("B"), ctx.Build("B.A"));

  /// Further ImportPaths are semantically incorrect, but we must
  /// check that comparing them does not cause compiler to crash.
  EXPECT_LT(ctx.Build("."), ctx.Build("A"));
  EXPECT_LT(ctx.Build("A"), ctx.Build("AA."));
  EXPECT_LT(ctx.Build("A"), ctx.Build("AA.."));
  EXPECT_LT(ctx.Build(".A"), ctx.Build("AA"));
}
