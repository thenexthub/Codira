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

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiAnnotationsStaticTests : public ::testing::Test {};

static std::string TEST_ABC_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static.abc";
static std::string MODIFY_TEST_ABC_PATH =
    ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_md.abc";

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
    EXPECT_TRUE(ccfFinder.ccf != nullptr);

    return ccfFinder.ccf;
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
    EXPECT_TRUE(cifFinder.cif != nullptr);

    return cifFinder.cif;
}

static void CheckAnnotationInterfaceField(std::string outputPath, bool bAdd)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto *method = helpers::FindMethodByName(file, "foo");
    EXPECT_NE(method, nullptr);
    auto *module = g_implI->functionGetModule(method);

    std::string aiName = "interfacetesttwo";
    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    std::string fieldName = "newField";
    helpers::AnnotationInterfaceFieldByNameContext aiFieldFinder = {nullptr, fieldName.c_str()};
    g_implI->annotationInterfaceEnumerateFields(aiFinder.ai, &aiFieldFinder,
                                                helpers::AnnotationInterfaceFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    if (bAdd) {
        EXPECT_TRUE(aiFieldFinder.aif != nullptr);
        auto val1 = g_implI->annotationInterfaceFieldGetDefaultValue(aiFieldFinder.aif);
        bool value1 = g_implI->valueGetU1(val1);
        EXPECT_TRUE(value1 == false);
    } else {
        EXPECT_FALSE(aiFieldFinder.aif != nullptr);
    }
    g_impl->closeFile(file);
}

/**
 * @tc.name: ModuleAddAnnotationInterface
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ModuleAddAnnotationInterface, abc-kind=ArkTS2,
 *           category=positive, extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, ModuleAddAnnotationInterface)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto *module = g_implI->functionGetModule(method);
    std::string aiName("NewAnnotation");
    struct AbckitArktsAnnotationInterfaceCreateParams annoIfParams {};
    annoIfParams.name = aiName.c_str();
    g_implArkM->moduleAddAnnotationInterface(g_implArkI->coreModuleToArktsModule(module), &annoIfParams);

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    module = g_implI->functionGetModule(method);
    aiFinder.ai = nullptr;
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);
    g_impl->closeFile(file);
}

/**
 * @tc.name: AnnotationInterfaceAddField
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::AnnotationInterfaceAddField, abc-kind=ArkTS2,
 *           category=positive, extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, AnnotationInterfaceAddField)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto *module = g_implI->functionGetModule(method);
    std::string aiName = "interfacetesttwo";
    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    struct AbckitArktsAnnotationInterfaceFieldCreateParams annoFieldCreateParams {};
    AbckitType *type = g_implM->createType(file, ABCKIT_TYPE_ID_U1);
    AbckitValue *value = g_implM->createValueU1(file, false);
    annoFieldCreateParams.name = "newField";
    annoFieldCreateParams.type = type;
    annoFieldCreateParams.defaultValue = value;
    auto field = g_implArkM->annotationInterfaceAddField(
        g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(aiFinder.ai), &annoFieldCreateParams);
    EXPECT_NE(field, nullptr);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    CheckAnnotationInterfaceField(outputPath, true);
}

/**
 * @tc.name: AnnotationInterfaceRemoveField
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::AnnotationInterfaceRemoveField, abc-kind=ArkTS2,
 *           category=positive, extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, AnnotationInterfaceRemoveField)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto *module = g_implI->functionGetModule(method);

    std::string aiName = "interfacetesttwo";
    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    std::string fieldName = "a2F2";
    helpers::AnnotationInterfaceFieldByNameContext aiFieldFinder = {nullptr, fieldName.c_str()};
    g_implI->annotationInterfaceEnumerateFields(aiFinder.ai, &aiFieldFinder,
                                                helpers::AnnotationInterfaceFieldByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFieldFinder.aif != nullptr);

    g_implArkM->annotationInterfaceRemoveField(
        g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(aiFinder.ai),
        g_implArkI->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(aiFieldFinder.aif));

    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    std::string outputPath = MODIFY_TEST_ABC_PATH;
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    CheckAnnotationInterfaceField(outputPath, false);
}

/**
 * @tc.name: ClassFieldAddAnnotation
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ClassFieldAddAnnotation, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, ClassFieldAddAnnotation)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    std::string moduleName = "annotations_static";
    std::string classNames("c2");
    std::string paramNames("c2F1");
    auto ccf = GetAbckitCoreClassField(file, moduleName, classNames, paramNames);
    ASSERT_NE(ccf, nullptr);

    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto module = g_implI->functionGetModule(method);
    std::string aiName = "interfacetest";
    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(aiFinder.ai);
    bool ret = g_implArkM->classFieldAddAnnotation(g_implArkI->coreClassFieldToArktsClassField(ccf), &annoCreateParams);
    ASSERT_EQ(ret, true);
    helpers::AnnotationByNameContext annoFinder = {nullptr, aiName.c_str()};
    g_implI->classFieldEnumerateAnnotations(ccf, &annoFinder, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder.anno != nullptr);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto ccfModify = GetAbckitCoreClassField(file, moduleName, classNames, paramNames);
    ASSERT_NE(ccfModify, nullptr);
    annoFinder.anno = nullptr;
    g_implI->classFieldEnumerateAnnotations(ccfModify, &annoFinder, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder.anno != nullptr);
    g_impl->closeFile(file);
}

/**
 * @tc.name: ClassFieldRemoveAnnotation
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::ClassFieldRemoveAnnotation, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, ClassFieldRemoveAnnotation)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);

    std::string moduleName = "annotations_static";
    std::string classNames("c1");
    std::string paramNames("c1F2");
    auto ccf = GetAbckitCoreClassField(file, moduleName, classNames, paramNames);
    ASSERT_NE(ccf, nullptr);

    std::string aiName = "interfacetest";
    helpers::AnnotationByNameContext annoFinder = {nullptr, aiName.c_str()};
    g_implI->classFieldEnumerateAnnotations(ccf, &annoFinder, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder.anno != nullptr);

    bool ret = g_implArkM->classFieldRemoveAnnotation(g_implArkI->coreClassFieldToArktsClassField(ccf),
                                                      g_implArkI->coreAnnotationToArktsAnnotation(annoFinder.anno));
    ASSERT_EQ(ret, true);
    helpers::AnnotationByNameContext annoFinder2 = {nullptr, aiName.c_str()};
    g_implI->classFieldEnumerateAnnotations(ccf, &annoFinder2, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder2.anno == nullptr);
    std::string outputPath = MODIFY_TEST_ABC_PATH;
    // Write output
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto ccfModify = GetAbckitCoreClassField(file, moduleName, classNames, paramNames);
    ASSERT_NE(ccfModify, nullptr);
    g_implI->classFieldEnumerateAnnotations(ccfModify, &annoFinder2, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder2.anno == nullptr);

    g_impl->closeFile(file);
}

/**
 * @tc.name: InterfaceFieldRemoveAnnotation
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::InterfaceFieldRemoveAnnotation, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, InterfaceFieldRemoveAnnotation)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    std::string moduleName = "annotations_static";
    std::string interfaceNames("I1");
    std::string paramNames("%%property-i1F1");
    auto cif = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cif, nullptr);

    std::string aiName = "interfacetest";
    helpers::AnnotationByNameContext annoFind = {nullptr, aiName.c_str()};
    g_implI->interfaceFieldEnumerateAnnotations(cif, &annoFind, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFind.anno != nullptr);

    bool ret = g_implArkM->interfaceFieldRemoveAnnotation(g_implArkI->coreInterfaceFieldToArktsInterfaceField(cif),
                                                          g_implArkI->coreAnnotationToArktsAnnotation(annoFind.anno));
    ASSERT_EQ(ret, true);
    helpers::AnnotationByNameContext annoFind1 = {nullptr, aiName.c_str()};
    g_implI->interfaceFieldEnumerateAnnotations(cif, &annoFind1, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFind1.anno == nullptr);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto cifModify = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cifModify, nullptr);
    g_implI->interfaceFieldEnumerateAnnotations(cifModify, &annoFind1, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(annoFind1.anno == nullptr);
    g_impl->closeFile(file);
}

/**
 * @tc.name: InterfaceFieldAddAnnotation
 * @tc.desc: test-kind=api, api=ArktsModifyApiImpl::InterfaceFieldAddAnnotation, abc-kind=ArkTS2, category=positive,
 *           extension=c
 * @tc.type: FUNC
 */
TEST_F(LibAbcKitModifyApiAnnotationsStaticTests, InterfaceFieldAddAnnotation)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(TEST_ABC_PATH.c_str(), &file);
    std::string moduleName = "annotations_static";
    std::string interfaceNames("I2");
    std::string paramNames("%%property-i2F1");
    auto cif = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cif, nullptr);

    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto module = g_implI->functionGetModule(method);
    std::string aiName = "interfacetest";
    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, aiName.c_str()};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(aiFinder.ai != nullptr);

    helpers::AnnotationByNameContext annoFinder = {nullptr, aiName.c_str()};
    g_implI->interfaceFieldEnumerateAnnotations(cif, &annoFinder, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder.anno == nullptr);

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(aiFinder.ai);
    bool ret = g_implArkM->interfaceFieldAddAnnotation(g_implArkI->coreInterfaceFieldToArktsInterfaceField(cif),
                                                       &annoCreateParams);
    ASSERT_EQ(ret, true);
    helpers::AnnotationByNameContext annoFinder2 = {nullptr, aiName.c_str()};
    g_implI->interfaceFieldEnumerateAnnotations(cif, &annoFinder2, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder2.anno != nullptr);

    std::string outputPath = MODIFY_TEST_ABC_PATH;
    g_impl->writeAbc(file, outputPath.c_str(), outputPath.size());
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(outputPath.c_str(), &file);
    auto cifModify = GetAbckitCoreInterfaceField(file, moduleName, interfaceNames, paramNames);
    ASSERT_NE(cifModify, nullptr);
    helpers::AnnotationByNameContext annoFinder3 = {nullptr, aiName.c_str()};
    g_implI->interfaceFieldEnumerateAnnotations(cifModify, &annoFinder3, helpers::AnnotationByNameFinder);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(annoFinder3.anno != nullptr);
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
