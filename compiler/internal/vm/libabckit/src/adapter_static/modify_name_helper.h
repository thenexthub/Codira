/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_MODIFY_NAME_HELPER_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_MODIFY_NAME_HELPER_H

#include <string>
#include <variant>

#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit {

class ModifyNameHelper {
public:
    static bool ModuleRefreshName(AbckitCoreModule *m, const std::string &newName);
    static bool NamespaceRefreshName(AbckitCoreNamespace *ns, const std::string &newName = "");
    static bool FunctionRefreshName(AbckitCoreFunction *function, const std::string &newName = "");
    static bool ClassRefreshName(AbckitCoreClass *klass, const std::string &newName = "");
    static bool EnumRefreshName(AbckitCoreEnum *enm, const std::string &newName = "");
    static bool AnnotationRefreshName(AbckitCoreAnnotation *anno, const std::string &newName = "");
    static bool InterfaceRefreshName(AbckitCoreInterface *iface, const std::string &newName = "");
    static bool AnnotationInterfaceRefreshName(AbckitCoreAnnotationInterface *ai, const std::string &newName = "");
    static bool FieldRefreshName(
        std::variant<AbckitCoreModuleField *, AbckitCoreNamespaceField *, AbckitCoreClassField *, AbckitCoreEnumField *>
            field,
        const std::string &newName);
    static bool InterfaceFieldRefreshName(AbckitCoreInterfaceField *ifaceField, const std::string &newName = "");

private:
    static bool ModuleRefreshNamespaces(AbckitCoreModule *m);
    static bool ModuleRefreshFunctions(AbckitCoreModule *m);
    static bool ModuleRefreshClasses(AbckitCoreModule *m);
    static bool ModuleRefreshInterfaces(AbckitCoreModule *m);
    static bool ModuleRefreshEnums(AbckitCoreModule *m);
    static bool ModuleRefreshAnnotationInterfaces(AbckitCoreModule *m);

    static bool NamespaceRefreshNamespaces(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshFunctions(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshClasses(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshInterfaces(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshEnums(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshAnnotationInterfaces(AbckitCoreNamespace *ns);
    static bool NamespaceRefreshAnnotationElements(AbckitCoreNamespace *ns);

    static bool FunctionRefreshParams(AbckitCoreFunction *function);
    static bool FunctionRefreshReturnType(AbckitCoreFunction *function);
    static bool FunctionRefreshAnnotations(AbckitCoreFunction *function, const std::string &oldName,
                                           const std::string &newName);
    static void GenerateNewFunctionNames(AbckitCoreFunction *function, const std::string &newName,
                                         const std::string &oldFunctionKeyName, std::string &newFunctionKeyName,
                                         std::string &newFunctionName);

    static bool ClassRefreshMethods(AbckitCoreClass *klass);
    static bool ClassRefreshObjectLiteral(AbckitCoreClass *klass);
    static bool ClassRefreshSubClasses(AbckitCoreClass *klass);
    static bool ClassRefreshAnnotations(AbckitCoreClass *klass, const std::string &oldName, const std::string &newName);

    static bool EnumRefreshMethods(AbckitCoreEnum *enm);
    static bool EnumRefreshFieldsType(AbckitCoreEnum *enm);

    static bool InterfaceRefreshMethods(AbckitCoreInterface *iface);
    static bool InterfaceRefreshObjectLiteral(AbckitCoreInterface *iface);
    static bool InterfaceRefreshClasses(AbckitCoreInterface *iface);
    static bool InterfaceRefreshAnnotations(AbckitCoreInterface *iface, const std::string &oldName,
                                            const std::string &newName);
    static bool InterfaceRefreshSubInterfaces(AbckitCoreInterface *iface);

    static bool AnnotationInterfaceRefreshAnnotation(AbckitCoreAnnotationInterface *ai);
    static bool RefreshPartial(const std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> &object);
    static bool RefreshArray(const std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> &object);
    static bool ArrayRefreshName(AbckitCoreClass *array, const std::string &newName);

    static bool ObjectLiteralRefreshName(AbckitCoreClass *objectLiteral, const std::string &newName = "");
    static bool PartialRefreshName(AbckitCoreClass *partial, const std::string &newName = "");

    static bool ClassFieldRefreshAnnotations(AbckitCoreClassField *classField, const std::string &oldName,
                                             const std::string &newName);
    static bool GetSetMethodRefreshName(const std::variant<AbckitCoreClass *, AbckitCoreInterface *> &object,
                                        const std::string &oldName, const std::string &newName);
    static bool InterfaceRefreshObjectLiteralField(AbckitCoreInterface *iface, const std::string &oldName,
                                                   const std::string &newName);

    static bool FieldRefreshTypeName(
        std::variant<AbckitCoreModuleField *, AbckitCoreNamespaceField *, AbckitCoreClassField *, AbckitCoreEnumField *,
                     AbckitCoreAnnotationInterfaceField *>
            field);

    static bool ObjectRefreshTypeUsersName(
        std::variant<AbckitCoreClass *, AbckitCoreEnum *, AbckitCoreInterface *> object, const std::string &newName);
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_STATIC_MODIFY_NAME_HELPER_H