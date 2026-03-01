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
#include <gtest/gtest.h>

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_static/ir_static.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace {
constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/classes/classes_static_modify.abc";
constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/classes/classes_static_modify_modified.abc";
constexpr auto NEW_NAME = "New";
}  // namespace

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiClassesTest : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::classSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiClassesTest, ClassSetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;

    helpers::ClassByNameContext classFinder = {nullptr, "Class1"};
    g_implI->moduleEnumerateClasses(module, &classFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classFinder.klass, nullptr);
    auto klass = classFinder.klass;
    auto arkC = g_implArkI->coreClassToArktsClass(klass);
    g_implArkM->classSetName(arkC, NEW_NAME);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    helpers::ModuleByNameContext newFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &newFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newFinder.module, nullptr);
    auto newModule = newFinder.module;

    helpers::ClassByNameContext newClassFinder = {nullptr, NEW_NAME};
    g_implI->moduleEnumerateClasses(newModule, &newClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newClassFinder.klass, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "classes_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "classes_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiClassesTest, AbstractClassSetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;

    helpers::ClassByNameContext classFinder = {nullptr, "Class2"};
    g_implI->moduleEnumerateClasses(module, &classFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classFinder.klass, nullptr);
    auto klass = classFinder.klass;
    auto arkC = g_implArkI->coreClassToArktsClass(klass);
    g_implArkM->classSetName(arkC, NEW_NAME);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    helpers::ModuleByNameContext newFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &newFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newFinder.module, nullptr);
    auto newModule = newFinder.module;

    helpers::ClassByNameContext newClassFinder = {nullptr, NEW_NAME};
    g_implI->moduleEnumerateClasses(newModule, &newClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newClassFinder.klass, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "classes_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "classes_static_modify", "main"));
}

// Test: test-kind=api, api=ClassAddMethodStatic, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiClassesTest, ClassAddInstanceMethodStatic)
{
    // Class1 add method fooo() { return this.field1 + 1 ;}
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext classFinder = {nullptr, "Class1"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classFinder.klass, nullptr);

    auto *arkClass = g_implArkI->coreClassToArktsClass(classFinder.klass);
    ASSERT_NE(arkClass, nullptr);

    ArktsMethodCreateParams methodParams;
    methodParams.name = "fooo";
    methodParams.params = nullptr;
    methodParams.returnType = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
    methodParams.isStatic = false;
    methodParams.isAsync = false;
    methodParams.methodVisibility = ABCKIT_ARKTS_METHOD_VISIBILITY_PUBLIC;

    auto *newMethod = ClassAddMethodStatic(arkClass, &methodParams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newMethod, nullptr);
    // add function body
    auto *graph = g_implI->createGraphFromFunction(newMethod->core);
    ASSERT_NE(graph, nullptr);

    auto *thisParam = g_implG->gGetParameter(graph, 0);
    ASSERT_NE(thisParam, nullptr);

    helpers::CoreClassField fieldFinder = {nullptr, "field1"};
    g_implI->classEnumerateFields(classFinder.klass, &fieldFinder, helpers::ClassFieldFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(fieldFinder.filed, nullptr);

    auto strField1 = g_implM->createString(file, "classes_static_modify.Class1.field1",
                                           strlen("classes_static_modify.Class1.field1"));
    auto *loadField = g_statG->iCreateLoadObject(graph, thisParam, strField1, ABCKIT_TYPE_ID_I32);
    ASSERT_NE(loadField, nullptr);
    auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 1);
    ASSERT_NE(const1, nullptr);

    auto *addInst = g_statG->iCreateAdd(graph, loadField, const1);
    ASSERT_NE(addInst, nullptr);

    auto *returnInst = g_statG->iCreateReturn(graph, addInst);
    ASSERT_NE(returnInst, nullptr);

    auto *originalReturn = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    if (originalReturn != nullptr) {
        g_implG->iInsertBefore(loadField, originalReturn);
        g_implG->iInsertBefore(addInst, originalReturn);
        g_implG->iInsertBefore(returnInst, originalReturn);
        g_implG->iRemove(originalReturn);
    }

    g_implM->functionSetGraph(newMethod->core, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // modify main function to call the new added method
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);

    auto *mainFunc = helpers::FindMethodByName(file, "main");
    auto *foooFunc = helpers::FindMethodByName(file, "fooo");
    auto *mainGraph = g_implI->createGraphFromFunction(mainFunc);
    ASSERT_NE(mainGraph, nullptr);
    ASSERT_NE(foooFunc, nullptr);

    auto *consoleLog = helpers::FindFirstInst(mainGraph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(consoleLog, nullptr);

    auto *newObjectInst = helpers::FindFirstInst(mainGraph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(newObjectInst, nullptr);
    auto *callInst = g_statG->iCreateCallVirtual(mainGraph, newObjectInst, foooFunc, 0);
    ASSERT_NE(callInst, nullptr);

    g_implG->iInsertBefore(callInst, consoleLog);
    g_implG->iSetInput(consoleLog, callInst, 1);

    g_implM->functionSetGraph(mainFunc, mainGraph);
    g_impl->destroyGraph(mainGraph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(OUTPUT_PATH, "classes_static_modify", "main"), "2\n");
}

// Test: test-kind=api, api=ClassAddMethodStatic, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiClassesTest, ClassAddStaticMethodStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "classes_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext classFinder = {nullptr, "Class1"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classFinder.klass, nullptr);

    auto *arkClass = g_implArkI->coreClassToArktsClass(classFinder.klass);
    ASSERT_NE(arkClass, nullptr);

    ArktsMethodCreateParams staticMethodParams;
    staticMethodParams.name = "staticUtilMethod";
    staticMethodParams.params = nullptr;
    staticMethodParams.returnType = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
    staticMethodParams.isStatic = true;  // 静态方法
    staticMethodParams.isAsync = false;
    staticMethodParams.methodVisibility = ABCKIT_ARKTS_METHOD_VISIBILITY_PUBLIC;

    auto *staticMethod = ClassAddMethodStatic(arkClass, &staticMethodParams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(staticMethod, nullptr);

    auto *graph = g_implI->createGraphFromFunction(staticMethod->core);
    ASSERT_NE(graph, nullptr);

    auto *const42 = g_implG->gFindOrCreateConstantI32(graph, 42);
    ASSERT_NE(const42, nullptr);

    auto *returnInst = g_statG->iCreateReturn(graph, const42);
    ASSERT_NE(returnInst, nullptr);

    auto *originalReturn = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_NE(originalReturn, nullptr);

    g_implG->iInsertBefore(returnInst, originalReturn);
    g_implG->iRemove(originalReturn);

    g_implM->functionSetGraph(staticMethod->core, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *mainFunc = helpers::FindMethodByName(file, "main");
    auto *mainGraph = g_implI->createGraphFromFunction(mainFunc);
    ASSERT_NE(mainGraph, nullptr);
    g_implG->gDump(mainGraph, 1);
    auto *consoleLog = helpers::FindFirstInst(mainGraph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    g_implG->iDump(consoleLog, 1);
    ASSERT_NE(consoleLog, nullptr);

    auto *callInst = g_statG->iCreateCallStatic(mainGraph, staticMethod->core, 0);
    ASSERT_NE(callInst, nullptr);

    g_implG->iInsertBefore(callInst, consoleLog);
    g_implG->iSetInput(consoleLog, callInst, 1);

    g_implM->functionSetGraph(mainFunc, mainGraph);
    g_impl->destroyGraph(mainGraph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(OUTPUT_PATH, "classes_static_modify", "main"), "42\n");
}

}  // namespace libabckit::test
