//===-- UnwindPlanTest.cpp ------------------------------------------------===//
//
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

#include "lldb/Symbol/UnwindPlan.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

static UnwindPlan::Row make_simple_row(addr_t offset, uint64_t cfa_value) {
  UnwindPlan::Row row;
  row.SetOffset(offset);
  row.GetCFAValue().SetIsConstant(cfa_value);
  return row;
}

TEST(UnwindPlan, InsertRow) {
  UnwindPlan::Row row1 = make_simple_row(0, 42);
  UnwindPlan::Row row2 = make_simple_row(0, 47);
  UnwindPlan::Row row3 = make_simple_row(-1, 4242);

  UnwindPlan plan(eRegisterKindGeneric);
  plan.InsertRow(row1);
  EXPECT_THAT(plan.GetRowForFunctionOffset(0), testing::Pointee(row1));

  plan.InsertRow(row2, /*replace_existing=*/false);
  EXPECT_THAT(plan.GetRowForFunctionOffset(0), testing::Pointee(row1));

  plan.InsertRow(row2, /*replace_existing=*/true);
  EXPECT_THAT(plan.GetRowForFunctionOffset(0), testing::Pointee(row2));

  EXPECT_THAT(plan.GetRowForFunctionOffset(-1), nullptr);
  plan.InsertRow(row3);
  EXPECT_THAT(plan.GetRowForFunctionOffset(-1), testing::Pointee(row3));
}

TEST(UnwindPlan, GetRowForFunctionOffset) {
  UnwindPlan::Row row1 = make_simple_row(10, 42);
  UnwindPlan::Row row2 = make_simple_row(20, 47);

  UnwindPlan plan(eRegisterKindGeneric);
  plan.InsertRow(row1);
  plan.InsertRow(row2);

  EXPECT_THAT(plan.GetRowForFunctionOffset(0), nullptr);
  EXPECT_THAT(plan.GetRowForFunctionOffset(9), nullptr);
  EXPECT_THAT(plan.GetRowForFunctionOffset(10), testing::Pointee(row1));
  EXPECT_THAT(plan.GetRowForFunctionOffset(19), testing::Pointee(row1));
  EXPECT_THAT(plan.GetRowForFunctionOffset(20), testing::Pointee(row2));
  EXPECT_THAT(plan.GetRowForFunctionOffset(99), testing::Pointee(row2));
}

TEST(UnwindPlan, PlanValidAtAddress) {
  UnwindPlan::Row row1 = make_simple_row(0, 42);
  UnwindPlan::Row row2 = make_simple_row(10, 47);

  UnwindPlan plan(eRegisterKindGeneric);
  // When valid address range is not set, plans are valid as long as they have a
  // row that sets the CFA.
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(0)));
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(10)));

  plan.InsertRow(row2);
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(0)));
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(10)));

  plan.InsertRow(row1);
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(0)));
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(10)));

  // With an address range, they're only valid within that range.
  plan.SetPlanValidAddressRanges({AddressRange(0, 5), AddressRange(15, 5)});
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(0)));
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(5)));
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(10)));
  EXPECT_TRUE(plan.PlanValidAtAddress(Address(15)));
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(20)));
  EXPECT_FALSE(plan.PlanValidAtAddress(Address(25)));
}
