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

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"

#include "helpers/helpers.h"

#include <gtest/gtest.h>
#include <ostream>
#include <string>
#include <unordered_map>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

std::string GetMethodName(AbckitCoreFunction *method)
{
    auto mname = g_implI->functionGetName(method);
    auto methodName = helpers::AbckitStringToString(mname);
    auto modname = g_implI->moduleGetName(g_implI->functionGetModule(method));
    auto moduleName = helpers::AbckitStringToString(modname);
    auto *cls = g_implI->functionGetParentClass(method);
    if (cls == nullptr) {
        return std::string(moduleName) + ' ' + std::string(methodName);
    }
    auto cname = g_implI->classGetName(cls);
    auto className = helpers::AbckitStringToString(cname);
    return std::string(moduleName) + ' ' + std::string(className) + '.' + std::string(methodName);
}

void EnumerateInsts(AbckitBasicBlock *bb, std::unordered_map<std::string, int> *methodsMap)
{
    auto *inst = g_implG->bbGetFirstInst(bb);
    while (inst != nullptr) {
        if (g_implG->iCheckIsCall(inst)) {
            auto *method = g_implG->iGetFunction(inst);
            if (method != nullptr && !g_implI->functionIsExternal(method)) {
                auto methodName = GetMethodName(method);
                methodsMap->at(methodName) += 1;
            }
        }
        inst = g_implG->iGetNext(inst);
    }
}

void VisitAllBBs(AbckitGraph *graph, std::unordered_map<std::string, int> &methodsMap)
{
    g_implG->gVisitBlocksRpo(graph, &methodsMap, [](AbckitBasicBlock *bb, void *data) {
        EnumerateInsts(bb, reinterpret_cast<std::unordered_map<std::string, int> *>(data));
        return true;
    });
}

bool CheckRessult(std::unordered_map<std::string, int> &methodsMap)
{
    std::unordered_map<std::string, int> expected = {
        {"api_scanner_static main:void;", 0},
        {"api_scanner_static bar:void;", 1},
        {"api_scanner_static bar2:void;", 0},
        {"api_scanner_static _cctor_:void;", 0},
        {"api_scanner_static MyClass.foo2:api_scanner_static.MyClass;void;", 0},
        {"api_scanner_static MyClass.foo1:api_scanner_static.MyClass;void;", 2},
        {"api_scanner_static bar3:void;", 0},
        {"api_scanner_static MyClass._ctor_:api_scanner_static.MyClass;void;", 1}};

    if (expected.size() != methodsMap.size()) {
        return false;
    }

    for (auto &exp : expected) {
        auto f = methodsMap.find(exp.first);
        if (f == methodsMap.end()) {
            return false;
        }
        if (f->second != exp.second) {
            return false;
        }
    }
    return true;
}

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestApiScannerStatic)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "scenarios/api_scanner/static/api_scanner_static.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));

    std::unordered_map<std::string, int> methodsMap;
    helpers::EnumerateAllMethods(file,
                                 [&](AbckitCoreFunction *method) { methodsMap.emplace(GetMethodName(method), 0); });

    helpers::EnumerateAllMethods(file, [&](AbckitCoreFunction *method) {
        auto *graph = g_implI->createGraphFromFunction(method);
        VisitAllBBs(graph, methodsMap);
        g_impl->destroyGraph(graph);
    });
    for (auto &foo : methodsMap) {
        LIBABCKIT_LOG_TEST(DEBUG) << foo.first << "    " << foo.second << std::endl;
    }

    ASSERT_TRUE(CheckRessult(methodsMap));

    constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "scenarios/api_scanner/static/api_scanner_static_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
