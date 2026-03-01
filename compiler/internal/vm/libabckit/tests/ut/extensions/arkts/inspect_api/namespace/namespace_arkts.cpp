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
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/helpers.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArktsI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSInspectApiNamespace : public ::testing::Test {};

static const std::unordered_map<std::string, std::string> NAMESPACE_TO_FUNC = {
    {"N", "f"},
    {"N2", "f2"},
};

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsV1NamespaceGetConstructor, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiNamespace, ArktsV1NamespaceGetConstructor)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/namespace/namespace_arkts_dynamic.abc",
                           &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *m, void *) {
        g_implI->moduleEnumerateNamespaces(m, nullptr, [](AbckitCoreNamespace *n, void *) {
            auto *arkN = g_implArktsI->coreNamespaceToArktsNamespace(n);
            auto *arkF = g_implArktsI->arktsV1NamespaceGetConstructor(arkN);
            auto *f = g_implArktsI->arktsFunctionToCoreFunction(arkF);
            auto *g = g_implI->createGraphFromFunction(f);
            auto *define = helpers::FindFirstInst(g, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            EXPECT_NE(define, nullptr);
            auto *namespaceFunc = g_implG->iGetFunction(define);
            auto namespaceName = std::string(helpers::AbckitStringToString(g_implI->namespaceGetName(n)));
            auto funcName = helpers::AbckitStringToString(g_implI->functionGetName(namespaceFunc));
            EXPECT_EQ(NAMESPACE_TO_FUNC.count(namespaceName), 1);
            EXPECT_EQ(NAMESPACE_TO_FUNC.at(namespaceName), funcName);
            g_impl->destroyGraph(g);
            return true;
        });
        return true;
    });

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
