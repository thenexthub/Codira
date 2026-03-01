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

#include <cassert>
#include <cstring>
#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "metadata_inspect_impl.h"
#include "logger.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/helpers_static.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSInspectApiHelpersTest : public ::testing::Test {};

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_GetStaticOpcode)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/helpers/helpers_static_test.abc", &file);

    helpers::TransformMethod(file, "main", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto instOpcode = GetStaticOpcode(inst->impl);
        ASSERT_EQ(instOpcode, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);

        inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        instOpcode = GetStaticOpcode(inst->impl);
        ASSERT_EQ(instOpcode, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC);

        inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        instOpcode = GetStaticOpcode(inst->impl);
        ASSERT_EQ(instOpcode, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_TypeToTypeId)
{
    auto type = ark::compiler::DataType::Type::REFERENCE;
    auto abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE);

    type = ark::compiler::DataType::Type::BOOL;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_U1);

    type = ark::compiler::DataType::Type::UINT8;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_U8);

    type = ark::compiler::DataType::Type::INT8;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_I8);

    type = ark::compiler::DataType::Type::UINT16;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_U16);

    type = ark::compiler::DataType::Type::INT16;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_I16);

    type = ark::compiler::DataType::Type::UINT32;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_U32);

    type = ark::compiler::DataType::Type::INT32;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_I32);

    type = ark::compiler::DataType::Type::UINT64;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_U64);

    type = ark::compiler::DataType::Type::INT64;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_I64);

    type = ark::compiler::DataType::Type::FLOAT32;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_F32);

    type = ark::compiler::DataType::Type::FLOAT64;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_F64);

    type = ark::compiler::DataType::Type::ANY;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_ANY);

    type = ark::compiler::DataType::Type::VOID;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_VOID);

    type = ark::compiler::DataType::Type::POINTER;
    abckitTypeId = TypeToTypeId(type);
    ASSERT_EQ(abckitTypeId, AbckitTypeId::ABCKIT_TYPE_ID_INVALID);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_TypeIdToType)
{
    AbckitTypeId typeId = AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE;
    auto type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::REFERENCE);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_U1;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::BOOL);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_U8;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::UINT8);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_I8;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::INT8);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_U16;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::UINT16);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_I16;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::INT16);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_U32;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::UINT32);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_I32;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::INT32);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_U64;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::UINT64);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_I64;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::INT64);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_F32;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::FLOAT32);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::FLOAT64);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_ANY;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::ANY);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_VOID;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::VOID);

    typeId = AbckitTypeId::ABCKIT_TYPE_ID_INVALID;
    type = TypeIdToType(typeId);
    ASSERT_EQ(type, ark::compiler::DataType::Type::NO_TYPE);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_CcToCc1)
{
    auto dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ;
    auto arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_EQ);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_NE);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_LT);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_LE);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_GT);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_GE);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_B);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_BE);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_A);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_AE);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_TST_EQ);

    dynamicCc = AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE;
    arkCc = CcToCc(dynamicCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_TST_NE);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_CcToCc2)
{
    auto staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ;
    auto arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_EQ);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_NE);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_LT);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_LE);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_GT);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_GE);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_B);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_BE);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_A);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_AE);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_TST_EQ);

    staticCc = AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE;
    arkCc = CcToCc(staticCc);
    ASSERT_EQ(arkCc, ark::compiler::ConditionCode::CC_TST_NE);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_CcToDynamicCc)
{
    ark::compiler::ConditionCode arkCc = ark::compiler::ConditionCode::CC_EQ;
    auto dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);

    arkCc = ark::compiler::ConditionCode::CC_NE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);

    arkCc = ark::compiler::ConditionCode::CC_LT;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT);

    arkCc = ark::compiler::ConditionCode::CC_LE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE);

    arkCc = ark::compiler::ConditionCode::CC_GT;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT);

    arkCc = ark::compiler::ConditionCode::CC_GE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE);

    arkCc = ark::compiler::ConditionCode::CC_B;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B);

    arkCc = ark::compiler::ConditionCode::CC_BE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE);

    arkCc = ark::compiler::ConditionCode::CC_A;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A);

    arkCc = ark::compiler::ConditionCode::CC_AE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE);

    arkCc = ark::compiler::ConditionCode::CC_TST_EQ;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ);

    arkCc = ark::compiler::ConditionCode::CC_TST_NE;
    dynamicCc = CcToDynamicCc(arkCc);
    ASSERT_EQ(dynamicCc, AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_CcToStaticCc)
{
    ark::compiler::ConditionCode arkCc = ark::compiler::ConditionCode::CC_EQ;
    auto staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);

    arkCc = ark::compiler::ConditionCode::CC_NE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);

    arkCc = ark::compiler::ConditionCode::CC_LT;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT);

    arkCc = ark::compiler::ConditionCode::CC_LE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE);

    arkCc = ark::compiler::ConditionCode::CC_GT;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT);

    arkCc = ark::compiler::ConditionCode::CC_GE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE);

    arkCc = ark::compiler::ConditionCode::CC_B;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B);

    arkCc = ark::compiler::ConditionCode::CC_BE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE);

    arkCc = ark::compiler::ConditionCode::CC_A;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A);

    arkCc = ark::compiler::ConditionCode::CC_AE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE);

    arkCc = ark::compiler::ConditionCode::CC_TST_EQ;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ);

    arkCc = ark::compiler::ConditionCode::CC_TST_NE;
    staticCc = CcToStaticCc(arkCc);
    ASSERT_EQ(staticCc, AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_GetFuncName1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/helpers/helpers_static_test.abc", &file);

    helpers::ModuleByNameContext mdFinder = {nullptr, "helpers_static_test"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "Person"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    helpers::MethodByNameContext methodFinder = {nullptr, "showName"};
    g_implI->classEnumerateMethods(klass, &methodFinder, helpers::MethodByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(methodFinder.method, nullptr);
    auto *method = methodFinder.method;

    auto funcName = GetFuncName(method);
    ASSERT_EQ(funcName, "helpers_static_test.Person.showName");
    g_impl->closeFile(file);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_GetFuncName2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/helpers/helpers_dynamic_test.abc", &file);

    helpers::ModuleByNameContext mdFinder = {nullptr, "helpers_dynamic_test"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "DyPerson"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    helpers::MethodByNameContext methodFinder = {nullptr, "getName"};
    g_implI->classEnumerateMethods(klass, &methodFinder, helpers::MethodByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(methodFinder.method, nullptr);
    auto *method = methodFinder.method;

    auto funcName = GetFuncName(method);
    ASSERT_EQ(funcName, "helpers_dynamic_test.#~@0>#getName");
    g_impl->closeFile(file);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_GetBinaryImmOperationSize)
{
    ark::compiler::Opcode opcode = ark::compiler::Opcode::AddI;
    auto size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::SubI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::MulI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::DivI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::ModI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::ShlI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::ShrI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::AShrI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_8);

    opcode = ark::compiler::Opcode::AndI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_32);

    opcode = ark::compiler::Opcode::OrI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_32);

    opcode = ark::compiler::Opcode::XorI;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_32);

    opcode = ark::compiler::Opcode::LoadString;
    size = GetBinaryImmOperationSize(opcode);
    ASSERT_EQ(size, AbckitBitImmSize::BITSIZE_0);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiHelpersTest, Helpers_ArkPandasmTypeToAbckitTypeId)
{
    auto type = ark::pandasm::Type("u8", 0);
    auto typeId = ArkPandasmTypeToAbckitTypeId(type);
    ASSERT_EQ(typeId, AbckitTypeId::ABCKIT_TYPE_ID_U8);

    type = ark::pandasm::Type("u16", 0);
    typeId = ArkPandasmTypeToAbckitTypeId(type);
    ASSERT_EQ(typeId, AbckitTypeId::ABCKIT_TYPE_ID_U16);

    type = ark::pandasm::Type("u32", 0);
    typeId = ArkPandasmTypeToAbckitTypeId(type);
    ASSERT_EQ(typeId, AbckitTypeId::ABCKIT_TYPE_ID_U32);

    type = ark::pandasm::Type("u64", 0);
    typeId = ArkPandasmTypeToAbckitTypeId(type);
    ASSERT_EQ(typeId, AbckitTypeId::ABCKIT_TYPE_ID_U64);
}
}  // namespace libabckit::test
