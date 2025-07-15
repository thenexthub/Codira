//===--- DiagnosticFormattingTests.cpp ------------------------------------===//
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

#include "language/AST/Attr.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/Basic/SourceManager.h"
#include "toolchain/Support/raw_ostream.h"
#include "gtest/gtest.h"

using namespace language;

namespace {

static void testCase(StringRef formatStr, StringRef resultStr,
                     ArrayRef<DiagnosticArgument> args) {
  SourceManager sourceMgr;
  DiagnosticEngine diags(sourceMgr);

  std::string actualResultStr;
  toolchain::raw_string_ostream os(actualResultStr);

  diags.formatDiagnosticText(os, formatStr, args);

  ASSERT_EQ(actualResultStr, resultStr.str());
}

TEST(DiagnosticFormatting, TypeAttribute) {
  EscapingTypeAttr attr{SourceLoc(), SourceLoc()};
  testCase("%0", "'@escaping'", {&attr});
  testCase("%kind0", "attribute 'escaping'", {&attr});
}

} // end anonymous namespace
