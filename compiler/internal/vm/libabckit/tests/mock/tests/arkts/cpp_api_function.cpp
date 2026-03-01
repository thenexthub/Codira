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

class LibAbcKitCppMockArktsTestFunction : public ::testing::Test {};

// Test: test-kind=mock, api=Function::AddAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestFunction, Function_AddAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsFunction(f).AddAnnotation(
            abckit::mock::helpers::GetMockArktsAnnotationInterface(f));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationToCoreAnnotation"));
        ASSERT_TRUE(CheckMockedApi("FunctionAddAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Function::IsNative, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestFunction, Function_IsNative)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsFunction(f).IsNative();
        ASSERT_TRUE(CheckMockedApi("FunctionIsNative"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Function::RemoveAnnotation, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestFunction, Function_RemoveAnnotation)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto anno = abckit::mock::helpers::GetMockArktsAnnotation(f);
        abckit::mock::helpers::GetMockArktsFunction(f).RemoveAnnotation(anno);
        ASSERT_TRUE(CheckMockedApi("FunctionRemoveAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test