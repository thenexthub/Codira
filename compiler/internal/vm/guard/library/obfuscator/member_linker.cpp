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

#include "member_linker.h"

#include <unordered_set>

#include "abckit_wrapper/class.h"

#include "logger.h"
#include "obfuscator_data_manager.h"
#include "util/assert_util.h"

bool ark::guard::MemberLinker::Visit(abckit_wrapper::Class *clazz)
{
    // clear last class members linked info
    this->linkerMembers_.clear();
    LOG_I << "link member for class:" << clazz->GetFullyQualifiedName();

    MemberVisitor *memberVisitor = this;
    // link class hierarchy members
    return clazz->HierarchyAccept(memberVisitor->Wrap<abckit_wrapper::ClassMemberVisitor>(), true, true, true, false);
}

bool ark::guard::MemberLinker::Visit(abckit_wrapper::Member *member)
{
    if (member->IsConstructor()) {
        return true;
    }
    LOG_I << "member:" << member->GetFullyQualifiedName();
    LOG_I << "rawName:" << member->GetRawName();
    LOG_I << "descriptor:" << member->GetDescriptor();

    std::string signature = member->GetSignature();
    LOG_I << "signature:" << signature;
    if (this->linkerMembers_.find(signature) == this->linkerMembers_.end()) {
        LOG_I << "add member to linked members";

        auto lastMember = LastMember(member);
        LOG_I << "lastMember:" << lastMember->GetFullyQualifiedName();
        LOG_I << "rawName:" << lastMember->GetRawName();
        LOG_I << "descriptor:" << lastMember->GetDescriptor();
        LOG_I << "signature:" << lastMember->GetSignature();

        this->linkerMembers_.emplace(signature, lastMember);
        return true;
    }

    Link(member, this->linkerMembers_.at(signature));

    return true;
}

abckit_wrapper::Member *ark::guard::MemberLinker::NextMember(abckit_wrapper::Member *member)
{
    ObfuscatorDataManager manager(member);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get member obfuscate data failed, member:" << member->GetFullyQualifiedName();
        return nullptr;
    }
    return obfuscateData->GetMember();
}

abckit_wrapper::Member *ark::guard::MemberLinker::LastMember(abckit_wrapper::Member *member)
{
    auto lastMember = member;
    std::unordered_set<size_t> visited;
    while (const auto nextMember = NextMember(lastMember)) {
        GUARD_ASSERT(visited.find(reinterpret_cast<size_t>(nextMember)) != visited.end(), ErrorCode::GENERIC_ERROR,
                         "Detected cycle in members chain, stopping traversal");

        visited.insert(reinterpret_cast<size_t>(nextMember));
        lastMember = nextMember;
    }

    return lastMember;
}

void ark::guard::MemberLinker::Link(abckit_wrapper::Member *member1, abckit_wrapper::Member *member2)
{
    const auto lastMember1 = LastMember(member1);
    const auto lastMember2 = LastMember(member2);
    // member1 and member2 already link the same last member
    if (lastMember1 == lastMember2) {
        return;
    }

    // lastMember1 --> lastMember2
    LOG_I << "link member";
    LOG_I << lastMember1->GetFullyQualifiedName() << " --> " << lastMember2->GetFullyQualifiedName();

    ObfuscatorDataManager manager(lastMember1);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get member obfuscate data failed, member:" << lastMember1->GetFullyQualifiedName();
        return;
    }
    obfuscateData->SetMember(lastMember2);
}

ark::guard::ErrorCode ark::guard::MemberLinker::Link()
{
    ClassVisitor *visitor = this;
    // link the members of the leaf class
    if (!fileView_.ModulesAccept(visitor->Wrap<abckit_wrapper::LeafClassVisitor>()
                                     .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                     .Wrap<abckit_wrapper::LocalModuleFilter>())) {  // CC-OFF(G.FMT.02)
        LOG_E << "Failed to link members";
        return ErrorCode::GENERIC_ERROR;
    }

    return ErrorCode::ERR_SUCCESS;
}
