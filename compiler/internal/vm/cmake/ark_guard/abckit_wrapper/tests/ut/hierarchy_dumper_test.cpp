/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
#include <gtest/gtest.h>
#include "abckit_wrapper/hierarchy_dumper.h"
#include "test_util/assert.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/module_static.abc";
constexpr std::string_view DUMP_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/hierarchy_dumper.txt";
abckit_wrapper::FileView fileView;
}  // namespace

class TestHierarchyDumper : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }
};

/**
 * @tc.name: hierarchy_dump_001
 * @tc.desc: test dump hierarchy of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestHierarchyDumper, hierarchy_dump_001, TestSize.Level4)
{
    abckit_wrapper::HierarchyDumper dumper(fileView, DUMP_FILE_PATH);
    ASSERT_SUCCESS(dumper.Dump());
}