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

#include "StructuralRuleGITF03.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGITF03::CheckParent(const std::string &childName, const InterfaceTy *ty,
    const InheritableDecl &inheritableDecl, std::set<Ptr<InterfaceTy>> tySet)
{
    for (auto &type : inheritableDecl.GetSuperInterfaceTys()) {
        if (type->name == childName) {
            continue;
        }
        for (auto &item : ty->GetSuperInterfaceTys()) {
            if (item->name == type->name) {
                diagEngine->Diagnose(inheritableDecl.begin, inheritableDecl.end,
                    CodeCheckDiagKind::G_ITF_03_avoid_declaring_both_parent_interface_and_sub_interface, childName,
                    type->name);
            } else {
                if (tySet.count(item) > 0) {
                    continue;
                }
                tySet.insert(item);
                CheckParent(childName, item, inheritableDecl, tySet);
            }
        }
    }
}

void StructuralRuleGITF03::CheckInheritedTypes(const InheritableDecl &inheritableDecl)
{
    for (auto &type : inheritableDecl.GetSuperInterfaceTys()) {
        CheckParent(type->name, type, inheritableDecl);
    }
}

void StructuralRuleGITF03::FindTypeDecl(Node *node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node *node) -> VisitAction {
        return match(*node)(
            [this](const InterfaceDecl &interfaceDecl) {
                interfaceDecl.GetSuperInterfaceTys();
                CheckInheritedTypes(interfaceDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ClassDecl &classDecl) {
                CheckInheritedTypes(classDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const StructDecl &structDecl) {
                CheckInheritedTypes(structDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ExtendDecl &extendDecl) {
                CheckInheritedTypes(extendDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const EnumDecl &enumDecl) {
                CheckInheritedTypes(enumDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGITF03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindTypeDecl(node);
}
}
