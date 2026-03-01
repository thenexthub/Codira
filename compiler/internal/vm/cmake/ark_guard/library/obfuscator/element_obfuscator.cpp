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

#include "element_obfuscator.h"

#include "logger.h"
#include "member_descriptor_util.h"
#include "member_linker.h"
#include "member_obfuscator.h"
#include "obfuscator_data_manager.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view LAMBDA_CLASS_PREFIX = "%%lambda-";
constexpr std::string_view LAMBDA_FIELD_PREFIX = "lambda_invoke-";

bool IsMainFunction(const std::string &name)
{
    return name == "main";
}

bool IsLambdaClass(const std::string &name)
{
    return ark::guard::StringUtil::IsSubStrMatched(name, LAMBDA_CLASS_PREFIX.data());
}

bool IsLambdaField(const std::string &name)
{
    return ark::guard::StringUtil::IsSubStrMatched(name, LAMBDA_FIELD_PREFIX.data());
}
}  // namespace

bool ark::guard::ElementObfuscator::Execute(abckit_wrapper::FileView &fileView)
{
    return fileView.ModulesAccept(this->Wrap<abckit_wrapper::LocalModuleFilter>());
}

bool ark::guard::ElementObfuscator::Visit(abckit_wrapper::Module *module)
{
    return VisitObject(module, true, ObjectType::MODULE);
}

bool ark::guard::ElementObfuscator::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    return VisitObject(ns);
}

bool ark::guard::ElementObfuscator::VisitMethod(abckit_wrapper::Method *method)
{
    if (method->IsConstructor()) {
        return true;
    }
    return VisitMember(method);
}

bool ark::guard::ElementObfuscator::VisitField(abckit_wrapper::Field *field)
{
    return VisitMember(field);
}

bool ark::guard::ElementObfuscator::VisitClass(abckit_wrapper::Class *clazz)
{
    if (clazz->IsExternal()) {
        return true;
    }
    return VisitObject(clazz, false, ObjectType::CLASS);
}

bool ark::guard::ElementObfuscator::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    return VisitObject(ai);
}

bool ark::guard::ElementObfuscator::VisitMember(abckit_wrapper::Member *member)
{
    ObfuscatorDataManager manager(member);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get member obfuscate data failed, member:" << member->GetFullyQualifiedName();
        return false;
    }
    std::string obfName = obfuscateData->GetObfName();
    if (obfuscateData->IsKept() && !obfName.empty()) {
        return true;
    }

    const auto rawName = member->GetRawName();
    if (IsMainFunction(rawName) || IsLambdaField(rawName)) {
        return true;
    }

    if (obfName.empty()) {
        const auto packageName = member->GetPackageName();
        const auto descriptor = MemberDescriptorUtil::GetOrCreateNewMemberDescriptor(member->GetDescriptor());
        const auto key = packageName + descriptor;
        const auto nameMapping = nameMappingManager_.GetNameMapping(key);
        GUARD_ASSERT(!nameMapping, ErrorCode::GENERIC_ERROR, "get member nameMapping failed, key:" + key);
        obfName = nameMapping->GetName(rawName);
    }

    if (rawName != obfName && !member->SetName(obfName)) {
        LOG_E << "Fail to set member obfName, method: " << member->GetFullyQualifiedName() << ", obfName:" << obfName;
        return false;
    }

    obfuscateData->SetObfName(obfName);
    return true;
}

bool ark::guard::ElementObfuscator::VisitObject(abckit_wrapper::Object *object, bool visitChild, ObjectType type)
{
    ObfuscatorDataManager manager(object);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get object obfuscate data failed, object:" << object->GetFullyQualifiedName();
        return false;
    }
    std::string obfName = obfuscateData->GetObfName();
    if (obfuscateData->IsKept() && !obfName.empty()) {
        return visitChild ? object->ChildrenAccept(*this) : true;
    }

    if (visitChild && !object->ChildrenAccept(*this)) {
        return false;
    }

    if (type == ObjectType::MODULE && !obfuscateData->GetKeptPath().empty()) {
        return ObfModuleWithKeptPath(object, obfuscateData->GetKeptPath());
    }

    const auto rawName = object->GetName();
    if (type == ObjectType::CLASS && IsLambdaClass(rawName)) {
        return true;
    }

    if (obfName.empty()) {
        const auto key = object->GetPackageName();
        const auto nameMapping = nameMappingManager_.GetNameMapping(key);
        GUARD_ASSERT(!nameMapping, ErrorCode::GENERIC_ERROR, "get object nameMapping failed, key:" + key);
        obfName = nameMapping->GetName(rawName);
    }

    if (rawName != obfName && !object->SetName(obfName)) {
        LOG_E << "Fail to set object obfName, method: " << object->GetFullyQualifiedName() << ", obfName:" << obfName;
        return false;
    }

    obfuscateData->SetObfName(obfName);
    return true;
}

bool ark::guard::ElementObfuscator::ObfModuleWithKeptPath(abckit_wrapper::Object *module, const std::string &path)
{
    const auto nameMapping = nameMappingManager_.GetNameMapping(path);
    GUARD_ASSERT(!nameMapping, ErrorCode::GENERIC_ERROR, "get module nameMapping failed, key:" + path);

    const auto rawName = module->GetName();
    const auto oriPartName = StringUtil::RemovePrefixIfMatches(rawName, path);
    GUARD_ASSERT(rawName == oriPartName, ErrorCode::GENERIC_ERROR, "keptPath not match with module: " + rawName);
    GUARD_ASSERT(oriPartName.empty(), ErrorCode::GENERIC_ERROR,
                     "needToObf path is empty, check KeepPath: " + rawName);

    const auto obfPartName = nameMapping->GetName(oriPartName);
    std::string obfName = path + obfPartName;
    if (oriPartName != obfPartName && !module->SetName(obfName)) {
        LOG_E << "Fail to set obfName, module: " << module->GetFullyQualifiedName() << ", obfName:" << obfName;
        return false;
    }

    ObfuscatorDataManager manager(module);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get module obfuscate data failed, module:" << module->GetFullyQualifiedName();
        return false;
    }
    obfuscateData->SetObfName(obfName);
    return true;
}