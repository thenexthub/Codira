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

class LibAbcKitCppMockArktsTestAnnotation : public ::testing::Test {};

// Test: test-kind=mock, api=Annotation::AddElement, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotation, Annotation_AddElement)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsAnnotation(f).AddElement(f.CreateValueU1(DEFAULT_BOOL), DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("AnnotationAddAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Annotation::AddAndGetElement, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotation, Annotation_AddAndGetElement)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockArktsAnnotation(f).AddAndGetElement(f.CreateValueU1(DEFAULT_BOOL),
                                                                          DEFAULT_CONST_CHAR);
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationElementToArktsAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationElementToCoreAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("AnnotationAddAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Annotation::AddAnnotationElement, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotation, Annotation_AddAnnotationElement)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto v = f.CreateValueU1(DEFAULT_BOOL);
        abckit::mock::helpers::GetMockArktsAnnotation(f).AddAnnotationElement(DEFAULT_CONST_CHAR, v);
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationElementToArktsAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("ArktsAnnotationElementToCoreAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("AnnotationAddAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
        ASSERT_TRUE(CheckMockedApi("CreateValueU1"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Annotation::RemoveAnnotationElement, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockArktsTestAnnotation, Annotation_RemoveAnnotationElement)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        auto ae = abckit::mock::helpers::GetMockArktsAnnotationElement(f);
        abckit::mock::helpers::GetMockArktsAnnotation(f).RemoveAnnotationElement(ae);
        ASSERT_TRUE(CheckMockedApi("AnnotationRemoveAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationElementToArktsAnnotationElement"));
        ASSERT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test