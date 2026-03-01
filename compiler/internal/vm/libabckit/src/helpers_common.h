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

#ifndef LIBABCKIT_SRC_HELPERS_COMMON_H
#define LIBABCKIT_SRC_HELPERS_COMMON_H

#include <variant>
#include "metadata_inspect_impl.h"

namespace libabckit {

bool ModuleEnumerateNamespacesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *ns, void *data));
bool ModuleEnumerateClassesHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));
bool ModuleEnumerateInterfacesHelper(AbckitCoreModule *m, void *data,
                                     bool (*cb)(AbckitCoreInterface *iface, void *data));
bool ModuleEnumerateEnumsHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));
bool ModuleEnumerateTopLevelFunctionsHelper(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data));
bool ModuleEnumerateFieldsHelper(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreModuleField *field, void *data));
bool ModuleEnumerateAnnotationInterfacesHelper(AbckitCoreModule *m, void *data,
                                               bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

bool NamespaceEnumerateNamespacesHelper(AbckitCoreNamespace *n, void *data,
                                        bool (*cb)(AbckitCoreNamespace *ns, void *data));
bool NamespaceEnumerateClassesHelper(AbckitCoreNamespace *n, void *data,
                                     bool (*cb)(AbckitCoreClass *klass, void *data));
bool NamespaceEnumerateInterfacesHelper(AbckitCoreNamespace *n, void *data,
                                        bool (*cb)(AbckitCoreInterface *iface, void *data));
bool NamespaceEnumerateEnumsHelper(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));
bool NamespaceEnumerateTopLevelFunctionsHelper(AbckitCoreNamespace *n, void *data,
                                               bool (*cb)(AbckitCoreFunction *function, void *data));
bool NamespaceEnumerateFieldsHelper(AbckitCoreNamespace *n, void *data,
                                    bool (*cb)(AbckitCoreNamespaceField *field, void *data));
bool NamespaceEnumerateAnnotationInterfacesHelper(AbckitCoreNamespace *n, void *data,
                                                  bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

bool ClassEnumerateMethodsHelper(AbckitCoreClass *klass, void *data,
                                 bool (*cb)(AbckitCoreFunction *method, void *data));
bool ClassEnumerateFieldsHelper(AbckitCoreClass *klass, void *data,
                                bool (*cb)(AbckitCoreClassField *field, void *data));
bool ClassEnumerateSubClassesHelper(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreClass *subClass, void *data));
bool ClassEnumerateInterfacesHelper(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreInterface *iface, void *data));

bool InterfaceEnumerateMethodsHelper(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreFunction *method, void *data));
bool InterfaceEnumerateFieldsHelper(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreInterfaceField *field, void *data));
bool InterfaceEnumerateSuperInterfacesHelper(AbckitCoreInterface *iface, void *data,
                                             bool (*cb)(AbckitCoreInterface *superInterface, void *data));
bool InterfaceEnumerateSubInterfacesHelper(AbckitCoreInterface *iface, void *data,
                                           bool (*cb)(AbckitCoreInterface *subInterface, void *data));
bool InterfaceEnumerateClassesHelper(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreClass *klass, void *data));
bool InterfaceEnumerateAnnotationsHelper(AbckitCoreInterface *iface, void *data,
                                         bool (*cb)(AbckitCoreAnnotation *anno, void *data));

bool EnumEnumerateMethodsHelper(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));
bool EnumEnumerateFieldsHelper(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data));

bool ClassFieldEnumerateAnnotationsHelper(AbckitCoreClassField *field, void *data,
                                          bool (*cb)(AbckitCoreAnnotation *anno, void *data));
bool InterfaceFieldEnumerateAnnotationsHelper(AbckitCoreInterfaceField *field, void *data,
                                              bool (*cb)(AbckitCoreAnnotation *anno, void *data));

bool IsDynamic(AbckitTarget target);

AbckitType *GetOrCreateType(
    AbckitFile *file, AbckitTypeId id, size_t rank,
    std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> reference,
    AbckitString *name);

void TypeSetNameHelper(AbckitType *type, const char *name, size_t len);

void TypeSetRankHelper(AbckitType *type, size_t rank);

void AddFunctionUserToAbckitType(AbckitType *abckitType, AbckitCoreFunction *function);

void AddFieldUserToAbckitType(AbckitType *abckitType,
                              std::variant<AbckitCoreModuleField *, AbckitCoreNamespaceField *, AbckitCoreClassField *,
                                           AbckitCoreEnumField *, AbckitCoreAnnotationInterfaceField *>
                                  field);

AbckitCoreModule *GetAnnotationOwningModule(const AbckitCoreAnnotation *annotation);

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_HELPERS_COMMON_H
