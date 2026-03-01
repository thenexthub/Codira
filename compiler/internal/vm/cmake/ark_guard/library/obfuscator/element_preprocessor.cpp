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

#include "element_preprocessor.h"

#include "member_descriptor_util.h"
#include "logger.h"
#include "member_linker.h"
#include "member_obfuscator.h"
#include "obfuscator_data_manager.h"
#include "util/assert_util.h"
#include "util/string_util.h"

bool ark::guard::ElementPreProcessor::Process(abckit_wrapper::FileView &fileView)
{
    return fileView.ModulesAccept(*this);
}

bool ark::guard::ElementPreProcessor::Visit(abckit_wrapper::Module *module)
{
    if (module->IsExternal()) {
        return true;
    }

    const auto name = module->GetName();
    auto nameMapping = GetNameMapping(module->GetPackageName());
    nameMapping->AddUsedNameList(name);

    ObfuscatorDataManager moduleManager(module);
    auto obfuscateData = moduleManager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get module obfuscate data failed, module:" << module->GetFullyQualifiedName();
        return false;
    }

    moduleManager.SetNameAndObfuscatedName(module->GetFullyQualifiedName(), module->GetName());
    const auto obfName = obfuscateData->GetObfName();
    if (!obfName.empty() && obfName != name) {
        nameMapping->AddUsedNameList(obfName);
    }
    StoreSeparatedModuleName(obfuscateData->GetKeptPath(), name, obfName);

    return module->ChildrenAccept(*this);
}

bool ark::guard::ElementPreProcessor::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    const auto name = ns->GetName();
    auto nameMapping = GetNameMapping(ns->GetPackageName());
    nameMapping->AddUsedNameList(name);

    ObfuscatorDataManager nsManager(ns);
    auto obfuscateData = nsManager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get namespace obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
        return false;
    }

    nsManager.SetNameAndObfuscatedName(name, name);
    const auto obfName = obfuscateData->GetObfName();
    if (!obfName.empty() && obfName != name) {
        nameMapping->AddUsedNameList(obfName);
    }

    return ns->ChildrenAccept(*this);
}

bool ark::guard::ElementPreProcessor::VisitMethod(abckit_wrapper::Method *method)
{
    if (method->IsConstructor()) {
        return true;
    }
    return VisitMember(method);
}

bool ark::guard::ElementPreProcessor::VisitField(abckit_wrapper::Field *field)
{
    return VisitMember(field);
}

bool ark::guard::ElementPreProcessor::VisitClass(abckit_wrapper::Class *clazz)
{
    const auto name = clazz->GetName();
    auto nameMapping = GetNameMapping(clazz->GetPackageName());
    nameMapping->AddUsedNameList(name);

    ObfuscatorDataManager clazzManager(clazz);
    auto obfuscateData = clazzManager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get class obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }
    if (clazz->IsExternal()) {
        LOG_I << "Keep external class: " << name;
        obfuscateData->SetKept();
    }

    clazzManager.SetNameAndObfuscatedName(name, name);
    const auto obfName = obfuscateData->GetObfName();
    if (!obfName.empty() && obfName != name) {
        nameMapping->AddUsedNameList(obfName);
    }

    return true;
}

bool ark::guard::ElementPreProcessor::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    const auto name = ai->GetName();
    auto nameMapping = GetNameMapping(ai->GetPackageName());
    nameMapping->AddUsedNameList(name);

    ObfuscatorDataManager aiManager(ai);
    auto obfuscateData = aiManager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get annotation interface obfuscate data failed, annotation interface:" << ai->GetFullyQualifiedName();
        return false;
    }

    aiManager.SetNameAndObfuscatedName(name, name);
    const auto obfName = obfuscateData->GetObfName();
    if (!obfName.empty() && obfName != name) {
        nameMapping->AddUsedNameList(obfName);
    }

    return true;
}

bool ark::guard::ElementPreProcessor::VisitMember(abckit_wrapper::Member *member)
{
    const auto packageName = member->GetPackageName();
    const auto descriptor = MemberDescriptorUtil::GetOrCreateNewMemberDescriptor(member->GetDescriptor());
    const auto key = packageName + descriptor;

    auto lastMember = MemberLinker::LastMember(member);
    const auto lastMemberRawName = lastMember->GetRawName();

    auto nameMapping = GetNameMapping(key);
    nameMapping->AddUsedNameList(lastMemberRawName);

    if (lastMember != member) {
        ObfuscatorDataManager memberManager(member);
        memberManager.SetNameAndObfuscatedName(member->GetName(), member->GetRawName());
    }
    ObfuscatorDataManager lastMemberManager(lastMember);
    lastMemberManager.SetNameAndObfuscatedName(lastMember->GetName(), lastMember->GetRawName());

    auto obfuscateData = lastMemberManager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get member obfuscate data failed, member:" << lastMember->GetFullyQualifiedName();
        return false;
    }
    const auto lastMemberObfName = obfuscateData->GetObfName();
    if (!lastMemberObfName.empty() && lastMemberObfName != lastMemberRawName) {
        nameMapping->AddUsedNameList(lastMemberObfName);
    }

    return true;
}

std::shared_ptr<ark::guard::NameMapping> ark::guard::ElementPreProcessor::GetNameMapping(const std::string &key) const
{
    const auto nameMapping = nameMappingManager_.GetNameMapping(key);
    GUARD_ASSERT(!nameMapping, ErrorCode::GENERIC_ERROR, "get object nameMapping failed, key:" + key);
    return nameMapping;
}

void ark::guard::ElementPreProcessor::StoreSeparatedModuleName(const std::string &path,
                                                               const std::string &oriName,
                                                               const std::string &obfName) const
{
    if (path.empty()) {
        return;
    }
    auto nameMapping = GetNameMapping(path);

    // oriName
    const auto oriPartName = StringUtil::RemovePrefixIfMatches(oriName, path);
    GUARD_ASSERT(oriName == oriPartName, ErrorCode::GENERIC_ERROR, "keptPath not match with module: " + oriName);
    nameMapping->AddUsedNameList(oriPartName);

    // obfName
    if (obfName.empty() || obfName == oriName) {
        return;
    }
    const auto obfPartName = StringUtil::RemovePrefixIfMatches(obfName, path);
    if (obfPartName == obfName) {
        LOG_W << "keptPath not match with module's obfName: " << oriName;
        return;
    }
    nameMapping->AddUsedNameList(obfPartName);
}