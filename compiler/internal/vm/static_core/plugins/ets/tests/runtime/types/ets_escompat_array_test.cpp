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

#include "include/thread_scopes.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsEscompatArrayTest : public testing::Test {
public:
    EtsEscompatArrayTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        vm_ = coroutine->GetPandaVM();

        coreStringClass_ = PlatformTypes(coroutine)->coreString;
    }

    ~EtsEscompatArrayTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsEscompatArrayTest);
    NO_MOVE_SEMANTIC(EtsEscompatArrayTest);

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsEscompatArray, actualLength_, "actualLength"),
                                             MIRROR_FIELD_INFO(EtsEscompatArray, buffer_, "buffer")};
    }

    void CheckObjectIsString(EtsCoroutine *coro, EtsObject *obj, std::string_view expected)
    {
        ASSERT_FALSE(coro->HasPendingException());
        ASSERT_NE(obj, nullptr);
        ASSERT_TRUE(obj->IsInstanceOf(coreStringClass_));
        ASSERT_TRUE(EtsString::FromEtsObject(obj)->IsEqual(expected.data()));
    }

    void CheckArrayElements(EtsCoroutine *coro, EtsHandle<EtsObjectArray> &dataArray)
    {
        for (size_t i = 0, end = dataArray->GetLength(); i < end; ++i) {
            const auto sampleString = std::to_string(i);
            auto *result = dataArray->Get(i);
            CheckObjectIsString(coro, result, sampleString);
        }
    }

public:
    static constexpr size_t ARRAY_LENGTH = 10;

protected:
    PandaEtsVM *vm_ = nullptr;             // NOLINT(misc-non-private-member-variables-in-classes)
    EtsClass *coreStringClass_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(EtsEscompatArrayTest, MemoryLayout)
{
    EtsClass *klass = PlatformTypes(vm_)->escompatArray;
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsEscompatArrayTest, EtsEscompatArrayCreate)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);

    auto *array = EtsEscompatArray::Create(coro, ARRAY_LENGTH);
    ASSERT_NE(array, nullptr);
    ASSERT_TRUE(array->IsExactlyEscompatArray(coro));

    ASSERT_EQ(array->GetActualLengthFromEscompatArray(), ARRAY_LENGTH);

    auto *dataArray = array->GetDataFromEscompatArray();
    ASSERT_NE(dataArray, nullptr);
    // Expect at least `ARRAY_LENGTH` length of array
    ASSERT_GE(dataArray->GetLength(), ARRAY_LENGTH);

    EtsInt length = 0;
    ASSERT_TRUE(array->GetLength(coro, &length));
    ASSERT_FALSE(coro->HasPendingException());
    ASSERT_EQ(length, ARRAY_LENGTH);
}

TEST_F(EtsEscompatArrayTest, GetRef)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    EtsHandle dataArray(coro, array->GetDataFromEscompatArray());
    for (size_t i = 0; i < ARRAY_LENGTH; ++i) {
        const auto sampleString = std::to_string(i);
        auto *str = EtsString::CreateFromUtf8(sampleString.c_str(), sampleString.size());
        ASSERT_NE(str, nullptr);
        dataArray->Set(i, str->AsObject());

        // Check that `GetRef` works correctly
        auto result = array->GetRef(coro, i);
        ASSERT_TRUE(result.has_value());
        CheckObjectIsString(coro, *result, sampleString);
    }
}

TEST_F(EtsEscompatArrayTest, EscompatArrayGet)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    EtsHandle dataArray(coro, array->GetDataFromEscompatArray());
    for (size_t i = 0; i < ARRAY_LENGTH; ++i) {
        const auto sampleString = std::to_string(i);
        auto *str = EtsString::CreateFromUtf8(sampleString.c_str(), sampleString.size());
        ASSERT_NE(str, nullptr);
        dataArray->Set(i, str->AsObject());

        // Check that `GetRef` works correctly
        EtsObject *result = array->EscompatArrayGet(i);
        CheckObjectIsString(coro, result, sampleString);
    }
}

TEST_F(EtsEscompatArrayTest, GetRefWithError)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    auto result = array->GetRef(coro, ARRAY_LENGTH);
    ASSERT_FALSE(result.has_value());
    ASSERT_TRUE(coro->HasPendingException());
}

TEST_F(EtsEscompatArrayTest, SetRef)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    EtsHandle dataArray(coro, array->GetDataFromEscompatArray());
    for (size_t i = 0; i < ARRAY_LENGTH; ++i) {
        const auto sampleString = std::to_string(i);
        auto *expectedString = EtsString::CreateFromUtf8(sampleString.c_str(), sampleString.size())->AsObject();
        ASSERT_TRUE(array->SetRef(coro, i, expectedString));
        ASSERT_FALSE(coro->HasPendingException());

        auto *result = dataArray->Get(i);
        CheckObjectIsString(coro, result, sampleString);
    }
    CheckArrayElements(coro, dataArray);
}

TEST_F(EtsEscompatArrayTest, EscompatArraySet)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    EtsHandle dataArray(coro, array->GetDataFromEscompatArray());
    for (size_t i = 0; i < ARRAY_LENGTH; ++i) {
        const auto sampleString = std::to_string(i);
        auto *expectedString = EtsString::CreateFromUtf8(sampleString.c_str(), sampleString.size())->AsObject();
        array->EscompatArraySet(i, expectedString);
        ASSERT_FALSE(coro->HasPendingException());

        auto *result = dataArray->Get(i);
        CheckObjectIsString(coro, result, sampleString);
    }
    CheckArrayElements(coro, dataArray);
}

TEST_F(EtsEscompatArrayTest, SetRefWithError)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread s(coro);
    EtsHandleScope sh(coro);

    EtsHandle array(coro, EtsEscompatArray::Create(coro, ARRAY_LENGTH));
    ASSERT_NE(array.GetPtr(), nullptr);

    auto result = array->SetRef(coro, ARRAY_LENGTH, nullptr);
    ASSERT_FALSE(result);
    ASSERT_TRUE(coro->HasPendingException());
}

}  // namespace ark::ets::test
