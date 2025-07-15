//===--- ConstraintSystemDumpTests.cpp ------------------------------------===//
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

#include "SemaFixture.h"
#include "language/Sema/ConstraintSystem.h"

using namespace language;
using namespace language::unittest;

TEST_F(SemaTest, DumpConstraintSystemBasic) {
  ConstraintSystemOptions options;
  ConstraintSystem cs(DC, options);

  auto *emptyLoc = cs.getConstraintLocator({});

  auto *t0 = cs.createTypeVariable(emptyLoc, TVO_CanBindToLValue);
  auto *t1 = cs.createTypeVariable(emptyLoc, 0);
  auto *t2 = cs.createTypeVariable(
      cs.getConstraintLocator(emptyLoc, ConstraintLocator::GenericArgument),
      TVO_CanBindToHole | TVO_CanBindToPack);

  cs.addUnsolvedConstraint(Constraint::create(
      cs, ConstraintKind::Bind, t2,
      TupleType::get({Type(t0), Type(t1)}, Context), emptyLoc));

  std::string expectedOutput =
      R"(Score: <default 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0>
Type Variables:
  $T0 [can bind to: lvalue] [adjacent to: $T1, $T2] [potential bindings: <none>] @ locator@ []
  $T1 [adjacent to: $T0, $T2] [potential bindings: <none>] @ locator@ []
  $T2 [can bind to: hole, pack] [potential bindings: <none>] @ locator@ [ → generic argument #0]
Inactive Constraints:
  $T2 bind ($T0, $T1) @ locator@ []
)";

  std::string actualOutput;
  {
    toolchain::raw_string_ostream os(actualOutput);
    cs.print(os);

    // Remove locator addresses.
    std::string adjustedOutput;
    const auto size = actualOutput.size();
    size_t pos = 0;
    while (pos < size) {
      auto addr_pos = actualOutput.find("0x", pos);
      if (addr_pos == std::string::npos) {
        adjustedOutput += actualOutput.substr(pos, std::string::npos);
        break;
      } else {
        adjustedOutput += actualOutput.substr(pos, addr_pos - pos);
      }

      pos = actualOutput.find(' ', addr_pos);
    }

    actualOutput = adjustedOutput;
  }

  EXPECT_EQ(expectedOutput, actualOutput);
}
