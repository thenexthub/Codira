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
#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/field_test.abc";

void AssertFieldType(const abckit_wrapper::FileView &fileView, const std::string &name, const std::string &type)
{
    const auto object = fileView.Get<abckit_wrapper::Field>(name);
    ASSERT_TRUE(object.has_value());
    const auto fieldType = object.value()->GetType();
    ASSERT_EQ(fieldType, type) << "field type not match, expected:" << type << ", actual:" << fieldType;
}

}  // namespace

/**
 * @tc.name: field_type_test_001
 * @tc.desc: test class field get type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(TestField, field_type_test_001, TestSize.Level0)
{
    abckit_wrapper::FileView fileView;

    ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));

    AssertFieldType(fileView, "field_test.ClassA.field1", "i32");
    AssertFieldType(fileView, "field_test.ClassA.field2", "std.core.String");

    AssertFieldType(fileView, "field_test.ClassB.field1", "field_test.ClassA");
    AssertFieldType(fileView, "field_test.ClassB.field2", "{Ufield_test.ClassA,std.core.String}");
}
