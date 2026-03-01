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

#include "name_marker.h"

#include "logger.h"
#include "obfuscator_data_manager.h"
#include "util/string_util.h"

#include <ostream>

namespace {
bool IsKeepAllMembers(const ark::guard::MemberSpecification &spec)
{
    return spec.keepMember_ && spec.annotationName_.empty() && spec.setAccessFlags_ == 0 && spec.unSetAccessFlags_ == 0;
}

bool MatchAnnotationName(const std::string &configAnnotationName, const std::vector<std::string> &annotationNames)
{
    if (configAnnotationName.empty()) {
        return true;
    }
    return std::any_of(annotationNames.begin(), annotationNames.end(),
                       [&](const auto &name) { return ark::guard::StringUtil::IsMatch(name, configAnnotationName); });
}

bool MatchAccessFlags(uint32_t accessFlag, uint32_t setAccessFlag, uint32_t unSetAccessFlag)
{
    // All set flag must be present
    if (setAccessFlag != 0 && (accessFlag & setAccessFlag) != setAccessFlag) {
        return false;
    }
    // unset flag may be present
    if (unSetAccessFlag != 0 && (accessFlag & unSetAccessFlag) != 0) {
        return false;
    }
    return true;
}

bool MatchCommonAttributes(const std::vector<std::string> &targetAnnotationNames, const std::string &specAnnotationName,
                           uint32_t targetAccessFlags, uint32_t setAccessFlags, uint32_t unSetAccessFlags)
{
    if (!specAnnotationName.empty() && !MatchAnnotationName(specAnnotationName, targetAnnotationNames)) {
        return false;
    }

    if (!MatchAccessFlags(targetAccessFlags, setAccessFlags, unSetAccessFlags)) {
        return false;
    }
    return true;
}

void CollectInterfaceFieldGetterSetterMethods(abckit_wrapper::Member *member,
                                              std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers)
{
    auto *field = static_cast<abckit_wrapper::Field *>(member);
    if (!field || !field->IsInterfaceField()) {
        return;
    }
    if (!field->parent_.has_value()) {
        return;
    }
    auto *clazz = static_cast<abckit_wrapper::Class *>(field->parent_.value());
    if (!clazz) {
        return;
    }

    auto fieldName = field->GetRawName();
    for (const auto &[methodFullName, method] : clazz->methodTable_) {
        if (method->GetRawName() != fieldName) {
            continue;
        }
        matchedMembers.insert({methodFullName, method});
    }
}

template <typename MemberType, typename Matcher>
bool CollectMatchedMembers(const std::unordered_map<std::string, MemberType *> &memberTable,
                           const std::vector<ark::guard::MemberSpecification> &specifications, Matcher matcher,
                           std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers)
{
    if (specifications.empty()) {
        return false;
    }

    bool hasKeepAllMember = std::any_of(specifications.begin(), specifications.end(),
                                        [](const auto &spec) { return IsKeepAllMembers(spec); });
    if (hasKeepAllMember) {
        for (const auto &[memberName, member] : memberTable) {
            if (matchedMembers.find(memberName) != matchedMembers.end()) {
                continue;
            }
            matchedMembers.insert({memberName, member});
            if constexpr (std::is_same_v<MemberType, abckit_wrapper::Field>) {
                CollectInterfaceFieldGetterSetterMethods(member, matchedMembers);
            }
        }
        return true;
    }

    std::vector<bool> matchedFlags(specifications.size(), false);
    for (const auto &[memberName, member] : memberTable) {
        if (matchedMembers.find(memberName) != matchedMembers.end()) {
            continue;
        }
        for (size_t index = 0; index < specifications.size(); ++index) {
            if (matcher(member, specifications[index])) {
                matchedMembers.insert({memberName, member});
                CollectInterfaceFieldGetterSetterMethods(member, matchedMembers);
                matchedFlags[index] = true;
            }
        }
    }

    return std::any_of(matchedFlags.begin(), matchedFlags.end(), [](bool matched) { return matched; });
}

bool IsKeepAll(const ark::guard::ClassSpecification &specification)
{
    bool keepAllField =
        std::any_of(specification.fieldSpecifications_.begin(), specification.fieldSpecifications_.end(),
                    [](const auto &spec) { return IsKeepAllMembers(spec); });
    bool keepAllMethod =
        std::any_of(specification.methodSpecifications_.begin(), specification.methodSpecifications_.end(),
                    [](const auto &spec) { return IsKeepAllMembers(spec); });
    return keepAllField && keepAllMethod;
}

void ResetKeptFlag(bool &keepFlag)
{
    if (keepFlag) {
        keepFlag = false;
    }
}

bool MatchTypeDeclarations(uint32_t typeDeclarations, uint32_t setTypeDeclarations, uint32_t unSetTypeDeclarations)
{
    if (setTypeDeclarations != 0 && (typeDeclarations & setTypeDeclarations) != setTypeDeclarations) {
        return false;
    }
    if (unSetTypeDeclarations != 0 && (typeDeclarations & unSetTypeDeclarations) != 0) {
        return false;
    }
    return true;
}
}  // namespace

bool ark::guard::NameMarker::Execute(abckit_wrapper::FileView &fileView)
{
    fileView.ModulesAccept(this->Wrap<abckit_wrapper::LocalModuleFilter>());
    return true;
}

bool ark::guard::NameMarker::NameMarker::Visit(abckit_wrapper::Module *module)
{
    MarkModuleKept(module);

    if (!configuration_.GetClassSpecifications().empty()) {
        module->ChildrenAccept(*this);
    }

    ResetKeptFlag(keepModuleAllChild_);

    return true;
}

bool ark::guard::NameMarker::NameMarker::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    if (keepModuleAllChild_ || keepNamespaceAllChild_) {
        ObfuscatorDataManager manager(ns);
        auto obfuscateData = manager.GetData();
        if (obfuscateData == nullptr) {
            LOG_E << "Get namespace obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
            ResetKeptFlag(keepNamespaceAllChild_);
            return false;
        }
        obfuscateData->SetKept();
        LOG_I << "keep namespace:" << ns->GetFullyQualifiedName();
    } else {
        MarkNamespaceKept(ns);
    }

    ns->ChildrenAccept(*this);

    ResetKeptFlag(keepNamespaceAllChild_);

    return true;
}

bool ark::guard::NameMarker::NameMarker::VisitMethod(abckit_wrapper::Method *method)
{
    if (method->IsConstructor()) {
        return true;
    }

    if (keepModuleAllChild_ || keepNamespaceAllChild_) {
        ObfuscatorDataManager::SetKeptWithMemberLink(method);
        LOG_I << "keep method:" << method->GetFullyQualifiedName();
        return true;
    }

    uint32_t typeDeclarations = abckit_wrapper::TypeDeclarations::NAMESPACE;
    const auto parent = method->parent_.value();
    if (parent == method->owningModule_.value()) {
        typeDeclarations = abckit_wrapper::TypeDeclarations::PACKAGE;
    }
    const auto matchedSpecs = CollectNameMatchedSpecification(parent->GetFullyQualifiedName(), typeDeclarations);
    if (matchedSpecs.empty()) {
        return true;
    }

    for (const auto &spec : matchedSpecs) {
        for (const auto &methodSpec : spec.methodSpecifications_) {
            if (MatchMethodSpecification(method, methodSpec)) {
                ObfuscatorDataManager::SetKeptWithMemberLink(method);
                LOG_I << "keep method:" << method->GetFullyQualifiedName();
                return true;
            }
        }
    }

    return true;
}

bool ark::guard::NameMarker::NameMarker::VisitField(abckit_wrapper::Field *field)
{
    if (keepModuleAllChild_ || keepNamespaceAllChild_) {
        ObfuscatorDataManager::SetKeptWithMemberLink(field);
        LOG_I << "keep field:" << field->GetFullyQualifiedName();
        return true;
    }

    const auto parent = field->parent_.value();
    uint32_t typeDeclarations = abckit_wrapper::TypeDeclarations::PACKAGE | abckit_wrapper::TypeDeclarations::NAMESPACE;
    const auto matchedSpecs = CollectNameMatchedSpecification(parent->GetFullyQualifiedName(), typeDeclarations);
    if (matchedSpecs.empty()) {
        return true;
    }

    for (const auto &spec : matchedSpecs) {
        for (const auto &fieldSpec : spec.fieldSpecifications_) {
            if (MatchFieldSpecification(field, fieldSpec)) {
                ObfuscatorDataManager::SetKeptWithMemberLink(field);
                LOG_I << "keep field:" << field->GetFullyQualifiedName();
                return true;
            }
        }
    }
    return true;
}

bool ark::guard::NameMarker::NameMarker::VisitClass(abckit_wrapper::Class *clazz)
{
    if (MarkClassKept(clazz)) {
        return true;
    }

    const auto matchedSpecs = CollectMatchedClassSpecification(clazz);
    if (matchedSpecs.empty()) {
        return true;
    }

    ObfuscatorDataManager classManger(clazz);
    auto classObfuscateData = classManger.GetData();
    if (classObfuscateData == nullptr) {
        LOG_E << "Get class obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }

    for (const auto &spec : matchedSpecs) {
        std::unordered_map<std::string, abckit_wrapper::Member *> matchedMembers;
        auto collectMethods = CollectMatchedMethods(clazz->methodTable_, spec.methodSpecifications_, matchedMembers);
        auto collectFields = CollectMatchedFields(clazz->fieldTable_, spec.fieldSpecifications_, matchedMembers);
        if (!collectMethods && !collectFields) {
            continue;
        }

        if (!classObfuscateData->IsKept() && spec.keepClassName_) {
            classObfuscateData->SetKept();
            LOG_I << "keep class:" << clazz->GetFullyQualifiedName();
        }
        for (auto &[matchedMemberName, matchedMember] : matchedMembers) {
            ObfuscatorDataManager::SetKeptWithMemberLink(matchedMember);
            LOG_I << "keep class member:" << matchedMemberName;
        }
    }
    return true;
}

bool ark::guard::NameMarker::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    auto name = ai->GetFullyQualifiedName();
    ObfuscatorDataManager manager(ai);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get annotation obfuscate data failed, annotation:" << ai->GetFullyQualifiedName();
        return false;
    }
    if (keepModuleAllChild_ || keepNamespaceAllChild_) {
        obfuscateData->SetKept();
        LOG_I << "keep annotation:" << ai->GetFullyQualifiedName();
        return true;
    }

    const auto matchedSpecs =
        CollectNameMatchedSpecification(ai->GetFullyQualifiedName(), abckit_wrapper::TypeDeclarations::INTERFACE);
    if (matchedSpecs.empty()) {
        return true;
    }
    for (const auto &spec : matchedSpecs) {
        if (!obfuscateData->IsKept() && spec.keepClassName_) {
            obfuscateData->SetKept();
            LOG_I << "keep annotation:" << ai->GetFullyQualifiedName();
            return true;
        }
    }
    return true;
}

std::vector<ark::guard::ClassSpecification> ark::guard::NameMarker::CollectMatchedClassSpecification(
    const abckit_wrapper::Class *clazz)
{
    std::vector<ClassSpecification> matchedSpecs;
    const auto classSpecifications = configuration_.GetClassSpecifications();
    for (const auto &spec : classSpecifications) {
        if (MatchClassSpecification(clazz, spec)) {
            matchedSpecs.emplace_back(spec);
        }
    }
    return matchedSpecs;
}

bool ark::guard::NameMarker::MatchClassSpecification(const abckit_wrapper::Class *clazz, const ClassSpecification &spec)
{
    uint32_t typeDeclarations = 0;
    if (clazz->IsInterface()) {
        typeDeclarations = abckit_wrapper::TypeDeclarations::INTERFACE;
    } else if (clazz->IsClass()) {
        typeDeclarations = abckit_wrapper::TypeDeclarations::CLASS;
    } else if (clazz->IsEnum()) {
        typeDeclarations = abckit_wrapper::TypeDeclarations::ENUM;
    }
    if (!MatchTypeDeclarations(typeDeclarations, spec.setTypeDeclarations_, spec.unSetTypeDeclarations_)) {
        return false;
    }

    if (!MatchCommonAttributes(clazz->GetAnnotationNames(), spec.annotationName_, clazz->GetAccessFlags(),
                               spec.setAccessFlags_, spec.unSetAccessFlags_)) {
        return false;
    }

    if (!spec.className_.empty() && !StringUtil::IsMatch(clazz->GetFullyQualifiedName(), spec.className_)) {
        return false;
    }

    auto effectiveExtensionType = spec.extensionType_;
    if (clazz->IsInterface() && spec.extensionType_ == abckit_wrapper::EXTENDS) {
        effectiveExtensionType = abckit_wrapper::IMPLEMENTS;
    }

    bool isExtensionMatch = true;
    switch (effectiveExtensionType) {
        case abckit_wrapper::EXTENDS:
            isExtensionMatch = MatchExtends(clazz, spec);
            break;
        case abckit_wrapper::IMPLEMENTS:
            isExtensionMatch = MatchImplements(clazz, spec);
            break;
        default:
            break;
    }
    return isExtensionMatch;
}

bool ark::guard::NameMarker::MatchExtends(const abckit_wrapper::Class *clazz, const ClassSpecification &spec)
{
    if (!clazz->superClass_.has_value()) {
        return false;
    }

    const auto extendsClass = clazz->superClass_.value();
    if (!spec.extensionAnnotationName_.empty() &&
        !MatchAnnotationName(spec.extensionAnnotationName_, extendsClass->GetAnnotationNames())) {
        return false;
    }

    if (!spec.extensionClassName_.empty() &&
        !StringUtil::IsMatch(extendsClass->GetFullyQualifiedName(), spec.extensionClassName_)) {
        return false;
    }

    return true;
}

bool ark::guard::NameMarker::MatchImplements(const abckit_wrapper::Class *clazz, const ClassSpecification &spec)
{
    for (const auto &[_, interface] : clazz->interfaces_) {
        bool annoMatch = spec.extensionAnnotationName_.empty() ||
                         MatchAnnotationName(spec.extensionAnnotationName_, interface->GetAnnotationNames());

        bool nameMatch = spec.extensionClassName_.empty() ||
                         StringUtil::IsMatch(interface->GetFullyQualifiedName(), spec.extensionClassName_);
        if (annoMatch && nameMatch) {
            return true;
        }
    }

    return false;
}

bool ark::guard::NameMarker::MatchMethodSpecification(abckit_wrapper::Method *method, const MemberSpecification &spec)
{
    if (method->IsConstructor()) {
        return false;
    }

    if (!MatchCommonAttributes(method->GetAnnotationNames(), spec.annotationName_, method->GetAccessFlags(),
                               spec.setAccessFlags_, spec.unSetAccessFlags_)) {
        return false;
    }

    if (spec.keepMember_) {
        return true;
    }

    if (!spec.memberName_.empty() && !StringUtil::IsMatch(method->GetRawName(), spec.memberName_)) {
        return false;
    }

    // method: memberType_(argument description), memberValue_(return type)
    const auto descriptor = "\\(" + spec.memberType_ + "\\)" + spec.memberValue_;
    if (descriptor.empty() && method->GetDescriptor().empty()) {
        return true;
    }
    if (!StringUtil::IsMatch(method->GetDescriptor(), descriptor)) {
        return false;
    }
    return true;
}

bool ark::guard::NameMarker::MatchFieldSpecification(abckit_wrapper::Field *field, const MemberSpecification &spec)
{
    if (!MatchCommonAttributes(field->GetAnnotationNames(), spec.annotationName_, field->GetAccessFlags(),
                               spec.setAccessFlags_, spec.unSetAccessFlags_)) {
        return false;
    }

    if (spec.keepMember_) {
        return true;
    }

    if (!spec.memberName_.empty() && !StringUtil::IsMatch(field->GetRawName(), spec.memberName_)) {
        return false;
    }

    if (!spec.memberType_.empty() && !StringUtil::IsMatch(field->GetType(), spec.memberType_)) {
        return false;
    }

    return true;
}

bool ark::guard::NameMarker::CollectMatchedMethods(
    const std::unordered_map<std::string, abckit_wrapper::Method *> &methodTable,
    const std::vector<MemberSpecification> &specifications,
    std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers)
{
    return CollectMatchedMembers(
        methodTable, specifications,
        [this](abckit_wrapper::Method *method, const MemberSpecification &spec) {
            return MatchMethodSpecification(method, spec);
        },
        matchedMembers);
}

bool ark::guard::NameMarker::CollectMatchedFields(
    const std::unordered_map<std::string, abckit_wrapper::Field *> &fieldTable,
    const std::vector<MemberSpecification> &specifications,
    std::unordered_map<std::string, abckit_wrapper::Member *> &matchedMembers)
{
    return CollectMatchedMembers(
        fieldTable, specifications,
        [this](abckit_wrapper::Field *field, const MemberSpecification &spec) {
            return MatchFieldSpecification(field, spec);
        },
        matchedMembers);
}

bool ark::guard::NameMarker::MarkClassKept(abckit_wrapper::Class *clazz) const
{
    if (!keepModuleAllChild_ && !keepNamespaceAllChild_) {
        return false;
    }

    ObfuscatorDataManager manager(clazz);
    auto classObfData = manager.GetData();
    if (classObfData == nullptr) {
        LOG_E << "Get class obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }
    classObfData->SetKept();
    LOG_I << "keep class:" << clazz->GetFullyQualifiedName();

    for (const auto &[methodName, method] : clazz->methodTable_) {
        ObfuscatorDataManager::SetKeptWithMemberLink(method);
        LOG_I << "keep method:" << methodName;
    }

    for (const auto &[fieldName, field] : clazz->fieldTable_) {
        ObfuscatorDataManager::SetKeptWithMemberLink(field);
        LOG_I << "keep field:" << fieldName;
    }

    return true;
}

std::vector<ark::guard::ClassSpecification> ark::guard::NameMarker::CollectNameMatchedSpecification(
    const std::string &name, uint32_t typeDeclarations) const
{
    std::vector<ClassSpecification> matchedSpecs;
    for (const auto &spec : configuration_.GetClassSpecifications()) {
        if (!MatchTypeDeclarations(typeDeclarations, spec.setTypeDeclarations_, spec.unSetTypeDeclarations_)) {
            continue;
        }
        if (!spec.className_.empty() && !StringUtil::IsMatch(name, spec.className_)) {
            continue;
        }
        matchedSpecs.emplace_back(spec);
    }
    return matchedSpecs;
}

void ark::guard::NameMarker::MarkModuleKept(abckit_wrapper::Module *module)
{
    ObfuscatorDataManager manager(module);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get module obfuscate data failed, module:" << module->GetFullyQualifiedName();
        return;
    }
    const auto fullName = module->GetFullyQualifiedName();

    // file name mark
    if (!configuration_.IsFileNameObfEnabled() || configuration_.IsKeepFileName(module->GetName())) {
        obfuscateData->SetKept();
        LOG_I << "keep file name, module:" << fullName;
    }

    // file path mark
    std::string keptPath;
    if (!obfuscateData->IsKept() && configuration_.IsKeepPath(fullName, keptPath)) {
        obfuscateData->SetKeptPath(keptPath);
        LOG_I << "keep path, module:" << fullName << ", path: " << keptPath;
    }

    // module child mark
    const auto matchedSpecs = CollectNameMatchedSpecification(fullName, abckit_wrapper::TypeDeclarations::PACKAGE);
    if (matchedSpecs.empty()) {
        return;
    }

    // There may be scenarios where multiple keep targets are configured, so this loop does not exit prematurely
    for (const auto &spec : matchedSpecs) {
        if (!obfuscateData->IsKept() && spec.keepClassName_) {
            obfuscateData->SetKept();
            LOG_I << "keep module:" << fullName;
        }
        if (IsKeepAll(spec)) {
            keepModuleAllChild_ = true;
        }
    }
}

void ark::guard::NameMarker::MarkNamespaceKept(abckit_wrapper::Namespace *ns)
{
    const auto matchedSpecs =
        CollectNameMatchedSpecification(ns->GetFullyQualifiedName(), abckit_wrapper::TypeDeclarations::NAMESPACE);
    if (matchedSpecs.empty()) {
        return;
    }

    ObfuscatorDataManager manager(ns);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get namespace obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
        return;
    }
    // There may be scenarios where multiple keep targets are configured, so this loop does not exit prematurely
    for (const auto &spec : matchedSpecs) {
        if (!obfuscateData->IsKept() && spec.keepClassName_) {
            obfuscateData->SetKept();
            LOG_I << "keep namespace:" << ns->GetFullyQualifiedName();
        }
        if (IsKeepAll(spec)) {
            keepNamespaceAllChild_ = true;
        }
    }
}
