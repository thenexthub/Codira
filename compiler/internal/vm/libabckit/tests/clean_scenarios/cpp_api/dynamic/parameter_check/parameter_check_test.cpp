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

#include "libabckit/cpp/abckit_cpp.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

namespace {

class ErrorHandler final : public abckit::IErrorHandler {
    void HandleError(abckit::Exception &&e) override
    {
        EXPECT_TRUE(false) << "Abckit exception raised: " << e.what();
    }
};

inline void TransformMethod(const abckit::core::Function &f,
                            const std::function<void(const abckit::File *, abckit::core::Function)> &cb)
{
    cb(f.GetFile(), f);
}

void AddParamChecker(const abckit::core::Function &method)
{
    abckit::Graph graph = method.CreateGraph();

    TransformMethod(method, [&]([[maybe_unused]] const abckit::File *file, const abckit::core::Function &method) {
        abckit::BasicBlock startBB = graph.GetStartBb();
        abckit::Instruction idx = startBB.GetLastInst();
        abckit::Instruction arr = idx.GetPrev();

        std::vector<abckit::BasicBlock> succBBs = startBB.GetSuccs();

        std::string str = "length";

        abckit::Instruction constant = graph.FindOrCreateConstantI32(-1);
        abckit::Instruction arrLength = graph.DynIsa().CreateLdobjbyname(arr, str);

        abckit::BasicBlock trueBB = succBBs[0];
        startBB.EraseSuccBlock(ABCKIT_TRUE_SUCC_IDX);
        abckit::BasicBlock falseBB = graph.CreateEmptyBb();
        abckit::BasicBlock endBb = graph.GetEndBb();
        falseBB.AppendSuccBlock(endBb);
        falseBB.AddInstBack(graph.DynIsa().CreateReturn(constant));
        abckit::BasicBlock ifBB = graph.CreateEmptyBb();
        abckit::Instruction intrinsicGreatereq = graph.DynIsa().CreateGreatereq(arrLength, idx);
        abckit::Instruction ifInst =
            graph.DynIsa().CreateIf(intrinsicGreatereq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
        ifBB.AddInstBack(arrLength);
        ifBB.AddInstBack(intrinsicGreatereq);
        ifBB.AddInstBack(ifInst);
        startBB.AppendSuccBlock(ifBB);
        ifBB.AppendSuccBlock(trueBB);
        ifBB.AppendSuccBlock(falseBB);

        method.SetGraph(graph);
    });
}

struct MethodInfo {
    std::string path;
    std::string className;
    std::string methodName;
};

abckit::core::ImportDescriptor GetImportDescriptor(const abckit::core::Module &module, MethodInfo &methodInfo)
{
    abckit::core::ImportDescriptor impDescriptor;
    module.EnumerateImports([&](const abckit::core::ImportDescriptor &id) -> bool {
        auto importName = id.GetName();
        auto importedModule = id.GetImportedModule();
        auto source = importedModule.GetName();
        if (source != methodInfo.path) {
            return false;
        }
        if (importName == methodInfo.className) {
            impDescriptor = id;
            return true;
        }
        return false;
    });
    return impDescriptor;
}

inline void EnumerateModuleFunctions(const abckit::core::Module &mod,
                                     const std::function<bool(abckit::core::Function)> &cb)
{
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    mod.EnumerateTopLevelFunctions(cb);
    mod.EnumerateClasses([&](const abckit::core::Class &klass) -> bool {
        klass.EnumerateMethods(cb);
        return true;
    });
}

inline void EnumerateFunctionInsts(const abckit::core::Function &func,
                                   const std::function<void(abckit::Instruction)> &cb)
{
    abckit::Graph graph = func.CreateGraph();
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) {
        for (auto inst = bb.GetFirstInst(); inst; inst = inst.GetNext()) {
            cb(inst);
        }
        return true;
    });
}

abckit::core::Function GetMethodToModify(const abckit::core::Class &klass, MethodInfo &methodInfo)
{
    abckit::core::Function foundMethod;
    klass.EnumerateMethods([&](const abckit::core::Function &method) -> bool {
        auto name = method.GetName();
        if (name == methodInfo.methodName) {
            foundMethod = method;
        }
        return true;
    });
    return foundMethod;
}

abckit::core::Function GetSubclassMethod(const abckit::core::ImportDescriptor &id, const abckit::Instruction &inst,
                                         MethodInfo &methodInfo)
{
    abckit::core::Function foundMethod;
    if (inst.GetGraph()->DynIsa().GetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return foundMethod;
    }

    if (inst.GetGraph()->DynIsa().GetImportDescriptor(inst) != id) {
        return foundMethod;
    }

    inst.VisitUsers([&](const abckit::Instruction &user) {
        if (user.GetGraph()->DynIsa().GetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto method = user.GetFunction();
            auto klass = method.GetParentClass();
            foundMethod = GetMethodToModify(klass, methodInfo);
            return false;
        }
        return true;
    });

    return foundMethod;
}
void ModifyFunction(const abckit::core::Function &method, abckit::core::ImportDescriptor id, MethodInfo &methodInfo)
{
    EnumerateFunctionInsts(method, [&](const abckit::Instruction &inst) {
        auto subclassMethod = GetSubclassMethod(id, inst, methodInfo);
        if (subclassMethod) {
            AddParamChecker(subclassMethod);
        }
    });
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynamicParameterCheckClean)
{
    const std::string testSandboxPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/parameter_check/";
    const std::string inputAbcPath = testSandboxPath + "parameter_check.abc";
    const std::string outputAbcPath = testSandboxPath + "parameter_check_modified.abc";
    const std::string entryPoint = "parameter_check";

    abckit::File file(inputAbcPath, std::make_unique<ErrorHandler>());

    auto output = helpers::ExecuteDynamicAbc(inputAbcPath, entryPoint);
    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "undefined\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "undefined\n"));

    MethodInfo methodInfo = {"modules/base", "Handler", "handle"};

    file.EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        abckit::core::ImportDescriptor impDescriptor = GetImportDescriptor(mod, methodInfo);
        if (impDescriptor) {
            EnumerateModuleFunctions(mod, [&](const abckit::core::Function &method) -> bool {
                ModifyFunction(method, impDescriptor, methodInfo);
                return true;
            });
        }
        return true;
    });

    file.WriteAbc(outputAbcPath);

    output = helpers::ExecuteDynamicAbc(outputAbcPath, entryPoint);

    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "-1\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "-1\n"));
}

}  // namespace libabckit::test
