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

#include <vector>

#include "libarkbase/mem/mem.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace ark::test {

using TaggedValue = coretypes::TaggedValue;
static constexpr size_t OBJECT_POINTER_SIZE = sizeof(ObjectPointerType);
static constexpr size_t POINTER_SIZE = ClassHelper::POINTER_SIZE;

TEST(ClassSizeTest, TestSizeOfEmptyClass)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    ASSERT_EQ(alignedClassSize, Class::ComputeClassSize(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWithVtbl)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t vtblSize = 5;
    ASSERT_EQ(alignedClassSize + vtblSize * POINTER_SIZE, Class::ComputeClassSize(vtblSize, 0, 0, 0, 0, 0, 0, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWith8BitFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t num8bitFields = 1;
    ASSERT_EQ(alignedClassSize + num8bitFields * sizeof(int8_t),
              Class::ComputeClassSize(0, 0, num8bitFields, 0, 0, 0, 0, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWith16BitFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t num16bitFields = 1;
    ASSERT_EQ(alignedClassSize + num16bitFields * sizeof(int16_t),
              Class::ComputeClassSize(0, 0, 0, num16bitFields, 0, 0, 0, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWith32BitFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t num32bitFields = 1;
    ASSERT_EQ(alignedClassSize + num32bitFields * sizeof(int32_t),
              Class::ComputeClassSize(0, 0, 0, 0, num32bitFields, 0, 0, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWith64BitFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t num64bitFields = 1;
    if (AlignUp(alignedClassSize, sizeof(int64_t)) == alignedClassSize) {
        ASSERT_EQ(alignedClassSize + num64bitFields * sizeof(int64_t),
                  Class::ComputeClassSize(0, 0, 0, 0, 0, num64bitFields, 0, 0));
    } else {
        ASSERT_EQ(AlignUp(alignedClassSize, sizeof(int64_t)) + num64bitFields * sizeof(int64_t),
                  Class::ComputeClassSize(0, 0, 0, 0, 0, num64bitFields, 0, 0));
    }
}

TEST(ClassSizeTest, TestSizeOfClassWithRefFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t numRefFields = 1;
    ASSERT_EQ(alignedClassSize + numRefFields * OBJECT_POINTER_SIZE,
              Class::ComputeClassSize(0, 0, 0, 0, 0, 0, numRefFields, 0));
}

TEST(ClassSizeTest, TestSizeOfClassWithAnyFields)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t numAnyFields = 1;
    if (AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) == alignedClassSize) {
        ASSERT_EQ(alignedClassSize + numAnyFields * TaggedValue::TaggedTypeSize(),
                  Class::ComputeClassSize(0, 0, 0, 0, 0, 0, 0, numAnyFields));
    } else {
        ASSERT_EQ(AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) +
                      numAnyFields * TaggedValue::TaggedTypeSize(),
                  Class::ComputeClassSize(0, 0, 0, 0, 0, 0, 0, numAnyFields));
    }
}

TEST(ClassSizeTest, TestHoleFilling)
{
    const size_t alignedClassSize = AlignUp(sizeof(Class), OBJECT_POINTER_SIZE);
    const size_t num64bitFields = 1;
    if (AlignUp(alignedClassSize, sizeof(int64_t)) != alignedClassSize) {
        const size_t num8bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, sizeof(int64_t)) + num64bitFields * sizeof(int64_t),
                  Class::ComputeClassSize(0, 0, num8bitFields, 0, 0, num64bitFields, 0, 0));

        const size_t num16bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, sizeof(int64_t)) + num64bitFields * sizeof(int64_t),
                  Class::ComputeClassSize(0, 0, 0, num16bitFields, 0, num64bitFields, 0, 0));

        const size_t num32bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, sizeof(int64_t)) + num64bitFields * sizeof(int64_t),
                  Class::ComputeClassSize(0, 0, 0, 0, num32bitFields, num64bitFields, 0, 0));
    }

    const size_t numAnyFields = 1;
    if (AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) != alignedClassSize) {
        const size_t num8bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) +
                      numAnyFields * TaggedValue::TaggedTypeSize(),
                  Class::ComputeClassSize(0, 0, num8bitFields, 0, 0, numAnyFields, 0, 0));

        const size_t num16bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) +
                      numAnyFields * TaggedValue::TaggedTypeSize(),
                  Class::ComputeClassSize(0, 0, 0, num16bitFields, 0, numAnyFields, 0, 0));

        const size_t num32bitFields = 1;
        ASSERT_EQ(AlignUp(alignedClassSize, TaggedValue::TaggedTypeSize()) +
                      numAnyFields * TaggedValue::TaggedTypeSize(),
                  Class::ComputeClassSize(0, 0, 0, 0, num32bitFields, numAnyFields, 0, 0));
    }
}

}  // namespace ark::test
