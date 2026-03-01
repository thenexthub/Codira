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
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto MODIFIED_DYNAMIC =
    ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic_modified.abc";
static constexpr auto MODIFIED_STATIC =
    ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/literals/literals_static_modified.abc";

class LibAbcKitInspectApiLiteralsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::literalGetBool, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetBool_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetBool(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU8, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU8_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    auto val = g_implI->literalGetU8(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU16, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU16_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU16(file, 1);
    auto val = g_implI->literalGetU16(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethodAffiliate, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethodAffiliate_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethodAffiliate(file, 1);
    auto val = g_implI->literalGetMethodAffiliate(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU32, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU32_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU32(file, 1);
    auto val = g_implI->literalGetU32(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU64_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU64(file, 1);
    auto val = g_implI->literalGetU64(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFloat, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFloat_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralFloat(file, 1.0);
    auto val = g_implI->literalGetFloat(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_FLOAT_EQ(val, 1.0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetDouble, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetDouble_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralDouble(file, 1.0);
    auto val = g_implI->literalGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_DOUBLE_EQ(val, 1.0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    const double val1 = 0.7;
    const int val2 = 7U;
    std::array<AbckitLiteral *, 2U> statlitarr = {g_implM->createLiteralDouble(file, val1),
                                                  g_implM->createLiteralU32(file, val2)};
    AbckitLiteralArray *litarr = g_implM->createLiteralArray(file, statlitarr.data(), 2U);
    AbckitLiteral *res = g_implM->createLiteralLiteralArray(file, litarr);
    auto val = g_implI->literalGetLiteralArray(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, litarr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetString_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    auto val = g_implI->literalGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::AbckitStringToString(val), "asdf");
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethod, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethod_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "foo"));
    auto val = g_implI->literalGetMethod(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::AbckitStringToString(val), "literals_dynamic.#*#foo");
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetTag, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetTag_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_STRING);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFile_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    ASSERT_EQ(g_implI->literalGetFile(res), file);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalArrayEnumerateElements, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralArrayEnumerateElements_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    const size_t loopCount = 10U;

    const uint32_t val0 = 2;
    const uint32_t shift = 30;
    const uint32_t val1 = (val0 << shift) + 1;
    const double val2 = 3.22;
    auto checkLit = [val1, val2](AbckitLiteral *lit) {
        auto tag = g_implI->literalGetTag(lit);
        switch (tag) {
            case ABCKIT_LITERAL_TAG_BOOL:
                ASSERT_EQ(g_implI->literalGetBool(lit), true);
                break;
            case ABCKIT_LITERAL_TAG_INTEGER:
                ASSERT_EQ(g_implI->literalGetU32(lit), val1);
                break;
            case ABCKIT_LITERAL_TAG_DOUBLE:
                ASSERT_EQ(g_implI->literalGetDouble(lit), val2);
                break;
            case ABCKIT_LITERAL_TAG_STRING:
                ASSERT_EQ(helpers::AbckitStringToString(g_implI->literalGetString(lit)), "asdf");
                break;
            default:
                ASSERT_TRUE(false);
        }
    };

    for (size_t i = 0; i < loopCount; ++i) {
        auto arr = std::vector<AbckitLiteral *>({
            g_implM->createLiteralBool(file, true),
            g_implM->createLiteralU32(file, val1),
            g_implM->createLiteralDouble(file, val2),
            g_implM->createLiteralString(file, "asdf", strlen("asdf")),
        });
        auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        std::vector<AbckitLiteral *> newArr;
        g_implI->literalArrayEnumerateElements(litArr, &newArr, [](AbckitFile *, AbckitLiteral *lit, void *data) {
            (reinterpret_cast<std::vector<AbckitLiteral *> *>(data))->emplace_back(lit);
            return true;
        });

        ASSERT_EQ(newArr.size(), 4U);
        std::for_each(newArr.begin(), newArr.end(), [&](AbckitLiteral *lit) { checkLit(lit); });
    }

    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetBool, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetBool_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetBool(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU8, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU8_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    auto val = g_implI->literalGetU8(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU16, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU16_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU16(file, 1);
    auto val = g_implI->literalGetU16(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethodAffiliate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethodAffiliate_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethodAffiliate(file, 1);
    auto val = g_implI->literalGetMethodAffiliate(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU32, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU32_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU32(file, 1);
    auto val = g_implI->literalGetU32(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU64_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU64(file, 1);
    auto val = g_implI->literalGetU64(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, 1);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFloat, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFloat_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralFloat(file, 1.0);
    auto val = g_implI->literalGetFloat(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_FLOAT_EQ(val, 1.0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetDouble, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetDouble_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralDouble(file, 1.0);
    auto val = g_implI->literalGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_DOUBLE_EQ(val, 1.0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetString_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    auto val = g_implI->literalGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::AbckitStringToString(val), "asdf");
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetTag, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetTag_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_STRING);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetTag, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetTag_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_STRING);
    res = g_implM->createLiteralBool(file, true);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_BOOL);
    res = g_implM->createLiteralU8(file, 0);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_BYTE);
    res = g_implM->createLiteralU16(file, 0);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_SHORT);
    res = g_implM->createLiteralU32(file, 0);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_INTEGER);
    res = g_implM->createLiteralU64(file, 0);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_LONG);
    res = g_implM->createLiteralFloat(file, 0);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_FLOAT);
    res = g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "ConsoleLogStr"));
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_METHOD);
    res = g_implM->createLiteralMethodAffiliate(file, 1);
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_METHODAFFILIATE);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetTag, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetTag_Coverage)
{
    struct LiteralMock {
        uint8_t tag;
        std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string> value;
    };

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, 0);
    auto *litImpl = reinterpret_cast<LiteralMock *>(res->val.get());
    litImpl->tag = 0x00;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_TAGVALUE);
    litImpl->tag = 0x01;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_BOOL);
    litImpl->tag = 0x02;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_INTEGER);
    litImpl->tag = 0x08;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_METHOD);
    litImpl->tag = 0x0d;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_BYTE);
    litImpl->tag = 0x0f;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_SHORT);
    litImpl->tag = 0x11;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_INTEGER);
    litImpl->tag = 0x13;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_LONG);
    litImpl->tag = 0x14;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_FLOAT);
    litImpl->tag = 0x15;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_DOUBLE);
    litImpl->tag = 0x16;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_STRING);
    litImpl->tag = 0x17;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_METHOD);
    litImpl->tag = 0xff;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_NULLVALUE);
    litImpl->tag = 0x20;
    ASSERT_EQ(g_implI->literalGetTag(res), ABCKIT_LITERAL_TAG_INVALID);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFile_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
    ASSERT_EQ(g_implI->literalGetFile(res), file);

    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetBool, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetBool_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    g_implI->literalGetBool(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetBool, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetBool_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    g_implI->literalGetBool(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU8, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU8_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU8(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU8, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU8_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU8(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU16, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU16_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU16(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU16, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU16_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU16(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethodAffiliate, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethodAffiliate_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetMethodAffiliate(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethodAffiliate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethodAffiliate_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetMethodAffiliate(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU32, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU32_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU32(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU32, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU32_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU32(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU64_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU64(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetU64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetU64_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetU64(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFloat, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFloat_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetFloat(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_FLOAT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetFloat, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetFloat_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetFloat(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_FLOAT_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetDouble, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetDouble_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_DOUBLE_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetDouble, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetDouble_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_DOUBLE_EQ(val, 0);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetLiteralArray_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetLiteralArray(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetLiteralArray, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetLiteralArray_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetLiteralArray(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetString_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetString_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetString_5)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    std::string testString = "testString";
    AbckitLiteral *res = g_implM->createLiteralString(file, testString.c_str(), testString.length());
    auto val = g_implI->literalGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(helpers::AbckitStringToString(val), testString);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethod, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethod_3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetMethod(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_LITERAL_TYPE);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::literalGetMethod, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiLiteralsTest, LiteralGetMethod_4)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    auto val = g_implI->literalGetMethod(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    ASSERT_EQ(val, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace libabckit::test
