/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "ets_coroutine.h"
#include "types/ets_object.h"
#include "types/ets_string.h"
#include "types/ets_bigint.h"
#include "types/ets_box_primitive-inl.h"

#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

// Include the intrinsic function we're testing
namespace ark::ets::intrinsics {
extern "C" EtsBoolean EtsWeakMapValidateKey(EtsObject *key);
}

namespace ark::ets::test {

class EtsWeakMapTest : public testing::Test {
public:
    EtsWeakMapTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        coro_ = EtsCoroutine::GetCurrent();
        vm_ = coro_->GetPandaVM();
    }

    ~EtsWeakMapTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsWeakMapTest);
    NO_MOVE_SEMANTIC(EtsWeakMapTest);

    void SetUp() override
    {
        coro_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coro_->ManagedCodeEnd();
    }

protected:
    EtsCoroutine *coro_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaEtsVM *vm_ = nullptr;      // NOLINT(misc-non-private-member-variables-in-classes)
};

// Test that regular objects are valid WeakMap keys
TEST_F(EtsWeakMapTest, ValidateKeyAcceptsObjects)
{
    // Create a simple object instance
    EtsClass *objectClass =
        coro_->GetPandaVM()->GetClassLinker()->GetClass(panda_file_items::class_descriptors::OBJECT.data());
    ASSERT_NE(objectClass, nullptr);

    EtsObject *obj = EtsObject::Create(coro_, objectClass);
    ASSERT_NE(obj, nullptr);

    // Object should be a valid WeakMap key
    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(obj);
    EXPECT_TRUE(result);
}

// Test that boxed Boolean is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsBooleanBox)
{
    EtsObject *boxedBool = EtsBoxPrimitive<EtsBoolean>::Create(coro_, static_cast<EtsBoolean>(1));
    ASSERT_NE(boxedBool, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedBool);
    EXPECT_FALSE(result) << "Boxed Boolean should not be a valid WeakMap key";
}

// Test that boxed Int is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsIntBox)
{
    EtsObject *boxedInt = EtsBoxPrimitive<EtsInt>::Create(coro_, 42);
    ASSERT_NE(boxedInt, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedInt);
    EXPECT_FALSE(result) << "Boxed Int should not be a valid WeakMap key";
}

// Test that boxed Long is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsLongBox)
{
    EtsObject *boxedLong = EtsBoxPrimitive<EtsLong>::Create(coro_, 100L);
    ASSERT_NE(boxedLong, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedLong);
    EXPECT_FALSE(result) << "Boxed Long should not be a valid WeakMap key";
}

// Test that boxed Double is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsDoubleBox)
{
    EtsObject *boxedDouble = EtsBoxPrimitive<EtsDouble>::Create(coro_, 3.14);
    ASSERT_NE(boxedDouble, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedDouble);
    EXPECT_FALSE(result) << "Boxed Double should not be a valid WeakMap key";
}

// Test that boxed Float is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsFloatBox)
{
    EtsObject *boxedFloat = EtsBoxPrimitive<EtsFloat>::Create(coro_, 2.5f);
    ASSERT_NE(boxedFloat, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedFloat);
    EXPECT_FALSE(result) << "Boxed Float should not be a valid WeakMap key";
}

// Test that boxed Byte is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsByteBox)
{
    EtsObject *boxedByte = EtsBoxPrimitive<EtsByte>::Create(coro_, static_cast<EtsByte>(10));
    ASSERT_NE(boxedByte, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedByte);
    EXPECT_FALSE(result) << "Boxed Byte should not be a valid WeakMap key";
}

// Test that boxed Short is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsShortBox)
{
    EtsObject *boxedShort = EtsBoxPrimitive<EtsShort>::Create(coro_, static_cast<EtsShort>(1000));
    ASSERT_NE(boxedShort, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedShort);
    EXPECT_FALSE(result) << "Boxed Short should not be a valid WeakMap key";
}

// Test that boxed Char is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsCharBox)
{
    EtsObject *boxedChar = EtsBoxPrimitive<EtsChar>::Create(coro_, static_cast<EtsChar>('A'));
    ASSERT_NE(boxedChar, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(boxedChar);
    EXPECT_FALSE(result) << "Boxed Char should not be a valid WeakMap key";
}

// Test that String is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsString)
{
    const char *testStr = "test string";
    EtsString *str = EtsString::CreateFromMUtf8(testStr);
    ASSERT_NE(str, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(str->AsObject());
    EXPECT_FALSE(result) << "String should not be a valid WeakMap key";
}

// Test that BigInt is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsBigInt)
{
    // Create a BigInt
    EtsClass *bigIntClass =
        coro_->GetPandaVM()->GetClassLinker()->GetClass(panda_file_items::class_descriptors::BIG_INT.data());
    ASSERT_NE(bigIntClass, nullptr);

    // Create a simple BigInt object (we can't easily construct a proper BigInt from C++,
    // but we can test that the class detection works by creating an object of BigInt class)
    EtsObject *bigInt = EtsObject::Create(coro_, bigIntClass);
    ASSERT_NE(bigInt, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(bigInt);
    EXPECT_FALSE(result) << "BigInt should not be a valid WeakMap key";
}

// Test that NullValue is rejected
TEST_F(EtsWeakMapTest, ValidateKeyRejectsNullValue)
{
    EtsClass *nullValueClass =
        coro_->GetPandaVM()->GetClassLinker()->GetClass(panda_file_items::class_descriptors::NULL_VALUE.data());
    ASSERT_NE(nullValueClass, nullptr);

    EtsObject *nullValue = EtsObject::Create(coro_, nullValueClass);
    ASSERT_NE(nullValue, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(nullValue);
    EXPECT_FALSE(result) << "NullValue should not be a valid WeakMap key";
}

// Test that arrays are valid WeakMap keys (they are reference types)
TEST_F(EtsWeakMapTest, ValidateKeyAcceptsArray)
{
    // Create an array using the static Create method
    EtsArray *arr = EtsIntArray::Create(10);
    ASSERT_NE(arr, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(arr->AsObject());
    EXPECT_TRUE(result) << "Array should be a valid WeakMap key";
}

// Test multiple valid reference types
TEST_F(EtsWeakMapTest, ValidateKeyAcceptsMultipleReferenceTypes)
{
    // Test with Error object
    EtsClass *errorClass =
        coro_->GetPandaVM()->GetClassLinker()->GetClass(panda_file_items::class_descriptors::ERROR.data());
    ASSERT_NE(errorClass, nullptr);

    EtsObject *error = EtsObject::Create(coro_, errorClass);
    ASSERT_NE(error, nullptr);

    EtsBoolean result = intrinsics::EtsWeakMapValidateKey(error);
    EXPECT_TRUE(result) << "Error object should be a valid WeakMap key";
}

}  // namespace ark::ets::test
