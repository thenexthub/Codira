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

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {

class GTestAssertErrorHandler final : public abckit::IErrorHandler {
public:
    void HandleError(abckit::Exception &&err) override
    {
        EXPECT_TRUE(false) << "Abckit expection raised: " << err.what();
    }
};

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

static abckit::core::Function FindRunFunc(const abckit::File *file)
{
    std::vector<abckit::core::Function> functions;
    for (auto &module : file->GetModules()) {
        for (auto &klass : module.GetClasses()) {
            for (auto &function : klass.GetAllMethods()) {
                functions.push_back(function);
            }
        }
        for (auto &function : module.GetTopLevelFunctions()) {
            functions.push_back(function);
        }
    }

    abckit::core::Function runFunc;
    for (auto &func : functions) {
        if (func.GetName() == "run") {
            runFunc = func;
            break;
        }
    }

    return runFunc;
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynamicAddTryCatchClean)
{
    const std::string inputPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/add_try_catch/add_try_catch.abc";
    auto output = helpers::ExecuteDynamicAbc(inputPath, "add_try_catch");
    EXPECT_TRUE(helpers::Match(output, "THROW\n"));

    abckit::File file(inputPath, std::make_unique<GTestAssertErrorHandler>());

    auto runFunc = FindRunFunc(&file);
    ASSERT_TRUE(runFunc);

    abckit::Graph graph = runFunc.CreateGraph();
    abckit::BasicBlock startBB = graph.GetStartBb();
    abckit::BasicBlock endBB = graph.GetEndBb();
    abckit::BasicBlock bb = startBB.GetSuccs()[0];
    abckit::Instruction initInst = bb.GetFirstInst();
    abckit::Instruction prevRetInst = bb.GetLastInst().GetPrev();
    abckit::BasicBlock tryBegin = bb.SplitBlockAfterInstruction(initInst, true);
    abckit::BasicBlock tryEnd = prevRetInst.GetBasicBlock().SplitBlockAfterInstruction(prevRetInst, true);

    // Fill catch block
    abckit::BasicBlock catchBlock = graph.CreateEmptyBb();
    abckit::Instruction catchPhi = catchBlock.CreateCatchPhi();
    abckit::Instruction print = graph.DynIsa().CreateTryldglobalbyname("print");
    abckit::Instruction callArg = graph.DynIsa().CreateCallarg1(print, catchPhi);
    catchBlock.AddInstBack(print).AddInstBack(callArg);

    abckit::BasicBlock epilogueBB = endBB.GetPreds()[0];
    abckit::Instruction retInst = epilogueBB.GetLastInst();
    abckit::Instruction firstPhiInput = retInst.GetInput(0);
    abckit::Instruction secondPhiInput = graph.DynIsa().CreateLdfalse();
    bb.AddInstBack(secondPhiInput);
    abckit::Instruction phiInst = epilogueBB.CreatePhi(firstPhiInput, secondPhiInput);
    retInst.SetInput(0, phiInst);

    graph.InsertTryCatch(tryBegin, tryEnd, catchBlock, catchBlock);

    runFunc.SetGraph(graph);

    const std::string modifiedPath =
        ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/add_try_catch/add_try_catch_modified.abc";
    file.WriteAbc(modifiedPath);

    output = helpers::ExecuteDynamicAbc(modifiedPath, "add_try_catch");
    EXPECT_TRUE(helpers::Match(output,
                               "THROW\n"
                               "Error: DUMMY_ERROR\n"
                               "false\n"));
}

}  // namespace libabckit::test
