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

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitMemoryHandling : public ::testing::Test {};

// Test: test-kind=api, api=ApiImpl::openAbc, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitMemoryHandling, OpenAbcStatic)
{
    constexpr auto PATH1 = ABCKIT_ABC_DIR "internal/mem_manager/abc_static_1.abc";
    constexpr auto PATH2 = ABCKIT_ABC_DIR "internal/mem_manager/abc_static_2.abc";
    auto *file = g_impl->openAbc(PATH1, strlen(PATH1));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *ctxI2 = g_impl->openAbc(PATH2, strlen(PATH2));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *main = helpers::FindMethodByName(file, "main");
    ASSERT_NE(main, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *main2 = helpers::FindMethodByName(ctxI2, "main");
    ASSERT_NE(main2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto transformMain = [](AbckitCoreFunction *method) {
        auto *graph = g_implI->createGraphFromFunction(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implM->functionSetGraph(method, graph);
        g_impl->destroyGraph(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    transformMain(main);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    transformMain(main2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, PATH1, strlen(PATH1));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(ctxI2, PATH2, strlen(PATH2));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(ctxI2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ApiImpl::openAbc, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitMemoryHandling, OpenAbcDynamic)
{
    constexpr auto PATH1 = ABCKIT_ABC_DIR "internal/mem_manager/abc_dynamic_1.abc";
    constexpr auto PATH2 = ABCKIT_ABC_DIR "internal/mem_manager/abc_dynamic_2.abc";
    auto *file = g_impl->openAbc(PATH1, strlen(PATH1));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *ctxI2 = g_impl->openAbc(PATH2, strlen(PATH2));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *main = helpers::FindMethodByName(file, "func_main_0");
    ASSERT_NE(main, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *main2 = helpers::FindMethodByName(ctxI2, "func_main_0");
    ASSERT_NE(main2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto transformMain = [](AbckitCoreFunction *method) {
        auto *graph = g_implI->createGraphFromFunction(method);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implM->functionSetGraph(method, graph);
        g_impl->destroyGraph(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    transformMain(main);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    transformMain(main2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, PATH1, strlen(PATH1));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(ctxI2, PATH2, strlen(PATH2));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(ctxI2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
