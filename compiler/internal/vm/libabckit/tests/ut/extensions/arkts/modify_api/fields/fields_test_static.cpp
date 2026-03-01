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
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/cpp/abckit_cpp.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/cpp/headers/core/field.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"

#include "libabckit/src/ir_impl.h"
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiFieldsStaticTests : public ::testing::Test {};

static std::string TEST_ABC_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/fields/fields_static.abc";
static std::string MODIFY_TEST_ABC_PATH =
    ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/fields/fields_static_modify.abc";

static AbckitCoreModuleField *GetAbckitCoreModuleField(AbckitFile *file, std::string &moduleName,
                                                       std::string &moduleFieldName)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::ModuleFieldByNameContext mfFinder = {nullptr, moduleFieldName.c_str()};
    g_implI->moduleEnumerateFields(module, &mfFinder, helpers::ModuleFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    return mfFinder.cmf;
}

static AbckitCoreNamespaceField *GetAbckitCoreNamespaceField(AbckitFile *file, std::string &moduleName,
                                                             std::string &nsName, std::string &nsFieldName)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::NamespaceByNameContext nsFinder = {nullptr, nsName.c_str()};
    g_implI->moduleEnumerateNamespaces(module, &nsFinder, helpers::NamespaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(nsFinder.n != nullptr);
    auto ns = nsFinder.n;

    helpers::NamespaceFieldByNameContext nfFinder = {nullptr, nsFieldName.c_str()};
    g_implI->namespaceEnumerateFields(ns, &nfFinder, helpers::NamespaceFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    return nfFinder.cnf;
}

static AbckitCoreClassField *GetAbckitCoreClassField(AbckitFile *file, std::string &moduleName, std::string &className,
                                                     std::string &classFieldName)
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
    auto ck = classFinder.klass;

    helpers::ClassFieldByNameContext ccfFinder = {nullptr, classFieldName.c_str()};
    g_implI->classEnumerateFields(ck, &ccfFinder, helpers::ClassFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    return ccfFinder.ccf;
}

static AbckitCoreEnumField *GetAbckitCoreEnumField(AbckitFile *file, std::string &moduleName, std::string &enumName,
                                                   std::string &enumFieldName)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::EnumByNameContext ceFinder = {nullptr, enumName.c_str()};
    g_implI->moduleEnumerateEnums(module, &ceFinder, helpers::EnumByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(ceFinder.enm != nullptr);
    auto ce = ceFinder.enm;

    helpers::EnumFieldByNameContext cefFinder = {nullptr, enumFieldName.c_str()};
    g_implI->enumEnumerateFields(ce, &cefFinder, helpers::EnumFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    return cefFinder.cef;
}

static AbckitCoreInterfaceField *GetAbckitCoreInterfaceField(AbckitFile *file, std::string &moduleName,
                                                             std::string &interfaceName,
                                                             std::string &interfaceFieldName)
{
    helpers::ModuleByNameContext mdlFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(mdlFinder.module != nullptr);
    auto *module = mdlFinder.module;

    helpers::InterfaceByNameContext ciFinder = {nullptr, interfaceName.c_str()};
    g_implI->moduleEnumerateInterfaces(module, &ciFinder, helpers::InterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(ciFinder.iface != nullptr);
    auto ci = ciFinder.iface;

    helpers::InterfaceFieldByNameContext cifFinder = {nullptr, interfaceFieldName.c_str()};
    g_implI->interfaceEnumerateFields(ci, &cifFinder, helpers::InterfaceFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    return cifFinder.cif;
}

static void TransformMethodModuleFieldSetType(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    std::string moduleName = "fields_static";
    std::string moduleFieldName = "moduleField3";
    auto cmf = GetAbckitCoreModuleField(file, moduleName, moduleFieldName);
    ASSERT_NE(cmf, nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->moduleFieldGetType(cmf)), AbckitTypeId::ABCKIT_TYPE_ID_F64);

    AbckitType *type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    auto arkModuleField = g_implArkI->coreModuleFieldToArktsModuleField(cmf);
    auto coreModuleField = g_implArkI->arktsModuleFieldToCoreModuleField(arkModuleField);
    ASSERT_NE(coreModuleField, nullptr);
    bool ret = g_implArkM->moduleFieldSetType(arkModuleField, type);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->moduleFieldGetType(cmf)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);

    AbckitInst *callOp = nullptr;
    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });
    for (auto *bb : bbs) {
        auto *curInst = g_implG->bbGetFirstInst(bb);
        while (curInst != nullptr) {
            callOp = curInst;
            if (g_statG->iGetOpcode(curInst) == ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID) {
                break;
            }
            curInst = g_implG->iGetNext(curInst);
            g_implG->iRemove(callOp);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        }
    }

    ASSERT_NE(callOp, nullptr);
    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "printLogStr"};
    g_implI->moduleEnumerateTopLevelFunctions(g_implI->moduleFieldGetModule(cmf), &ctxMethodFinder,
                                              helpers::MethodByNameFinder);
    EXPECT_NE(ctxMethodFinder.method, nullptr);

    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);
    AbckitInst *log = g_statG->iCreateCallStatic(graph, ctxMethodFinder.method, 1, loadStr);
    ASSERT_NE(log, nullptr);
    g_implG->iInsertBefore(log, callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(loadStr, log);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformMethodModuleFieldSetTypeModifyCctor(AbckitFile *file, AbckitCoreFunction *method,
                                                         AbckitGraph *graph)
{
    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });
    for (auto *bb : bbs) {
        auto *curInst = g_implG->bbGetFirstInst(bb);
        while (curInst != nullptr) {
            if (g_statG->iGetOpcode(curInst) == ABCKIT_ISA_API_STATIC_OPCODE_STORESTATIC) {
                callOp = curInst;
            }
            curInst = g_implG->iGetNext(curInst);
        }
    }

    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

/**
 * @tc.name: ModuleFieldSetType
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ModuleFieldSetType, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ModuleFieldSetType)
{
    auto result = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "moduleFieldSetType");
    EXPECT_TRUE(helpers::Match(result, "5.5\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string moduleFieldName = "moduleField3";
    auto cmf = GetAbckitCoreModuleField(file, moduleName, moduleFieldName);
    ASSERT_NE(cmf, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetType(cmf), nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->moduleFieldGetType(cmf)), AbckitTypeId::ABCKIT_TYPE_ID_F64);

    helpers::MethodByNameContext ctxMethodFinder1 = {nullptr, "_cctor_"};
    g_implI->moduleEnumerateTopLevelFunctions(g_implI->moduleFieldGetModule(cmf), &ctxMethodFinder1,
                                              helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder1.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder1.method, TransformMethodModuleFieldSetTypeModifyCctor);

    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "moduleFieldSetType"};
    g_implI->moduleEnumerateTopLevelFunctions(g_implI->moduleFieldGetModule(cmf), &ctxMethodFinder,
                                              helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder.method, TransformMethodModuleFieldSetType);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    std::string moduleName2 = "fields_static";
    std::string moduleFieldName2 = "moduleField3";
    auto cmfModify = GetAbckitCoreModuleField(file, moduleName2, moduleFieldName2);
    ASSERT_NE(cmfModify, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetType(cmfModify), nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->moduleFieldGetType(cmfModify)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    g_impl->closeFile(file);

    result = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "moduleFieldSetType");
    EXPECT_TRUE(helpers::Match(result, "hello\n"));
}

static void TransformMethodModuleFieldSetValue(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    std::string moduleName = "fields_static";
    std::string moduleFieldName = "moduleField1";
    auto cmf = GetAbckitCoreModuleField(file, moduleName, moduleFieldName);
    ASSERT_NE(cmf, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetValue(cmf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->moduleFieldGetValue(cmf))),
                 "ModuleFieldOne");

    std::string moduleFieldValue = "hello";
    AbckitValue *value = g_implM->createValueString(file, moduleFieldValue.c_str(), moduleFieldValue.length());
    auto arkModuleField = g_implArkI->coreModuleFieldToArktsModuleField(cmf);
    bool ret = g_implArkM->moduleFieldSetValue(arkModuleField, value);
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->moduleFieldGetValue(cmf))), "hello");

    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
    ASSERT_NE(callOp, nullptr);
    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);
    helpers::ReplaceInst(callOp, loadStr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

/**
 * @tc.name: ModuleFieldSetValue
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ModuleFieldSetValue, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ModuleFieldSetValue)
{
    auto result = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "moduleFieldSetValue");
    EXPECT_TRUE(helpers::Match(result, "ModuleFieldOne\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string moduleFieldName = "moduleField1";
    auto cmf = GetAbckitCoreModuleField(file, moduleName, moduleFieldName);
    ASSERT_NE(cmf, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetValue(cmf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->moduleFieldGetValue(cmf))),
                 "ModuleFieldOne");

    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "moduleFieldSetValue"};
    g_implI->moduleEnumerateTopLevelFunctions(g_implI->moduleFieldGetModule(cmf), &ctxMethodFinder,
                                              helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder.method, TransformMethodModuleFieldSetValue);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    std::string moduleName2 = "fields_static";
    std::string moduleFieldName2 = "moduleField1";
    auto cmfModify = GetAbckitCoreModuleField(file, moduleName2, moduleFieldName2);
    ASSERT_NE(cmfModify, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetValue(cmfModify), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->moduleFieldGetValue(cmfModify))),
                 "hello");
    g_impl->closeFile(file);

    result = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "moduleFieldSetValue");
    EXPECT_TRUE(helpers::Match(result, "hello\n"));
}

static void TransformMethodClassFieldSetType(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    std::string moduleName = "fields_static";
    std::string className("C3");
    std::string paramName("c3F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetType(ccf), nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->classFieldGetType(ccf)), AbckitTypeId::ABCKIT_TYPE_ID_F64);

    AbckitType *type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    bool ret = g_implArkM->classFieldSetType(g_implArkI->coreClassFieldToArktsClassField(ccf), type);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->classFieldGetType(ccf)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);

    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_NE(originalRet, nullptr);
    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);

    std::string fieldFullName = moduleName + "." + className + "." + paramName;
    AbckitString *newKeyString = g_implM->createString(file, fieldFullName.c_str(), fieldFullName.size());
    callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);

    auto *newStOwn = g_statG->iCreateStoreObject(graph, callOp, newKeyString, loadStr, ABCKIT_TYPE_ID_REFERENCE);
    g_implG->iInsertBefore(newStOwn, originalRet);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(loadStr, newStOwn);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

/**
 * @tc.name: ClassFieldSetType
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ClassFieldSetType, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ClassFieldSetType)
{
    auto result = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "classSetType");
    EXPECT_TRUE(helpers::Match(result, "fields_static\\.C3 \\{c3F1: 999\\}\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string className("C3");
    std::string paramName("c3F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetType(ccf), nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->classFieldGetType(ccf)), AbckitTypeId::ABCKIT_TYPE_ID_F64);

    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "_ctor_"};
    g_implI->classEnumerateMethods(g_implI->classFieldGetClass(ccf), &ctxMethodFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder.method, TransformMethodClassFieldSetType);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    auto ccfModify = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccfModify, nullptr);
    ASSERT_NE(g_implI->classFieldGetType(ccfModify), nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->classFieldGetType(ccfModify)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    g_impl->closeFile(file);

    result = helpers::ExecuteStaticAbc(outputPath, "fields_static", "classSetType");
    EXPECT_TRUE(helpers::Match(result, "fields_static\\.C3 \\{c3F1: hello\\}\n"));
}

static void TransformMethodClassFieldSetValue(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    std::string moduleName = "fields_static";
    std::string className("C2");
    std::string paramName("c2F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetValue(ccf), nullptr);

    std::string fieldValue = "hello";
    AbckitValue *value = g_implM->createValueString(file, fieldValue.c_str(), fieldValue.length());
    bool ret = g_implArkM->classFieldSetValue(g_implArkI->coreClassFieldToArktsClassField(ccf), value);
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->classFieldGetValue(ccf))), "hello");

    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
    ASSERT_NE(callOp, nullptr);
    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);
    helpers::ReplaceInst(callOp, loadStr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

/**
 * @tc.name: ClassFieldSetValue
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ClassFieldSetValue, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ClassFieldSetValue)
{
    auto result = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "classSetValueString");
    EXPECT_TRUE(helpers::Match(result, "Anno1\n"));
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string className("C2");
    std::string paramName("c2F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetValue(ccf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->classFieldGetValue(ccf))), "Anno1");

    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "_ctor_"};
    g_implI->classEnumerateMethods(g_implI->classFieldGetClass(ccf), &ctxMethodFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder.method, TransformMethodClassFieldSetValue);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    auto ccfModify = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccfModify, nullptr);
    ASSERT_NE(g_implI->classFieldGetValue(ccfModify), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->valueGetString(g_implI->classFieldGetValue(ccfModify))),
                 "hello");
    g_impl->closeFile(file);

    result = helpers::ExecuteStaticAbc(outputPath, "fields_static", "classSetValueString");
    EXPECT_TRUE(helpers::Match(result, "hello\n"));
}

static void TransformMethodClassFieldCtorModify(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_NE(originalRet, nullptr);
    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);

    std::string moduleName = "fields_static";
    std::string className("C5");
    std::string paramName("%%property-i2F1");
    std::string fieldFullName = moduleName + "." + className + "." + paramName;
    AbckitString *newKeyString = g_implM->createString(file, fieldFullName.c_str(), fieldFullName.size());
    callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);

    auto *newStOwn = g_statG->iCreateStoreObject(graph, callOp, newKeyString, loadStr, ABCKIT_TYPE_ID_REFERENCE);
    g_implG->iInsertBefore(newStOwn, originalRet);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(loadStr, newStOwn);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformMethodClassFieldGetModify(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT);
    ASSERT_NE(callOp, nullptr);

    auto *param = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
    ASSERT_NE(param, nullptr);

    std::string moduleName = "fields_static";
    std::string className("C5");
    std::string paramName("%%property-i2F1");
    std::string fieldFullName = moduleName + "." + className + "." + paramName;
    AbckitString *newKeyString = g_implM->createString(file, fieldFullName.c_str(), fieldFullName.size());
    auto *newLdOwn = g_statG->iCreateLoadObject(graph, param, newKeyString, ABCKIT_TYPE_ID_REFERENCE);
    helpers::ReplaceInst(callOp, newLdOwn);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(originalRet, nullptr);

    auto *newReturnInst = g_statG->iCreateReturn(graph, newLdOwn);
    ASSERT_NE(newReturnInst, nullptr);
    helpers::ReplaceInst(originalRet, newReturnInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformMethodClassFieldSetModify(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *param1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
    ASSERT_NE(param1, nullptr);

    auto *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT);
    ASSERT_NE(callOp, nullptr);
    g_implG->iRemove(callOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    ASSERT_NE(originalRet, nullptr);
    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);

    std::string moduleName = "fields_static";
    std::string className("C5");
    std::string paramName("%%property-i2F1");
    std::string fieldFullName = moduleName + "." + className + "." + paramName;
    AbckitString *newKeyString = g_implM->createString(file, fieldFullName.c_str(), fieldFullName.size());

    auto *newStOwn = g_statG->iCreateStoreObject(graph, param1, newKeyString, loadStr, ABCKIT_TYPE_ID_REFERENCE);
    g_implG->iInsertBefore(newStOwn, originalRet);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(loadStr, newStOwn);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void InterfaceFieldSetTypeForClassModify(AbckitFile *file, AbckitCoreClass *kclass)
{
    helpers::MethodByNameContext ctxMethodFinder = {nullptr, "_ctor_"};
    g_implI->classEnumerateMethods(kclass, &ctxMethodFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxMethodFinder.method, nullptr);
    helpers::TransformMethod(ctxMethodFinder.method, TransformMethodClassFieldCtorModify);

    helpers::MethodByNameContext getMethodFinder = {nullptr, "%%get-i2F1"};
    g_implI->classEnumerateMethods(kclass, &getMethodFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(getMethodFinder.method, nullptr);
    helpers::TransformMethod(getMethodFinder.method, TransformMethodClassFieldGetModify);

    helpers::MethodByNameContext setMethodFinder = {nullptr, "%%set-i2F1"};
    g_implI->classEnumerateMethods(kclass, &setMethodFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(setMethodFinder.method, nullptr);
    helpers::TransformMethod(setMethodFinder.method, TransformMethodClassFieldSetModify);
}

/**
 * @tc.name: InterfaceFieldSetType
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::InterfaceFieldSetType, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, InterfaceFieldSetType)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "interfaceFieldSetType");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C5 \\{i2F1: 3\\}\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string interfaceNames("I2");
    std::string paramNames("%%property-i2F1");
    auto cif = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cif, nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->interfaceFieldGetType(cif)), AbckitTypeId::ABCKIT_TYPE_ID_F64);

    std::string className("C5");  // implement class
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramNames);
    ASSERT_NE(ccf, nullptr);

    InterfaceFieldSetTypeForClassModify(file, g_implI->classFieldGetClass(ccf));

    AbckitType *type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    bool ret = g_implArkM->interfaceFieldSetType(g_implArkI->coreInterfaceFieldToArktsInterfaceField(cif), type);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->interfaceFieldGetType(cif)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    auto cifModify = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cifModify, nullptr);
    ASSERT_EQ(g_implI->typeGetTypeId(g_implI->interfaceFieldGetType(cifModify)), AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "interfaceFieldSetType");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C5 \\{i2F1: hello\\}\n"));
}

/**
 * @tc.name: ModuleFieldSetName
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ModuleFieldSetName, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ModuleFieldSetName)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "moduleFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "ModuleFieldOne\n"));
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string moduleFieldName = "moduleField1";
    auto cmf = GetAbckitCoreModuleField(file, moduleName, moduleFieldName);
    ASSERT_NE(cmf, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetName(cmf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->moduleFieldGetName(cmf)), "moduleField1");

    std::string fieldNewNames("newModuleField");
    auto arkModuleField = g_implArkI->coreModuleFieldToArktsModuleField(cmf);
    bool ret = g_implArkM->moduleFieldSetName(arkModuleField, fieldNewNames.c_str());
    ASSERT_EQ(ret, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->moduleFieldGetName(cmf)), fieldNewNames.c_str());

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    ASSERT_EQ(GetAbckitCoreModuleField(file, moduleName, moduleFieldName), nullptr);

    auto cmfModify = GetAbckitCoreModuleField(file, moduleName, fieldNewNames);
    ASSERT_NE(cmfModify, nullptr);
    ASSERT_NE(g_implI->moduleFieldGetName(cmfModify), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->moduleFieldGetName(cmfModify)), "newModuleField");
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(outputPath, "fields_static", "moduleFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "ModuleFieldOne\n"));
}

/**
 * @tc.name: InterfaceFieldSetName
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::InterfaceFieldSetName, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, InterfaceFieldSetName)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "interfaceFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C4 \\{i1F1: ABC\\}\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    std::string moduleName = "fields_static";
    std::string interfaceNames("I1");
    std::string paramNames("%%property-i1F1");
    auto cif = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cif, nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->interfaceFieldGetName(cif)), paramNames.c_str());

    std::string newParamNames("%%property-i1NewField");
    std::string cropNewParamNames("i1NewField");
    auto arkInterfaceField = g_implArkI->coreInterfaceFieldToArktsInterfaceField(cif);
    bool ret = g_implArkM->interfaceFieldSetName(arkInterfaceField, cropNewParamNames.c_str());
    ASSERT_EQ(ret, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->interfaceFieldGetName(cif)), newParamNames.c_str());

    std::string implementClassName("C4");
    auto ccf = GetAbckitCoreClassField(file, moduleName, implementClassName, paramNames);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetName(ccf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->classFieldGetName(ccf)), paramNames.c_str());

    auto arkClassField = g_implArkI->coreClassFieldToArktsClassField(ccf);
    bool ret2 = g_implArkM->classFieldSetName(arkClassField, newParamNames.c_str());
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->classFieldGetName(ccf)), newParamNames.c_str());

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto cifModify = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, newParamNames);
    ASSERT_NE(cifModify, nullptr);
    ASSERT_NE(cifModify->name, nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->interfaceFieldGetName(cifModify)), newParamNames.c_str());

    auto cifModifyFindOldName = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_EQ(cifModifyFindOldName, nullptr);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "interfaceFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C4 \\{i1NewField: ABC\\}\n"));
}

/**
 * @tc.name: EnumFieldSetName
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::EnumFieldSetName, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, EnumFieldSetName)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "enumFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "AAAA\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string enumNames("E1");
    std::string paramNames("ONE");
    auto cef = GetAbckitCoreEnumField(file, moduleName, enumNames, paramNames);
    ASSERT_NE(cef, nullptr);

    std::string newParamNames("THREE");
    auto arkEnumField = g_implArkI->coreEnumFieldToArktsEnumField(cef);
    bool ret = g_implArkM->enumFieldSetName(arkEnumField, newParamNames.c_str());
    ASSERT_EQ(ret, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->enumFieldGetName(cef)), newParamNames.c_str());

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto cefModify = GetAbckitCoreEnumField(file, moduleName, enumNames, newParamNames);
    ASSERT_NE(cefModify, nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->enumFieldGetName(cefModify)), "THREE");

    auto cefModifyFindOldName = GetAbckitCoreEnumField(file, moduleName, enumNames, paramNames);
    ASSERT_EQ(cefModifyFindOldName, nullptr);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "enumFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "AAAA\n"));
}

/**
 * @tc.name: ClassFieldSetName
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ClassFieldSetName, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, ClassFieldSetName)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "classFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C1 \\{c1F1: Anno1, c1F2: 999\\}\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string className("C1");
    std::string paramName("c1F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, className, paramName);
    ASSERT_NE(ccf, nullptr);
    ASSERT_NE(g_implI->classFieldGetName(ccf), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->classFieldGetName(ccf)), "c1F1");

    std::string newParamName("newParam");
    auto arkClassField = g_implArkI->coreClassFieldToArktsClassField(ccf);
    bool ret = g_implArkM->classFieldSetName(arkClassField, newParamName.c_str());
    ASSERT_EQ(ret, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->classFieldGetName(ccf)), newParamName.c_str());

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);

    ASSERT_EQ(GetAbckitCoreClassField(file, moduleName, className, paramName), nullptr);

    auto ccfModify = GetAbckitCoreClassField(file, moduleName, className, newParamName);
    ASSERT_NE(ccfModify, nullptr);
    ASSERT_NE(g_implI->classFieldGetName(ccfModify), nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->classFieldGetName(ccfModify)), "newParam");
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "classFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "fields_static\\.C1 \\{newParam: Anno1, c1F2: 999\\}\n"));
}

/**
 * @tc.name: NamespaceFieldSetName
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::NamespaceFieldSetName, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiFieldsStaticTests, NamespaceFieldSetName)
{
    auto output = helpers::ExecuteStaticAbc(TEST_ABC_PATH, "fields_static", "namespaceFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "NamespaceFieldOne\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "fields_static";
    std::string nsName("NS1");
    std::string nsFieldName("nS1F1");
    auto coreNsField = GetAbckitCoreNamespaceField(file, moduleName, nsName, nsFieldName);
    ASSERT_NE(coreNsField, nullptr);

    std::string newNsFieldName("newField");
    auto arkNsField = g_implArkI->coreNamespaceFieldToArktsNamespaceField(coreNsField);
    bool ret = g_implArkM->namespaceFieldSetName(arkNsField, newNsFieldName.c_str());
    ASSERT_EQ(ret, true);
    ASSERT_EQ(helpers::AbckitStringToString(g_implI->namespaceFieldGetName(coreNsField)), newNsFieldName.c_str());

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto nsFieldModify = GetAbckitCoreNamespaceField(file, moduleName, nsName, newNsFieldName);
    ASSERT_NE(nsFieldModify, nullptr);
    ASSERT_STREQ(g_implI->abckitStringToString(g_implI->namespaceFieldGetName(nsFieldModify)), newNsFieldName.c_str());

    auto nsFieldModifyFindOldName = GetAbckitCoreNamespaceField(file, moduleName, nsName, nsFieldName);
    ASSERT_EQ(nsFieldModifyFindOldName, nullptr);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(MODIFY_TEST_ABC_PATH, "fields_static", "namespaceFieldSetName");
    EXPECT_TRUE(helpers::Match(output, "NamespaceFieldOne\n"));
}

}  // namespace libabckit::test