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
#include <unordered_map>
#include <fstream>

#include "libabckit/c/abckit.h"
#include "adapter_static/ir_static.h"
#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto EXPECTED =
    "Method: Lgraph_dynamic;::func_main_0 0x[a-f0-9]+\n\n"
    "BB 1\n"
    "prop: start, bc: 0x[a-f0-9]+\n"
    "hotness=0\n"
    "\\s+0.any  Parameter\\s+arg 0\\s+\n"
    "r8187 -> r8187 \\[u64\\]\n"
    "\\s+1.any  Parameter\\s+arg 1\\s+\n"
    "r8188 -> r8188 \\[u64\\]\n"
    "\\s+2.any  Parameter\\s+arg 2\\s+\n"
    "r8189 -> r8189 \\[u64\\]\n"
    "\\s+12.i32  Constant\\s+0x0 -> \\(v13\\)\\s+\n"
    "\\s+18.i32  Constant\\s+0x1 -> \\(v19\\)\\s+\n"
    "\\s+22.i32  Constant\\s+0x4 -> \\(v26\\)\\s+\n"
    "\\s+23.i32  Constant\\s+0x5 -> \\(v26\\)\\s+\n"
    "\\s+24.i32  Constant\\s+0x6 -> \\(v26\\)\\s+\n"
    "\\s+25.i32  Constant\\s+0x7 -> \\(v26\\)\\s+\n"
    "succs: \\[bb 0\\]\n\n"
    "BB 0  preds: \\[bb 1\\]\n"
    "prop: bc: 0x[a-f0-9]+\n"
    "hotness=0\n"
    "\\s+3.any  Intrinsic.definefunc\\s+0x[a-f0-9]+, 0x[a-f0-9]+, 0x1 \\(v21\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+4.any  Intrinsic.definefunc\\s+0x[a-f0-9]+, 0x[a-f0-9]+, 0x5 \\(v26\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+5.any  Intrinsic.ldhole\\s+\\(v6\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+6.any  Intrinsic.defineclasswithbuffer 0x[a-f0-9]+, 0x[a-f0-9]+, "
    // CC-OFFNXT(WordsTool.190)
    "0x[a-f0-9]+, 0x[a-f0-9]+ v5 -> \\(v8, v7\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+7.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v6\\s+bc: 0x[a-f0-9]+\n"
    "\\s+8.any  Intrinsic.newobjrange\\s+0x5, 0x[a-f0-9]+ v6 -> "
    "\\(v21, v17, v16, v11, v10\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+9.any  Intrinsic.tryldglobalbyname 0x[a-f0-9]+, 0x[a-f0-9]+ \\(v14\\)\\s+bc: 0x[a-f0-9]+\n"
    // CC-OFFNXT(WordsTool.190)
    "\\s+10.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v8 -> \\(v11\\)\\s+bc: 0x[a-f0-9]+\n"
    // CC-OFFNXT(WordsTool.190)
    "\\s+11.any  Intrinsic.callthis0\\s+0x[a-f0-9]+ v10, v8 -> \\(v13\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+13.any  Intrinsic.ldobjbyvalue\\s+0x[a-f0-9]+ v12, v11 -> \\(v14\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+14.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v9, v13\\s+bc: 0x[a-f0-9]+\n"
    "\\s+15.any  Intrinsic.tryldglobalbyname 0x[a-f0-9]+, 0x[a-f0-9]+ \\(v20\\)\\s+bc: 0x[a-f0-9]+\n"
    // CC-OFFNXT(WordsTool.190)
    "\\s+16.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v8 -> \\(v17\\)\\s+bc: 0x[a-f0-9]+\n"
    // CC-OFFNXT(WordsTool.190)
    "\\s+17.any  Intrinsic.callthis0\\s+0x[a-f0-9]+ v16, v8 -> \\(v19\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+19.any  Intrinsic.ldobjbyvalue\\s+0x[a-f0-9]+ v18, v17 -> \\(v20\\)\\s+bc: 0x[a-f0-9]+\n"
    "\\s+20.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v15, v19\\s+bc: 0x[a-f0-9]+\n"
    // CC-OFFNXT(WordsTool.190)
    "\\s+21.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v3, v8\\s+bc: 0x[a-f0-9]+\n"
    "\\s+26.any  Intrinsic.callrange\\s+0x[a-f0-9]+, 0x4 v4, v22, v23, v24, v25\\s+bc: 0x[a-f0-9]+\n"
    "\\s+27.void Intrinsic.returnundefined\\s+bc: 0x[a-f0-9]+\n"
    "succs: \\[bb 2\\]\n\n"
    "BB 2  preds: \\[bb 0\\]\n"
    "prop: end, bc: 0x[a-f0-9]+\n"
    "hotness=0\n\n\n";

class LibAbcKitIrGraphTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::gDump, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, Gdump_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto fileName = ABCKIT_ABC_DIR "ut/ir_core/graph/tmpGD.txt";
        std::FILE *fp = fopen(fileName, "we");
        ASSERT_NE(fp, nullptr);
        g_implG->gDump(graph, fileno(fp));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(fclose(fp), 0);
        std::ifstream tmpFile(fileName);
        std::string line;
        std::stringstream ss;
        ASSERT_TRUE(tmpFile.is_open());
        while (tmpFile.good()) {
            getline(tmpFile, line);
            ss << line << std::endl;
        }
        // CC-OFFNXT(G.FMT.02)
        tmpFile.close();
        std::remove(fileName);

        EXPECT_TRUE(helpers::Match(ss.str(), EXPECTED));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/graph/graph_dynamic.abc", "func_main_0", cb);
}

// Test: test-kind=api, api=GraphApiImpl::gDump, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, Gdump_2)
{
    helpers::InspectMethod(
        ABCKIT_ABC_DIR "ut/ir_core/graph/graph_static.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto fileName = ABCKIT_ABC_DIR "ut/ir_core/graph/tmpGS.txt";
            std::FILE *fp = fopen(fileName, "we");
            ASSERT_NE(fp, nullptr);
            g_implG->gDump(graph, fileno(fp));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_EQ(fclose(fp), 0);
            std::ifstream tmpFile(fileName);
            std::string line;
            std::stringstream ss;
            ASSERT_TRUE(tmpFile.is_open());
            while (tmpFile.good()) {
                getline(tmpFile, line);
                ss << line << std::endl;
            }
            tmpFile.close();
            std::remove(fileName);
            auto expected =
                "Method: graph_static.ETSGLOBAL::main 0x[a-f0-9]+\n\n"
                "BB 1\n"
                "prop: start, bc: 0x[a-f0-9]+\n"
                "hotness=0\n"
                "\\s+2.i32  Constant\\s+0x1 -> \\(v3\\)\\s+\n"
                "succs: \\[bb 0\\]\n\n"
                "BB 0  preds: \\[bb 1\\]\n"
                "prop: bc: 0x[a-f0-9]+\n"
                "hotness=0\n"
                "\\s+0.ref  Intrinsic.AbckitInitObjectShort 0x[a-f0-9]+ \\(v7, v3, v1\\)\\s+bc: 0x[a-f0-9]+\n"
                "\\s+1.void CallVirtual [a-f0-9]+ graph_static.Test::test v0\\s+bc: 0x[a-f0-9]+\n"
                "\\s+3.i32  CallVirtual [a-f0-9]+ graph_static.Test::test2 v0, v2\\s+bc: 0x[a-f0-9]+\n"
                "\\s+4.void CallStatic [a-f0-9]+ graph_static.ETSGLOBAL::test0\\s+bc: 0x[a-f0-9]+\n"
                "\\s+5.ref  Intrinsic.AbckitLoadString\\s+0x[a-f0-9]+ \\(v6\\)\\s+bc: 0x[a-f0-9]+\n"
                "\\s+6.void CallStatic [a-f0-9]+ graph_static.ETSGLOBAL::test1 v5\\s+bc: 0x[a-f0-9]+\n"
                "\\s+7.void CallVirtual [a-f0-9]+ graph_static.Test::test2 v0\\s+bc: 0x[a-f0-9]+\n"
                "\\s+8.void ReturnVoid\\s+bc: 0x[a-f0-9]+\n"
                "succs: \\[bb 2\\]\n\n"
                "BB 2  preds: \\[bb 0\\]\n"
                "prop: end, bc: 0x[a-f0-9]+\n"
                "hotness=0\n\n\n";
            EXPECT_TRUE(helpers::Match(ss.str(), expected));
        });
}

// Test: test-kind=api, api=GraphApiImpl::gGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, GgetFile_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/graph/graph_dynamic.abc", "func_main_0",
                           [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto f = g_implG->gGetFile(graph);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(f, file);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gGetFile, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, GgetFile_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/graph/graph_static.abc", "main",
                           [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto f = g_implG->gGetFile(graph);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(f, file);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gGetIsa, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, GgetIsa_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/graph/graph_dynamic.abc", "func_main_0",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto isa = g_implG->gGetIsa(graph);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(isa, AbckitIsaType::ABCKIT_ISA_TYPE_DYNAMIC);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gGetIsa, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrGraphTest, GgetIsa_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/graph/graph_static.abc", "main",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto isa = g_implG->gGetIsa(graph);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(isa, AbckitIsaType::ABCKIT_ISA_TYPE_STATIC);
                           });
}
}  // namespace libabckit::test
