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

#ifndef ABCKIT_INSPECT_IMPL_MOCK
#define ABCKIT_INSPECT_IMPL_MOCK

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../../include/libabckit/c/metadata_core.h"

#include <gtest/gtest.h>

namespace libabckit::mock::metadata_inspect {

// NOLINTBEGIN(readability-identifier-naming)

// ========================================
// File
// ========================================

AbckitFileVersion FileGetVersion(AbckitFile *file)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return DEFAULT_FILE_VERSION;
}

bool FileEnumerateModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

bool FileEnumerateExternalModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    return cb(DEFAULT_CORE_MODULE, data);
}

// ========================================
// Module
// ========================================

AbckitFile *ModuleGetFile(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_FILE;
}

AbckitTarget ModuleGetTarget(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_TARGET;
}

AbckitString *ModuleGetName(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_STRING;
}

bool ModuleIsExternal(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return true;
}

bool ModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_IMPORT_DESCRIPTOR, data);
}

bool ModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_EXPORT_DESCRIPTOR, data);
}

bool ModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

bool ModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool ModuleEnumerateInterfaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_INTERFACE, data);
}

bool ModuleEnumerateEnums(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_ENUM, data);
}

bool ModuleEnumerateFields(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreModuleField *field, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_MODULE_FIELD, data);
}

bool ModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                      bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                       bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                         bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE, data);
}

// ========================================
// Namespace
// ========================================

AbckitCoreModule *NamespaceGetModule(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_CORE_MODULE;
}

AbckitString *NamespaceGetName(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_STRING;
}

bool NamespaceIsExternal(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return true;
}

AbckitCoreNamespace *NamespaceGetParentNamespace(AbckitCoreNamespace *n)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return DEFAULT_CORE_NAMESPACE;
}

bool NamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                  bool (*cb)(AbckitCoreNamespace *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_NAMESPACE, data);
}

bool NamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool NamespaceEnumerateInterfaces(AbckitCoreNamespace *n, void *data,
                                  bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_INTERFACE, data);
}

bool NamespaceEnumerateEnums(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_ENUM, data);
}

bool NamespaceEnumerateFields(AbckitCoreNamespace *n, void *data,
                              bool (*cb)(AbckitCoreNamespaceField *field, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_NAMESPACE_FIELD, data);
}

bool NamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreFunction *func, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(n == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool NamespaceEnumerateAnnotationInterfaces(AbckitCoreNamespace *m, void *data,
                                            bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(m == DEFAULT_CORE_NAMESPACE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE, data);
}

// ========================================
// ImportDescriptor
// ========================================

AbckitFile *ImportDescriptorGetFile(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

AbckitCoreModule *ImportDescriptorGetImportedModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitCoreModule *ImportDescriptorGetImportingModule(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ImportDescriptorGetName(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

AbckitString *ImportDescriptorGetAlias(AbckitCoreImportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// ExportDescriptor
// ========================================

AbckitFile *ExportDescriptorGetFile(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_FILE;
}

AbckitCoreModule *ExportDescriptorGetExportingModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitCoreModule *ExportDescriptorGetExportedModule(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ExportDescriptorGetName(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

AbckitString *ExportDescriptorGetAlias(AbckitCoreExportDescriptor *i)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(i == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_STRING;
}

// ========================================
// Class
// ========================================

AbckitFile *ClassGetFile(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_FILE;
}

AbckitCoreModule *ClassGetModule(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ClassGetName(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_STRING;
}

bool ClassIsExternal(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return true;
}

bool ClassIsFinal(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return true;
}

bool ClassIsAbstract(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return true;
}

AbckitCoreFunction *ClassGetParentFunction(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_FUNCTION;
}

AbckitCoreNamespace *ClassGetParentNamespace(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_NAMESPACE;
}

AbckitCoreClass *ClassGetSuperClass(AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return DEFAULT_CORE_CLASS;
}

bool ClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool ClassEnumerateAnnotations(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

bool ClassEnumerateSubClasses(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreClass *subClass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool ClassEnumerateInterfaces(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_INTERFACE, data);
}

bool ClassEnumerateFields(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreClassField *field, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
    return cb(DEFAULT_CORE_CLASS_FIELD, data);
}

// ========================================
// Interface
// ========================================

AbckitFile *InterfaceGetFile(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return DEFAULT_FILE;
}

AbckitCoreModule *InterfaceGetModule(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return DEFAULT_CORE_MODULE;
}

AbckitString *InterfaceGetName(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return DEFAULT_STRING;
}

bool InterfaceIsExternal(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return true;
}

AbckitCoreNamespace *InterfaceGetParentNamespace(AbckitCoreInterface *iface)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return DEFAULT_CORE_NAMESPACE;
}

bool InterfaceEnumerateSuperInterfaces(AbckitCoreInterface *iface, void *data,
                                       bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_INTERFACE, data);
}

bool InterfaceEnumerateSubInterfaces(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_INTERFACE, data);
}

bool InterfaceEnumerateClasses(AbckitCoreInterface *iface, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool InterfaceEnumerateMethods(AbckitCoreInterface *iface, void *data,
                               bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool InterfaceEnumerateAnnotations(AbckitCoreInterface *iface, void *data,
                                   bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

bool InterfaceEnumerateFields(AbckitCoreInterface *iface, void *data,
                              bool (*cb)(AbckitCoreInterfaceField *field, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(iface == DEFAULT_CORE_INTERFACE);
    return cb(DEFAULT_CORE_INTERFACE_FIELD, data);
}

// ========================================
// Enum
// ========================================

AbckitFile *EnumGetFile(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return DEFAULT_FILE;
}

AbckitCoreModule *EnumGetModule(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return DEFAULT_CORE_MODULE;
}

AbckitString *EnumGetName(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return DEFAULT_STRING;
}

bool EnumIsExternal(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return true;
}

AbckitCoreNamespace *EnumGetParentNamespace(AbckitCoreEnum *enm)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return DEFAULT_CORE_NAMESPACE;
}

bool EnumEnumerateMethods(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *function, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool EnumEnumerateFields(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(enm == DEFAULT_CORE_ENUM);
    return cb(DEFAULT_CORE_ENUM_FIELD, data);
}

// ========================================
// Module Field
// ========================================

AbckitCoreModule *ModuleFieldGetModule(AbckitCoreModuleField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_MODULE_FIELD);
    return DEFAULT_CORE_MODULE;
}

AbckitString *ModuleFieldGetName(AbckitCoreModuleField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_MODULE_FIELD);
    return DEFAULT_STRING;
}

AbckitType *ModuleFieldGetType(AbckitCoreModuleField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_MODULE_FIELD);
    return DEFAULT_TYPE;
}

AbckitValue *ModuleFieldGetValue(AbckitCoreModuleField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_MODULE_FIELD);
    return DEFAULT_VALUE;
}

// ========================================
// Namespace Field
// ========================================

AbckitCoreNamespace *NamespaceFieldGetNamespace(AbckitCoreNamespaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_NAMESPACE_FIELD);
    return DEFAULT_CORE_NAMESPACE;
}

AbckitString *NamespaceFieldGetName(AbckitCoreNamespaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_NAMESPACE_FIELD);
    return DEFAULT_STRING;
}

AbckitType *NamespaceFieldGetType(AbckitCoreNamespaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_NAMESPACE_FIELD);
    return DEFAULT_TYPE;
}

// ========================================
// Class Field
// ========================================

AbckitCoreClass *ClassFieldGetClass(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_CORE_CLASS;
}

AbckitString *ClassFieldGetName(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_STRING;
}

AbckitType *ClassFieldGetType(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_TYPE;
}

AbckitValue *ClassFieldGetValue(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_VALUE;
}

bool ClassFieldIsPublic(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldIsProtected(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldIsPrivate(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldIsInternal(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldIsStatic(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldIsFinal(AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_BOOL;
}

bool ClassFieldEnumerateAnnotations(AbckitCoreClassField *field, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

// ========================================
// Interface Field
// ========================================

AbckitCoreInterface *InterfaceFieldGetInterface(AbckitCoreInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return DEFAULT_CORE_INTERFACE;
}

AbckitString *InterfaceFieldGetName(AbckitCoreInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return DEFAULT_STRING;
}

AbckitType *InterfaceFieldGetType(AbckitCoreInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return DEFAULT_TYPE;
}

bool InterfaceFieldIsReadonly(AbckitCoreInterfaceField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return DEFAULT_BOOL;
}

bool InterfaceFieldEnumerateAnnotations(AbckitCoreInterfaceField *field, void *data,
                                        bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_INTERFACE_FIELD);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

// ========================================
// Enum Field
// ========================================

AbckitCoreEnum *EnumFieldGetEnum(AbckitCoreEnumField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_ENUM_FIELD);
    return DEFAULT_CORE_ENUM;
}

AbckitString *EnumFieldGetName(AbckitCoreEnumField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_ENUM_FIELD);
    return DEFAULT_STRING;
}

AbckitType *EnumFieldGetType(AbckitCoreEnumField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_ENUM_FIELD);
    return DEFAULT_TYPE;
}

AbckitValue *EnumFieldGetValue(AbckitCoreEnumField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(field == DEFAULT_CORE_ENUM_FIELD);
    return DEFAULT_VALUE;
}

// ========================================
// AnnotationInterface
// ========================================

AbckitFile *AnnotationInterfaceGetFile(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_FILE;
}

AbckitCoreModule *AnnotationInterfaceGetModule(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_CORE_MODULE;
}

AbckitCoreNamespace *AnnotationInterfaceGetParentNamespace(AbckitCoreAnnotationInterface *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_CORE_NAMESPACE;
}

AbckitString *AnnotationInterfaceGetName(AbckitCoreAnnotationInterface *ai)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return DEFAULT_STRING;
}

bool AnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                        bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ai == DEFAULT_CORE_ANNOTATION_INTERFACE);
    return cb(DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD, data);
}

// ========================================
// AnnotationInterfaceField
// ========================================

AbckitFile *AnnotationInterfaceFieldGetFile(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_FILE;
}

AbckitCoreAnnotationInterface *AnnotationInterfaceFieldGetInterface(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

AbckitString *AnnotationInterfaceFieldGetName(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_STRING;
}

AbckitType *AnnotationInterfaceFieldGetType(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_TYPE;
}

AbckitValue *AnnotationInterfaceFieldGetDefaultValue(AbckitCoreAnnotationInterfaceField *fld)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(fld == DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD);
    return DEFAULT_VALUE;
}

// ========================================
// Function
// ========================================

AbckitFile *FunctionGetFile(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_FILE;
}

AbckitCoreModule *FunctionGetModule(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_MODULE;
}

AbckitString *FunctionGetName(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_STRING;
}

AbckitCoreFunction *FunctionGetParentFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_FUNCTION;
}

AbckitCoreClass *FunctionGetParentClass(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_CLASS;
}

AbckitCoreNamespace *FunctionGetParentNamespace(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_CORE_NAMESPACE;
}

bool FunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_FUNCTION, data);
}

bool FunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                    bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_CLASS, data);
}

bool FunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                  bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_ANNOTATION, data);
}

AbckitGraph *CreateGraphFromFunction(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_GRAPH;
}

bool FunctionIsStatic(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsCtor(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsCctor(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsAnonymous(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsPublic(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsProtected(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsPrivate(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsInternal(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionIsExternal(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_BOOL;
}

bool FunctionEnumerateParameters(AbckitCoreFunction *function, void *data,
                                 bool (*cb)(AbckitCoreFunctionParam *param, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return cb(DEFAULT_CORE_FUNCTION_PARAM, data);
}

AbckitType *FunctionGetReturnType(AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    return DEFAULT_TYPE;
}

// ========================================
// Function Param
// ========================================

AbckitType *FunctionParamGetType(AbckitCoreFunctionParam *param)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(param == DEFAULT_CORE_FUNCTION_PARAM);
    return DEFAULT_TYPE;
}

// ========================================
// Annotation
// ========================================

AbckitFile *AnnotationGetFile(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_FILE;
}

AbckitString *AnnotationGetName(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_STRING;
}

AbckitCoreAnnotationInterface *AnnotationGetInterface(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_CORE_ANNOTATION_INTERFACE;
}

bool AnnotationIsExternal(AbckitCoreAnnotation *anno)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return DEFAULT_BOOL;
}

bool AnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                 bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(anno == DEFAULT_CORE_ANNOTATION);
    return cb(DEFAULT_CORE_ANNOTATION_ELEMENT, data);
}

// ========================================
// AnnotationElement
// ========================================

AbckitFile *AnnotationElementGetFile(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_FILE;
}

AbckitCoreAnnotation *AnnotationElementGetAnnotation(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_CORE_ANNOTATION;
}

AbckitString *AnnotationElementGetName(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_STRING;
}

AbckitValue *AnnotationElementGetValue(AbckitCoreAnnotationElement *ae)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(ae == DEFAULT_CORE_ANNOTATION_ELEMENT);
    return DEFAULT_VALUE;
}

// ========================================
// Type
// ========================================

AbckitTypeId TypeGetTypeId(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_TYPE_ID;
}

AbckitCoreClass *TypeGetReferenceClass(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_CORE_CLASS;
}

AbckitString *TypeGetName(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_STRING;
}

size_t TypeGetRank(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_SIZE_T;
}

bool TypeIsUnion(AbckitType *type)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return DEFAULT_BOOL;
}

bool TypeEnumerateUnionTypes(AbckitType *type, void *data, bool (*cb)(AbckitType *unionType, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(type == DEFAULT_TYPE);
    return cb(DEFAULT_TYPE, data);
}

// ========================================
// Value
// ========================================

AbckitFile *ValueGetFile(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_FILE;
}

AbckitType *ValueGetType(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_TYPE;
}

bool ValueGetU1(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_BOOL;
}

int ValueGetInt(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_I32;
}

double ValueGetDouble(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_DOUBLE;
}

AbckitString *ValueGetString(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_STRING;
}

AbckitLiteralArray *ArrayValueGetLiteralArray(AbckitValue *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_VALUE);
    return DEFAULT_LITERAL_ARRAY;
}

// ========================================
// String
// ========================================

const char *AbckitStringToString(AbckitString *value)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(value == DEFAULT_STRING);
    return DEFAULT_CONST_CHAR;
}

// ========================================
// LiteralArray
// ========================================

bool LiteralArrayEnumerateElements(AbckitLiteralArray *litArr, void *data,
                                   bool (*cb)(AbckitFile *file, AbckitLiteral *v, void *data))
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(litArr == DEFAULT_LITERAL_ARRAY);
    return cb(DEFAULT_FILE, DEFAULT_LITERAL, data);
}

// ========================================
// Literal
// ========================================

AbckitFile *LiteralGetFile(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FILE;
}

AbckitLiteralTag LiteralGetTag(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_TAG;
}

bool LiteralGetBool(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_BOOL;
}

uint8_t LiteralGetU8(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U8;
}

uint16_t LiteralGetU16(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

uint16_t LiteralGetMethodAffiliate(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U16;
}

uint32_t LiteralGetU32(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U32;
}

uint64_t LiteralGetU64(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_U64;
}

float LiteralGetFloat(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_FLOAT;
}

double LiteralGetDouble(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_DOUBLE;
}

AbckitLiteralArray *LiteralGetLiteralArray(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_LITERAL_ARRAY;
}

AbckitString *LiteralGetString(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_STRING;
}

AbckitString *LiteralGetMethod(AbckitLiteral *lit)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(lit == DEFAULT_LITERAL);
    return DEFAULT_STRING;
}

static AbckitInspectApi g_inspectApiImpl = {
    // ========================================
    // File
    // ========================================

    FileGetVersion,
    FileEnumerateModules,
    FileEnumerateExternalModules,

    // ========================================
    // String
    // ========================================

    AbckitStringToString,

    // ========================================
    // Type
    // ========================================

    TypeGetTypeId,
    TypeGetReferenceClass,
    TypeGetName,
    TypeGetRank,
    TypeIsUnion,
    TypeEnumerateUnionTypes,

    // ========================================
    // Value
    // ========================================

    ValueGetFile,
    ValueGetType,
    ValueGetU1,
    ValueGetInt,
    ValueGetDouble,
    ValueGetString,
    ArrayValueGetLiteralArray,

    // ========================================
    // Literal
    // ========================================

    LiteralGetFile,
    LiteralGetTag,
    LiteralGetBool,
    LiteralGetU8,
    LiteralGetU16,
    LiteralGetMethodAffiliate,
    LiteralGetU32,
    LiteralGetU64,
    LiteralGetFloat,
    LiteralGetDouble,
    LiteralGetLiteralArray,
    LiteralGetString,
    LiteralGetMethod,

    // ========================================
    // LiteralArray
    // ========================================

    LiteralArrayEnumerateElements,

    // ========================================
    // Module
    // ========================================

    ModuleGetFile,
    ModuleGetTarget,
    ModuleGetName,
    ModuleIsExternal,
    ModuleEnumerateImports,
    ModuleEnumerateExports,
    ModuleEnumerateNamespaces,
    ModuleEnumerateClasses,
    ModuleEnumerateInterfaces,
    ModuleEnumerateEnums,
    ModuleEnumerateFields,
    ModuleEnumerateTopLevelFunctions,
    ModuleEnumerateAnonymousFunctions,
    ModuleEnumerateAnnotationInterfaces,

    // ========================================
    // Namespace
    // ========================================
    NamespaceGetModule,
    NamespaceGetName,
    NamespaceIsExternal,
    NamespaceGetParentNamespace,
    NamespaceEnumerateNamespaces,
    NamespaceEnumerateClasses,
    NamespaceEnumerateInterfaces,
    NamespaceEnumerateEnums,
    NamespaceEnumerateFields,
    NamespaceEnumerateTopLevelFunctions,
    NamespaceEnumerateAnnotationInterfaces,

    // ========================================
    // ImportDescriptor
    // ========================================

    ImportDescriptorGetFile,
    ImportDescriptorGetImportingModule,
    ImportDescriptorGetImportedModule,
    ImportDescriptorGetName,
    ImportDescriptorGetAlias,

    // ========================================
    // ExportDescriptor
    // ========================================

    ExportDescriptorGetFile,
    ExportDescriptorGetExportingModule,
    ExportDescriptorGetExportedModule,
    ExportDescriptorGetName,
    ExportDescriptorGetAlias,

    // ========================================
    // Class
    // ========================================

    ClassGetFile,
    ClassGetModule,
    ClassGetName,
    ClassIsExternal,
    ClassIsFinal,
    ClassIsAbstract,
    ClassGetParentFunction,
    ClassGetParentNamespace,
    ClassGetSuperClass,
    ClassEnumerateMethods,
    ClassEnumerateAnnotations,
    ClassEnumerateSubClasses,
    ClassEnumerateInterfaces,
    ClassEnumerateFields,

    // ========================================
    // Interface
    // ========================================

    InterfaceGetFile,
    InterfaceGetModule,
    InterfaceGetName,
    InterfaceIsExternal,
    InterfaceGetParentNamespace,
    InterfaceEnumerateSuperInterfaces,
    InterfaceEnumerateSubInterfaces,
    InterfaceEnumerateClasses,
    InterfaceEnumerateMethods,
    InterfaceEnumerateAnnotations,
    InterfaceEnumerateFields,

    // ========================================
    // Enum
    // ========================================

    EnumGetFile,
    EnumGetModule,
    EnumGetName,
    EnumIsExternal,
    EnumGetParentNamespace,
    EnumEnumerateMethods,
    EnumEnumerateFields,

    // ========================================
    // Module Field
    // ========================================

    ModuleFieldGetModule,
    ModuleFieldGetName,
    ModuleFieldGetType,
    ModuleFieldGetValue,

    // ========================================
    // Namespace Field
    // ========================================

    NamespaceFieldGetNamespace,
    NamespaceFieldGetName,
    NamespaceFieldGetType,

    // ========================================
    // Class Field
    // ========================================

    ClassFieldGetClass,
    ClassFieldGetName,
    ClassFieldGetType,
    ClassFieldGetValue,
    ClassFieldIsPublic,
    ClassFieldIsProtected,
    ClassFieldIsPrivate,
    ClassFieldIsInternal,
    ClassFieldIsStatic,
    ClassFieldIsFinal,
    ClassFieldEnumerateAnnotations,

    // ========================================
    // Interface Field
    // ========================================

    InterfaceFieldGetInterface,
    InterfaceFieldGetName,
    InterfaceFieldGetType,
    InterfaceFieldIsReadonly,
    InterfaceFieldEnumerateAnnotations,

    // ========================================
    // Enum Field
    // ========================================

    EnumFieldGetEnum,
    EnumFieldGetName,
    EnumFieldGetType,
    EnumFieldGetValue,

    // ========================================
    // Function
    // ========================================

    FunctionGetFile,
    FunctionGetModule,
    FunctionGetName,
    FunctionGetParentFunction,
    FunctionGetParentClass,
    FunctionGetParentNamespace,
    FunctionEnumerateNestedFunctions,
    FunctionEnumerateNestedClasses,
    FunctionEnumerateAnnotations,
    CreateGraphFromFunction,
    FunctionIsStatic,
    FunctionIsCtor,
    FunctionIsCctor,
    FunctionIsAnonymous,
    FunctionIsPublic,
    FunctionIsProtected,
    FunctionIsPrivate,
    FunctionIsInternal,
    FunctionIsExternal,
    FunctionEnumerateParameters,
    FunctionGetReturnType,

    // ========================================
    // Function Param
    // ========================================

    FunctionParamGetType,

    // ========================================
    // Annotation
    // ========================================

    AnnotationGetFile,
    AnnotationGetName,
    AnnotationGetInterface,
    AnnotationIsExternal,
    AnnotationEnumerateElements,
    AnnotationElementGetFile,
    AnnotationElementGetAnnotation,
    AnnotationElementGetName,
    AnnotationElementGetValue,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceGetFile,
    AnnotationInterfaceGetModule,
    AnnotationInterfaceGetParentNamespace,
    AnnotationInterfaceGetName,
    AnnotationInterfaceEnumerateFields,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    AnnotationInterfaceFieldGetFile,
    AnnotationInterfaceFieldGetInterface,
    AnnotationInterfaceFieldGetName,
    AnnotationInterfaceFieldGetType,
    AnnotationInterfaceFieldGetDefaultValue,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::metadata_inspect

AbckitInspectApi const *AbckitGetMockInspectApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::metadata_inspect::g_inspectApiImpl;
}

#endif  // ABCKIT_INSPECT_IMPL_MOCK
