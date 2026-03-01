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

#include "name_util.h"

#include <regex>

#include "helpers_static.h"
#include "metadata_inspect_static.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"

namespace {
constexpr std::string_view OBJECT_LITERAL_SUFFIX = "$ObjectLiteral";
constexpr std::string_view PARTIAL_PREFIX = "%%partial-";
constexpr std::string_view NAME_DELIMITER = ".";
constexpr std::string_view GLOBAL_CLASS = "ETSGLOBAL";
}  // namespace

std::string libabckit::NameUtil::GetName(
    const std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                       AbckitCoreEnum *> &object)
{
    if (const auto coreModule = std::get_if<AbckitCoreModule *>(&object)) {
        return (*coreModule)->moduleName->impl.data();
    }

    if (const auto coreNameSpace = std::get_if<AbckitCoreNamespace *>(&object)) {
        return NamespaceGetNameStatic(*coreNameSpace)->impl.data();
    }

    if (const auto coreClass = std::get_if<AbckitCoreClass *>(&object)) {
        return ClassGetNameStatic(*coreClass)->impl.data();
    }

    if (const auto coreEnum = std::get_if<AbckitCoreEnum *>(&object)) {
        return EnumGetNameStatic(*coreEnum)->impl.data();
    }

    if (const auto coreInterface = std::get_if<AbckitCoreInterface *>(&object)) {
        return InterfaceGetNameStatic(*coreInterface)->impl.data();
    }

    ASSERT(false);
    return "";
}

std::string libabckit::NameUtil::GetPackageName(const std::variant<AbckitCoreNamespace *, AbckitCoreFunction *> &object)
{
    if (const auto coreNameSpace = std::get_if<AbckitCoreNamespace *>(&object)) {
        return NamespaceGetPackageName(*coreNameSpace);
    }

    if (const auto coreFunction = std::get_if<AbckitCoreFunction *>(&object)) {
        return FunctionGetPackageName(*coreFunction);
    }

    ASSERT(false);
    return "";
}

std::string libabckit::NameUtil::GetFullName(
    const std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreClassField *, AbckitCoreEnum *,
                       AbckitCoreInterface *, AbckitCoreAnnotationInterface *, AbckitCoreFunction *,
                       AbckitCoreAnnotation *> &object,
    const std::string &newName, bool isObjectLiteral, bool isPartial)
{
    if (const auto coreNameSpace = std::get_if<AbckitCoreNamespace *>(&object)) {
        return NamespaceGetFullName(*coreNameSpace, newName);
    }

    if (const auto coreClass = std::get_if<AbckitCoreClass *>(&object)) {
        if (isObjectLiteral) {
            return ObjectLiteralGetFullName(*coreClass, newName);
        }
        if (isPartial) {
            return PartialGetFullName(newName);
        }
        return ClassGetFullName(*coreClass, newName);
    }

    if (const auto coreClassField = std::get_if<AbckitCoreClassField *>(&object)) {
        return ClassFieldGetFullName(*coreClassField, newName);
    }

    if (const auto coreEnum = std::get_if<AbckitCoreEnum *>(&object)) {
        return EnumGetFullName(*coreEnum, newName);
    }

    if (const auto coreInterface = std::get_if<AbckitCoreInterface *>(&object)) {
        return InterfaceGetFullName(*coreInterface, newName);
    }

    if (const auto coreAnnoInterface = std::get_if<AbckitCoreAnnotationInterface *>(&object)) {
        return AnnotationInterfaceGetFullName(*coreAnnoInterface, newName);
    }

    if (const auto coreFunction = std::get_if<AbckitCoreFunction *>(&object)) {
        return FunctionGetFullName(*coreFunction);
    }

    if (const auto coreAnno = std::get_if<AbckitCoreAnnotation *>(&object)) {
        return AnnotationGetFullName(*coreAnno);
    }

    ASSERT(false);
    return "";
}

std::string libabckit::NameUtil::GetFieldFullName(const ark::pandasm::Record *owner, const ark::pandasm::Field *field)
{
    return owner->name + NAME_DELIMITER.data() + field->name;
}

std::string libabckit::NameUtil::ObjectLiteralGetFullName(const AbckitCoreClass *objectLiteral,
                                                          const std::string &newName)
{
    auto objectLiteralName = std::regex_replace(newName, std::regex(R"(\.)"), "$") + OBJECT_LITERAL_SUFFIX.data();
    return std::string {objectLiteral->owningModule->moduleName->impl} + NAME_DELIMITER.data() + objectLiteralName;
}

std::string libabckit::NameUtil::PartialGetFullName(const std::string &newName)
{
    auto [moduleName, className] = ClassGetNames(newName);
    return moduleName + NAME_DELIMITER.data() + std::string(PARTIAL_PREFIX) + className;
}

std::string libabckit::NameUtil::NamespaceGetPackageName(AbckitCoreNamespace *ns)
{
    if (ns->parentNamespace != nullptr) {
        return NamespaceGetFullName(ns->parentNamespace);
    }

    return std::string(ns->owningModule->moduleName->impl);
}

std::string libabckit::NameUtil::FunctionGetPackageName(AbckitCoreFunction *function)
{
    if (const auto klass = std::get_if<AbckitCoreClass *>(&function->parent)) {
        return ClassGetFullName(*klass);
    }

    if (const auto iface = std::get_if<AbckitCoreInterface *>(&function->parent)) {
        return InterfaceGetFullName(*iface);
    }

    if (const auto enm = std::get_if<AbckitCoreEnum *>(&function->parent)) {
        return EnumGetFullName(*enm);
    }

    if (const auto ns = std::get_if<AbckitCoreNamespace *>(&function->parent)) {
        return NamespaceGetFullName(*ns);
    }

    auto packageName = std::string(function->owningModule->moduleName->impl);
    const auto funcImpl = function->GetArkTSImpl()->GetStaticImpl();
    const auto fullName = ark::pandasm::MangleFunctionName(funcImpl->name, funcImpl->params, funcImpl->returnType);
    if (fullName.find(GLOBAL_CLASS) != std::string::npos) {
        packageName.append(NAME_DELIMITER).append(GLOBAL_CLASS);
    }
    return packageName;
}

std::string libabckit::NameUtil::NamespaceGetFullName(AbckitCoreNamespace *ns, const std::string &newName)
{
    const auto nsName = newName.empty() ? GetName(ns) : newName;
    if (ns->parentNamespace != nullptr) {
        return NamespaceGetFullName(ns->parentNamespace) + NAME_DELIMITER.data() + nsName;
    }

    return std::string(ns->owningModule->moduleName->impl) + NAME_DELIMITER.data() + nsName;
}

std::string libabckit::NameUtil::ClassGetFullName(AbckitCoreClass *klass, const std::string &newName)
{
    const auto className = newName.empty() ? GetName(klass) : newName;
    if (klass->parentNamespace != nullptr) {
        return NamespaceGetFullName(klass->parentNamespace) + NAME_DELIMITER.data() + className;
    }

    return std::string(klass->owningModule->moduleName->impl) + NAME_DELIMITER.data() + className;
}

std::string libabckit::NameUtil::ClassFieldGetFullName(AbckitCoreClassField *field, const std::string &newName)
{
    const auto fieldName = newName.empty() ? ClassFieldGetNameStatic(field)->impl.data() : newName;
    return ClassGetFullName(field->owner) + NAME_DELIMITER.data() + fieldName;
}

std::string libabckit::NameUtil::EnumGetFullName(AbckitCoreEnum *enm, const std::string &newName)
{
    const auto enumName = newName.empty() ? EnumGetNameStatic(enm)->impl.data() : newName;
    if (enm->parentNamespace != nullptr) {
        return NamespaceGetFullName(enm->parentNamespace) + NAME_DELIMITER.data() + enumName;
    }

    return std::string(enm->owningModule->moduleName->impl) + NAME_DELIMITER.data() + enumName;
}

std::string libabckit::NameUtil::InterfaceGetFullName(AbckitCoreInterface *iface, const std::string &newName)
{
    const auto ifaceName = newName.empty() ? InterfaceGetNameStatic(iface)->impl.data() : newName;
    if (iface->parentNamespace != nullptr) {
        return NamespaceGetFullName(iface->parentNamespace) + NAME_DELIMITER.data() + ifaceName;
    }

    return std::string(iface->owningModule->moduleName->impl) + NAME_DELIMITER.data() + ifaceName;
}

std::string libabckit::NameUtil::AnnotationInterfaceGetFullName(AbckitCoreAnnotationInterface *ai,
                                                                const std::string &newName)
{
    const auto aiName = newName.empty() ? AnnotationInterfaceGetNameStatic(ai)->impl.data() : newName;
    if (ai->parentNamespace != nullptr) {
        return NamespaceGetFullName(ai->parentNamespace) + NAME_DELIMITER.data() + aiName;
    }

    return std::string(ai->owningModule->moduleName->impl) + NAME_DELIMITER.data() + aiName;
}

std::string libabckit::NameUtil::FunctionGetFullName(AbckitCoreFunction *function)
{
    return FunctionGetPackageName(function) + NAME_DELIMITER.data() + FunctionGetNameStatic(function)->impl.data();
}

std::string libabckit::NameUtil::AnnotationGetFullName(AbckitCoreAnnotation *anno)
{
    if (anno->ai != nullptr) {
        return AnnotationInterfaceGetFullName(anno->ai);
    }
    return anno->name->impl.data();
}
