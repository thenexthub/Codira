//===-- StrictConcurrencyTests.cpp ------------------------------*- C++ -*-===//
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

#include "FeatureParsingTest.h"

using namespace language;

namespace {

static const FeatureWrapper strictConcurrencyF(Feature::StrictConcurrency);

using StrictConcurrencyTestCase = ArgParsingTestCase<StrictConcurrency>;

class StrictConcurrencyTest
    : public FeatureParsingTest,
      public ::testing::WithParamInterface<StrictConcurrencyTestCase> {};

TEST_P(StrictConcurrencyTest, ) {
  auto &testCase = GetParam();
  parseArgs(testCase.args);
  ASSERT_EQ(getLangOptions().StrictConcurrencyLevel, testCase.expectedResult);
}

// clang-format off
static const StrictConcurrencyTestCase strictConcurrencyTestCases[] = {
  StrictConcurrencyTestCase(
      {},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-language-version", strictConcurrencyF.langMode},
      StrictConcurrency::Complete),
  StrictConcurrencyTestCase(
      {"-enable-upcoming-feature", strictConcurrencyF.name},
      StrictConcurrency::Complete),
  StrictConcurrencyTestCase(
      {"-enable-upcoming-feature", strictConcurrencyF.name + ":migrate"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-experimental-feature", strictConcurrencyF.name},
      StrictConcurrency::Complete),
  StrictConcurrencyTestCase(
      {"-enable-experimental-feature", strictConcurrencyF.name + ":migrate"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-upcoming-feature", strictConcurrencyF.name},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-experimental-feature", strictConcurrencyF.name},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-upcoming-feature", strictConcurrencyF.name + "=minimal"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-experimental-feature", strictConcurrencyF.name + "=minimal"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-upcoming-feature", strictConcurrencyF.name + "=minimal"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-experimental-feature", strictConcurrencyF.name + "=minimal"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-upcoming-feature", strictConcurrencyF.name + "=targeted"},
      StrictConcurrency::Targeted),
  StrictConcurrencyTestCase({
        "-enable-upcoming-feature",
        strictConcurrencyF.name + "=targeted:migrate",
      },
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-experimental-feature", strictConcurrencyF.name + "=targeted"},
      StrictConcurrency::Targeted),
  StrictConcurrencyTestCase({
        "-enable-experimental-feature",
        strictConcurrencyF.name + "=targeted:migrate",
      },
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-upcoming-feature", strictConcurrencyF.name + "=targeted"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-experimental-feature", strictConcurrencyF.name + "=targeted"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-enable-upcoming-feature", strictConcurrencyF.name + "=complete"},
      StrictConcurrency::Complete),
  StrictConcurrencyTestCase(
      {"-enable-experimental-feature", strictConcurrencyF.name + "=complete"},
      StrictConcurrency::Complete),
  StrictConcurrencyTestCase(
      {"-disable-upcoming-feature", strictConcurrencyF.name + "=complete"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase(
      {"-disable-experimental-feature", strictConcurrencyF.name + "=complete"},
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase({
        "-enable-upcoming-feature", strictConcurrencyF.name + "=targeted",
        "-disable-upcoming-feature", strictConcurrencyF.name,
        "-enable-upcoming-feature", strictConcurrencyF.name + "=minimal",
      },
      StrictConcurrency::Minimal),
  StrictConcurrencyTestCase({
        "-enable-upcoming-feature", strictConcurrencyF.name + "=targeted",
        "-enable-upcoming-feature", strictConcurrencyF.name + "=complete",
        "-disable-upcoming-feature", strictConcurrencyF.name,
      },
      StrictConcurrency::Complete), // FIXME?
};
// clang-format on
INSTANTIATE_TEST_SUITE_P(, StrictConcurrencyTest,
                         ::testing::ValuesIn(strictConcurrencyTestCases));

} // end anonymous namespace
