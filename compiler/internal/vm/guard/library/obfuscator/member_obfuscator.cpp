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

#include "member_obfuscator.h"

#include "logger.h"
#include "member_descriptor_util.h"
#include "member_linker.h"
#include "obfuscator_data_manager.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view LAMBDA_CLASS_PREFIX = "%%lambda-";
constexpr std::string_view LAMBDA_FIELD_PREFIX = "lambda_invoke-";

bool IsInEnumWhiteList(const std::string &name)
{
    static const std::unordered_set<std::string> whiteList = {
        "_ordinal", "_StringValuesArray", "_NamesArray", "_ValuesArray", "_ItemsArray", "valueOf", "values",
        "toString", "getValueOf", "fromValue", "getName", "getOrdinal", "$_get"};
    return whiteList.find(name) != whiteList.end();
}

bool IsEnumMember(const abckit_wrapper::Member *member)
{
    if (member->parent_.has_value()) {
        if (const auto *clazz = static_cast<abckit_wrapper::Class *>(member->parent_.value())) {
            if (clazz->IsEnum()) {
                return true;
            }
        }
    }
    return false;
}

bool IsLambdaClass(const std::string &name)
{
    return ark::guard::StringUtil::IsSubStrMatched(name, LAMBDA_CLASS_PREFIX.data());
}

bool IsLambdaField(const std::string &name)
{
    return ark::guard::StringUtil::IsSubStrMatched(name, LAMBDA_FIELD_PREFIX.data());
}
} // namespace

bool ark::guard::MemberObfuscator::Execute(abckit_wrapper::FileView &fileView)
{
    ClassVisitor *visitor = this;
    return fileView.ModulesAccept(
        visitor->Wrap<abckit_wrapper::ModuleClassVisitor>().Wrap<abckit_wrapper::LocalModuleFilter>());
}

bool ark::guard::MemberObfuscator::Visit(abckit_wrapper::Class *clazz)
{
    MemberVisitor *memberVisitor = this;
    if (IsLambdaClass(clazz->GetName())) {
        return true;
    }
    return clazz->HierarchyAccept(memberVisitor->Wrap<abckit_wrapper::ClassMemberVisitor>(), true, true, true, true);
}

bool ark::guard::MemberObfuscator::Visit(abckit_wrapper::Member *member)
{
    if (member->IsConstructor()) {
        return true;
    }
    auto lastMember = MemberLinker::LastMember(member);
    ObfuscatorDataManager lastMemberManager(lastMember);
    auto lastMemberData = lastMemberManager.GetData();
    if (lastMemberData == nullptr) {
        LOG_E << "Get last member obfuscate data failed, member:" << lastMember->GetFullyQualifiedName();
        return false;
    }

    const auto rawName = lastMember->GetRawName();
    if ((IsEnumMember(member) && IsInEnumWhiteList(rawName)) || IsLambdaField(rawName)) {
        LOG_I << "Member in WhiteList: " << rawName;
        return true;
    }

    std::string obfName = lastMemberData->GetObfName();
    if (obfName.empty()) {
        auto descriptor = MemberDescriptorUtil::GetOrCreateNewMemberDescriptor(member->GetDescriptor());
        auto nameMapping = nameMappingManager_.GetNameMapping(descriptor);
        obfName = nameMapping->GetName(rawName);

        lastMemberData->SetObfName(obfName);
    }

    if ((rawName != obfName) && !lastMember->SetName(obfName)) {
        return false;
    }

    if (lastMember != member) {
        return SetMemberName(member, obfName);
    }

    return true;
}

bool ark::guard::MemberObfuscator::SetMemberName(abckit_wrapper::Member *member, const std::string &obfName)
{
    auto name = member->GetRawName();
    if (name == obfName) {
        return true;
    }
    ObfuscatorDataManager manager(member);
    auto data = manager.GetData();
    if (!data) {
        LOG_E << "Get member obfuscate data failed, member:" << member->GetFullyQualifiedName();
        return false;
    }
    data->SetObfName(obfName);
    return member->SetName(obfName);
}