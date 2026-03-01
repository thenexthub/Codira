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
#include <cstddef>
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/metadata_inspect_impl.h"  // NOTE(mredkin)
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

constexpr auto MODIFIED_DYNAMIC = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic_modified.abc";
constexpr auto MODIFIED_STATIC = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static_modified.abc";

class LibAbcKitInspectApiValuesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    auto val = g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetInt, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetInt)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueInt(file, 1);
    auto val = g_implI->valueGetInt(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetFile_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(g_implI->valueGetFile(res), file);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    const double implVal = 1.2;
    auto *res = g_implM->createValueDouble(file, implVal);
    auto val = g_implI->valueGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, implVal);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto val = g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->impl, "test");
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, CreateLiteralArrayValue_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2U);
    auto *litArr = g_implI->arrayValueGetLiteralArray(arr);
    std::vector<AbckitLiteral *> newArr;
    g_implI->literalArrayEnumerateElements(litArr, &newArr, [](AbckitFile *, AbckitLiteral *lit, void *data) {
        (reinterpret_cast<std::vector<AbckitLiteral *> *>(data))->emplace_back(lit);
        return true;
    });

    ASSERT_EQ(newArr.size(), 2U);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arr, nullptr);

    ASSERT_EQ(g_implI->literalGetString(newArr[0])->impl, "test");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_TRUE(g_implI->literalGetBool(newArr[1]));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::arrayValueGetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);
    auto *larr = g_implI->arrayValueGetLiteralArray(arr);
    size_t counter = 0;
    g_implI->literalArrayEnumerateElements(larr, &counter, [](AbckitFile *, AbckitLiteral *, void *data) {
        (*(reinterpret_cast<uint32_t *>(data)))++;
        return true;
    });
    ASSERT_EQ(counter, 2U);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetType, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetType_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_STRING);
    res = g_implM->createValueU1(file, true);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U1);
    const double implVal = 1.2;
    res = g_implM->createValueDouble(file, implVal);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_F64);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    auto val = g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    const double implVal = 1.2;
    auto *res = g_implM->createValueDouble(file, implVal);
    auto val = g_implI->valueGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, implVal);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto val = g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->impl, "test");
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, CreateLiteralArrayValue_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);

    ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(arr, nullptr);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetFile_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(g_implI->valueGetFile(res), file);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::arrayValueGetLiteralArray, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);
    auto *larr = g_implI->arrayValueGetLiteralArray(arr);

    ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(larr, nullptr);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetType_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_STRING);
    res = g_implM->createValueU1(file, true);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U1);
    const double implVal = 1.2;
    res = g_implM->createValueDouble(file, implVal);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_F64);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetType_Coverage)
{
    struct ValueMock {
        uint8_t tmp[8];
        uint32_t type;
    };
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto valImpl = reinterpret_cast<ValueMock *>(res->val.get());
    const uint32_t typeU8 = 2;
    valImpl->type = typeU8;
    auto val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U8);

    const uint32_t typeU16 = 4;
    valImpl->type = typeU16;
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U16);

    const uint32_t typeU32 = 6;
    valImpl->type = typeU32;
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U32);

    const uint32_t typeU64 = 8;
    valImpl->type = typeU64;
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U64);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ValueGetTypeDynamic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ValueGetTypeStatic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    const double initVal = .1f;
    auto *res = g_implM->createValueDouble(file, initVal);
    ValueGetTypeDynamic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    const double initVal = .1f;
    auto *res = g_implM->createValueDouble(file, initVal);
    ValueGetTypeStatic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    ValueGetTypeDynamic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    ValueGetTypeStatic(res)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);
    ValueGetTypeDynamic(arr)->id = ABCKIT_TYPE_ID_INVALID;
    g_implI->arrayValueGetLiteralArray(arr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::arrayValueGetLiteralArray, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);

    auto *arr = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arr, nullptr);

    auto *larr = g_implI->arrayValueGetLiteralArray(arr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    ASSERT_EQ(larr, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
