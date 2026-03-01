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

const std::string ROUTER_MAP_FILE_MODULE_NAME = "modules/routerMap";
const std::string ROUTER_ANNOTATION_NAME = "Router";
const std::string FUNC_MAIN_0 = "func_main_0";

struct RouterAnnotation {
    abckit::core::Class owner;
    abckit::core::Annotation anno;
    std::string scheme;
    std::string path;
};

struct UserData {
    std::string classStr;
    std::string moduleStr;
    RouterAnnotation routerInfo;
};

abckit::Instruction FindFirstInst(const abckit::Graph &graph, AbckitIsaApiDynamicOpcode opcode)
{
    std::vector<abckit::BasicBlock> bbs = graph.GetBlocksRPO();
    for (const auto &bb : bbs) {
        auto curInst = bb.GetFirstInst();
        while (curInst) {
            if (curInst.GetGraph()->DynIsa().GetOpcode(curInst) == opcode) {
                return curInst;
            }
            curInst = curInst.GetNext();
        }
    }
    return abckit::Instruction();
}

void TransformMethod(const abckit::core::Function &method, const UserData &ud,
                     const abckit::core::ImportDescriptor &coreImport)
{
    size_t idx = 0;
    auto ctxG = method.CreateGraph();
    abckit::BasicBlock startBB = ctxG.GetStartBb();
    std::vector<abckit::BasicBlock> succBBs = startBB.GetSuccs();
    abckit::Instruction createEmptyArray = FindFirstInst(ctxG, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY);

    const auto &routerInfo = ud.routerInfo;
    std::string fullPath = routerInfo.scheme + routerInfo.path;
    auto arr = std::vector<abckit::Literal>();
    auto filePtr = method.GetFile();
    abckit::Literal str = filePtr->CreateLiteralString(fullPath);
    arr.emplace_back(str);

    auto litArr = filePtr->CreateLiteralArray(arr);
    auto createArrayWithBuffer = ctxG.DynIsa().CreateCreatearraywithbuffer(litArr);

    auto insertAfterInst = createEmptyArray;

    auto ldExternal = ctxG.DynIsa().CreateLdexternalmodulevar(coreImport);
    auto classThrow = ctxG.DynIsa().CreateThrowUndefinedifholewithname(ldExternal, routerInfo.owner.GetName());
    auto newObj = ctxG.DynIsa().CreateNewobjrange(ldExternal);
    auto stownByIndex1 = ctxG.DynIsa().CreateStownbyindex(newObj, createArrayWithBuffer, 1);
    auto stownByIndex2 = ctxG.DynIsa().CreateStownbyindex(createArrayWithBuffer, createEmptyArray, idx++);

    createArrayWithBuffer.InsertAfter(insertAfterInst);
    ldExternal.InsertAfter(createArrayWithBuffer);
    classThrow.InsertAfter(ldExternal);
    newObj.InsertAfter(classThrow);
    stownByIndex1.InsertAfter(newObj);
    stownByIndex2.InsertAfter(stownByIndex1);
    insertAfterInst = stownByIndex2;

    method.SetGraph(ctxG);
}

void ModifyRouterTable(const abckit::core::Function &method, const std::vector<UserData> &udContainer)
{
    for (const auto &ud : udContainer) {
        std::optional<abckit::arkts::Module> foundModule;

        method.GetFile()->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
            if (ud.moduleStr == mod.GetName()) {
                foundModule = abckit::arkts::Module(mod);
            }
            return true;
        });
        EXPECT_TRUE(foundModule.has_value());

        abckit::arkts::Module arktsModule = abckit::arkts::Module(method.GetModule());
        auto newImport = arktsModule.AddImportFromArktsV1ToArktsV1(*foundModule, ud.classStr, ud.classStr);
        auto coreImport = *static_cast<abckit::core::ImportDescriptor *>(&newImport);
        TransformMethod(method, ud, coreImport);
    }
}

abckit::core::Function FindMethodWithRouterTable(const abckit::File *filePtr)
{
    abckit::core::Function routerFunc;
    filePtr->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        if (mod.GetName() != ROUTER_MAP_FILE_MODULE_NAME) {
            return true;
        }
        mod.EnumerateTopLevelFunctions([&](const abckit::core::Function &func) -> bool {
            if (func.GetName() == FUNC_MAIN_0) {
                routerFunc = func;
            }
            return true;
        });
        return true;
    });
    return routerFunc;
}

void RemoveAnnotations(const std::vector<UserData> &udContainer)
{
    for (const auto &ud : udContainer) {
        const auto &routerInfo = ud.routerInfo;
        abckit::arkts::Class(routerInfo.owner).RemoveAnnotation(abckit::arkts::Annotation(routerInfo.anno));
    }
}

void CollectClassInfo(std::vector<UserData> &udContainer, const abckit::core::Module &mod,
                      const abckit::core::Class &klass)
{
    klass.EnumerateAnnotations([&](const abckit::core::Annotation &anno) -> bool {
        auto annoClass = anno.GetInterface();
        auto annoName = annoClass.GetName();
        if (annoName != ROUTER_ANNOTATION_NAME) {
            return true;
        }

        std::string scheme;
        std::string path;
        anno.EnumerateElements([&](const abckit::core::AnnotationElement &annoElem) -> bool {
            auto annoElemName = annoElem.GetName();
            auto value = annoElem.GetValue();
            if (annoElemName == "scheme") {
                scheme = value.GetString();
            }
            if (annoElemName == "path") {
                path = value.GetString();
            }
            return true;
        });
        UserData ud {klass.GetName(), mod.GetName(), RouterAnnotation {klass, anno, scheme, path}};
        udContainer.push_back(ud);
        return true;
    });
}

bool ClassHasAnnotation(const abckit::File *filePtr, const UserData &ud)
{
    bool found = false;

    auto classAnnotationsEnumCb = [&](const abckit::core::Annotation &anno) -> bool {
        auto annoClass = anno.GetInterface();
        if (annoClass.GetName() == ROUTER_ANNOTATION_NAME) {
            found = true;
        }
        return true;
    };

    filePtr->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        if (mod.GetName() != ud.moduleStr) {
            return true;
        }
        mod.EnumerateClasses([&](const abckit::core::Class &klass) -> bool {
            if (klass.GetName() == ud.classStr) {
                klass.EnumerateAnnotations(classAnnotationsEnumCb);
            }
            return true;
        });
        return true;
    });
    return found;
}

void CollectClassesInfo(const abckit::File *filePtr, std::vector<UserData> &udContainer)
{
    filePtr->EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        mod.EnumerateClasses([&](const abckit::core::Class &klass) -> bool {
            CollectClassInfo(udContainer, mod, klass);
            return true;
        });
        return true;
    });
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCppTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitScenarioCppTestClean, LibAbcKitTestDynammicRouterTableClean)
{
    const std::string testSandboxPath = ABCKIT_ABC_DIR "clean_scenarios/cpp_api/dynamic/router_table/";
    const std::string inputPath = testSandboxPath + "router_table.abc";
    const std::string outputPath = testSandboxPath + "router_table_modified.abc";

    auto output = helpers::ExecuteDynamicAbc(inputPath, "router_table");
    EXPECT_TRUE(helpers::Match(output, ""));

    abckit::File file(inputPath, std::make_unique<ErrorHandler>());

    const abckit::File *filePtr = &file;

    std::vector<UserData> userData;

    CollectClassesInfo(filePtr, userData);
    RemoveAnnotations(userData);
    auto method = FindMethodWithRouterTable(filePtr);
    ModifyRouterTable(method, userData);

    file.WriteAbc(outputPath);

    for (const auto &ud : userData) {
        ASSERT_FALSE(ClassHasAnnotation(filePtr, ud));
    }

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/router_table/router_table_modified.abc", "router_table");

    EXPECT_TRUE(helpers::Match(output,
                               "xxxHandler.handle was called\n"
                               "handle1/xxx\n"));
}

}  // namespace libabckit::test
