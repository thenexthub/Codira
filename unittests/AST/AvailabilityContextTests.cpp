//===--- AvailabilityContextTests.cpp - Tests for AvailabilityContext -----===//
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
#include "language/AST/AvailabilityContext.h"
#include "toolchain/TargetParser/Triple.h"
#include "gtest/gtest.h"

using namespace language;
using namespace language::unittest;

static toolchain::VersionTuple getPlatformIntro(const AvailabilityContext &context) {
  return context.getPlatformRange().getRawVersionRange().getLowerEndpoint();
}

static AvailabilityRange getAvailabilityRange(unsigned major, unsigned minor) {
  return AvailabilityRange(toolchain::VersionTuple(major, minor));
}

class AvailabilityContextTest : public ::testing::Test {
public:
  const TestContext defaultTestContext{
      toolchain::Triple("x86_64", "apple", "macosx10.9")};

  struct {
    const AvailabilityDomain universal = AvailabilityDomain::forUniversal();
    const AvailabilityDomain macOS =
        AvailabilityDomain::forPlatform(PlatformKind::macOS);
    const AvailabilityDomain macOSAppExt = AvailabilityDomain::forPlatform(
        PlatformKind::macOSApplicationExtension);
  } domains;
};

TEST_F(AvailabilityContextTest, PlatformRange) {
  auto &ctx = defaultTestContext.Ctx;

  auto macOS10_9 = AvailabilityContext::forDeploymentTarget(ctx);
  EXPECT_EQ(getPlatformIntro(macOS10_9), toolchain::VersionTuple(10, 9));

  // Attempt to constrain the macOS version to >= 10.8. Since the context is
  // already >= 10.9, this should have no effect.
  auto macOS10_8 = macOS10_9;
  macOS10_8.constrainWithPlatformRange(getAvailabilityRange(10, 8), ctx);
  EXPECT_EQ(macOS10_8, macOS10_9);

  // Attempt to constrain the macOS version to >= 10.9. Since the context is
  // already >= 10.9, this should have no effect.
  auto stillMacOS10_9 = macOS10_9;
  stillMacOS10_9.constrainWithPlatformRange(getAvailabilityRange(10, 9), ctx);
  EXPECT_EQ(stillMacOS10_9, macOS10_9);

  // Constrain the macOS version to >= 10.10 instead. The resulting context
  // should be a new context that is less available than the deployment target
  // context (a.k.a. "contained by").
  auto macOS10_10 = macOS10_9;
  macOS10_10.constrainWithPlatformRange(getAvailabilityRange(10, 10), ctx);
  EXPECT_EQ(getPlatformIntro(macOS10_10), toolchain::VersionTuple(10, 10));
  EXPECT_NE(macOS10_9, macOS10_10);
  EXPECT_TRUE(macOS10_10.isContainedIn(macOS10_9));
  EXPECT_FALSE(macOS10_9.isContainedIn(macOS10_10));
}

TEST_F(AvailabilityContextTest, UnavailableDomains) {
  auto &ctx = defaultTestContext.Ctx;

  auto macOS10_9 = AvailabilityContext::forDeploymentTarget(ctx);
  EXPECT_FALSE(macOS10_9.isUnavailable());
  EXPECT_FALSE(macOS10_9.containsUnavailableDomain(domains.macOS));
  EXPECT_FALSE(macOS10_9.containsUnavailableDomain(domains.macOSAppExt));
  EXPECT_FALSE(macOS10_9.containsUnavailableDomain(domains.universal));

  // Constrain the deployment target context by adding unavailability on macOS.
  // The resulting context should be a new context that is less available than
  // the deployment target context (a.k.a. "contained by").
  auto unavailableOnMacOS = macOS10_9;
  unavailableOnMacOS.constrainWithUnavailableDomain(domains.macOS, ctx);
  EXPECT_TRUE(unavailableOnMacOS.isUnavailable());
  EXPECT_TRUE(unavailableOnMacOS.containsUnavailableDomain(domains.macOS));
  EXPECT_TRUE(
      unavailableOnMacOS.containsUnavailableDomain(domains.macOSAppExt));
  EXPECT_FALSE(unavailableOnMacOS.containsUnavailableDomain(domains.universal));
  EXPECT_NE(unavailableOnMacOS, macOS10_9);
  EXPECT_TRUE(unavailableOnMacOS.isContainedIn(macOS10_9));
  EXPECT_FALSE(macOS10_9.isContainedIn(unavailableOnMacOS));

  // Constraining a context that is already unavailable on macOS by adding
  // unavailability on macOS should have no effect.
  auto stillUnavailableOnMacOS = unavailableOnMacOS;
  stillUnavailableOnMacOS.constrainWithUnavailableDomain(domains.macOS, ctx);
  EXPECT_EQ(unavailableOnMacOS, stillUnavailableOnMacOS);

  // Constraining unavailability on macOS application extensions should also
  // have no effect.
  auto unavailableInAppExt = unavailableOnMacOS;
  unavailableInAppExt.constrainWithUnavailableDomain(domains.macOSAppExt, ctx);
  EXPECT_EQ(unavailableOnMacOS, unavailableInAppExt);

  // FIXME: [availability] Test adding unavailability for an independent domain.

  // Constraining the context to be universally unavailable should create a
  // new context that contains the context that was unavailable on macOS only.
  auto unavailableUniversally = unavailableOnMacOS;
  unavailableUniversally.constrainWithUnavailableDomain(domains.universal, ctx);
  EXPECT_TRUE(unavailableUniversally.isUnavailable());
  EXPECT_TRUE(unavailableUniversally.containsUnavailableDomain(domains.macOS));
  EXPECT_TRUE(
      unavailableUniversally.containsUnavailableDomain(domains.macOSAppExt));
  EXPECT_TRUE(
      unavailableUniversally.containsUnavailableDomain(domains.universal));
  EXPECT_NE(unavailableUniversally, unavailableOnMacOS);
  EXPECT_TRUE(unavailableUniversally.isContainedIn(unavailableOnMacOS));
  EXPECT_TRUE(unavailableUniversally.isContainedIn(macOS10_9));
  EXPECT_FALSE(unavailableOnMacOS.isContainedIn(unavailableUniversally));
  EXPECT_FALSE(macOS10_9.isContainedIn(unavailableUniversally));
}
