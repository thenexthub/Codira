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

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../include/libabckit/c/extensions/arkts/metadata_arkts.h"
#include "../../include/libabckit/c/metadata_core.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock::arkts_inspect_api {

// NOLINTBEGIN(readability-identifier-naming)

AbckitCoreModule *ArktsModuleToCoreModule(AbckitArktsModule *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_ARKTS_MODULE);
    return DEFAULT_CORE_MODULE;
}

AbckitArktsModule *CoreModuleToArktsModule(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_ARKTS_MODULE;
}

AbckitCoreNamespace *ArktsNamespaceToCoreNamespace(AbckitArktsNamespace *ns)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ns == DEFAULT_NAMESPACE);
    return DEFAULT_CORE_NAMESPACE;
}

AbckitArktsNamespace *CoreNamespaceToArktsNamespace(AbckitCoreNamespace *ns)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ns == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_NAMESPACE;
}

AbckitArktsFunction *ArktsV1NamespaceGetConstructor(AbckitArktsNamespace *ns)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ns == DEFAULT_NAMESPACE);
    return DEFAULT_ARKTS_FUNCTION;
}

AbckitCoreImportDescriptor *ArktsImportDescriptorToCoreImportDescriptor(AbckitArktsImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(id == DEFAULT_ARKTS_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_IMPORT_DESCRIPTOR;
}

AbckitArktsImportDescriptor *CoreImportDescriptorToArktsImportDescriptor(AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_ARKTS_IMPORT_DESCRIPTOR;
}

AbckitCoreExportDescriptor *ArktsExportDescriptorToCoreExportDescriptor(AbckitArktsExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ed == DEFAULT_ARKTS_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_EXPORT_DESCRIPTOR;
}

AbckitArktsExportDescriptor *CoreExportDescriptorToArktsExportDescriptor(AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_ARKTS_EXPORT_DESCRIPTOR;
}

AbckitCoreClass *ArktsClassToCoreClass(AbckitArktsClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_CORE_CLASS;
}

AbckitArktsClass *CoreClassToArktsClass(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_ARKTS_CLASS;
}

bool ArktsClassIsFinal(AbckitArktsClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_BOOL;
}

bool ArktsClassIsAbstract(AbckitArktsClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(klass == DEFAULT_ARKTS_CLASS);
    return DEFAULT_BOOL;
}

AbckitCoreInterface *ArktsInterfaceToCoreInterface(AbckitArktsInterface *iface)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(iface == DEFAULT_ARKTS_INTERFACE);
    return DEFAULT_CORE_INTERFACE;
}

AbckitArktsInterface *CoreInterfaceToArktsInterface(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return DEFAULT_ARKTS_INTERFACE;
}

AbckitCoreEnum *ArktsEnumToCoreEnum(AbckitArktsEnum *enm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(enm == DEFAULT_ARKTS_ENUM);
    return DEFAULT_CORE_ENUM;
}

AbckitArktsEnum *CoreEnumToArktsEnum(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return DEFAULT_ARKTS_ENUM;
}

AbckitCoreModuleField *ArktsModuleFieldToCoreModuleField(AbckitArktsModuleField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_ARKTS_MODULE_FIELD);
    return DEFAULT_CORE_MODULE_FIELD;
}

AbckitArktsModuleField *CoreModuleFieldToArktsModuleField(AbckitCoreModuleField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_CORE_MODULE_FIELD);
    return DEFAULT_ARKTS_MODULE_FIELD;
}

AbckitArktsNamespaceField *CoreNamespaceFieldToArktsNamespaceField(AbckitCoreNamespaceField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_CORE_NAMESPACE_FIELD);
    return DEFAULT_ARKTS_NAMESPACE_FIELD;
}

AbckitCoreClassField *ArktsClassFieldToCoreClassField(AbckitArktsClassField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_ARKTS_CLASS_FIELD);
    return DEFAULT_CORE_CLASS_FIELD;
}

AbckitArktsClassField *CoreClassFieldToArktsClassField(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_ARKTS_CLASS_FIELD;
}

AbckitCoreInterfaceField *ArktsInterfaceFieldToCoreInterfaceField(AbckitArktsInterfaceField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    return DEFAULT_CORE_INTERFACE_FIELD;
}

AbckitArktsInterfaceField *CoreInterfaceFieldToArktsInterfaceField(AbckitCoreInterfaceField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return DEFAULT_ARKTS_INTERFACE_FIELD;
}

bool ArktsInterfaceFieldIsReadonly(AbckitArktsInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_ARKTS_INTERFACE_FIELD);
    return DEFAULT_BOOL;
}

AbckitCoreEnumField *ArktsEnumFieldToCoreEnumField(AbckitArktsEnumField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_ARKTS_ENUM_FIELD);
    return DEFAULT_CORE_ENUM_FIELD;
}

AbckitArktsEnumField *CoreEnumFieldToArktsEnumField(AbckitCoreEnumField *field)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(field == DEFAULT_CORE_ENUM_FIELD);
    return DEFAULT_ARKTS_ENUM_FIELD;
}

AbckitCoreFunction *ArktsFunctionToCoreFunction(AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_CORE_FUNCTION;
}

AbckitArktsFunction *CoreFunctionToArktsFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_ARKTS_FUNCTION;
}

bool FunctionIsNative(AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsAsync(AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsFinal(AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsAbstract(AbckitArktsFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_ARKTS_FUNCTION);
    return DEFAULT_BOOL;
}

AbckitCoreAnnotation *ArktsAnnotationToCoreAnnotation(AbckitArktsAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(anno == DEFAULT_ANNOTATION);
    return DEFAULT_CORE_ANNOTATION;
}

AbckitArktsAnnotation *CoreAnnotationToArktsAnnotation(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_ANNOTATION;
}

AbckitCoreAnnotationElement *ArktsAnnotationElementToCoreAnnotationElement(AbckitArktsAnnotationElement *annoElem)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoElem == DEFAULT_ANNOTATION_ELEMENT);
    return DEFAULT_CORE_ANNOTATION_ELEMENT;
}

AbckitArktsAnnotationElement *CoreAnnotationElementToArktsAnnotationElement(AbckitCoreAnnotationElement *annoElem)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoElem == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_ANNOTATION_ELEMENT;
}

AbckitCoreAnnotationInterface *ArktsAnnotationInterfaceToCoreAnnotationInterface(
    AbckitArktsAnnotationInterface *annoInterface)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoInterface == DEFAULT_ANNOTATION_INTERFACE);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

AbckitArktsAnnotationInterface *CoreAnnotationInterfaceToArktsAnnotationInterface(
    AbckitCoreAnnotationInterface *annoInterface)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoInterface == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_ANNOTATION_INTERFACE;
}

AbckitCoreAnnotationInterfaceField *ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(
    AbckitArktsAnnotationInterfaceField *annoInterfaceField)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoInterfaceField == DEFAULT_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD;
}

AbckitArktsAnnotationInterfaceField *CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(
    AbckitCoreAnnotationInterfaceField *annoInterfaceField)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(annoInterfaceField == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_ANNOTATION_INTERFACE_FIELD;
}

AbckitArktsInspectApi g_arktsInspectApiImpl = {
    // ========================================
    // File
    // ========================================

    // ========================================
    // Module
    // ========================================

    ArktsModuleToCoreModule, CoreModuleToArktsModule,

    /* ========================================
     * Namespace
     * ======================================== */

    ArktsNamespaceToCoreNamespace, CoreNamespaceToArktsNamespace, ArktsV1NamespaceGetConstructor,

    // ========================================
    // ImportDescriptor
    // ========================================

    ArktsImportDescriptorToCoreImportDescriptor, CoreImportDescriptorToArktsImportDescriptor,

    // ========================================
    // ExportDescriptor
    // ========================================

    ArktsExportDescriptorToCoreExportDescriptor, CoreExportDescriptorToArktsExportDescriptor,

    // ========================================
    // Class
    // ========================================

    ArktsClassToCoreClass, CoreClassToArktsClass, ArktsClassIsFinal, ArktsClassIsAbstract,

    // ========================================
    // Interface
    // ========================================

    ArktsInterfaceToCoreInterface, CoreInterfaceToArktsInterface,

    // ========================================
    // Enum
    // ========================================

    ArktsEnumToCoreEnum, CoreEnumToArktsEnum,

    // ========================================
    // Module Field
    // ========================================

    ArktsModuleFieldToCoreModuleField, CoreModuleFieldToArktsModuleField,

    // ========================================
    // Namespace Field
    // ========================================

    CoreNamespaceFieldToArktsNamespaceField,

    // ========================================
    // Class Field
    // ========================================

    ArktsClassFieldToCoreClassField, CoreClassFieldToArktsClassField,

    // ========================================
    // Interface Field
    // ========================================

    ArktsInterfaceFieldToCoreInterfaceField, CoreInterfaceFieldToArktsInterfaceField, ArktsInterfaceFieldIsReadonly,

    // ========================================
    // Enum Field
    // ========================================

    ArktsEnumFieldToCoreEnumField, CoreEnumFieldToArktsEnumField,

    // ========================================
    // Function
    // ========================================

    ArktsFunctionToCoreFunction, CoreFunctionToArktsFunction, FunctionIsNative, FunctionIsAsync, FunctionIsFinal,
    FunctionIsAbstract,

    // ========================================
    // Annotation
    // ========================================

    ArktsAnnotationToCoreAnnotation, CoreAnnotationToArktsAnnotation,

    // ========================================
    // AnnotationElement
    // ========================================

    ArktsAnnotationElementToCoreAnnotationElement, CoreAnnotationElementToArktsAnnotationElement,

    // ========================================
    // AnnotationInterface
    // ========================================

    ArktsAnnotationInterfaceToCoreAnnotationInterface, CoreAnnotationInterfaceToArktsAnnotationInterface,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField,
    CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField,

    // ========================================
    // Type
    // ========================================

    // ========================================
    // Value
    // ========================================

    // ========================================
    // String
    // ========================================

    // ========================================
    // LiteralArray
    // ========================================

    // ========================================
    // Literal
    // ========================================
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::arkts_inspect_api

AbckitArktsInspectApi const *AbckitGetMockArktsInspectApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::arkts_inspect_api::g_arktsInspectApiImpl;
}
