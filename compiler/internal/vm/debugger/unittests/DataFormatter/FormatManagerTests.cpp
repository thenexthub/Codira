//===-- FormatManagerTests.cpp --------------------------------------------===//
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

#include "lldb/DataFormatters/FormatManager.h"

#include "gtest/gtest.h"

using namespace lldb;
using namespace lldb_private;

TEST(FormatManagerTests, CompatibleLangs) {
  std::vector<LanguageType> candidates = {eLanguageTypeC_plus_plus,
                                          eLanguageTypeObjC};
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC), candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC89), candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC99), candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC11), candidates);

  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC_plus_plus),
            candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC_plus_plus_03),
            candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC_plus_plus_11),
            candidates);
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeC_plus_plus_14),
            candidates);

  candidates = {eLanguageTypeObjC};
  EXPECT_EQ(FormatManager::GetCandidateLanguages(eLanguageTypeObjC),
            candidates);
}
