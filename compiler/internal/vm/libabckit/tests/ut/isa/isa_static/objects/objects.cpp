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
#include "logger.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "ir_impl.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct LogFunctions {
    AbckitCoreFunction *consoleLogStr = nullptr;
    AbckitCoreFunction *consoleLogNum = nullptr;
    AbckitCoreFunction *consoleLogInt = nullptr;
};

class LibAbcKitICreateObjectsStaticTest : public ::testing::Test {};

static void TransformIrCreateNewObject(AbckitGraph *graph)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "objects"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext ctxClassFinder = {nullptr, "B"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &ctxClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxClassFinder.klass, nullptr);

    auto method = helpers::FindMethodByName(graph->file, "bar");

    AbckitInst *newObjectInst = g_statG->iCreateNewObject(graph, ctxClassFinder.klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    g_implG->iInsertBefore(newObjectInst, lastInst);
    AbckitInst *callInst = g_statG->iCreateCallStatic(graph, method, 1, newObjectInst);
    g_implG->iInsertAfter(callInst, newObjectInst);
}

static AbckitInst *GetInitObjectInst(AbckitGraph *graph, AbckitInst *constantInst, size_t numArg)
{
    switch (numArg) {
        case 0: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg);
        }
        case 1: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst);
        }
        case 2U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst);
        }
        case 3U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst);
        }
        case 4U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst,
                                              constantInst);
        }
        case 5U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst,
                                              constantInst, constantInst);
        }
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCreateInitObject(AbckitGraph *graph, size_t numArg)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "objects"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext ctxClassFinder = {nullptr, "C"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &ctxClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxClassFinder.klass, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    AbckitInst *constantInst = numArg > 0 ? g_implG->gFindOrCreateConstantI32(graph, 0x1) : nullptr;
    if (constantInst != nullptr) {
        g_implG->iInsertBefore(constantInst, lastInst);
    }

    g_implG->iInsertBefore(GetInitObjectInst(graph, constantInst, numArg), lastInst);
}

void InitObjectTest(size_t numArg, helpers::InstSchema<AbckitIsaApiStaticOpcode> initObjectInst,
                    const std::string &expectedOutput)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc", "objects", "main");
    EXPECT_TRUE(helpers::Match(output, "A\n"));

    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> startBB = {};
    if (numArg != 0) {
        startBB = {{0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}};
    }

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        [numArg](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateInitObject(graph, numArg);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
                {{}, {1}, startBB},
                {
                    {0},
                    {2U},
                    {
                        {1, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                        {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {1}},
                        initObjectInst,
                        {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},

                    },
                },
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchema);
        });

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "objects", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

static void TransformIrCreateStoreObject(AbckitGraph *graph, AbckitString *fieldId, AbckitTypeId valueTypeId,
                                         AbckitInst *newValue, std::vector<AbckitInst *> preInsts)
{
    AbckitInst *inputObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(inputObj, nullptr);

    AbckitInst *store = g_statG->iCreateStoreObject(graph, inputObj, fieldId, newValue, valueTypeId);
    ASSERT_NE(store, nullptr);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);

    g_implG->iInsertBefore(store, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    for (auto &inst : preInsts) {
        g_implG->iInsertBefore(inst, store);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateNewObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateNewObject)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc", "objects", "main");
    EXPECT_TRUE(helpers::Match(output, "A\n"));

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{}, {1}, {}},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {0}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT, {}},
                {5, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {4}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},

            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateNewObject(graph);
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "objects", "main");
    EXPECT_TRUE(helpers::Match(output, "A\nB\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {},
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "objects", "main");
    EXPECT_TRUE(helpers::Match(output, "A\nB\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg0)
{
    InitObjectTest(0, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}}, "A\nC\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg1)
{
    InitObjectTest(1, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0}}, "A\nC 1\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg2)
{
    InitObjectTest(2U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0}}, "A\nC 11\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg3)
{
    InitObjectTest(3U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0}}, "A\nC 111\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg4)
{
    InitObjectTest(4U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0, 0}}, "A\nC 1111\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg5)
{
    InitObjectTest(5U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0, 0, 0}}, "A\nC 11111\n");
}

static void GetLogFunctions(AbckitFile *file, LogFunctions &logFunctions)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "load_object"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    EXPECT_NE(ctxFinder.module, nullptr);

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "ConsoleLogStr"};
    g_implI->moduleEnumerateTopLevelFunctions(ctxFinder.module, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_NE(methodCtxFinder.method, nullptr);
    logFunctions.consoleLogStr = methodCtxFinder.method;

    methodCtxFinder = {nullptr, "ConsoleLogNum"};
    g_implI->moduleEnumerateTopLevelFunctions(ctxFinder.module, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_NE(methodCtxFinder.method, nullptr);
    logFunctions.consoleLogNum = methodCtxFinder.method;

    methodCtxFinder = {nullptr, "ConsoleLogInt"};
    g_implI->moduleEnumerateTopLevelFunctions(ctxFinder.module, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_NE(methodCtxFinder.method, nullptr);
    logFunctions.consoleLogInt = methodCtxFinder.method;
}

static void TransformInst(AbckitFile *file, AbckitGraph *graph, AbckitInst *ObjectInst,
                          const LogFunctions &logFunctions)
{
    auto *strFiled1 = g_implM->createString(file, "load_object.MyClass.name", strlen("load_object.MyClass.name"));
    AbckitInst *newLoadObjectInst1 =
        g_statG->iCreateLoadObject(graph, ObjectInst, strFiled1, AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE);
    ASSERT_NE(newLoadObjectInst1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *strFiled2 = g_implM->createString(file, "load_object.MyClass.id", strlen("load_object.MyClass.id"));
    AbckitInst *newLoadObjectInst2 =
        g_statG->iCreateLoadObject(graph, ObjectInst, strFiled2, AbckitTypeId::ABCKIT_TYPE_ID_I32);
    ASSERT_NE(newLoadObjectInst2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *strFiled3 = g_implM->createString(file, "load_object.MyClass.total", strlen("load_object.MyClass.total"));
    AbckitInst *newLoadObjectInst3 =
        g_statG->iCreateLoadObject(graph, ObjectInst, strFiled3, AbckitTypeId::ABCKIT_TYPE_ID_F64);
    ASSERT_NE(newLoadObjectInst3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_NE(ret, nullptr);

    g_implG->iInsertBefore(newLoadObjectInst1, ret);
    AbckitInst *logStr = g_statG->iCreateCallStatic(graph, logFunctions.consoleLogStr, 1, newLoadObjectInst1);
    ASSERT_NE(logStr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(logStr, ret);

    g_implG->iInsertBefore(newLoadObjectInst2, ret);
    AbckitInst *log32 = g_statG->iCreateCallStatic(graph, logFunctions.consoleLogInt, 1, newLoadObjectInst2);
    ASSERT_NE(log32, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(log32, ret);

    g_implG->iInsertBefore(newLoadObjectInst3, ret);
    AbckitInst *log64 = g_statG->iCreateCallStatic(graph, logFunctions.consoleLogNum, 1, newLoadObjectInst3);
    ASSERT_NE(log64, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(log64, ret);
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateLoadObject)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/load_object.abc", "load_object", "main");
    EXPECT_TRUE(helpers::Match(output, "loadObj logic...\n"));

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{}, {1}, {}},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {0}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {0}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {2}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {0}},
                {5, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {4}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {0}},
                {7, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {6}},
                {8, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},

            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/load_object.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/load_object_modified.abc", "main",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *ObjectInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
            ASSERT_NE(ObjectInst, nullptr);

            LogFunctions logFunctions;
            GetLogFunctions(file, logFunctions);
            TransformInst(file, graph, ObjectInst, logFunctions);
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/load_object_modified.abc",
                                       "load_object", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "loadObj logic...\n"
                               "Alice\n"
                               "66\n"
                               "2025\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, IcreateStoreObjectTest0)
{
    auto input = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object.abc";
    auto output = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object_out.abc";

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{},
         {1},
         {
             {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {4, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
         }},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {1, 2, 0}},
                {5, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT, {4, 3}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        input, output, "store_object",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_I32;
            AbckitInst *newValue = g_implG->gFindOrCreateConstantI32(graph, 4);
            ASSERT_NE(newValue, nullptr);
            auto fieldName = "store_object.MyClass.a";
            auto fieldId = g_implM->createString(file, fieldName, strlen(fieldName));
            ASSERT_NE(fieldId, nullptr);
            TransformIrCreateStoreObject(graph, fieldId, valueTypeId, newValue, {});
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    auto result = helpers::ExecuteStaticAbc(input, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest\n1\ntest class\n"));
    result = helpers::ExecuteStaticAbc(output, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "4\n2\ntest\n1\ntest class\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, IcreateStoreObjectTest1)
{
    auto input = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object.abc";
    auto output = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object_out.abc";

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{},
         {1},
         {
             {0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {5, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
         }},
        {
            {0},
            {2U},
            {
                {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 1, 2}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT, {5, 3}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        input, output, "store_object",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;
            AbckitInst *newValue = g_implG->gFindOrCreateConstantF64(graph, 5.2);
            ASSERT_NE(newValue, nullptr);
            auto fieldName = "store_object.MyClass.b";
            auto fieldId = g_implM->createString(file, fieldName, strlen(fieldName));
            ASSERT_NE(fieldId, nullptr);
            TransformIrCreateStoreObject(graph, fieldId, valueTypeId, newValue, {});
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    auto result = helpers::ExecuteStaticAbc(input, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest\n1\ntest class\n"));
    result = helpers::ExecuteStaticAbc(output, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n5.2\ntest\n1\ntest class\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, IcreateStoreObjectTest2)
{
    auto input = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object.abc";
    auto output = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object_out.abc";

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{},
         {1},
         {
             {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
         }},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {1, 2, 0}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {5, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT, {4, 3}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        input, output, "store_object",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE;
            auto testStr = "test modified";
            AbckitInst *str = g_statG->iCreateLoadString(graph, g_implM->createString(file, testStr, strlen(testStr)));
            ASSERT_NE(str, nullptr);
            auto fieldName = "store_object.MyClass.c";
            auto fieldId = g_implM->createString(file, fieldName, strlen(fieldName));
            ASSERT_NE(fieldId, nullptr);
            TransformIrCreateStoreObject(graph, fieldId, valueTypeId, str, {str});
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    auto result = helpers::ExecuteStaticAbc(input, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest\n1\ntest class\n"));
    result = helpers::ExecuteStaticAbc(output, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest modified\n1\ntest class\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitICreateObjectsStaticTest, IcreateStoreObjectTest3)
{
    auto input = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object.abc";
    auto output = ABCKIT_ABC_DIR "ut/isa/isa_static/objects/store_object_out.abc";

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{},
         {1},
         {
             {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             {5, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
         }},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {1, 2, 0}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {5, 4}},
                {7, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT, {6, 3}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        input, output, "store_object",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto ctor = helpers::FindMethodByName(file, "_ctor_:store_object.TestClass;f64;std.core.String;void;");
            ASSERT_NE(ctor, nullptr);

            AbckitInst *number = g_implG->gFindOrCreateConstantF64(graph, 6.1);
            ASSERT_NE(number, nullptr);

            auto testStr = "test modify";
            AbckitInst *str = g_statG->iCreateLoadString(graph, g_implM->createString(file, testStr, strlen(testStr)));
            ASSERT_NE(number, nullptr);

            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE;
            AbckitInst *newValue = g_statG->iCreateInitObject(graph, ctor, 2, number, str);
            ASSERT_NE(newValue, nullptr);

            auto fieldName = "store_object.MyClass.d";
            auto fieldId = g_implM->createString(file, fieldName, strlen(fieldName));
            ASSERT_NE(fieldId, nullptr);

            TransformIrCreateStoreObject(graph, fieldId, valueTypeId, newValue, {str, newValue});
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    auto result = helpers::ExecuteStaticAbc(input, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest\n1\ntest class\n"));
    result = helpers::ExecuteStaticAbc(output, "store_object", "main");
    EXPECT_TRUE(helpers::Match(result, "0\n2\ntest\n6.1\ntest modify\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
