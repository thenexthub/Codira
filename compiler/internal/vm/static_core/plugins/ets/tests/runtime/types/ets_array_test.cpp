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

#include "get_test_class.h"
#include "ets_coroutine.h"

#include "types/ets_array.h"
#include "types/ets_class.h"
#include "libarkbase/utils/utils.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

class EtsArrayTest : public testing::Test {
public:
    EtsArrayTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsArrayTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsArrayTest);
    NO_MOVE_SEMANTIC(EtsArrayTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

template <class ClassType, EtsClassRoot ETS_CLASS_ROOT>
static void TestEtsPrimitiveArray(uint32_t arrayLength, ClassType element)
{
    auto *array = EtsPrimitiveArray<ClassType, ETS_CLASS_ROOT>::Create(arrayLength);
    ASSERT_NE(array, nullptr);

    ASSERT_EQ(array->GetLength(), arrayLength);
    ASSERT_EQ(array->GetElementSize(), sizeof(ClassType));
    ASSERT_EQ(array->IsPrimitive(), true);

    for (uint32_t idx = 0; idx < arrayLength; ++idx) {
        array->Set(idx, element);
        ASSERT_EQ(array->Get(idx), element);
    }
}

TEST_F(EtsArrayTest, PrimitiveEtsArray)
{
    uint32_t arrayLength = 100U;

    TestEtsPrimitiveArray<EtsBoolean, EtsClassRoot::BOOLEAN_ARRAY>(arrayLength, 1U);       // EtsBooleanArray
    TestEtsPrimitiveArray<EtsByte, EtsClassRoot::BYTE_ARRAY>(arrayLength, 127_I);          // EtsByteArray
    TestEtsPrimitiveArray<EtsChar, EtsClassRoot::CHAR_ARRAY>(arrayLength, 65000U);         // EtsCharArray
    TestEtsPrimitiveArray<EtsShort, EtsClassRoot::SHORT_ARRAY>(arrayLength, 150_I);        // EtsShortArray
    TestEtsPrimitiveArray<EtsInt, EtsClassRoot::INT_ARRAY>(arrayLength, 65000_I);          // EtsIntArray
    TestEtsPrimitiveArray<EtsLong, EtsClassRoot::LONG_ARRAY>(arrayLength, 65000L);         // EtsLongArray
    TestEtsPrimitiveArray<EtsFloat, EtsClassRoot::FLOAT_ARRAY>(arrayLength, 65000.0F);     // EtsFloatArray
    TestEtsPrimitiveArray<EtsDouble, EtsClassRoot::DOUBLE_ARRAY>(arrayLength, 65000.0_D);  // EtsDoubleArray
}

static void TestEtsObjectArray(const char *className, const char *source, uint32_t arrayLength)
{
    EtsClass *klass = GetTestClass(source, className);
    ASSERT_NE(klass, nullptr);

    auto *array = EtsObjectArray::Create(klass, arrayLength);
    ASSERT_NE(array, nullptr);

    ASSERT_EQ(array->GetLength(), arrayLength);
    ASSERT_EQ(array->GetElementSize(), array->GetClass()->GetComponentSize());
    ASSERT_EQ(array->IsPrimitive(), false);

    for (uint32_t idx = 0; idx < arrayLength; idx++) {
        auto *obj = EtsObject::Create(klass);
        array->Set(idx, obj);
        ASSERT_EQ(array->Get(idx), obj);
    }
}

TEST_F(EtsArrayTest, EtsObjectArray)
{
    const char *source = R"(
        .language eTS
        .record Rectangle {
            i32 Width
            f32 Height
            i64 COLOR <static>
        }
    )";
    TestEtsObjectArray("LRectangle;", source, 100U);

    source = R"(
        .language eTS
        .record Triangle {
            i32 firSide
            i32 secSide
            i32 thirdSide
            i64 COLOR <static>
        }
    )";
    TestEtsObjectArray("LTriangle;", source, 1000U);

    source = R"(
        .language eTS
        .record Ball {
            f32 RADIUS <static>
            i32 SPEED <static>
            f32 vx
            i32 vy
        }
    )";
    TestEtsObjectArray("LBall;", source, 10000U);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
