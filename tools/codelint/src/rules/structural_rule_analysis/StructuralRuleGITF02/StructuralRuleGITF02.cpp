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

#include "StructuralRuleGITF02.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

void StructuralRuleGITF02::CheckExtendMemberDeclsHelper(
    Ptr<Codira::AST::InterfaceDecl> interface, Ptr<Codira::AST::Decl> member, bool& label)
{
    for (auto& memberDecl : interface->GetMemberDecls()) {
        if (memberDecl->identifier == member->identifier && memberDecl->ty == member->ty &&
            !CommonFunc::IsStdDerivedMacro(diagEngine, member->begin)) {
            Diagnose(member->begin, member->end,
                CodeCheckDiagKind::G_ITF_02_prefer_implement_interfaces_at_type_definition, member->identifier.Val());
            label = true;
            break;
        }
    }
}

void StructuralRuleGITF02::CheckExtendMemberDecls(const Codira::AST::ExtendDecl& extendDecl)
{
    if (extendDecl.GetSuperInterfaceTys().empty()) {
        return;
    }
    for (auto& member : extendDecl.members) {
        bool label = false;
        for (auto& interfaceTy : extendDecl.GetSuperInterfaceTys()) {
            auto interface = RawStaticCast<AST::InterfaceDecl*>(AST::Ty::GetDeclOfTy(interfaceTy));
            if (interface->TestAttr(Attribute::IMPORTED)) {
                continue;
            }
            CheckExtendMemberDeclsHelper(interface, member.get(), label);
            if (label) {
                break;
            }
        }
    }
}

void StructuralRuleGITF02::FindExtendDecl(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl& extendDecl) {
                CheckExtendMemberDecls(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGITF02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindExtendDecl(node);
}
} // namespace Codira::CodeCheck
