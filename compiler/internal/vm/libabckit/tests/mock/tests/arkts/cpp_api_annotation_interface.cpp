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

#include "include/libabckit/cpp/abckit_cpp.h"
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockArktsTestAnnotationInterface : public ::testing::Test {};

// Test: test-kind=mock, api=AnnotationInterface::AddField, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotationInterface, AnnotationInterface_AddField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto t = abckit::mock::helpers::GetMockType(f);
        auto v = f.CreateValueU1(DEFAULT_BOOL);
        abckit::mock::helpers::GetMockArktsAnnotationInterface(f).AddField(DEFAULT_CONST_CHAR, t, v);
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceAddField"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=AnnotationInterface::RemoveField, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotationInterface, AnnotationInterface_RemoveField)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto field = abckit::mock::helpers::GetMockArktsAnnotationInterfaceField(f);
        abckit::mock::helpers::GetMockArktsAnnotationInterface(f).RemoveField(field);
        ASSERT_TRUE(CheckMockedApi("AnnotationInterfaceRemoveField"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test