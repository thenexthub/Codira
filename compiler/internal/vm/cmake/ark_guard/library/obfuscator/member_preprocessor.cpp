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

#include "member_preprocessor.h"

#include "logger.h"
#include "member_descriptor_util.h"
#include "member_linker.h"
#include "member_obfuscator.h"
#include "obfuscator_data_manager.h"
#include "util/assert_util.h"

bool ark::guard::MemberPreProcessor::Process(abckit_wrapper::FileView &fileView)
{
    return fileView.ModulesAccept(*this);
}

bool ark::guard::MemberPreProcessor::Visit(abckit_wrapper::Module *module)
{
    if (module->IsExternal()) {
        return true;
    }
    return module->ChildrenAccept(*this);
}

bool ark::guard::MemberPreProcessor::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    return ns->ChildrenAccept(*this);
}

bool ark::guard::MemberPreProcessor::VisitClass(abckit_wrapper::Class *clazz)
{
    MemberVisitor *memberVisitor = this;
    return clazz->HierarchyAccept(memberVisitor->Wrap<abckit_wrapper::ClassMemberVisitor>(), true, true, true, true);
}

bool ark::guard::MemberPreProcessor::Visit(abckit_wrapper::Member *member)
{
    if (member->IsConstructor()) {
        return true;
    }
    const auto descriptor = MemberDescriptorUtil::GetOrCreateNewMemberDescriptor(member->GetDescriptor());
    const auto nameMapping = nameMappingManager_.GetNameMapping(descriptor);
    GUARD_ASSERT(!nameMapping, ErrorCode::GENERIC_ERROR, "get object nameMapping failed, descriptor:" + descriptor);

    auto lastMember = MemberLinker::LastMember(member);
    const auto lastMemberRawName = lastMember->GetRawName();
    nameMapping->AddUsedNameList(lastMemberRawName);

    if (lastMember != member) {
        ObfuscatorDataManager memberManager(member);
        memberManager.SetNameAndObfuscatedName(member->GetName(), member->GetRawName());
    }
    ObfuscatorDataManager lastMemberManager(lastMember);
    lastMemberManager.SetNameAndObfuscatedName(lastMember->GetName(), lastMember->GetRawName());

    auto lastMemberData = lastMemberManager.GetData();
    if (lastMemberData == nullptr) {
        LOG_E << "Get member obfuscate data failed, member:" << lastMember->GetFullyQualifiedName();
        return false;
    }
    const auto lastMemberObfName = lastMemberData->GetObfName();
    if (!lastMemberObfName.empty() && lastMemberObfName != lastMemberRawName) {
        nameMapping->AddUsedNameList(lastMemberObfName);
    }

    return true;
}