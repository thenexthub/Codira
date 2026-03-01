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

#include <sstream>
#include <iostream>

#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers_runtime.h"

#include "branch_eliminator.h"
#include "helpers/helpers.h"

namespace libabckit::test {

static constexpr auto VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
static const AbckitApi *g_impl = AbckitGetApiImpl(VERSION);
static const AbckitInspectApi *g_implI = AbckitGetInspectApiImpl(VERSION);
static const AbckitGraphApi *g_implG = AbckitGetGraphApiImpl(VERSION);
static const AbckitIsaApiDynamic *g_dynG = AbckitGetIsaApiDynamicImpl(VERSION);

static bool MethodHasBranch(const std::string &moduleName, const std::string &methodName, AbckitFile *file)
{
    EXPECT_NE(file, nullptr);
    EXPECT_NE(g_impl, nullptr);
    EXPECT_NE(g_implI, nullptr);
    EXPECT_NE(g_implG, nullptr);
    EXPECT_NE(g_dynG, nullptr);
    VisitHelper vh = VisitHelper(file, g_impl, g_implI, g_implG, g_dynG);
    bool found = false;
    bool ret = false;
    vh.EnumerateModules([&](AbckitCoreModule *mod) {
        if (vh.GetString(g_implI->moduleGetName(mod)) != moduleName) {
            return;
        }
        vh.EnumerateModuleFunctions(mod, [&](AbckitCoreFunction *func) {
            if (found || vh.GetString(g_implI->functionGetName(func)) != methodName) {
                return;
            }
            found = true;
            auto *graph = g_implI->createGraphFromFunction(func);
            auto *ifInst = vh.GraphInstsFindIf(
                graph, [&](AbckitInst *inst) { return g_dynG->iGetOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_IF; });
            ret = ifInst != nullptr;
            g_impl->destroyGraph(graph);
        });
    });

    EXPECT_TRUE(found);
    return ret;
}

class AbckitScenarioTest : public ::testing::Test {};

static const std::string DIR = std::string(ABCKIT_ABC_DIR "scenarios/branch_eliminator/dynamic/");
static const std::string INPUT = DIR + "branch_eliminator.abc";
static constexpr bool CONFIG_IS_DEBUG_ORIGIN_VALUE = false;

/*
 * @param value: the value of isDebug when we handle, it can be different from configIsDebugOriginValue
 */
static std::string GetExpectOutput(bool value)
{
    std::stringstream expectOutput;
    expectOutput << std::boolalpha;
    expectOutput << "Config.isDebug = " << value << std::endl;
    expectOutput << "myfoo: Config.isDebug is " << value << std::endl;
    expectOutput << "Mybar.test1: Config.isDebug is " << value << std::endl;
    expectOutput << "Mybar.test2: Config.isDebug is " << value << std::endl;
    return expectOutput.str();
}

static void GeneralBranchEliminatorTest(bool configIsDebugFinal)
{
    auto *impl = AbckitGetApiImpl(VERSION);
    const auto originOutput = helpers::ExecuteDynamicAbc(INPUT, "branch_eliminator");

    auto expectOutput = GetExpectOutput(CONFIG_IS_DEBUG_ORIGIN_VALUE);
    ASSERT_EQ(originOutput, expectOutput);

    AbckitFile *file = impl->openAbc(INPUT.c_str(), INPUT.size());
    ASSERT_NE(file, nullptr);
    ASSERT_TRUE(MethodHasBranch("modules/myfoo", "myfoo", file));
    ASSERT_TRUE(MethodHasBranch("modules/mybar", "test1", file));
    ASSERT_TRUE(MethodHasBranch("modules/mybar", "test2", file));
    // Delete true branch
    const std::vector<ConstantInfo> infos = {{"modules/config", "Config", "isDebug", configIsDebugFinal}};
    BranchEliminator b(VERSION, file, infos);
    b.Run();
    ASSERT_FALSE(MethodHasBranch("modules/myfoo", "myfoo", file));
    ASSERT_FALSE(MethodHasBranch("modules/mybar", "test1", file));
    ASSERT_FALSE(MethodHasBranch("modules/mybar", "test2", file));

    const auto modifiedPath = DIR + "branch_eliminator_modified.abc";
    impl->writeAbc(file, modifiedPath.c_str(), modifiedPath.size());
    g_impl->closeFile(file);
    auto output = helpers::ExecuteDynamicAbc(modifiedPath, "branch_eliminator");
    auto expectOutput2 = GetExpectOutput(configIsDebugFinal);
    ASSERT_EQ(output, expectOutput2);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestBranchEliminatorDynamic1)
{
    GeneralBranchEliminatorTest(false);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestBranchEliminatorDynamic2)
{
    GeneralBranchEliminatorTest(true);
}

}  // namespace libabckit::test
