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

#ifndef LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H
#define LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H

#include "libabckit/c/metadata_core.h"

namespace libabckit {

// ========================================
// Module
// ========================================

bool ArkTSModuleEnumerateImports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreImportDescriptor *i, void *data));
bool ArkTSModuleEnumerateExports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreExportDescriptor *e, void *data));
bool ArkTSModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data));
bool ArkTSModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data));
bool ArkTSModuleEnumerateInterfaces(AbckitCoreModule *m, void *data,
                                    bool (*cb)(AbckitCoreInterface *iface, void *data));
bool ArkTSModuleEnumerateEnums(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));
bool ArkTSModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreFunction *function, void *data));
bool ArkTSModuleEnumerateFields(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreModuleField *field, void *data));
bool ArkTSModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data));
bool ArkTSModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

// ========================================
// Namespace
// ========================================

bool ArkTSNamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreNamespace *klass, void *data));
bool ArkTSNamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));
bool ArkTSNamespaceEnumerateInterfaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreInterface *iface, void *data));
bool ArkTSNamespaceEnumerateEnums(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));
bool ArkTSNamespaceEnumerateFields(AbckitCoreNamespace *n, void *data,
                                   bool (*cb)(AbckitCoreNamespaceField *field, void *data));
bool ArkTSNamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                              bool (*cb)(AbckitCoreFunction *func, void *data));
bool ArkTSNamespaceEnumerateAnnotationInterfaces(AbckitCoreNamespace *n, void *data,
                                                 bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

// ========================================
// Class
// ========================================

bool ArkTSClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));
bool ArkTSClassEnumerateFields(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreClassField *field, void *data));
bool ArkTSClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data));
bool ArkTSClassEnumerateSubClasses(AbckitCoreClass *klass, void *data,
                                   bool (*cb)(AbckitCoreClass *subClass, void *data));
bool ArkTSClassEnumerateInterfaces(AbckitCoreClass *klass, void *data,
                                   bool (*cb)(AbckitCoreInterface *iface, void *data));

// ========================================
// Interface
// ========================================

bool ArkTSInterfaceEnumerateMethods(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreFunction *method, void *data));
bool ArkTSInterfaceEnumerateFields(AbckitCoreInterface *iface, void *data,
                                   bool (*cb)(AbckitCoreInterfaceField *field, void *data));
bool ArkTSInterfaceEnumerateSuperInterfaces(AbckitCoreInterface *iface, void *data,
                                            bool (*cb)(AbckitCoreInterface *superInterface, void *data));
bool ArkTSInterfaceEnumerateSubInterfaces(AbckitCoreInterface *iface, void *data,
                                          bool (*cb)(AbckitCoreInterface *subInterface, void *data));
bool ArkTSInterfaceEnumerateClasses(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreClass *klass, void *data));
bool ArkTSInterfaceEnumerateAnnotations(AbckitCoreInterface *iface, void *data,
                                        bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Enum
// ========================================

bool ArkTSEnumEnumerateMethods(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));
bool ArkTSEnumEnumerateFields(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data));

// ========================================
// Field
// ========================================

bool ArkTSClassFieldEnumerateAnnotations(AbckitCoreClassField *field, void *data,
                                         bool (*cb)(AbckitCoreAnnotation *anno, void *data));
bool ArkTSInterfaceFieldEnumerateAnnotations(AbckitCoreInterfaceField *field, void *data,
                                             bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Function
// ========================================

bool ArkTSFunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                           bool (*cb)(AbckitCoreFunction *nestedFunc, void *data));
bool ArkTSFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreClass *nestedClass, void *data));
bool ArkTSFunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                       bool (*cb)(AbckitCoreAnnotation *anno, void *data));
bool ArkTSFunctionEnumerateParameters(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreFunctionParam *param, void *data));

// ========================================
// Annotation
// ========================================

bool ArkTSAnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                      bool (*cb)(AbckitCoreAnnotationElement *ae, void *data));

// ========================================
// AnnotationInterface
// ========================================

bool ArkTSAnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                             bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data));

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H
