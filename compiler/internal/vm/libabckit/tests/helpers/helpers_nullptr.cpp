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

#include "helpers_nullptr.h"

#include <gtest/gtest.h>
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"
#include "metadata_inspect_impl.h"

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace libabckit::test::helpers_nullptr {

static constexpr uintptr_t ABCKIT_TEST_POINTER_TAG = 0x1;
// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
[[maybe_unused]] static AbckitFile *g_abckitFile = reinterpret_cast<AbckitFile *>(0x1);
[[maybe_unused]] static AbckitGraph *g_abckitGraph = reinterpret_cast<AbckitGraph *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationElement *g_abckitArktsAnnotationelement =
    reinterpret_cast<AbckitArktsAnnotationElement *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationInterfaceField *g_abckitArktsAnnotationinterfacefield =
    reinterpret_cast<AbckitArktsAnnotationInterfaceField *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationInterface *g_abckitArktsAnnotationinterface =
    reinterpret_cast<AbckitArktsAnnotationInterface *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotation *g_abckitArktsAnnotation = reinterpret_cast<AbckitArktsAnnotation *>(0x1);
[[maybe_unused]] static AbckitArktsClass *g_abckitArktsClass = reinterpret_cast<AbckitArktsClass *>(0x1);
[[maybe_unused]] static AbckitArktsExportDescriptor *g_abckitArktsExportdescriptor =
    reinterpret_cast<AbckitArktsExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitArktsClassField *g_abckitArktsClassField = reinterpret_cast<AbckitArktsClassField *>(0x1);
[[maybe_unused]] static AbckitArktsInterface *g_abckitArktsInterface = reinterpret_cast<AbckitArktsInterface *>(0x1);
[[maybe_unused]] static AbckitArktsInterfaceField *g_abckitArktsInterfaceField =
    reinterpret_cast<AbckitArktsInterfaceField *>(0x1);
[[maybe_unused]] static AbckitArktsImportDescriptor *g_abckitArktsImportdescriptor =
    reinterpret_cast<AbckitArktsImportDescriptor *>(0x1);
[[maybe_unused]] static AbckitArktsFunction *g_abckitArktsMethod = reinterpret_cast<AbckitArktsFunction *>(0x1);
[[maybe_unused]] static AbckitArktsModule *g_abckitArktsModule = reinterpret_cast<AbckitArktsModule *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationElement *g_abckitAnnotationelement =
    reinterpret_cast<AbckitCoreAnnotationElement *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationInterfaceField *g_abckitAnnotationinterfacefield =
    reinterpret_cast<AbckitCoreAnnotationInterfaceField *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotationInterface *g_abckitAnnotationinterface =
    reinterpret_cast<AbckitCoreAnnotationInterface *>(0x1);
[[maybe_unused]] static AbckitCoreAnnotation *g_abckitAnnotation = reinterpret_cast<AbckitCoreAnnotation *>(0x1);
[[maybe_unused]] static AbckitCoreClass *g_abckitClass = reinterpret_cast<AbckitCoreClass *>(0x1);
[[maybe_unused]] static AbckitCoreClassField *g_abckitClassField = reinterpret_cast<AbckitCoreClassField *>(0x1);
[[maybe_unused]] static AbckitCoreInterface *g_abckitInterface = reinterpret_cast<AbckitCoreInterface *>(0x1);
[[maybe_unused]] static AbckitCoreInterfaceField *g_abckitInterfaceField =
    reinterpret_cast<AbckitCoreInterfaceField *>(0x1);
[[maybe_unused]] static AbckitCoreEnum *g_abckitEnum = reinterpret_cast<AbckitCoreEnum *>(0x1);
[[maybe_unused]] static AbckitCoreExportDescriptor *g_abckitExportdescriptor =
    reinterpret_cast<AbckitCoreExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitCoreImportDescriptor *g_abckitImportdescriptor =
    reinterpret_cast<AbckitCoreImportDescriptor *>(0x1);
[[maybe_unused]] static AbckitCoreFunction *g_abckitFunction = reinterpret_cast<AbckitCoreFunction *>(0x1);
[[maybe_unused]] static AbckitCoreModule *g_abckitModule = reinterpret_cast<AbckitCoreModule *>(0x1);
[[maybe_unused]] static AbckitArktsAnnotationElementCreateParams *g_abckitArktsAnnotationelementcreateparams =
    reinterpret_cast<AbckitArktsAnnotationElementCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationInterfaceFieldCreateParams
    *g_constAbckitArktsAnnotationinterfacefieldcreateparams =
        reinterpret_cast<const AbckitArktsAnnotationInterfaceFieldCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationCreateParams *g_constAbckitArktsAnnotationcreateparams =
    reinterpret_cast<const AbckitArktsAnnotationCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsAnnotationInterfaceCreateParams
    *g_constAbckitArktsAnnotationinterfacecreateparams =
        reinterpret_cast<const AbckitArktsAnnotationInterfaceCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsDynamicModuleExportCreateParams
    *g_constAbckitArktsDynamicmoduleexportcreateparams =
        reinterpret_cast<const AbckitArktsDynamicModuleExportCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsImportFromDynamicModuleCreateParams
    *g_constAbckitArktsImportfromdynamicmodulecreateparams =
        reinterpret_cast<const AbckitArktsImportFromDynamicModuleCreateParams *>(0x1);
[[maybe_unused]] static const AbckitArktsV1ExternalModuleCreateParams *g_constAbckitArktsV1ExternalModuleCreateParams =
    reinterpret_cast<const AbckitArktsV1ExternalModuleCreateParams *>(0x1);
[[maybe_unused]] static AbckitBasicBlock *g_abckitBasicblock = reinterpret_cast<AbckitBasicBlock *>(0x1);
[[maybe_unused]] static AbckitInst *g_abckitInst = reinterpret_cast<AbckitInst *>(0x1);
[[maybe_unused]] static void *g_void = reinterpret_cast<void *>(0x1);
[[maybe_unused]] static AbckitLiteralArray *g_abckitLiteralarray = reinterpret_cast<AbckitLiteralArray *>(0x1);
[[maybe_unused]] static AbckitString *g_abckitString = reinterpret_cast<AbckitString *>(0x1);
[[maybe_unused]] static AbckitValue *g_abckitValue = reinterpret_cast<AbckitValue *>(0x1);
[[maybe_unused]] static AbckitLiteral *g_abckitLiteral = reinterpret_cast<AbckitLiteral *>(0x1);
[[maybe_unused]] static AbckitCoreNamespace *g_abckitNamespace = reinterpret_cast<AbckitCoreNamespace *>(0x1);
[[maybe_unused]] static AbckitType *g_abckitType = reinterpret_cast<AbckitType *>(0x1);
[[maybe_unused]] static AbckitJsClass *g_abckitJsClass = reinterpret_cast<AbckitJsClass *>(0x1);
[[maybe_unused]] static AbckitJsFunction *g_abckitJsMethod = reinterpret_cast<AbckitJsFunction *>(0x1);
[[maybe_unused]] static AbckitJsModule *g_abckitJsModule = reinterpret_cast<AbckitJsModule *>(0x1);
[[maybe_unused]] static const AbckitJsDynamicModuleExportCreateParams *g_constAbckitJsDynamicmoduleexportcreateparams =
    reinterpret_cast<const AbckitJsDynamicModuleExportCreateParams *>(0x1);
[[maybe_unused]] static const AbckitJsImportFromDynamicModuleCreateParams
    *g_constAbckitJsImportfromdynamicmodulecreateparams =
        reinterpret_cast<const AbckitJsImportFromDynamicModuleCreateParams *>(0x1);
[[maybe_unused]] static AbckitJsExportDescriptor *g_abckitJsExportdescriptor =
    reinterpret_cast<AbckitJsExportDescriptor *>(0x1);
[[maybe_unused]] static AbckitJsImportDescriptor *g_abckitJsImportdescriptor =
    reinterpret_cast<AbckitJsImportDescriptor *>(0x1);
[[maybe_unused]] static const AbckitJsExternalModuleCreateParams *g_constAbckitJsExternalmodulecreateparams =
    reinterpret_cast<const AbckitJsExternalModuleCreateParams *>(0x1);
[[maybe_unused]] static AbckitArktsModuleField *g_abckitArktsModuleField =
    reinterpret_cast<AbckitArktsModuleField *>(ABCKIT_TEST_POINTER_TAG);
[[maybe_unused]] static AbckitArktsClassField *g_arktsClassField =
    reinterpret_cast<AbckitArktsClassField *>(ABCKIT_TEST_POINTER_TAG);
[[maybe_unused]] static AbckitArktsEnumField *g_abckitArktsEnumField =
    reinterpret_cast<AbckitArktsEnumField *>(ABCKIT_TEST_POINTER_TAG);
[[maybe_unused]] static AbckitArktsEnum *g_abckitArktsEnum =
    reinterpret_cast<AbckitArktsEnum *>(ABCKIT_TEST_POINTER_TAG);
[[maybe_unused]] static const AbckitArktsFieldCreateParams *g_constAbckitArktsFieldCreateParams =
    reinterpret_cast<const AbckitArktsFieldCreateParams *>(ABCKIT_TEST_POINTER_TAG);
[[maybe_unused]] static const AbckitArktsInterfaceFieldCreateParams *g_constAbckitArktsInterfaceFieldCreateParams =
    reinterpret_cast<const AbckitArktsInterfaceFieldCreateParams *>(ABCKIT_TEST_POINTER_TAG);
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

static const char *GetConstChar()
{
    return reinterpret_cast<const char *>(0x1);
}

void TestNullptr(void (*apiToCheck)(AbckitFile *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(GetConstChar(), 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    const char *invalidPath = "/path/to/nonexistent/file.abc";
    ASSERT_EQ(apiToCheck(invalidPath, strlen(invalidPath)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitFile *, const char *, size_t))
{
    apiToCheck(nullptr, GetConstChar(), 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, GetConstChar(), 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationElement *(*apiToCheck)(AbckitArktsAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterfaceField *(*apiToCheck)(AbckitArktsAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitArktsAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitArktsAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitArktsClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitArktsExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitArktsImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitArktsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitArktsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto element = std::make_unique<AbckitCoreAnnotationElement>();
    auto ann = std::make_unique<AbckitCoreAnnotation>();
    element->ann = ann.get();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    ann->ai = ai.get();
    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(element.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsAnnotationElement>();
    element->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(element.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(element.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto field = std::make_unique<AbckitCoreAnnotationInterfaceField>();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    field->ai = ai.get();
    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsAnnotationInterfaceField>();
    field->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(ai.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsAnnotationInterface>();
    ai->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(ai.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(ai.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto annotation = std::make_unique<AbckitCoreAnnotation>();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    annotation->ai = ai.get();
    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(annotation.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsAnnotation>();
    annotation->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(annotation.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(annotation.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsClass *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto descriptor = std::make_unique<AbckitCoreExportDescriptor>();
    auto module = std::make_unique<AbckitCoreModule>();
    descriptor->exportingModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsExportDescriptor>();
    descriptor->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto descriptor = std::make_unique<AbckitCoreImportDescriptor>();
    auto module = std::make_unique<AbckitCoreModule>();
    descriptor->importingModule = module.get();
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsImportDescriptor>();
    descriptor->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(descriptor.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsFunction *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitArktsAnnotation *,
                                                             AbckitArktsAnnotationElementCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsAnnotationelementcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsAnnotation, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto anno = std::make_unique<AbckitArktsAnnotation>();
    auto params = std::make_unique<AbckitArktsAnnotationElementCreateParams>();
    params->name = nullptr;
    ASSERT_EQ(apiToCheck(anno.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    params->name = name;
    params->value = nullptr;
    ASSERT_EQ(apiToCheck(anno.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto deleterFunc = [](libabckit::pandasm_Value *ptr) {};
    AbckitValueImplT valImpl(reinterpret_cast<libabckit::pandasm_Value *>(0x1), deleterFunc);
    auto value = std::make_unique<AbckitValue>(nullptr, std::move(valImpl));
    params->value = value.get();
    anno->core = nullptr;
    ASSERT_EQ(apiToCheck(anno.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto core = std::make_unique<AbckitCoreAnnotation>();
    anno->core = core.get();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    core->ai = ai.get();
    ai->owningModule = nullptr;
    ASSERT_EQ(apiToCheck(anno.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->file = nullptr;
    ASSERT_EQ(apiToCheck(anno.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(
    AbckitArktsAnnotationInterface *, const AbckitArktsAnnotationInterfaceFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationinterfacefieldcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsAnnotationinterface, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto ai = std::make_unique<AbckitArktsAnnotationInterface>();
    auto params = std::make_unique<AbckitArktsAnnotationInterfaceFieldCreateParams>();
    params->name = nullptr;
    ASSERT_EQ(apiToCheck(ai.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    params->name = name;
    params->type = nullptr;
    ASSERT_EQ(apiToCheck(ai.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto type = std::make_unique<AbckitType>();
    params->type = type.get();
    ai->core = nullptr;
    ASSERT_EQ(apiToCheck(ai.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotationInterface *, AbckitArktsAnnotationInterfaceField *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotationinterfacefield);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsAnnotationinterface, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto ai = std::make_unique<AbckitArktsAnnotationInterface>();
    auto field = std::make_unique<AbckitArktsAnnotationInterfaceField>();
    ai->core = nullptr;
    apiToCheck(ai.get(), field.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto aiCore = std::make_unique<AbckitCoreAnnotationInterface>();
    ai->core = aiCore.get();
    field->core = nullptr;
    apiToCheck(ai.get(), field.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto fieldCore = std::make_unique<AbckitCoreAnnotationInterfaceField>();
    field->core = fieldCore.get();
    auto aiModule = std::make_unique<AbckitCoreModule>();
    aiCore->owningModule = aiModule.get();
    aiModule->target = ABCKIT_TARGET_ARK_TS_V2;
    apiToCheck(ai.get(), field.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotation *, AbckitArktsAnnotationElement *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotationelement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsAnnotation, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto anno = std::make_unique<AbckitArktsAnnotation>();
    auto elem = std::make_unique<AbckitArktsAnnotationElement>();
    auto core = std::make_unique<AbckitCoreAnnotation>();
    anno->core = core.get();
    core->ai = nullptr;
    apiToCheck(anno.get(), elem.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    core->ai = ai.get();
    ai->owningModule = nullptr;
    apiToCheck(anno.get(), elem.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->file = nullptr;
    apiToCheck(anno.get(), elem.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsClass *, const AbckitArktsAnnotationCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    panda::pandasm::Function *func = nullptr;
    auto klass = std::make_unique<AbckitArktsClass>(func);
    auto params = std::make_unique<AbckitArktsAnnotationCreateParams>();
    params->ai = nullptr;
    ASSERT_EQ(apiToCheck(klass.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto ai = std::make_unique<AbckitArktsAnnotationInterface>();
    params->ai = ai.get();
    klass->core = nullptr;
    ASSERT_EQ(apiToCheck(klass.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto classCore = std::make_unique<AbckitCoreClass>(nullptr, *klass.get());
    klass->core = classCore.get();
    params->ai->core = nullptr;
    ASSERT_EQ(apiToCheck(klass.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto paramsCore = std::make_unique<AbckitCoreAnnotationInterface>();
    params->ai->core = paramsCore.get();
    auto classModule = std::make_unique<AbckitCoreModule>();
    classCore->owningModule = classModule.get();
    auto paramsModule = std::make_unique<AbckitCoreModule>();
    paramsCore->owningModule = paramsModule.get();
    classModule->target = ABCKIT_TARGET_ARK_TS_V1;
    paramsModule->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_EQ(apiToCheck(klass.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    classModule->target = ABCKIT_TARGET_ARK_TS_V2;
    paramsModule->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_EQ(apiToCheck(klass.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsClass *, AbckitArktsAnnotation *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsClass, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    panda::pandasm::Function *func = nullptr;
    auto klass = std::make_unique<AbckitArktsClass>(func);
    auto anno = std::make_unique<AbckitArktsAnnotation>();
    klass->core = nullptr;
    apiToCheck(klass.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto classCore = std::make_unique<AbckitCoreClass>(nullptr, *klass.get());
    klass->core = classCore.get();
    anno->core = nullptr;
    apiToCheck(klass.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto annoCore = std::make_unique<AbckitCoreAnnotation>();
    anno->core = annoCore.get();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    annoCore->ai = ai.get();
    auto classModule = std::make_unique<AbckitCoreModule>();
    classCore->owningModule = classModule.get();
    auto annoModule = std::make_unique<AbckitCoreModule>();
    ai->owningModule = annoModule.get();
    classModule->target = ABCKIT_TARGET_ARK_TS_V1;
    annoModule->target = ABCKIT_TARGET_ARK_TS_V2;
    apiToCheck(klass.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    classModule->target = ABCKIT_TARGET_ARK_TS_V2;
    annoModule->target = ABCKIT_TARGET_ARK_TS_V1;
    apiToCheck(klass.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsMethod), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsClassField *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsClassField), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, const char *))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar()), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsMethod), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsInterfaceField), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, const char *))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar()), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsInterface), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsFunction *, const AbckitArktsAnnotationCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsMethod, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto function = std::make_unique<AbckitArktsFunction>();
    auto params = std::make_unique<AbckitArktsAnnotationCreateParams>();
    function->core = nullptr;
    ASSERT_EQ(apiToCheck(function.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto functionCore = std::make_unique<AbckitCoreFunction>();
    function->core = functionCore.get();
    auto ai = std::make_unique<AbckitArktsAnnotationInterface>();
    params->ai = ai.get();
    ai->core = nullptr;
    ASSERT_EQ(apiToCheck(function.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto paramsCore = std::make_unique<AbckitCoreAnnotationInterface>();
    ai->core = paramsCore.get();
    auto functionModule = std::make_unique<AbckitCoreModule>();
    functionCore->owningModule = functionModule.get();
    auto paramsModule = std::make_unique<AbckitCoreModule>();
    paramsCore->owningModule = paramsModule.get();
    functionModule->target = ABCKIT_TARGET_ARK_TS_V1;
    paramsModule->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_EQ(apiToCheck(function.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    functionModule->target = ABCKIT_TARGET_ARK_TS_V2;
    paramsModule->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_EQ(apiToCheck(function.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsFunction *, AbckitArktsAnnotation *))
{
    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsMethod, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto function = std::make_unique<AbckitArktsFunction>();
    auto anno = std::make_unique<AbckitArktsAnnotation>();
    function->core = nullptr;
    apiToCheck(function.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto functionCore = std::make_unique<AbckitCoreFunction>();
    function->core = functionCore.get();
    anno->core = nullptr;
    apiToCheck(function.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto annoCore = std::make_unique<AbckitCoreAnnotation>();
    anno->core = annoCore.get();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    annoCore->ai = ai.get();
    auto functionModule = std::make_unique<AbckitCoreModule>();
    functionCore->owningModule = functionModule.get();
    auto annoModule = std::make_unique<AbckitCoreModule>();
    ai->owningModule = annoModule.get();
    functionModule->target = ABCKIT_TARGET_ARK_TS_V1;
    annoModule->target = ABCKIT_TARGET_ARK_TS_V2;
    apiToCheck(function.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    functionModule->target = ABCKIT_TARGET_ARK_TS_V2;
    annoModule->target = ABCKIT_TARGET_ARK_TS_V1;
    apiToCheck(function.get(), anno.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitArktsModule *,
                                                               const AbckitArktsAnnotationInterfaceCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsAnnotationinterfacecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto arktsModule = std::make_unique<AbckitArktsModule>();
    auto params = std::make_unique<AbckitArktsAnnotationInterfaceCreateParams>();
    params->name = nullptr;
    ASSERT_EQ(apiToCheck(arktsModule.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    params->name = name;
    arktsModule->core = nullptr;
    ASSERT_EQ(apiToCheck(arktsModule.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
}
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsDynamicModuleExportCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule, g_constAbckitArktsDynamicmoduleexportcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr, g_constAbckitArktsDynamicmoduleexportcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto exporting = std::make_unique<AbckitArktsModule>();
    auto exported = std::make_unique<AbckitArktsModule>();
    auto params = std::make_unique<AbckitArktsDynamicModuleExportCreateParams>();
    params->name = nullptr;
    ASSERT_EQ(apiToCheck(exporting.get(), exported.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    params->name = name;
    exporting->core = nullptr;
    ASSERT_EQ(apiToCheck(exporting.get(), exported.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto core = std::make_unique<AbckitCoreModule>();
    exporting->core = core.get();
    exported->core = nullptr;
    ASSERT_EQ(apiToCheck(exporting.get(), exported.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    exported->core = core.get();
    exporting->core->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_EQ(apiToCheck(exporting.get(), exported.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsImportFromDynamicModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsModule, g_constAbckitArktsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr, g_constAbckitArktsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    char alias[] = "aliasTest";
    auto params = std::make_unique<AbckitArktsImportFromDynamicModuleCreateParams>();
    params->name = nullptr;
    params->alias = alias;
    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    params->name = name;
    params->alias = nullptr;
    ASSERT_EQ(apiToCheck(g_abckitArktsModule, g_abckitArktsModule, params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    params->alias = alias;

    auto importingModule = std::make_unique<AbckitArktsModule>();
    auto importedModule = std::make_unique<AbckitArktsModule>();
    auto core = std::make_unique<AbckitCoreModule>();
    importingModule->core = nullptr;
    ASSERT_EQ(apiToCheck(importingModule.get(), importedModule.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
    importingModule->core = core.get();

    importedModule->core = nullptr;
    ASSERT_EQ(apiToCheck(importingModule.get(), importedModule.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
    importedModule->core = core.get();

    ASSERT_EQ(apiToCheck(importingModule.get(), importedModule.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitArktsExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto arktsModule = std::make_unique<AbckitArktsModule>();
    auto descriptor = std::make_unique<AbckitArktsExportDescriptor>();
    arktsModule->core = nullptr;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto moduleCore = std::make_unique<AbckitCoreModule>();
    arktsModule->core = moduleCore.get();
    descriptor->core = nullptr;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto descriptorCore = std::make_unique<AbckitCoreExportDescriptor>();
    auto exportingModule = std::make_unique<AbckitCoreModule>();
    descriptor->core = descriptorCore.get();
    descriptorCore->exportingModule = exportingModule.get();
    moduleCore->target = ABCKIT_TARGET_ARK_TS_V2;
    descriptorCore->exportingModule->target = ABCKIT_TARGET_ARK_TS_V1;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    descriptorCore->exportingModule->target = ABCKIT_TARGET_ARK_TS_V2;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitArktsImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitArktsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto arktsModule = std::make_unique<AbckitArktsModule>();
    auto descriptor = std::make_unique<AbckitArktsImportDescriptor>();
    arktsModule->core = nullptr;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto moduleCore = std::make_unique<AbckitCoreModule>();
    arktsModule->core = moduleCore.get();
    descriptor->core = nullptr;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto descriptorCore = std::make_unique<AbckitCoreImportDescriptor>();
    auto importingModule = std::make_unique<AbckitCoreModule>();
    descriptor->core = descriptorCore.get();
    descriptor->core->importingModule = importingModule.get();
    arktsModule->core->target = ABCKIT_TARGET_ARK_TS_V2;
    descriptor->core->importingModule->target = ABCKIT_TARGET_ARK_TS_V1;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    descriptor->core->importingModule->target = ABCKIT_TARGET_ARK_TS_V2;
    apiToCheck(arktsModule.get(), descriptor.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, AbckitInst *, bool))
{
    apiToCheck(nullptr, g_abckitInst, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, uint32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitBasicBlock *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, uint32_t))
{
    apiToCheck(nullptr, g_abckitBasicblock, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitGraph *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *))
{
    apiToCheck(nullptr, g_abckitBasicblock, g_abckitBasicblock, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, nullptr, g_abckitBasicblock, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_abckitBasicblock, nullptr, g_abckitBasicblock);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitBasicblock, g_abckitBasicblock, g_abckitBasicblock, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaType (*apiToCheck)(AbckitGraph *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitGraph *, void *, bool (*cb)(AbckitBasicBlock *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitBasicBlock *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitGraph, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, AbckitInst *))
{
    apiToCheck(nullptr, g_abckitInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, int32_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(int64_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(int32_t (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *, size_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, uint64_t))
{
    apiToCheck(nullptr, 0, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitBitImmSize (*apiToCheck)(AbckitInst *, size_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *, uint32_t))
{
    apiToCheck(nullptr, g_abckitInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, ...))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitLiteralArray *))
{
    apiToCheck(nullptr, g_abckitLiteralarray);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreFunction *))
{
    apiToCheck(nullptr, g_abckitFunction);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitString *))
{
    apiToCheck(nullptr, g_abckitString);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, size_t, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitInst *, size_t, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitInst *, void *) { return true; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationElement *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotation *, void *, bool (*cb)(AbckitCoreAnnotationElement *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationElement *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitAnnotation, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto anno = std::make_unique<AbckitCoreAnnotation>();
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    anno->ai = ai.get();
    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ai->owningModule = module.get();
    module->file = nullptr;
    ASSERT_FALSE(apiToCheck(anno.get(), nullptr, [](AbckitCoreAnnotationElement *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);

    auto file = std::make_unique<AbckitFile>();
    module->file = file.get();
    auto element = std::make_unique<AbckitCoreAnnotationElement>();
    anno->elements.emplace_back(std::move(element));
    ASSERT_FALSE(apiToCheck(anno.get(), nullptr, [](AbckitCoreAnnotationElement *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_JS;
    ASSERT_FALSE(apiToCheck(anno.get(), nullptr, [](AbckitCoreAnnotationElement *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotation *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotation *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotationInterface *, void *,
                                    bool (*cb)(AbckitCoreAnnotationInterfaceField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationInterfaceField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitAnnotationinterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    auto module = std::make_unique<AbckitCoreModule>();
    ai->owningModule = module.get();
    module->target = ABCKIT_TARGET_ARK_TS_V2;
    auto field = std::make_unique<AbckitCoreAnnotationInterfaceField>();
    ai->fields.emplace_back(std::move(field));
    ASSERT_FALSE(apiToCheck(ai.get(), nullptr, [](AbckitCoreAnnotationInterfaceField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_JS;
    ASSERT_FALSE(apiToCheck(ai.get(), nullptr, [](AbckitCoreAnnotationInterfaceField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto file = std::make_unique<AbckitFile>();
    file->frontend = Mode::STATIC;
    auto deleterFunc = [](libabckit::pandasm_Value *ptr) {};
    AbckitValueImplT valImpl(reinterpret_cast<libabckit::pandasm_Value *>(0x1), deleterFunc);
    auto value = std::make_unique<AbckitValue>(file.get(), std::move(valImpl));
    apiToCheck(value.get());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreClassField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClassField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    AbckitArktsClass arktsClass(dummyRecord);
    auto coreClass = std::make_unique<AbckitCoreClass>(coreModule.get(), arktsClass);
    coreClass->owningModule = coreModule.get();
    coreClass->fields.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreClass.get(), g_void, [](AbckitCoreClassField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    AbckitArktsClass arktsClass(dummyRecord);
    auto coreClass = std::make_unique<AbckitCoreClass>(coreModule.get(), arktsClass);
    coreClass->owningModule = coreModule.get();
    coreClass->subClasses.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreClass.get(), g_void, [](AbckitCoreClass *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    AbckitArktsClass arktsClass(dummyRecord);
    auto coreClass = std::make_unique<AbckitCoreClass>(coreModule.get(), arktsClass);
    coreClass->owningModule = coreModule.get();
    coreClass->interfaces.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreClass.get(), g_void, [](AbckitCoreInterface *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitClass, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreModule *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreModule *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFileVersion (*apiToCheck)(AbckitFile *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitLiteralArray *, void *, bool (*cb)(AbckitFile *, AbckitLiteral *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitFile *, AbckitLiteral *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitLiteralarray, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto litArr = std::make_unique<AbckitLiteralArray>();
    auto file = std::make_unique<AbckitFile>();
    file->frontend = Mode::STATIC;
    litArr->file = file.get();
    apiToCheck(litArr.get(), g_void, [](AbckitFile *, AbckitLiteral *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(float (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralTag (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint16_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint32_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint64_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(uint8_t (*apiToCheck)(AbckitLiteral *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto func = std::make_unique<AbckitCoreFunction>();
    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_JS;
    func->owningModule = module.get();
    bool res = apiToCheck(func.get(), g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_FALSE(res);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreFunctionParam *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunctionParam *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_FALSE(apiToCheck(nullptr, nullptr, nullptr));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto func = std::make_unique<AbckitCoreFunction>();
    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_ARK_TS_V2;
    func->owningModule = module.get();
    auto param = std::make_unique<AbckitCoreFunctionParam>();
    func->parameters.emplace_back(std::move(param));
    ASSERT_FALSE(apiToCheck(func.get(), nullptr, [](AbckitCoreFunctionParam *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto coreFunc = std::make_unique<AbckitCoreFunction>();
    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_JS;
    coreFunc->owningModule = coreModule.get();
    ASSERT_FALSE(apiToCheck(coreFunc.get(), g_void, [](AbckitCoreFunctionParam *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto func = std::make_unique<AbckitCoreFunction>();
    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_JS;
    func->owningModule = module.get();
    ASSERT_EQ(apiToCheck(func.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreFunctionParam *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreAnnotationInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotationInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreModuleField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreModuleField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    coreModule->fields.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreModule.get(), g_void, [](AbckitCoreModuleField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    coreModule->it.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(coreModule.get(), g_void, [](AbckitCoreInterface *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreEnum *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreEnum *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    coreModule->et.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(coreModule.get(), g_void, [](AbckitCoreEnum *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreExportDescriptor *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreExportDescriptor *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreImportDescriptor *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreImportDescriptor *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreNamespace *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreNamespace *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitModule, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *,
                                    const struct AbckitArktsAnnotationInterfaceCreateParams *))
{
    apiToCheck(g_abckitArktsModuleField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_constAbckitArktsAnnotationinterfacecreateparams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, const char *))
{
    apiToCheck(g_abckitArktsModuleField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, GetConstChar());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, AbckitType *))
{
    apiToCheck(g_abckitArktsModuleField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitType);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, AbckitValue *))
{
    apiToCheck(g_abckitArktsModuleField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitValue);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, const struct AbckitArktsAnnotationCreateParams *))
{
    apiToCheck(g_arktsClassField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitArktsAnnotation *))
{
    apiToCheck(g_arktsClassField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, const char *))
{
    apiToCheck(g_arktsClassField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, GetConstChar());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitType *))
{
    apiToCheck(g_arktsClassField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitType);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitValue *))
{
    apiToCheck(g_arktsClassField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitValue);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, const struct AbckitArktsAnnotationCreateParams *))
{
    apiToCheck(g_abckitArktsInterfaceField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_constAbckitArktsAnnotationcreateparams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, AbckitArktsAnnotation *))
{
    apiToCheck(g_abckitArktsInterfaceField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitArktsAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, const char *))
{
    apiToCheck(g_abckitArktsInterfaceField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, GetConstChar());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, AbckitType *))
{
    apiToCheck(g_abckitArktsInterfaceField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_abckitType);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsEnumField *, const char *))
{
    apiToCheck(g_abckitArktsEnumField, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, GetConstChar());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTarget (*apiToCheck)(AbckitCoreModule *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto arktsInterface = AbckitArktsInterface(dummyRecord);
    auto coreInterface = std::make_unique<AbckitCoreInterface>(coreModule.get(), arktsInterface);
    coreInterface->owningModule = coreModule.get();
    coreInterface->methods.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreInterface.get(), g_void, [](AbckitCoreFunction *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitInterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreInterfaceField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreInterfaceField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto arktsInterface = AbckitArktsInterface(dummyRecord);
    auto coreInterface = std::make_unique<AbckitCoreInterface>(coreModule.get(), arktsInterface);
    coreInterface->owningModule = coreModule.get();
    coreInterface->fields.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(coreInterface.get(), g_void, [](AbckitCoreInterfaceField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitInterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto arktsInterface = AbckitArktsInterface(dummyRecord);
    auto coreInterface = std::make_unique<AbckitCoreInterface>(coreModule.get(), arktsInterface);
    coreInterface->owningModule = coreModule.get();
    coreInterface->superInterfaces.emplace_back(nullptr);
    coreInterface->subInterfaces.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreInterface.get(), g_void, [](AbckitCoreInterface *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitInterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto arktsInterface = AbckitArktsInterface(dummyRecord);
    auto coreInterface = std::make_unique<AbckitCoreInterface>(coreModule.get(), arktsInterface);
    coreInterface->owningModule = coreModule.get();
    coreInterface->classes.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(coreInterface.get(), g_void, [](AbckitCoreClass *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitInterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInterface, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto arktsInterface = AbckitArktsInterface(dummyRecord);
    auto coreInterface = std::make_unique<AbckitCoreInterface>(coreModule.get(), arktsInterface);
    coreInterface->owningModule = coreModule.get();
    coreInterface->annotationTable.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(coreInterface.get(), g_void, [](AbckitCoreAnnotation *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreEnum *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    auto arktsEnum = std::make_unique<AbckitArktsEnum>(nullptr);
    auto enm = std::make_unique<AbckitCoreEnum>(module.get(), *arktsEnum.get());
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_EQ(apiToCheck(enm.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreEnum *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    auto arktsEnum = std::make_unique<AbckitArktsEnum>(nullptr);
    auto enm = std::make_unique<AbckitCoreEnum>(module.get(), *arktsEnum.get());
    module->file = nullptr;
    ASSERT_EQ(apiToCheck(enm.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreEnum *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    auto arktsEnum = std::make_unique<AbckitArktsEnum>(nullptr);
    auto enm = std::make_unique<AbckitCoreEnum>(module.get(), *arktsEnum.get());
    ASSERT_NE(apiToCheck(enm.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitEnum, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto callBack = [](AbckitCoreFunction *, void *) -> bool { return false; };
    auto module = std::make_unique<AbckitCoreModule>();
    auto arktsEnum = std::make_unique<AbckitArktsEnum>(nullptr);
    auto enm = std::make_unique<AbckitCoreEnum>(module.get(), *arktsEnum.get());
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_FALSE(apiToCheck(enm.get(), nullptr, callBack));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *, void *, bool (*cb)(AbckitCoreEnumField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreEnumField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitEnum, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto callBack = [](AbckitCoreEnumField *, void *) -> bool { return false; };
    auto module = std::make_unique<AbckitCoreModule>();
    auto arktsEnum = std::make_unique<AbckitArktsEnum>(nullptr);
    auto enm = std::make_unique<AbckitCoreEnum>(module.get(), *arktsEnum.get());
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_FALSE(apiToCheck(enm.get(), nullptr, callBack));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreClass *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreClass *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreInterface *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreInterface *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    auto namespc = std::make_unique<AbckitCoreNamespace>(coreModule.get());
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    namespc->it.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(namespc.get(), g_void, [](AbckitCoreInterface *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreEnum *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreEnum *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    auto namespc = std::make_unique<AbckitCoreNamespace>(coreModule.get());
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    namespc->et.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(namespc.get(), g_void, [](AbckitCoreEnum *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreNamespaceField *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreNamespaceField *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    auto namespc = std::make_unique<AbckitCoreNamespace>(coreModule.get());
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    namespc->fields.emplace_back(nullptr);
    ASSERT_FALSE(apiToCheck(namespc.get(), g_void, [](AbckitCoreNamespaceField *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreNamespace *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreNamespace *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    auto namespc = std::make_unique<AbckitCoreNamespace>(coreModule.get());
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    namespc->nt.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(namespc.get(), g_void, [](AbckitCoreNamespace *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreFunction *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto func = std::make_unique<AbckitCoreFunction>();
    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    func->owningModule = coreModule.get();
    auto param = std::make_unique<AbckitCoreFunctionParam>();
    func->parameters.emplace_back(std::move(param));
    auto namespc = std::make_unique<AbckitCoreNamespace>(coreModule.get());
    namespc->functions.emplace_back(std::move(func));
    ASSERT_FALSE(apiToCheck(namespc.get(), g_void, [](AbckitCoreFunction *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    apiToCheck(g_abckitNamespace, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    auto coreNamespace = std::make_unique<AbckitCoreNamespace>(module.get());
    module->target = ABCKIT_TARGET_ARK_TS_V2;
    auto function = std::make_unique<AbckitCoreFunction>();
    coreNamespace->functions.emplace_back(std::move(function));
    ASSERT_FALSE(apiToCheck(coreNamespace.get(), nullptr, [](AbckitCoreFunction *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_JS;
    ASSERT_FALSE(apiToCheck(coreNamespace.get(), nullptr, [](AbckitCoreFunction *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreModuleField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    auto field = std::make_unique<AbckitCoreModuleField>(module.get(), nullptr);
    field->owner = nullptr;
    ASSERT_EQ(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreModuleField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreModuleField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreClassField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto field = std::make_unique<AbckitCoreClassField>(nullptr, nullptr);
    ASSERT_EQ(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreClassField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreClassField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClassField *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    panda::pandasm::Function *func = nullptr;
    auto arktsClass = std::make_unique<AbckitArktsClass>(func);
    auto coreClass = std::make_unique<AbckitCoreClass>(nullptr, *arktsClass.get());
    auto field = std::make_unique<AbckitCoreClassField>(coreClass.get(), nullptr);
    auto module = std::make_unique<AbckitCoreModule>();
    ASSERT_NE(field->owner, nullptr);
    field->owner->owningModule = module.get();
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_FALSE(apiToCheck(field.get()));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreClassField *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitClassField, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto coreModule = std::make_unique<AbckitCoreModule>();
    coreModule->target = ABCKIT_TARGET_ARK_TS_V2;
    auto *dummyRecord = reinterpret_cast<ark::pandasm::Record *>(0x1);
    auto *dummyField = reinterpret_cast<ark::pandasm::Field *>(0x2);
    AbckitArktsClass arktsClass(dummyRecord);
    auto coreClass = std::make_unique<AbckitCoreClass>(coreModule.get(), arktsClass);
    coreClass->owningModule = coreModule.get();
    auto coreClassField = std::make_unique<AbckitCoreClassField>(coreClass.get(), dummyField);
    coreClassField->annotationTable.emplace("0", nullptr);
    ASSERT_FALSE(apiToCheck(coreClassField.get(), g_void, [](AbckitCoreAnnotation *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    panda::pandasm::Function *func = nullptr;
    auto abckitArktsClass = std::make_unique<AbckitArktsClass>(func);
    auto abckitCoreClass = std::make_unique<AbckitCoreClass>(nullptr, *abckitArktsClass.get());
    auto field = std::make_unique<AbckitCoreClassField>(abckitCoreClass.get(), nullptr);
    ASSERT_FALSE(apiToCheck(field.get(), nullptr, nullptr));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(field->owner, nullptr);
    field->owner->owningModule = module.get();
    ASSERT_FALSE(apiToCheck(field.get(), nullptr, [](AbckitCoreAnnotation *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(AbckitCoreInterface *(*apiToCheck)(AbckitCoreInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto field = std::make_unique<AbckitCoreInterfaceField>();
    field->owner = nullptr;
    ASSERT_EQ(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreInterfaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterfaceField *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_JS;
    auto interface = std::make_unique<AbckitCoreInterface>(module.get(), AbckitArktsInterface {0});
    auto field = std::make_unique<AbckitCoreInterfaceField>();
    field->owner = interface.get();
    ASSERT_FALSE(apiToCheck(field.get()));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterfaceField *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreAnnotation *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInterfaceField, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto field = std::make_unique<AbckitCoreInterfaceField>();
    ASSERT_FALSE(apiToCheck(field.get(), g_void, nullptr));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto module = std::make_unique<AbckitCoreModule>();
    module->target = ABCKIT_TARGET_JS;
    auto interface = std::make_unique<AbckitCoreInterface>(module.get(), AbckitArktsInterface {0});

    auto fieldWithUnsupportedTarget = std::make_unique<AbckitCoreInterfaceField>();
    fieldWithUnsupportedTarget->owner = interface.get();
    ASSERT_FALSE(
        apiToCheck(fieldWithUnsupportedTarget.get(), g_void, [](AbckitCoreAnnotation *, void *) { return false; }));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(AbckitCoreEnum *(*apiToCheck)(AbckitCoreEnumField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto enumField = std::make_unique<AbckitCoreEnumField>(nullptr, nullptr);
    ASSERT_EQ(apiToCheck(enumField.get()), nullptr);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreEnumField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreEnumField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitType *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(size_t (*apiToCheck)(AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr), 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitType *, void *, bool (*cb)(AbckitType *, void *)))
{
    ASSERT_EQ(apiToCheck(nullptr, g_void, [](AbckitType *, void *) { return false; }), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(double (*apiToCheck)(AbckitValue *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitValue *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitValue *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(const char *(*apiToCheck)(AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitLiteralArray *))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, g_abckitLiteralarray), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, 0, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, AbckitLiteralArray *, uint64_t,
                                           AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, g_abckitLiteralarray, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitLiteralarray, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitFunction, nullptr, 0, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitFunction, g_abckitLiteralarray, 0, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *,
                                           AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitBasicBlock *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitBasicblock, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitLiteralArray *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitLiteralarray), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitInst *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, 0, g_abckitInst, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, 0, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitString, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitString, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitString, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitString, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitModule), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitIsaApiDynamicConditionCode))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitImportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitString), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitExportdescriptor), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiDynamicConditionCode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiDynamicOpcode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiDynamicConditionCode))
{
    apiToCheck(nullptr, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreModule *))
{
    apiToCheck(nullptr, g_abckitModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, size_t, ...))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitFunction, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitType *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitType), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitType), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitIsaApiStaticConditionCode))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE),
              nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *, AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitClass, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitInst, g_abckitInst, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *inputObj, AbckitString *field, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitInst, g_abckitString, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, g_abckitString, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitString, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitInst *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiStaticConditionCode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitIsaApiStaticOpcode (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitInst *))
{
    apiToCheck(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreClass *))
{
    apiToCheck(nullptr, g_abckitClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiStaticConditionCode))
{
    apiToCheck(nullptr, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitTypeId))
{
    apiToCheck(nullptr, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitInst, ABCKIT_TYPE_ID_INVALID);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsClass *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsFunction *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitCoreModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitJsClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitJsFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitJsModule *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsImportFromDynamicModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitJsModule, g_constAbckitJsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, nullptr, g_constAbckitJsImportfromdynamicmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, g_abckitJsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsExportDescriptor *))
{
    apiToCheck(nullptr, g_abckitJsExportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitJsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsImportDescriptor *))
{
    apiToCheck(nullptr, g_abckitJsImportdescriptor);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitJsModule, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitFile *, AbckitLiteral **, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, &g_abckitLiteral, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, AbckitValue **, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, &g_abckitValue, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, float))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, GetConstChar(), 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitFunction), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint16_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint32_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint64_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint8_t))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitClass), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, GetConstChar(), 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitTypeId))
{
    ASSERT_EQ(apiToCheck(nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, double))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, const char *, size_t))
{
    ASSERT_EQ(apiToCheck(nullptr, GetConstChar(), 1), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, GetConstChar(), 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, bool))
{
    ASSERT_EQ(apiToCheck(nullptr, 0), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitFile *, const AbckitArktsV1ExternalModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsV1ExternalModuleCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto file = std::make_unique<AbckitFile>();
    auto params = std::make_unique<AbckitArktsV1ExternalModuleCreateParams>();
    params->name = nullptr;
    ASSERT_EQ(apiToCheck(file.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    char name[] = "nameTest";
    params->name = name;
    file->frontend = Mode::STATIC;
    ASSERT_EQ(apiToCheck(file.get(), params.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
}
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitFile *, const AbckitJsExternalModuleCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitJsExternalmodulecreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitFile, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitCoreFunction *, AbckitGraph *))
{
    apiToCheck(nullptr, g_abckitGraph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsDynamicModuleExportCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, g_abckitJsModule, g_constAbckitJsDynamicmoduleexportcreateparams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitJsModule, g_abckitJsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsNamespace *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitArktsNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    AbckitArktsNamespace arktsNamespace;
    auto module = std::make_unique<AbckitCoreModule>();
    auto core = std::make_unique<AbckitCoreNamespace>(module.get(), std::move(arktsNamespace));
    arktsNamespace.core = core.get();
    core->owningModule = module.get();
    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_EQ(apiToCheck(&arktsNamespace), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitLiteral *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *,
                                    bool (*cb)(AbckitCoreFunction *nestedFunc, void *data)))
{
    apiToCheck(nullptr, g_void, [](AbckitCoreFunction *, void *) { return false; });
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(nullptr, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFunction, g_void, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreFunction *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreClass *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitJsImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitJsExportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitLiteralArray *))
{
    apiToCheck(nullptr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

void TestNullptr(AbckitArktsNamespace *(*apiToCheck)(AbckitCoreNamespace *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    AbckitArktsNamespace arktsNamespace;
    auto module = std::make_unique<AbckitCoreModule>();
    auto coreNamespace = std::make_unique<AbckitCoreNamespace>(module.get(), std::move(arktsNamespace));
    module->target = ABCKIT_TARGET_UNKNOWN;
    ASSERT_EQ(apiToCheck(coreNamespace.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    auto impl = std::make_unique<AbckitArktsNamespace>();
    coreNamespace->impl = std::move(impl);
    module->target = ABCKIT_TARGET_ARK_TS_V1;
    ASSERT_NE(apiToCheck(coreNamespace.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    module->target = ABCKIT_TARGET_ARK_TS_V2;
    ASSERT_NE(apiToCheck(coreNamespace.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(const AbckitArktsModifyApi *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(const AbckitArktsInspectApi *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(const AbckitModifyApi *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreNamespaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    auto field = std::make_unique<AbckitCoreNamespaceField>(nullptr, nullptr);
    ASSERT_EQ(apiToCheck(field.get()), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreNamespaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreNamespaceField *))
{
    ASSERT_EQ(apiToCheck(nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(const AbckitInspectApi *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(const AbckitApi *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(const AbckitIsaApiDynamic *(*apiToCheck)(AbckitApiVersion version))
{
    ASSERT_EQ(apiToCheck(static_cast<AbckitApiVersion>(-1)), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNKNOWN_API_VERSION);
}
void TestNullptr(AbckitArktsClass *(*apiToCheck)(AbckitArktsModule *, const char *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, "test"), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsInterface *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsInterface), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsClass *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_abckitArktsClass), false);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsInterface *(*apiToCheck)(AbckitArktsModule *, const char *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, "test"), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsModuleField *(*apiToCheck)(AbckitArktsModule *,
                                                       const struct AbckitArktsFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsModule, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsFieldCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsClassField *(*apiToCheck)(AbckitArktsClass *, const struct AbckitArktsFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsClass, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsFieldCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsInterfaceField *(*apiToCheck)(AbckitArktsInterface *,
                                                          const struct AbckitArktsInterfaceFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsInterface, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsInterfaceFieldCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitArktsEnumField *(*apiToCheck)(AbckitArktsEnum *, const struct AbckitArktsFieldCreateParams *))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(g_abckitArktsEnum, nullptr), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    ASSERT_EQ(apiToCheck(nullptr, g_constAbckitArktsFieldCreateParams), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitType *, const char *, size_t))
{
    apiToCheck(nullptr, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    apiToCheck(g_abckitType, nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(void (*apiToCheck)(AbckitType *, size_t))
{
    apiToCheck(nullptr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                           AbckitInst *value, AbckitTypeId typeId))
{
    ASSERT_EQ(apiToCheck(nullptr, nullptr, nullptr, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(apiToCheck(g_abckitGraph, nullptr, nullptr, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, nullptr, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(apiToCheck(g_abckitGraph, g_abckitInst, g_abckitString, nullptr, ABCKIT_TYPE_ID_INVALID), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

}  // namespace libabckit::test::helpers_nullptr
