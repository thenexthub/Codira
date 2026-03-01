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
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION (reinterpret_cast<AbckitCoreAnnotation *>(0xcafebabe))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION_INTERFACE (reinterpret_cast<AbckitCoreAnnotationInterface *>(0xcafebabe))

class LibAbcKitModifyApiAnnotationsTests : public ::testing::Test {};

static void ModifyModule(AbckitFile * /*unused*/, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    auto *module = g_implI->functionGetModule(method);
    const char *str = "NewAnnotation";
    struct AbckitArktsAnnotationInterfaceCreateParams annoIfParams {};
    annoIfParams.name = str;
    g_implArkM->moduleAddAnnotationInterface(g_implArkI->coreModuleToArktsModule(module), &annoIfParams);

    g_implI->moduleEnumerateAnnotationInterfaces(module, module, [](AbckitCoreAnnotationInterface *annoI, void *) {
        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "_ESSlotNumberAnnotation" || name == "_ESConcurrentModuleRequestsAnnotation" ||
                    name == "Anno1" || name == "Anno2" || name == "NewAnnotation");

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

struct AnnoIFind {
    std::string name;
    AbckitCoreAnnotationInterface *ai = nullptr;
};

struct AnnoFind {
    std::string name;
    AbckitCoreAnnotation *anno = nullptr;
};

struct AnnoElemFind {
    std::string name;
    AbckitCoreAnnotationElement *annoElem = nullptr;
};

struct AnnoFieldFind {
    std::string name;
    AbckitCoreAnnotationInterfaceField *fld = nullptr;
};

static void ModifyClassAddAnnotation(AbckitFile * /*unused*/, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    auto module = g_implI->functionGetModule(method);
    const char *str = "Anno2";
    AnnoIFind af;
    af.name = str;
    af.ai = nullptr;

    g_implI->moduleEnumerateAnnotationInterfaces(module, &af, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto af1 = reinterpret_cast<AnnoIFind *>(data);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno1" || name == "Anno2");

        if (name == af1->name) {
            af1->ai = annoI;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.ai != nullptr);
    EXPECT_TRUE(af.name == str);

    auto klass = g_implI->functionGetParentClass(method);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai);
    auto anno = g_implArkM->classAddAnnotation(g_implArkI->coreClassToArktsClass(klass), &annoCreateParams);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto annoI = g_implI->annotationGetInterface(g_implArkI->arktsAnnotationToCoreAnnotation(anno));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto str1 = g_implI->annotationInterfaceGetName(annoI);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name1 = helpers::AbckitStringToString(str1);
    EXPECT_TRUE(name1 == str);
}

static void ModifyClassRemoveAnnotation(AbckitFile * /*unused*/, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    const char *str = "Anno1";
    AnnoFind af;
    af.name = str;
    af.anno = nullptr;

    auto klass = g_implI->functionGetParentClass(method);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    g_implI->classEnumerateAnnotations(klass, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto ai = g_implI->annotationGetInterface(anno);

        auto str = g_implI->annotationInterfaceGetName(ai);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);
        if (name == af1->name) {
            af1->anno = anno;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.anno != nullptr);
    EXPECT_TRUE(af.name == str);

    g_implArkM->classRemoveAnnotation(g_implArkI->coreClassToArktsClass(klass),
                                      g_implArkI->coreAnnotationToArktsAnnotation(af.anno));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    g_implI->classEnumerateAnnotations(klass, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto ai = g_implI->annotationGetInterface(anno);

        auto str = g_implI->annotationInterfaceGetName(ai);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name != af1->name);

        return true;
    });
}

static void ModifyFunctionAddAnnotation(AbckitFile * /*unused*/, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    auto module = g_implI->functionGetModule(method);
    const char *str = "Anno2";
    AnnoIFind af;
    af.name = str;
    af.ai = nullptr;

    g_implI->moduleEnumerateAnnotationInterfaces(module, &af, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto af1 = reinterpret_cast<AnnoIFind *>(data);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno1" || name == "Anno2");

        if (name == af1->name) {
            af1->ai = annoI;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.ai != nullptr);
    EXPECT_TRUE(af.name == str);

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai);
    auto anno = g_implArkM->functionAddAnnotation(g_implArkI->coreFunctionToArktsFunction(method), &annoCreateParams);
    EXPECT_TRUE(anno != nullptr);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto annoI = g_implI->annotationGetInterface(g_implArkI->arktsAnnotationToCoreAnnotation(anno));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto str1 = g_implI->annotationInterfaceGetName(annoI);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name1 = helpers::AbckitStringToString(str1);
    EXPECT_TRUE(name1 == str);
}

static void ModifyFunctionRemoveAnnotation(AbckitFile * /*unused*/, AbckitCoreFunction *method,
                                           AbckitGraph * /*unused*/)
{
    const char *str = "Anno1";
    AnnoFind af;
    af.name = str;
    af.anno = nullptr;

    g_implI->functionEnumerateAnnotations(method, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto ai = g_implI->annotationGetInterface(anno);

        auto str = g_implI->annotationInterfaceGetName(ai);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);
        if (name == af1->name) {
            af1->anno = anno;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.anno != nullptr);
    EXPECT_TRUE(af.name == str);

    g_implArkM->functionRemoveAnnotation(g_implArkI->coreFunctionToArktsFunction(method),
                                         g_implArkI->coreAnnotationToArktsAnnotation(af.anno));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    g_implI->functionEnumerateAnnotations(method, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto ai = g_implI->annotationGetInterface(anno);

        auto str = g_implI->annotationInterfaceGetName(ai);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name != af1->name);

        return true;
    });
}

static void ModifyAnnotationAddAnnotationElement(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    const char *str = "Anno1";
    AnnoFind af;
    af.name = str;
    af.anno = nullptr;

    g_implI->functionEnumerateAnnotations(method, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto annoI = g_implI->annotationGetInterface(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->annotationInterfaceGetName(annoI);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name == "Anno1");

        if (name == af1->name) {
            af1->anno = anno;
        }

        return true;
    });

    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.anno != nullptr);
    EXPECT_TRUE(af.name == str);

    struct AbckitArktsAnnotationElementCreateParams annoElementCreateParams {};
    const double initVal = 0.12;
    auto value = g_implM->createValueDouble(file, initVal);
    annoElementCreateParams.value = value;
    annoElementCreateParams.name = "newValue";

    auto annoElem = g_implArkM->annotationAddAnnotationElement(g_implArkI->coreAnnotationToArktsAnnotation(af.anno),
                                                               &annoElementCreateParams);
    EXPECT_TRUE(annoElem != nullptr);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto str1 = g_implI->annotationElementGetName(g_implArkI->arktsAnnotationElementToCoreAnnotationElement(annoElem));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name1 = helpers::AbckitStringToString(str1);

    EXPECT_TRUE(name1 == annoElementCreateParams.name);

    double val = g_implI->valueGetDouble(value);
    EXPECT_TRUE(val == 12e-2);
}

static void ModifyAnnotationRemoveAnnotationElement(AbckitFile * /*unused*/, AbckitCoreFunction *method,
                                                    AbckitGraph * /*unused*/)
{
    const char *str = "Anno1";
    AnnoFind af;
    af.name = str;
    af.anno = nullptr;

    g_implI->functionEnumerateAnnotations(method, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto annoI = g_implI->annotationGetInterface(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->annotationInterfaceGetName(annoI);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name == "Anno1");

        if (name == af1->name) {
            af1->anno = anno;
        }

        return true;
    });

    AnnoElemFind aef;
    aef.name = "a";
    g_implI->annotationEnumerateElements(af.anno, &aef, [](AbckitCoreAnnotationElement *annoElem, void *data) {
        auto aef1 = reinterpret_cast<AnnoElemFind *>(data);

        auto str = g_implI->annotationElementGetName(annoElem);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name == "a" || name == "b");

        if (name == aef1->name) {
            aef1->annoElem = annoElem;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    EXPECT_TRUE(af.anno != nullptr);
    EXPECT_TRUE(af.name == str);

    g_implArkM->annotationRemoveAnnotationElement(
        g_implArkI->coreAnnotationToArktsAnnotation(af.anno),
        g_implArkI->coreAnnotationElementToArktsAnnotationElement(aef.annoElem));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    g_implI->annotationEnumerateElements(af.anno, &aef, [](AbckitCoreAnnotationElement *annoElem, void *data) {
        auto aef1 = reinterpret_cast<AnnoElemFind *>(data);

        auto str = g_implI->annotationElementGetName(annoElem);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name != aef1->name);

        return true;
    });
}

static void TestAnnotationRemoveAnnotationElement(AbckitFile * /*unused*/, AbckitCoreFunction *method,
                                                  AbckitGraph * /*unused*/)
{
    const char *str = "Anno1";
    AnnoFind af;
    af.name = str;
    af.anno = nullptr;

    g_implI->functionEnumerateAnnotations(method, &af, [](AbckitCoreAnnotation *anno, void *data) {
        auto af1 = reinterpret_cast<AnnoFind *>(data);

        auto annoI = g_implI->annotationGetInterface(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->annotationInterfaceGetName(annoI);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name == "Anno1");

        if (name == af1->name) {
            af1->anno = anno;
        }

        return true;
    });

    AnnoElemFind aef;
    aef.name = "a";
    g_implI->annotationEnumerateElements(af.anno, &aef, [](AbckitCoreAnnotationElement *annoElem, void *data) {
        auto aef1 = reinterpret_cast<AnnoElemFind *>(data);

        auto str = g_implI->annotationElementGetName(annoElem);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        EXPECT_TRUE(name == "a" || name == "b");

        if (name == aef1->name) {
            aef1->annoElem = annoElem;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    EXPECT_TRUE(af.anno != nullptr);
    EXPECT_TRUE(af.name == str);

    auto annoElem = g_implArkI->coreAnnotationElementToArktsAnnotationElement(aef.annoElem);
    annoElem->core->ann = DEFAULT_ANNOTATION;

    g_implArkM->annotationRemoveAnnotationElement(g_implArkI->coreAnnotationToArktsAnnotation(af.anno), annoElem);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_BAD_ARGUMENT);
}

static void ModifyAnnotationInterfaceAddField(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph * /*unused*/)
{
    auto module = g_implI->functionGetModule(method);
    const char *str = "Anno2";
    AnnoIFind af;
    af.name = str;
    af.ai = nullptr;

    g_implI->moduleEnumerateAnnotationInterfaces(module, &af, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto af1 = reinterpret_cast<AnnoIFind *>(data);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno1" || name == "Anno2");

        if (name == af1->name) {
            af1->ai = annoI;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.ai != nullptr);
    EXPECT_TRUE(af.name == str);

    struct AbckitArktsAnnotationInterfaceFieldCreateParams annoFieldCreateParams {};
    AbckitType *type = g_implM->createType(file, ABCKIT_TYPE_ID_U1);
    AbckitValue *value = g_implM->createValueU1(file, false);
    annoFieldCreateParams.name = "newField";
    annoFieldCreateParams.type = type;
    annoFieldCreateParams.defaultValue = value;
    auto field = g_implArkM->annotationInterfaceAddField(
        g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai), &annoFieldCreateParams);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto annoI1 = g_implI->annotationInterfaceFieldGetInterface(
        g_implArkI->arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(field));
    EXPECT_TRUE(annoI1 == af.ai);

    auto str1 = g_implI->annotationInterfaceFieldGetName(
        g_implArkI->arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(field));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name1 = helpers::AbckitStringToString(str1);
    EXPECT_TRUE(name1 == "newField");

    auto val1 = g_implI->annotationInterfaceFieldGetDefaultValue(
        g_implArkI->arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(field));
    bool value1 = g_implI->valueGetU1(val1);
    EXPECT_TRUE(value1 == false);

    g_implI->annotationInterfaceEnumerateFields(af.ai, nullptr, [](AbckitCoreAnnotationInterfaceField *fld, void *) {
        auto str = g_implI->annotationInterfaceFieldGetName(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "newField");
        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static void ModifyAnnotationInterfaceRemoveField(AbckitFile * /*unused*/, AbckitCoreFunction *method,
                                                 AbckitGraph * /*unused*/)
{
    auto module = g_implI->functionGetModule(method);
    const char *str = "Anno2";
    AnnoIFind af;
    af.name = str;
    af.ai = nullptr;

    g_implI->moduleEnumerateAnnotationInterfaces(module, &af, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto af1 = reinterpret_cast<AnnoIFind *>(data);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno1" || name == "Anno2");

        if (name == af1->name) {
            af1->ai = annoI;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.ai != nullptr);
    EXPECT_TRUE(af.name == str);

    AnnoFieldFind aff;
    aff.name = "d";
    aff.fld = nullptr;

    g_implI->annotationInterfaceEnumerateFields(af.ai, &aff, [](AbckitCoreAnnotationInterfaceField *fld, void *data) {
        auto aff1 = reinterpret_cast<AnnoFieldFind *>(data);

        auto str = g_implI->annotationInterfaceFieldGetName(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        if (name == aff1->name) {
            aff1->fld = fld;
        }
        return true;
    });

    EXPECT_TRUE(aff.name == "d");
    EXPECT_TRUE(aff.fld != nullptr);

    g_implArkM->annotationInterfaceRemoveField(
        g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai),
        g_implArkI->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(aff.fld));

    g_implI->annotationInterfaceEnumerateFields(af.ai, &aff, [](AbckitCoreAnnotationInterfaceField *fld, void *) {
        auto str = g_implI->annotationInterfaceFieldGetName(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "a" || name == "b");
        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static void TestAnnotationInterfaceRemoveField(AbckitFile * /*unused*/, AbckitCoreFunction *method,
                                               AbckitGraph * /*unused*/)
{
    auto module = g_implI->functionGetModule(method);
    const char *str = "Anno2";
    AnnoIFind af;
    af.name = str;
    af.ai = nullptr;

    g_implI->moduleEnumerateAnnotationInterfaces(module, &af, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto af1 = reinterpret_cast<AnnoIFind *>(data);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno1" || name == "Anno2");

        if (name == af1->name) {
            af1->ai = annoI;
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(af.ai != nullptr);
    EXPECT_TRUE(af.name == str);

    AnnoFieldFind aff;
    aff.name = "d";
    aff.fld = nullptr;

    g_implI->annotationInterfaceEnumerateFields(af.ai, &aff, [](AbckitCoreAnnotationInterfaceField *fld, void *data) {
        auto aff1 = reinterpret_cast<AnnoFieldFind *>(data);

        auto str = g_implI->annotationInterfaceFieldGetName(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        if (name == aff1->name) {
            aff1->fld = fld;
        }
        return true;
    });

    EXPECT_TRUE(aff.fld != nullptr);

    aff.fld->name = nullptr;
    g_implArkM->annotationInterfaceRemoveField(
        g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai),
        g_implArkI->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(aff.fld));
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_INTERNAL_ERROR);

    auto annoField = g_implArkI->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(aff.fld);
    annoField->core->ai = DEFAULT_ANNOTATION_INTERFACE;
    g_implArkM->annotationInterfaceRemoveField(g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(af.ai),
                                               annoField);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_BAD_ARGUMENT);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddAnnotationInterface, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ModuleAddAnnotationInterface)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "foo", ModifyModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classAddAnnotation, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ClassAddAnnotation)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "foo", ModifyClassAddAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classAddAnnotation, abc-kind=ArkTS1, category=negative, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ClassAddAnnotation_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    helpers::ModuleByNameContext mdlFinder = {nullptr, "annotations_dynamic"};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder.module, nullptr);
    auto *module = mdlFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto klass = clsFinder.klass;

    helpers::ModuleByNameContext mdlFromFinder = {nullptr, "annotations_imports"};
    g_implI->fileEnumerateModules(file, &mdlFromFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFromFinder.module, nullptr);
    auto *moduleFrom = mdlFromFinder.module;

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, "Anno3"};
    g_implI->moduleEnumerateAnnotationInterfaces(moduleFrom, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(aiFinder.ai, nullptr);
    auto ai = aiFinder.ai;

    EXPECT_TRUE(ai->owningModule != klass->owningModule);

    ai->owningModule->target = ABCKIT_TARGET_ARK_TS_V2;

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(ai);
    g_implArkM->classAddAnnotation(g_implArkI->coreClassToArktsClass(klass), &annoCreateParams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveAnnotation, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ClassRemoveAnnotation)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "foo", ModifyClassRemoveAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::classRemoveAnnotation, abc-kind=ArkTS1, category=negative, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ClassRemoveAnnotation_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    helpers::ModuleByNameContext mdlFinder = {nullptr, "annotations_dynamic"};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder.module, nullptr);
    auto *module = mdlFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto klass = clsFinder.klass;

    helpers::AnnotationByNameContext annoFinder = {nullptr, "Anno1"};
    g_implI->classEnumerateAnnotations(klass, &annoFinder, helpers::AnnotationByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(annoFinder.anno, nullptr);
    auto anno = annoFinder.anno;

    EXPECT_TRUE(anno->ai->owningModule == klass->owningModule);

    helpers::ModuleByNameContext mdlFinder1 = {nullptr, "annotations_imports"};
    g_implI->fileEnumerateModules(file, &mdlFinder1, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder1.module, nullptr);

    klass->owningModule = mdlFinder1.module;
    anno->ai->owningModule->target = ABCKIT_TARGET_ARK_TS_V2;

    g_implArkM->classRemoveAnnotation(g_implArkI->coreClassToArktsClass(klass),
                                      g_implArkI->coreAnnotationToArktsAnnotation(anno));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionAddAnnotation, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, ClassRemoveAnnotation_WrongName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    helpers::ModuleByNameContext mdFinder = {nullptr, "annotations_dynamic"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::ClassByNameContext clsFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &clsFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(clsFinder.klass, nullptr);
    auto *klass = clsFinder.klass;

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, "Anno1"};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(aiFinder.ai, nullptr);
    auto *ai = aiFinder.ai;

    auto *newAnno = new AbckitCoreAnnotation();
    newAnno->ai = ai;
    newAnno->owner = klass;
    newAnno->name = g_implM->createString(file, "wrongName", sizeof("wrongName"));
    auto *newArktsAnno = new AbckitArktsAnnotation {newAnno};

    g_implArkM->classRemoveAnnotation(g_implArkI->coreClassToArktsClass(klass), newArktsAnno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    delete newArktsAnno;
    delete newAnno;

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionAddAnnotation, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, FunctionAddAnnotation)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "foo", ModifyFunctionAddAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionAddAnnotation, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitModifyApiAnnotationsTests, FunctionAddAnnotation_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    helpers::ModuleByNameContext mdlFinder = {nullptr, "annotations_imports"};
    g_implI->fileEnumerateModules(file, &mdlFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder.module, nullptr);
    auto *module = mdlFinder.module;

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, "Anno3"};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(aiFinder.ai, nullptr);
    auto ai = aiFinder.ai;

    auto method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);

    EXPECT_TRUE(ai->owningModule != method->owningModule);
    ai->owningModule->target = ABCKIT_TARGET_ARK_TS_V2;

    struct AbckitArktsAnnotationCreateParams annoCreateParams {};
    annoCreateParams.ai = g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(ai);
    g_implArkM->functionAddAnnotation(g_implArkI->coreFunctionToArktsFunction(method), &annoCreateParams);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionRemoveAnnotation, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, FunctionRemoveAnnotation)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "foo", ModifyFunctionRemoveAnnotation);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionRemoveAnnotation, abc-kind=ArkTS1, category=negative,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, FunctionRemoveAnnotation_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    auto method = helpers::FindMethodByName(file, "bar");
    ASSERT_NE(method, nullptr);

    helpers::AnnotationByNameContext annoFinder = {nullptr, "Anno1"};
    g_implI->functionEnumerateAnnotations(method, &annoFinder, helpers::AnnotationByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(annoFinder.anno, nullptr);
    auto anno = annoFinder.anno;

    EXPECT_TRUE(anno->ai->owningModule == method->owningModule);

    helpers::ModuleByNameContext mdlFinder1 = {nullptr, "annotations_imports"};
    g_implI->fileEnumerateModules(file, &mdlFinder1, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdlFinder1.module, nullptr);

    method->owningModule = mdlFinder1.module;
    anno->ai->owningModule->target = ABCKIT_TARGET_ARK_TS_V2;

    g_implArkM->functionRemoveAnnotation(g_implArkI->coreFunctionToArktsFunction(method),
                                         g_implArkI->coreAnnotationToArktsAnnotation(anno));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::functionRemoveAnnotation, abc-kind=ArkTS1, category=negative,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, FunctionRemoveAnnotation_WrongName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    auto *method = helpers::FindMethodByName(file, "bar");
    ASSERT_NE(method, nullptr);

    helpers::ModuleByNameContext mdFinder = {nullptr, "annotations_dynamic"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, "Anno2"};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(aiFinder.ai, nullptr);
    auto *ai = aiFinder.ai;

    auto *newAnno = new AbckitCoreAnnotation();
    newAnno->ai = ai;
    newAnno->owner = method;
    newAnno->name = g_implM->createString(file, "wrongName", sizeof("wrongName"));
    auto *newArktsAnno = new AbckitArktsAnnotation {newAnno};

    g_implArkM->functionRemoveAnnotation(g_implArkI->coreFunctionToArktsFunction(method), newArktsAnno);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    delete newArktsAnno;
    delete newAnno;

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationAddAnnotationElement, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationAddAnnotationElement)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", ModifyAnnotationAddAnnotationElement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationRemoveAnnotationElement, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationRemoveAnnotationElement)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", ModifyAnnotationRemoveAnnotationElement);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationRemoveAnnotationElement, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationRemoveAnnotationElement_2)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", TestAnnotationRemoveAnnotationElement);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationRemoveAnnotationElement, abc-kind=ArkTS1, category=negative,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationRemoveAnnotationElement_WrongName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    auto *method = helpers::FindMethodByName(file, "bar");
    ASSERT_NE(method, nullptr);

    helpers::AnnotationByNameContext annoFinder = {nullptr, "Anno1"};
    g_implI->functionEnumerateAnnotations(method, &annoFinder, helpers::AnnotationByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(annoFinder.anno, nullptr);
    auto *anno = annoFinder.anno;

    auto *newAnnoElem = new AbckitCoreAnnotationElement();
    newAnnoElem->ann = anno;
    newAnnoElem->name = g_implM->createString(file, "wrongName", sizeof("wrongName"));
    auto *newArktsAnnoElem = new AbckitArktsAnnotationElement {newAnnoElem};

    g_implArkM->annotationRemoveAnnotationElement(g_implArkI->coreAnnotationToArktsAnnotation(anno), newArktsAnnoElem);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    delete newArktsAnnoElem;
    delete newAnnoElem;

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceAddField, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationInterfaceAddField)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", ModifyAnnotationInterfaceAddField);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceRemoveField, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationInterfaceRemoveField)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", ModifyAnnotationInterfaceRemoveField);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceRemoveField, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationInterfaceRemoveField_2)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc",
                             ABCKIT_ABC_DIR
                             "ut/extensions/arkts/modify_api/annotations/annotations_dynamic_modified.abc",
                             "bar", TestAnnotationInterfaceRemoveField);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceRemoveField, abc-kind=ArkTS1, category=negative,
// extension=c
TEST_F(LibAbcKitModifyApiAnnotationsTests, AnnotationInterfaceRemoveField_WrongName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_dynamic.abc", &file);

    helpers::ModuleByNameContext mdFinder = {nullptr, "annotations_dynamic"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::AnnotationInterfaceByNameContext aiFinder = {nullptr, "Anno2"};
    g_implI->moduleEnumerateAnnotationInterfaces(module, &aiFinder, helpers::AnnotationInterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(aiFinder.ai, nullptr);
    auto *ai = aiFinder.ai;

    auto *newAIF = new AbckitCoreAnnotationInterfaceField();
    newAIF->ai = ai;
    newAIF->name = g_implM->createString(file, "wrongName", sizeof("wrongName"));
    auto *newArktsAIF = new AbckitArktsAnnotationInterfaceField {newAIF};

    g_implArkM->annotationInterfaceRemoveField(g_implArkI->coreAnnotationInterfaceToArktsAnnotationInterface(ai),
                                               newArktsAIF);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    delete newArktsAIF;
    delete newAIF;

    g_impl->closeFile(file);
}
}  // namespace libabckit::test
