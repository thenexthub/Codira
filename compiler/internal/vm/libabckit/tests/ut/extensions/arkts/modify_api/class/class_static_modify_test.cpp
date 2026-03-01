/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiClassTests : public ::testing::Test {};

void SuperClassTransformCtorIr(AbckitFile *file)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "super_class"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::MethodByNameContext funcFinder = {nullptr, "_ctor_"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &funcFinder, helpers::MethodByNameFinder);

    classCtxFinder = {nullptr, "SuClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::MethodByNameContext targetFuncFinder = {nullptr, "_ctor_"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &targetFuncFinder, helpers::MethodByNameFinder);

    AbckitGraph *graph = g_implI->createGraphFromFunction(funcFinder.method);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    auto refObj = g_implG->iGetInput(call, 0);
    ASSERT_NE(refObj, nullptr);
    auto newInst = g_statG->iCreateCallStatic(graph, targetFuncFinder.method, 1, refObj);
    ASSERT_NE(newInst, nullptr);
    g_implG->iInsertBefore(newInst, call);
    g_implG->iRemove(call);

    g_implM->functionSetGraph(funcFinder.method, graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->destroyGraph(graph);
}

void SuperClassTransformMainIr(AbckitFile *file)
{
    auto transformCb = [](AbckitFile *file, AbckitCoreFunction *func, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "super_class"};
        g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

        helpers::ClassByNameContext classCtxFinder = {nullptr, "SuClass"};
        g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

        helpers::MethodByNameContext targetFuncFinder = {nullptr, "suHandle"};
        g_implI->classEnumerateMethods(classCtxFinder.klass, &targetFuncFinder, helpers::MethodByNameFinder);

        auto call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
        ASSERT_NE(call, nullptr);
        auto objRef = g_implG->iGetInput(call, 0);
        ASSERT_NE(objRef, nullptr);
        auto newInst = g_statG->iCreateCallVirtual(graph, objRef, targetFuncFinder.method, 0);
        ASSERT_NE(newInst, nullptr);
        g_implG->iInsertBefore(newInst, call);
        g_implG->iRemove(call);
    };

    helpers::TransformMethod(file, "main", transformCb);
}

static AbckitCoreClass *GetAbckitCoreClass(AbckitFile *file, std::string moduleName, std::string className)
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

// Test: test-kind=api, api=ArktsModifyApiImpl::createClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, CreateClassTest0)
{
    auto inputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify.abc";
    auto outputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/create_class_out.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(inputPath, &file);

    helpers::ModuleByNameContext mdlFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder.module, nullptr);
    auto *module = mdlFinder.module;

    const char *className = "NewClass";
    AbckitArktsClass *arktsClass = g_implArkM->createClass(module->GetArkTSImpl(), className);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arktsClass, nullptr);
    ASSERT_NE(arktsClass->core, nullptr);
    ASSERT_NE(arktsClass->core->owningModule, nullptr);
    ASSERT_EQ(arktsClass->core->owningModule->target, ABCKIT_TARGET_ARK_TS_V2);
    ASSERT_NE(module->ct.find(className), module->ct.end());
    ASSERT_NE(module->ct.find(className)->second, nullptr);

    g_impl->writeAbc(file, outputPath, strlen(outputPath));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(outputPath, &file);
    mdlFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classFinder = {nullptr, className};
    g_implI->moduleEnumerateClasses(mdlFinder.module, &classFinder, helpers::ClassByNameFinder);
    EXPECT_NE(classFinder.klass, nullptr);

    g_impl->closeFile(file);
}

void ClassRemoveMethod(AbckitCoreFunction *method)
{
    auto klass = g_implI->functionGetParentClass(method);
    ASSERT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    ASSERT_TRUE(g_implArkM->classRemoveMethod(g_implArkI->coreClassToArktsClass(klass),
                                              g_implArkI->coreFunctionToArktsFunction(method)));
    ASSERT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveMethod, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassRemoveMethodHandleNoUseRm)
{
    auto inputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify.abc";
    auto outputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_handleNullptr.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(inputPath, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "Test"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "handleNoUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_NE(methodCtxFinder.method, nullptr);

    ClassRemoveMethod(methodCtxFinder.method);

    methodCtxFinder = {nullptr, "handleNoUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(methodCtxFinder.method, nullptr);

    g_impl->writeAbc(file, outputPath, strlen(outputPath));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(outputPath, &file);

    ctxFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    classCtxFinder = {nullptr, "Test"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    methodCtxFinder = {nullptr, "handleNoUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(methodCtxFinder.method, nullptr);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveMethod, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassRemoveMethodHandleUseRm)
{
    auto inputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify.abc";
    auto outputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_method_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(inputPath, &file);
    helpers::TransformMethod(file, "main",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "Test"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "handleUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_NE(methodCtxFinder.method, nullptr);

    ClassRemoveMethod(methodCtxFinder.method);

    methodCtxFinder = {nullptr, "handleUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(methodCtxFinder.method, nullptr);

    g_impl->writeAbc(file, outputPath, strlen(outputPath));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(outputPath, &file);

    ctxFinder = {nullptr, "class_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    classCtxFinder = {nullptr, "Test"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    methodCtxFinder = {nullptr, "handleUseRm"};
    g_implI->classEnumerateMethods(classCtxFinder.klass, &methodCtxFinder, helpers::MethodByNameFinder);
    EXPECT_EQ(methodCtxFinder.method, nullptr);

    g_impl->closeFile(file);

    auto output = helpers::ExecuteStaticAbc(inputPath, "class_static_modify", "main");
    EXPECT_TRUE(helpers::Match(output, "handleUseRm\ntest\n"));

    output = helpers::ExecuteStaticAbc(outputPath, "class_static_modify", "main");
    EXPECT_TRUE(helpers::Match(output, "test\n"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classAddInterface, abc-kind=ArkTS2, category=negative, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassAddInterfaceTest0)
{
    std::string input0 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/myclass.abc";
    std::string input1 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/interface.abc";

    AbckitFile *file0 = nullptr;
    AbckitFile *file1 = nullptr;
    helpers::AssertOpenAbc(input0.c_str(), &file0);
    helpers::AssertOpenAbc(input1.c_str(), &file1);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "myclass"};
    g_implI->fileEnumerateModules(file0, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinderTs = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinderTs, helpers::ClassByNameFinder);

    ctxFinder = {nullptr, "interface"};
    g_implI->fileEnumerateModules(file1, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinderEts = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinderEts, helpers::InterfaceByNameFinder);

    g_implArkM->classAddInterface(classCtxFinderTs.klass->GetArkTSImpl(), ifaceCtxFinderEts.iface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ifaceCtxFinderEts.iface->owningModule->target = ABCKIT_TARGET_ARK_TS_V1;
    g_implArkM->classAddInterface(classCtxFinderTs.klass->GetArkTSImpl(), ifaceCtxFinderEts.iface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);

    g_impl->closeFile(file0);
    g_impl->closeFile(file1);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classAddInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassAddInterfaceTest1)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_interface.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_add_interface_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "TestInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    g_implArkM->classAddInterface(classCtxFinder.klass->GetArkTSImpl(), ifaceCtxFinder.iface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ifaceCtxFinder = {nullptr, "TestInterface"};
    g_implI->classEnumerateInterfaces(classCtxFinder.klass, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_NE(ifaceCtxFinder.iface, nullptr);

    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(output.c_str(), &file);
    ctxFinder = {nullptr, "class_interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    ifaceCtxFinder = {nullptr, "TestInterface"};
    g_implI->classEnumerateInterfaces(classCtxFinder.klass, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_NE(ifaceCtxFinder.iface, nullptr);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveInterface, abc-kind=ArkTS2, category=negative, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassRemoveInterfaceTest0)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_interface.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "TestInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    g_implArkM->classRemoveInterface(classCtxFinder.klass->GetArkTSImpl(), ifaceCtxFinder.iface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassRemoveInterfaceTest1)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_interface.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_remove_interface_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "class_interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->classEnumerateInterfaces(classCtxFinder.klass, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_NE(ifaceCtxFinder.iface, nullptr);

    g_implArkM->classRemoveInterface(classCtxFinder.klass->GetArkTSImpl(), ifaceCtxFinder.iface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->classEnumerateInterfaces(classCtxFinder.klass, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_EQ(ifaceCtxFinder.iface, nullptr);

    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(output.c_str(), &file);

    ctxFinder = {nullptr, "class_interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    classCtxFinder = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder, helpers::ClassByNameFinder);

    ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->classEnumerateInterfaces(classCtxFinder.klass, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_EQ(ifaceCtxFinder.iface, nullptr);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classSetSuperClass, abc-kind=ArkTS2, category=negative, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetSuperClassTest0)
{
    std::string input0 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/super_class.abc";
    std::string input1 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/myclass.abc";
    std::string input2 = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/suclass.abc";

    AbckitFile *file0 = nullptr;
    AbckitFile *file1 = nullptr;
    AbckitFile *file2 = nullptr;
    helpers::AssertOpenAbc(input0.c_str(), &file0);
    helpers::AssertOpenAbc(input1.c_str(), &file1);
    helpers::AssertOpenAbc(input2.c_str(), &file2);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "super_class"};
    g_implI->fileEnumerateModules(file0, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder0 = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder0, helpers::ClassByNameFinder);

    helpers::ClassByNameContext classCtxFinder1 = {nullptr, "SuClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder1, helpers::ClassByNameFinder);

    ctxFinder = {nullptr, "myclass"};
    g_implI->fileEnumerateModules(file1, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinderTs = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinderTs, helpers::ClassByNameFinder);

    ctxFinder = {nullptr, "suclass"};
    g_implI->fileEnumerateModules(file2, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinderEts = {nullptr, "SuClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinderEts, helpers::ClassByNameFinder);

    g_implArkM->classSetSuperClass(classCtxFinderTs.klass->GetArkTSImpl(), classCtxFinderEts.klass->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    classCtxFinder0.klass->owningModule->target = ABCKIT_TARGET_ARK_TS_V1;
    g_implArkM->classSetSuperClass(classCtxFinder0.klass->GetArkTSImpl(), classCtxFinder1.klass->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);

    g_impl->closeFile(file0);
    g_impl->closeFile(file1);
    g_impl->closeFile(file2);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classSetSuperClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetSuperClassTest1)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/super_class.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/super_class_out.abc";
    std::string transform = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/super_class_trans.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "super_class"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::ClassByNameContext classCtxFinder0 = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder0, helpers::ClassByNameFinder);

    helpers::ClassByNameContext classCtxFinder1 = {nullptr, "SuClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder1, helpers::ClassByNameFinder);

    ASSERT_TRUE(
        g_implArkM->classSetSuperClass(classCtxFinder0.klass->GetArkTSImpl(), classCtxFinder1.klass->GetArkTSImpl()));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(classCtxFinder0.klass->superClass, classCtxFinder1.klass);

    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(output.c_str(), &file);

    ctxFinder = {nullptr, "super_class"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    classCtxFinder0 = {nullptr, "MyClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder0, helpers::ClassByNameFinder);

    classCtxFinder1 = {nullptr, "SuClass"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &classCtxFinder1, helpers::ClassByNameFinder);

    EXPECT_EQ(classCtxFinder0.klass->superClass, classCtxFinder1.klass);

    SuperClassTransformCtorIr(file);
    SuperClassTransformMainIr(file);

    g_impl->writeAbc(file, transform.c_str(), transform.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    auto result = helpers::ExecuteStaticAbc(input, "super_class", "main");
    EXPECT_TRUE(helpers::Match(result, "key: key value: 0\n"));
    result = helpers::ExecuteStaticAbc(output, "super_class", "main");
    EXPECT_TRUE(helpers::Match(result, "key: key value: 0\n"));
    result = helpers::ExecuteStaticAbc(transform, "super_class", "main");
    EXPECT_TRUE(helpers::Match(result, "key: sukey value: 1\n"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ClassSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetName1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc",
                                            "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "PersonTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    auto className = "Person";
    bool ret = g_implArkM->classSetName(g_implArkI->coreClassToArktsClass(klass), className);
    auto getClassName = g_implI->abckitStringToString(g_implI->classGetName(klass));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    ASSERT_EQ(std::string(getClassName), "Person");

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_classSetName_1.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(mdFinder.module, nullptr);
    module = mdFinder.module;
    clsFinder = {nullptr, "PersonTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(clsFinder.klass, nullptr);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ClassSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetName2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc",
                                            "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "ParentTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    auto className = "Parent";
    auto ret = g_implArkM->classSetName(g_implArkI->coreClassToArktsClass(klass), className);
    auto getClassName = g_implI->abckitStringToString(g_implI->classGetName(klass));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    ASSERT_EQ(std::string(getClassName), "Parent");

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_classSetName_2.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(mdFinder.module, nullptr);
    module = mdFinder.module;
    clsFinder = {nullptr, "ParentTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(clsFinder.klass, nullptr);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ClassAddField, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassAddField1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    auto coreClass = GetAbckitCoreClass(file, "classset_static", "PersonTest");

    struct AbckitArktsFieldCreateParams params;
    params.name = "newClassField";
    params.type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    std::string moduleFieldValue = "hello";
    params.value = g_implM->createValueString(file, moduleFieldValue.c_str(), moduleFieldValue.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    params.isStatic = true;
    params.fieldVisibility = AbckitArktsFieldVisibility::PUBLIC;

    auto ret = g_implArkM->classAddField(g_implArkI->coreClassToArktsClass(coreClass), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_ClassAddField.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);

    auto coreClass2 = GetAbckitCoreClass(file, "classset_static", "PersonTest");

    std::vector<std::string> FieldNames;
    g_implI->classEnumerateFields(coreClass2, &FieldNames, [](AbckitCoreClassField *field, void *data) {
        auto ctx = static_cast<std::vector<std::string> *>(data);
        auto filedName = g_implI->abckitStringToString(g_implI->classFieldGetName(field));
        (*ctx).emplace_back(filedName);
        return true;
    });
    sort(FieldNames.begin(), FieldNames.end());
    std::vector<std::string> actualFieldNames = {"age", "name", "newClassField"};
    ASSERT_EQ(FieldNames, actualFieldNames);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc",
                                        "classset_static", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "classset_static", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ClassSetOwningModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetOwningModule1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc",
                                            "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "PersonTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    auto moduleTest = std::make_unique<AbckitCoreModule>();
    const char *moduleName = "LocalModule";
    moduleTest->file = file;
    moduleTest->moduleName = libabckit::CreateStringStatic(file, moduleName, strlen(moduleName));
    moduleTest->isExternal = false;
    moduleTest->target = ABCKIT_TARGET_ARK_TS_V2;
    moduleTest->impl = std::make_unique<AbckitArktsModule>();
    moduleTest->GetArkTSImpl()->core = moduleTest.get();

    bool ret = g_implArkM->classSetOwningModule(g_implArkI->coreClassToArktsClass(klass),
                                                g_implArkI->coreModuleToArktsModule(moduleTest.get()));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    auto setNewModuleName = g_implI->abckitStringToString(klass->owningModule->moduleName);
    ASSERT_EQ(std::string(setNewModuleName), "LocalModule");

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_ClassSetOwningModule_1.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ClassSetOwningModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiClassTests, ClassSetOwningModule2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc",
                                            "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "classset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "ParentTest"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    auto moduleTest = std::make_unique<AbckitCoreModule>();
    const char *moduleName = "LocalModule";
    moduleTest->file = file;
    moduleTest->moduleName = libabckit::CreateStringStatic(file, moduleName, strlen(moduleName));
    moduleTest->isExternal = false;
    moduleTest->target = ABCKIT_TARGET_ARK_TS_V2;
    moduleTest->impl = std::make_unique<AbckitArktsModule>();
    moduleTest->GetArkTSImpl()->core = moduleTest.get();

    auto ret = g_implArkM->classSetOwningModule(g_implArkI->coreClassToArktsClass(klass),
                                                g_implArkI->coreModuleToArktsModule(moduleTest.get()));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    auto setNewModuleName = g_implI->abckitStringToString(klass->owningModule->moduleName);
    ASSERT_EQ(std::string(setNewModuleName), "LocalModule");

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/class_static_modify_ClassSetOwningModule_2.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "classset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Alice\n"));
}

TEST_F(LibAbcKitArkTSModifyApiClassTests, EnumerateSetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static.abc", &file);

    g_implI->fileEnumerateModules(file, nullptr, [](AbckitCoreModule *module, void * /*data*/) -> bool {
        g_implI->moduleEnumerateNamespaces(module, nullptr, [](AbckitCoreNamespace *ns, void * /*data*/) -> bool {
            auto nsName = g_implI->abckitStringToString(g_implI->namespaceGetName(ns));
            if (nsName != nullptr) {
                std::string newNsName = std::string(nsName) + "_Modified";
                auto arktsNs = g_implArkI->coreNamespaceToArktsNamespace(ns);
                g_implArkM->namespaceSetName(arktsNs, newNsName.c_str());
                EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            }

            g_implI->namespaceEnumerateClasses(ns, nullptr, [](AbckitCoreClass *klass, void * /*data*/) -> bool {
                auto className = g_implI->abckitStringToString(g_implI->classGetName(klass));
                if (className == nullptr) {
                    return true;
                }

                std::string newName = std::string(className) + "_Modified";
                auto arktsClass = g_implArkI->coreClassToArktsClass(klass);
                bool ret = g_implArkM->classSetName(arktsClass, newName.c_str());
                EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                EXPECT_TRUE(ret);

                return true;
            });
            return true;
        });
        return true;
    });
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static_out.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/class/classset_static_out.abc"));
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
