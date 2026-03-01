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
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArktsI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSInspectApiCasts : public ::testing::Test {};

static void ProcessAnnotationElement(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    bool isVisitedAny = false;
    g_implI->functionEnumerateAnnotations(method, &isVisitedAny, [](AbckitCoreAnnotation *anno, void *data) {
        g_implI->annotationEnumerateElements(anno, data, [](AbckitCoreAnnotationElement *annoElem, void *data) {
            AbckitArktsAnnotationElement *annoArkTSelem =
                g_implArktsI->coreAnnotationElementToArktsAnnotationElement(annoElem);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            AbckitCoreAnnotationElement *annoElemCast =
                g_implArktsI->arktsAnnotationElementToCoreAnnotationElement(annoArkTSelem);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_EQ(annoElem, annoElemCast);
            *reinterpret_cast<bool *>(data) = true;
            return true;
        });
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessAnnotation(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    g_implI->functionEnumerateAnnotations(method, nullptr, [](AbckitCoreAnnotation *anno, void *) {
        AbckitArktsAnnotation *annoArkTS = g_implArktsI->coreAnnotationToArktsAnnotation(anno);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreAnnotation *annoElemCast = g_implArktsI->arktsAnnotationToCoreAnnotation(annoArkTS);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(anno, annoElemCast);
        return true;
    });
}

static void ProcessAnnotationInterface(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateAnnotationInterfaces(
        module, &isVisitedAny, [](AbckitCoreAnnotationInterface *annoI, void *data) {
            AbckitArktsAnnotationInterface *annoIArkTS =
                g_implArktsI->coreAnnotationInterfaceToArktsAnnotationInterface(annoI);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            AbckitCoreAnnotationInterface *annoICast =
                g_implArktsI->arktsAnnotationInterfaceToCoreAnnotationInterface(annoIArkTS);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_EQ(annoI, annoICast);
            *reinterpret_cast<bool *>(data) = true;
            return true;
        });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessAnnotationInterfaceField(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateAnnotationInterfaces(
        module, &isVisitedAny, [](AbckitCoreAnnotationInterface *annoI, void *data) {
            g_implI->annotationInterfaceEnumerateFields(
                annoI, data, [](AbckitCoreAnnotationInterfaceField *annoIf, void *data) {
                    AbckitArktsAnnotationInterfaceField *annoIfArkTS =
                        g_implArktsI->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(annoIf);
                    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                    AbckitCoreAnnotationInterfaceField *annoIfCast =
                        g_implArktsI->arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(annoIfArkTS);
                    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                    EXPECT_EQ(annoIf, annoIfCast);
                    *reinterpret_cast<bool *>(data) = true;
                    return true;
                });
            return true;
        });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessClass(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreClass *klass = g_implI->functionGetParentClass(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitArktsClass *klassArkTS = g_implArktsI->coreClassToArktsClass(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreClass *klassCast = g_implArktsI->arktsClassToCoreClass(klassArkTS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(klass, klassCast);
}

static void ProcessClassNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreClass *klass = g_implI->functionGetParentClass(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitArktsClass *klassArkTS = g_implArktsI->coreClassToArktsClass(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(klassArkTS, nullptr);
}

static void ProcessMethod(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitArktsFunction *methodArkTS = g_implArktsI->coreFunctionToArktsFunction(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreFunction *methodCast = g_implArktsI->arktsFunctionToCoreFunction(methodArkTS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(method, methodCast);
}

static void ProcessMethodNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitArktsFunction *methodArk = g_implArktsI->coreFunctionToArktsFunction(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(methodArk, nullptr);
}

static void ProcessModule(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitArktsModule *moduleArkTS = g_implArktsI->coreModuleToArktsModule(module);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreModule *moduleCast = g_implArktsI->arktsModuleToCoreModule(moduleArkTS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(module, moduleCast);
}

static void ProcessModuleNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitArktsModule *moduleArkTS = g_implArktsI->coreModuleToArktsModule(module);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(moduleArkTS, nullptr);
}

static void ProcessNamespace(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    bool visited = false;
    g_implI->moduleEnumerateNamespaces(module, &visited, [](AbckitCoreNamespace *n, void *visited) {
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitArktsNamespace *namespaceArkTS = g_implArktsI->coreNamespaceToArktsNamespace(n);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreNamespace *namespaceCast = g_implArktsI->arktsNamespaceToCoreNamespace(namespaceArkTS);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(n, namespaceCast);
        *reinterpret_cast<bool *>(visited) = true;
        return true;
    });
    EXPECT_TRUE(visited);
}

static void ProcessImportDescriptor(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateImports(module, &isVisitedAny, [](AbckitCoreImportDescriptor *id, void *data) {
        AbckitArktsImportDescriptor *idArkTS = g_implArktsI->coreImportDescriptorToArktsImportDescriptor(id);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreImportDescriptor *idCast = g_implArktsI->arktsImportDescriptorToCoreImportDescriptor(idArkTS);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(id, idCast);
        *reinterpret_cast<bool *>(data) = true;
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessImportDescriptorNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateImports(module, &isVisitedAny, [](AbckitCoreImportDescriptor *id, void *data) {
        AbckitArktsImportDescriptor *idArk = g_implArktsI->coreImportDescriptorToArktsImportDescriptor(id);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
        EXPECT_EQ(idArk, nullptr);
        *reinterpret_cast<bool *>(data) = true;
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessExportDescriptor(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateExports(module, &isVisitedAny, [](AbckitCoreExportDescriptor *ed, void *data) {
        AbckitArktsExportDescriptor *edArkTS = g_implArktsI->coreExportDescriptorToArktsExportDescriptor(ed);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreExportDescriptor *edCast = g_implArktsI->arktsExportDescriptorToCoreExportDescriptor(edArkTS);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(ed, edCast);
        *reinterpret_cast<bool *>(data) = true;
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

static void ProcessExportDescriptorNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateExports(module, &isVisitedAny, [](AbckitCoreExportDescriptor *ed, void *data) {
        AbckitArktsExportDescriptor *edArkTS = g_implArktsI->coreExportDescriptorToArktsExportDescriptor(ed);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
        EXPECT_EQ(edArkTS, nullptr);
        *reinterpret_cast<bool *>(data) = true;
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsAnnotationElementToCoreAnnotationElement, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsAnnotationElementToCoreAnnotationElement)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationElement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreAnnotationElementToArktsAnnotationElement, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreAnnotationElementToArktsAnnotationElement)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationElement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsAnnotationToCoreAnnotation, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsAnnotationToCoreAnnotation)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreAnnotationToArktsAnnotation, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreAnnotationToArktsAnnotation)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreAnnotationInterfaceToArktsAnnotationInterface, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreAnnotationInterfaceToArktsAnnotationInterface)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationInterface);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsAnnotationInterfaceToCoreAnnotationInterface, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsAnnotationInterfaceToCoreAnnotationInterface)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationInterface);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField,
// abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationInterfaceField);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField,
// abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessAnnotationInterfaceField);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreClassToArktsClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreClassToArktsClass)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreClassToArktsClass, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreClassToArktsClassNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessClassNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsClassToCoreClass, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsClassToCoreClass)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreFunctionToArktsFunction, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreFunctionToArktsFunction)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreFunctionToArktsFunction, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreFunctionToArktsFunctionNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessMethodNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsFunctionToCoreFunction, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsFunctionToCoreFunction)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreModuleToArktsModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreModuleToArktsModule)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreModuleToArktsModule, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreModuleToArktsModuleNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessModuleNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsModuleToCoreModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsModuleToCoreModule)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreNamespaceToArktsNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreNamespaceToArktsNamespace)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessNamespace);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsNamespaceToCoreNamespace, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsNamespaceToCoreNamespace)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessNamespace);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreImportDescriptorToArktsImportDescriptor, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreImportDescriptorToArktsImportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessImportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreImportDescriptorToArktsImportDescriptor, abc-kind=ArkTS2,
// category=negative
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreImportDescriptorToArktsImportDescriptorNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessImportDescriptorNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsImportDescriptorToCoreImportDescriptor, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsImportDescriptorToCoreImportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessImportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreExportDescriptorToArktsExportDescriptor, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreExportDescriptorToArktsExportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessExportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::coreExportDescriptorToArktsExportDescriptor, abc-kind=ArkTS2,
// category=negative
TEST_F(LibAbcKitArkTSInspectApiCasts, CoreExportDescriptorToArktsExportDescriptorNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessExportDescriptorNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsInspectApiImpl::arktsExportDescriptorToCoreExportDescriptor, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSInspectApiCasts, ArktsExportDescriptorToCoreExportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessExportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
