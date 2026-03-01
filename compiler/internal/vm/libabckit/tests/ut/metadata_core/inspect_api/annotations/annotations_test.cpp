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
#include <string_view>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiAnnotationsTests : public ::testing::Test {};

struct AnnoInfo {
    AbckitFile *file = nullptr;
    AbckitCoreAnnotation *anno = nullptr;
    std::string a;
    std::string b;
    std::string d;
};

struct AnnoInterfaceInfo {
    AbckitFile *file = nullptr;
    AbckitCoreAnnotationInterface *annoIf = nullptr;
    AbckitCoreAnnotation *anno = nullptr;
};

template <bool IS_CHECK_VALUE>
static void ProcessAnnotationsInner(std::string_view name, AbckitCoreAnnotationElement *annoElement, AnnoInfo *newData2)
{
    if (name == "a") {
        newData2->a = name;
        if constexpr (IS_CHECK_VALUE) {
            AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto vs = g_implI->valueGetDouble(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(vs == 10U);
        }
    }

    if (name == "b") {
        newData2->b = name;
        if constexpr (IS_CHECK_VALUE) {
            AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto vs = g_implI->arrayValueGetLiteralArray(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            g_implI->literalArrayEnumerateElements(vs, nullptr, [](AbckitFile *, AbckitLiteral *v, void *) {
                double value = g_implI->literalGetDouble(v);
                EXPECT_TRUE(value == 1 || value == 2U || value == 3U);
                return true;
            });
        }
    }

    if (name == "d") {
        newData2->d = name;
        if constexpr (IS_CHECK_VALUE) {
            AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto vs = g_implI->valueGetU1(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(vs == true);
        }
    }

    if (name == "str") {
        if constexpr (IS_CHECK_VALUE) {
            AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto str = g_implI->valueGetString(val);
            auto vs = helpers::AbckitStringToString(str);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(vs == "DEF");
        }
    }
}

template <bool IS_CHECK_VALUE, bool IS_CHECK_ANNOTATION, bool IS_CHECK_CONTEXT>
static void ProcessAnnotations(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AnnoInfo newData;
    g_implI->functionEnumerateAnnotations(method, &newData, [](AbckitCoreAnnotation *anno, void *data1) {
        auto *newData1 = reinterpret_cast<AnnoInfo *>(data1);
        newData1->anno = anno;
        newData1->file = g_implI->annotationGetFile(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->annotationEnumerateElements(anno, data1, [](AbckitCoreAnnotationElement *annoElement, void *data2) {
            auto abckitstr = g_implI->annotationElementGetName(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto name = helpers::AbckitStringToString(abckitstr);

            auto *newData2 = reinterpret_cast<AnnoInfo *>(data2);

            if constexpr (IS_CHECK_ANNOTATION) {
                [[maybe_unused]] auto gotAnno = g_implI->annotationElementGetAnnotation(annoElement);
                EXPECT_TRUE(newData2->anno == gotAnno);
            }

            if constexpr (IS_CHECK_CONTEXT) {
                [[maybe_unused]] auto gotCtx = g_implI->annotationElementGetFile(annoElement);
                EXPECT_TRUE(newData2->file == gotCtx);
            }

            EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "str");

            ProcessAnnotationsInner<IS_CHECK_VALUE>(name, annoElement, newData2);

            return true;
        });
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static auto g_processClassEnumCb = [](AbckitCoreAnnotationElement *annoElement, void *data2) {
    auto abckitstr = g_implI->annotationElementGetName(annoElement);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(abckitstr);

    auto *newData2 = reinterpret_cast<AnnoInfo *>(data2);

    EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "str");
    if (name == "a") {
        newData2->a = name;
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetDouble(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == 20U);
    }

    if (name == "b") {
        newData2->b = name;
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->arrayValueGetLiteralArray(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->literalArrayEnumerateElements(vs, nullptr, [](AbckitFile *, AbckitLiteral *v, void *) {
            [[maybe_unused]] double value = g_implI->literalGetDouble(v);
            EXPECT_TRUE(value == 13U || value == 10U);
            return true;
        });
    }

    if (name == "d") {
        newData2->d = name;
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetU1(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == true);
    }

    if (name == "str") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->valueGetString(val);
        auto vs = helpers::AbckitStringToString(str);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == "ABC");
    }

    return true;
};

static void ProcessClass(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AnnoInfo newData;
    auto klass = g_implI->functionGetParentClass(method);
    g_implI->classEnumerateAnnotations(klass, &newData, [](AbckitCoreAnnotation *anno, void *data1) {
        auto *newData1 = reinterpret_cast<AnnoInfo *>(data1);
        newData1->anno = anno;
        newData1->file = g_implI->annotationGetFile(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->annotationEnumerateElements(anno, data1, g_processClassEnumCb);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

template <bool IS_CHECK_NAME, bool IS_CHECK_CONTEXT>
static void ProcessAnnotationInterface(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AnnoInterfaceInfo newData;
    g_implI->functionEnumerateAnnotations(method, &newData, [](AbckitCoreAnnotation *anno, void *data1) {
        auto *newData1 = reinterpret_cast<AnnoInterfaceInfo *>(data1);
        newData1->anno = anno;
        newData1->annoIf = g_implI->annotationGetInterface(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(newData1->annoIf != nullptr);
        auto file = g_implI->annotationGetFile(anno);
        if constexpr (IS_CHECK_CONTEXT) {
            EXPECT_TRUE(file == g_implI->annotationInterfaceGetFile(newData1->annoIf));
        }
        if constexpr (IS_CHECK_NAME) {
            auto str = g_implI->annotationInterfaceGetName(newData1->annoIf);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            auto name = helpers::AbckitStringToString(str);
            EXPECT_TRUE(name == "Anno");
        }

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

template <bool IS_CHECK_MODULE>
static void ProcessModule(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    auto module = g_implI->functionGetModule(method);
    g_implI->moduleEnumerateAnnotationInterfaces(module, module, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->annotationInterfaceGetName(annoI);

        if constexpr (IS_CHECK_MODULE) {
            auto curModule = g_implI->annotationInterfaceGetModule(annoI);
            EXPECT_TRUE(curModule == data);
            return true;
        }

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto name = helpers::AbckitStringToString(str);

        EXPECT_TRUE(name == "Anno");

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

template <bool IS_CHECK_TYPE>
static void NameIsCheckType(std::string_view name, AbckitCoreAnnotationInterfaceField *fld)
{
    if constexpr (IS_CHECK_TYPE) {
        auto type = g_implI->annotationInterfaceFieldGetType(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        if (name == "a") {
            EXPECT_TRUE(ABCKIT_TYPE_ID_F64 == g_implI->typeGetTypeId(type));
        }
        if (name == "b") {
            EXPECT_TRUE(ABCKIT_TYPE_ID_F64 == g_implI->typeGetTypeId(type));
        }
        if (name == "d") {
            EXPECT_TRUE(ABCKIT_TYPE_ID_U1 == g_implI->typeGetTypeId(type));
        }
    }
}

template <bool IS_CHECK_CONTEXT, bool IS_CHECK_NAME, bool IS_CHECK_INTERFACE, bool IS_CHECK_TYPE, bool IS_CHECK_VALUE>
static auto g_processAnnotationInterfaceFieldsEnumeratorCb = [](AbckitCoreAnnotationInterfaceField *fld, void *data2) {
    auto *newData2 = reinterpret_cast<AnnoInterfaceInfo *>(data2);

    if constexpr (IS_CHECK_INTERFACE) {
        auto annoIf = g_implI->annotationInterfaceFieldGetInterface(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(newData2->annoIf == annoIf);
    }

    auto file = g_implI->annotationInterfaceFieldGetFile(fld);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    if constexpr (IS_CHECK_CONTEXT) {
        EXPECT_TRUE(newData2->file == file);
    }

    auto str = g_implI->annotationInterfaceFieldGetName(fld);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    auto name = helpers::AbckitStringToString(str);
    if constexpr (IS_CHECK_NAME) {
        EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "str");
    }

    NameIsCheckType<IS_CHECK_TYPE>(name, fld);

    if constexpr (IS_CHECK_VALUE) {
        auto val = g_implI->annotationInterfaceFieldGetDefaultValue(fld);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        if (name == "a") {
            auto valueDb = g_implI->valueGetDouble(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(valueDb == 3U);
        }
        if (name == "b") {
            auto vs = g_implI->arrayValueGetLiteralArray(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            g_implI->literalArrayEnumerateElements(vs, nullptr, [](AbckitFile *, AbckitLiteral *v, void *) {
                double value = g_implI->literalGetDouble(v);
                EXPECT_TRUE(value == 13U || value == 9U);
                return true;
            });
        }
        if (name == "d") {
            auto valueB = g_implI->valueGetU1(val);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(valueB == false);
        }
        if (name == "str") {
            auto str = g_implI->valueGetString(val);
            auto valueStr = helpers::AbckitStringToString(str);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            EXPECT_TRUE(valueStr == "Hello");
        }
    }

    return true;
};

template <bool IS_CHECK_CONTEXT, bool IS_CHECK_NAME, bool IS_CHECK_INTERFACE, bool IS_CHECK_TYPE, bool IS_CHECK_VALUE>
static void ProcessAnnotationInterfaceFields(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    AnnoInterfaceInfo newData;
    g_implI->functionEnumerateAnnotations(method, &newData, [](AbckitCoreAnnotation *anno, void *data1) {
        auto *newData1 = reinterpret_cast<AnnoInterfaceInfo *>(data1);
        newData1->anno = anno;
        newData1->annoIf = g_implI->annotationGetInterface(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(newData1->annoIf != nullptr);
        auto file = g_implI->annotationInterfaceGetFile(newData1->annoIf);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        newData1->file = file;

        auto str = g_implI->annotationInterfaceGetName(newData1->annoIf);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto name = helpers::AbckitStringToString(str);
        if (name != "Anno") {
            return true;
        }

        g_implI->annotationInterfaceEnumerateFields(
            newData1->annoIf, newData1,
            g_processAnnotationInterfaceFieldsEnumeratorCb<IS_CHECK_CONTEXT, IS_CHECK_INTERFACE, IS_CHECK_NAME,
                                                           IS_CHECK_TYPE, IS_CHECK_VALUE>);

        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static void ProcessAnnotationElementsInner(std::string_view name, AbckitCoreAnnotationElement *annoElement)
{
    if (name == "a") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetDouble(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == 10U);
    }

    if (name == "b") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->arrayValueGetLiteralArray(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->literalArrayEnumerateElements(vs, nullptr, [](AbckitFile *, AbckitLiteral *v, void *) {
            double value = g_implI->literalGetDouble(v);
            EXPECT_TRUE(value == 1 || value == 2U || value == 3U);
            return true;
        });
    }

    if (name == "d") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetU1(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == true);
    }

    if (name == "str") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->valueGetString(val);
        auto vs = helpers::AbckitStringToString(str);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == "DEF");
    }
}

static void ProcessMethodAnnotations(AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    g_implI->functionEnumerateAnnotations(method, nullptr, [](AbckitCoreAnnotation *anno, void *) {
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->annotationEnumerateElements(anno, nullptr, [](AbckitCoreAnnotationElement *annoElement, void *) {
            auto abckitstr = g_implI->annotationElementGetName(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto name = helpers::AbckitStringToString(abckitstr);
            EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "str");

            ProcessAnnotationElementsInner(name, annoElement);

            return true;
        });
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static void ProcessClassAnnotationsInner(std::string_view name, AbckitCoreAnnotationElement *annoElement)
{
    if (name == "a") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetDouble(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == 20U);
    }

    if (name == "b") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->arrayValueGetLiteralArray(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->literalArrayEnumerateElements(vs, nullptr, [](AbckitFile *, AbckitLiteral *v, void *) {
            double value = g_implI->literalGetDouble(v);
            EXPECT_TRUE(value == 13U || value == 10U);
            return true;
        });
    }

    if (name == "d") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto vs = g_implI->valueGetU1(val);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == true);
    }

    if (name == "str") {
        AbckitValue *val = g_implI->annotationElementGetValue(annoElement);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        auto str = g_implI->valueGetString(val);
        auto vs = helpers::AbckitStringToString(str);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_TRUE(vs == "ABC");
    }
}

static void ProcessClassAnnotations(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    auto klass = g_implI->functionGetParentClass(method);
    g_implI->classEnumerateAnnotations(klass, file, [](AbckitCoreAnnotation *anno, void *) {
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        g_implI->annotationEnumerateElements(anno, nullptr, [](AbckitCoreAnnotationElement *annoElement, void *) {
            auto abckitstr = g_implI->annotationElementGetName(annoElement);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto name = helpers::AbckitStringToString(abckitstr);

            EXPECT_TRUE(name == "a" || name == "b" || name == "d" || name == "str");

            ProcessClassAnnotationsInner(name, annoElement);

            return true;
        });
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

static void ProcessAnnotationElements(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    ProcessMethodAnnotations(file, method, graph);
    ProcessClassAnnotations(file, method, graph);
}

static void AnnotationGetFile(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph * /*graph*/)
{
    g_implI->functionEnumerateAnnotations(method, file, [](AbckitCoreAnnotation *anno, void *data1) {
        auto file1 = reinterpret_cast<AbckitFile *>(data1);
        auto annoFile = g_implI->annotationGetFile(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(file1, annoFile);
        return true;
    });
    auto klass = g_implI->functionGetParentClass(method);
    g_implI->classEnumerateAnnotations(klass, file, [](AbckitCoreAnnotation *anno, void *data1) {
        auto file1 = reinterpret_cast<AbckitFile *>(data1);
        auto annoFile = g_implI->annotationGetFile(anno);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        EXPECT_EQ(file1, annoFile);
        return true;
    });
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationElementGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationElementGetName)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotations<false, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationElementGetValue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationElementGetValue)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotations<true, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationElementGetAnnotation, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationElementGetAnnotation)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotations<false, true, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationElementGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationElementGetFile)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotations<false, false, true>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateAnnotations, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, ClassEnumerateAnnotations)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "A", ProcessClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationGetInterface)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterface<false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceGetName)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterface<true, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceGetFile)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterface<false, true>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnnotationInterfaces, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, ModuleEnumerateAnnotationInterfaces)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessModule<false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceGetModule, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceGetModule)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessModule<true>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceEnumerateFields, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceEnumerateFields)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<false, false, false, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceFieldGetFile, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceFieldGetFile)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<true, false, false, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceFieldGetName, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceFieldGetName)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<false, true, false, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceFieldGetInterface, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceFieldGetInterface)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<false, false, true, false, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceFieldGetType, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceFieldGetType)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<false, false, false, true, false>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceFieldGetDefaultValue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationInterfaceFieldGetDefaultValue)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationInterfaceFields<false, false, false, false, true>);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationEnumerateElements, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationEnumerateElements)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessAnnotationElements);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, AnnotationGetFile)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", AnnotationGetFile);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::functionEnumerateAnnotations, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTests, FunctionEnumerateAnnotations)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_inspect_dynamic.abc",
                           "foo", ProcessMethodAnnotations);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
