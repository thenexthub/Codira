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
#include "libabckit/c/extensions/js/metadata_js.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsI = AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitJSInspectApiCasts : public ::testing::Test {};

static void ProcessClass(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreClass *klass = g_implI->functionGetParentClass(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitJsClass *klassJS = g_implJsI->coreClassToJsClass(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreClass *klassCast = g_implJsI->jsClassToCoreClass(klassJS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(klass, klassCast);
}

static void ProcessClassNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreClass *klass = g_implI->functionGetParentClass(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitJsClass *klassJS = g_implJsI->coreClassToJsClass(klass);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(klassJS, nullptr);
}

static void ProcessMethod(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitJsFunction *methodJS = g_implJsI->coreFunctionToJsFunction(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreFunction *methodCast = g_implJsI->jsFunctionToCoreFunction(methodJS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(method, methodCast);
}

static void ProcessMethodNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitJsFunction *methodJS = g_implJsI->coreFunctionToJsFunction(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(methodJS, nullptr);
}

static void ProcessModule(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitJsModule *moduleJS = g_implJsI->coreModuleToJsModule(module);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitCoreModule *moduleCast = g_implJsI->jsModuleToCoreModule(moduleJS);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_EQ(module, moduleCast);
}

static void ProcessModuleNeg(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitJsModule *moduleJS = g_implJsI->coreModuleToJsModule(module);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    EXPECT_EQ(moduleJS, nullptr);
}

static void ProcessImportDescriptor(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AbckitCoreModule *module = g_implI->functionGetModule(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    bool isVisitedAny = false;
    g_implI->moduleEnumerateImports(module, &isVisitedAny, [](AbckitCoreImportDescriptor *id, void *data) {
        AbckitJsImportDescriptor *idJS = g_implJsI->coreImportDescriptorToJsImportDescriptor(id);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreImportDescriptor *idCast = g_implJsI->jsImportDescriptorToCoreImportDescriptor(idJS);
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
        AbckitJsImportDescriptor *idJS = g_implJsI->coreImportDescriptorToJsImportDescriptor(id);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
        EXPECT_EQ(idJS, nullptr);
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
        AbckitJsExportDescriptor *edJS = g_implJsI->coreExportDescriptorToJsExportDescriptor(ed);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        AbckitCoreExportDescriptor *edCast = g_implJsI->jsExportDescriptorToCoreExportDescriptor(edJS);
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
        AbckitJsExportDescriptor *edJS = g_implJsI->coreExportDescriptorToJsExportDescriptor(ed);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
        EXPECT_EQ(edJS, nullptr);
        *reinterpret_cast<bool *>(data) = true;
        return true;
    });
    ASSERT_TRUE(isVisitedAny);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreClassToJsClass, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, CoreClassToJsClass)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::jsClassToCoreClass, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, JsClassToCoreClass)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreClassToJsClass, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSInspectApiCasts, CoreClassToJsClassNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessClassNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreFunctionToJsFunction, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, CoreFunctionToJsFunction)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::jsFunctionToCoreFunction, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, JsFunctionToCoreFunction)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreFunctionToJsFunction, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSInspectApiCasts, CoreFunctionToJsFunctionNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessMethodNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreModuleToJsModule, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, CoreModuleToJsModule)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::jsModuleToCoreModule, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, JsModuleToCoreModule)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreModuleToJsModule, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSInspectApiCasts, CoreModuleToJsModuleNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessModuleNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreImportDescriptorToJsImportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, CoreImportDescriptorToJsImportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessImportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::jsImportDescriptorToCoreImportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, JsImportDescriptorToCoreImportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessImportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreImportDescriptorToJsImportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSInspectApiCasts, CoreImportDescriptorToJsImportDescriptorNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessImportDescriptorNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreExportDescriptorToJsExportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, CoreExportDescriptorToJsExportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessExportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::coreExportDescriptorToJsExportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSInspectApiCasts, CoreExportDescriptorToJsExportDescriptorNeg)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/inspect_api/api_casts/api_casts.abc", "foo",
                           ProcessExportDescriptorNeg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsInspectApiImpl::jsExportDescriptorToCoreExportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiCasts, JsExportDescriptorToCoreExportDescriptor)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/api_casts/JSapi_casts.abc", "foo",
                           ProcessExportDescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
