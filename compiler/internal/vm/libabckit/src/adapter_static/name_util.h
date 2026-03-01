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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_NAME_UTIL_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_NAME_UTIL_H

#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit {

class NameUtil {
public:
    static std::string GetName(const std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *,
                                                  AbckitCoreInterface *, AbckitCoreEnum *> &object);

    static std::string GetPackageName(const std::variant<AbckitCoreNamespace *, AbckitCoreFunction *> &object);

    static std::string GetFullName(
        const std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreClassField *, AbckitCoreEnum *,
                           AbckitCoreInterface *, AbckitCoreAnnotationInterface *, AbckitCoreFunction *,
                           AbckitCoreAnnotation *> &object,
        const std::string &newName = "", bool isObjectLiteral = false, bool isPartial = false);

    static std::string GetFieldFullName(const ark::pandasm::Record *owner, const ark::pandasm::Field *field);

private:
    static std::string NamespaceGetPackageName(AbckitCoreNamespace *ns);

    static std::string FunctionGetPackageName(AbckitCoreFunction *function);

    static std::string NamespaceGetFullName(AbckitCoreNamespace *ns, const std::string &newName = "");

    static std::string ClassGetFullName(AbckitCoreClass *klass, const std::string &newName = "");

    static std::string ClassFieldGetFullName(AbckitCoreClassField *field, const std::string &newName = "");

    static std::string EnumGetFullName(AbckitCoreEnum *enm, const std::string &newName = "");

    static std::string InterfaceGetFullName(AbckitCoreInterface *iface, const std::string &newName = "");

    static std::string AnnotationInterfaceGetFullName(AbckitCoreAnnotationInterface *ai,
                                                      const std::string &newName = "");

    static std::string FunctionGetFullName(AbckitCoreFunction *function);

    static std::string AnnotationGetFullName(AbckitCoreAnnotation *anno);

    static std::string ObjectLiteralGetFullName(const AbckitCoreClass *objectLiteral, const std::string &newName = "");

    static std::string PartialGetFullName(const std::string &newName = "");
};
}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_STATIC_NAME_UTIL_H
