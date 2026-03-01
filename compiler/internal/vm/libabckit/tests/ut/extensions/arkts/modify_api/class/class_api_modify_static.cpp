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

#include <cstring>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <vector>

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/include/libabckit/c/abckit.h"
#include "libabckit/include/libabckit/c/declarations.h"
#include "libabckit/include/libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/libabckit/c/metadata_core.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_staticG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyClassApiTests : public ::testing::Test {};

struct CaptureData {
    AbckitGraph *graph = nullptr;
    AbckitCoreFunction *mainFunc = nullptr;
    AbckitCoreFunction *targetFunc = nullptr;
};

void ClassAddIfaceTransformMainIr(AbckitCoreFunction *mainFunction, AbckitCoreFunction *targetFunc)
{
    AbckitGraph *graph = g_implI->createGraphFromFunction(mainFunction);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    CaptureData capData;
    capData.graph = graph;
    capData.targetFunc = targetFunc;
    g_implG->gVisitBlocksRpo(graph, &capData, [](AbckitBasicBlock *bb, void *data) -> bool {
        CaptureData *capData = reinterpret_cast<CaptureData *>(data);
        for (auto inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst)) {
            if (g_staticG->iGetOpcode(inst) == ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID) {
                auto newInst = g_staticG->iCreateCallStatic(capData->graph, capData->targetFunc, 0);
                g_implG->iInsertBefore(newInst, inst);
            }
        }
        return true;
    });

    g_implM->functionSetGraph(mainFunction, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

AbckitCoreClass *GetAbckitCoreClass(AbckitFile *file, std::string moduleName, std::string className)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::ClassByNameContext classFinder = {nullptr, className.c_str()};
    g_implI->moduleEnumerateClasses(module, &classFinder, helpers::ClassByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(classFinder.klass != nullptr);

    return classFinder.klass;
}

void ClassRemoveFieldTransformSayHelloIr(AbckitGraph *graph)
{
    auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT);
    g_implG->iRemove(call);
    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    auto preInst = g_implG->iGetPrev(ret);
    g_implG->iRemove(preInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

TEST_F(LibAbcKitModifyClassApiTests, ClassRemoveFieldTest0)
{
    std::string input1 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field_static_delete.abc";
    std::string removeCallPath =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field_static_del.abc";
    std::string removeCallPath1 =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field_static_del_1.abc";
    std::string output1 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field_static_del_out.abc";

    auto outPutRst = helpers::ExecuteStaticAbc(input1, "class_remove_field_static_delete", "main");
    EXPECT_TRUE(helpers::Match(outPutRst, "yyy\n"));
    helpers::TransformMethod(
        input1, removeCallPath, "sayHello:class_remove_field_static_delete.Student;void;",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { ClassRemoveFieldTransformSayHelloIr(graph); });
    helpers::TransformMethod(removeCallPath, removeCallPath1, "_ctor_:class_remove_field_static_delete.Student;void;",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT);
                                 g_implG->iRemove(inst);
                             });
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(removeCallPath1.c_str(), &file);
    auto klass = GetAbckitCoreClass(file, "class_remove_field_static_delete", "Student");

    helpers::CoreClassField ctxClassFieldFinder = {nullptr, "addr"};
    g_implI->classEnumerateFields(klass, &ctxClassFieldFinder, helpers::ClassFieldFinder);

    bool ret = g_implArkM->classRemoveField(g_implArkI->coreClassToArktsClass(klass),
                                            g_implArkI->coreClassFieldToArktsClassField(ctxClassFieldFinder.filed));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    g_impl->writeAbc(file, output1.c_str(), output1.length());
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(output1.c_str(), &file);
    klass = GetAbckitCoreClass(file, "class_remove_field_static_delete", "Student");
    ctxClassFieldFinder = {nullptr, "addr"};
    g_implI->classEnumerateFields(klass, &ctxClassFieldFinder, helpers::ClassFieldFinder);
    ASSERT_EQ(ctxClassFieldFinder.filed, nullptr);
    g_impl->closeFile(file);
}

TEST_F(LibAbcKitModifyClassApiTests, ClassRemoveFieldTest1)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_field_static_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_remove_field"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext ctxClassFinder = {nullptr, "Anm"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &ctxClassFinder, helpers::ClassByNameFinder);

    helpers::CoreClassField ctxClassFieldFinder = {nullptr, "age"};
    g_implI->classEnumerateFields(ctxClassFinder.klass, &ctxClassFieldFinder, helpers::ClassFieldFinder);

    bool ret = g_implArkM->classRemoveField(g_implArkI->coreClassToArktsClass(ctxClassFinder.klass),
                                            g_implArkI->coreClassFieldToArktsClassField(ctxClassFieldFinder.filed));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(output.c_str(), &file);
    ctxFinder = {nullptr, "class_remove_field"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    ctxClassFinder = {nullptr, "Anm"};
    g_implI->moduleEnumerateClasses(module, &ctxClassFinder, helpers::ClassByNameFinder);
    ASSERT_NE(ctxClassFinder.klass, nullptr);
    auto klass = ctxClassFinder.klass;
    ctxClassFieldFinder = {nullptr, "age"};
    g_implI->classEnumerateFields(klass, &ctxClassFieldFinder, helpers::ClassFieldFinder);
    ASSERT_EQ(ctxClassFieldFinder.filed, nullptr);
    g_impl->closeFile(file);
}
}  // namespace libabckit::test
