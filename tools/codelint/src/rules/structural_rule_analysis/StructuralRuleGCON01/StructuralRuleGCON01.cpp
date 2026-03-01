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

#include "StructuralRuleGCON01.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGCON01::DefaultPackageChecker(const std::set<Codira::AST::Modifier> modifiers,
    const Codira::Position start, const Codira::Position end)
{
    auto result = std::any_of(modifiers.begin(), modifiers.end(),
        [](const Codira::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PRIVATE; });
    if (!result) {
        Diagnose(start, end, CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
    }
}

void StructuralRuleGCON01::ClassModifierChecker(const Codira::AST::VarDecl &varDecl)
{
    auto result = std::any_of(varDecl.outerDecl->modifiers.begin(), varDecl.outerDecl->modifiers.end(),
        [](const Codira::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    if (result) {
        if (varDecl.outerDecl->TestAttr(Attribute::PUBLIC)) {
            if (varDecl.TestAttr(Attribute::PUBLIC)) {
                Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                    CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
            }
        }

        if (varDecl.outerDecl->TestAttr(Attribute::OPEN) || varDecl.outerDecl->TestAttr(Attribute::ABSTRACT)) {
            if (varDecl.TestAttr(Attribute::PUBLIC) || varDecl.TestAttr(Attribute::PROTECTED)) {
                Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                    CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_1, "private");
            }
        }
    }
}

void StructuralRuleGCON01::PackageGlobalVarChecker(const std::set<Codira::AST::Modifier> modifiers,
    const Codira::Position start, const Codira::Position end)
{
    auto result = std::any_of(modifiers.begin(), modifiers.end(),
        [](const Codira::AST::Modifier &modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    if (result) {
        Diagnose(start, end, CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_0, "public");
    }
}

void StructuralRuleGCON01::SynchronizedObjectFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl &varDecl) {
                if (varDecl.ty == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (varDecl.ty->name != "ReentrantMutex") {
                    return VisitAction::SKIP_CHILDREN;
                }

                if (varDecl.fullPackageName == "default") {
                    if (varDecl.TestAttr(Attribute::GLOBAL)) {
                        Diagnose(varDecl.identifier.Begin(), varDecl.identifier.End(),
                            CodeCheckDiagKind::G_CON_01_synchronized_object_modifier_information_2,
                            varDecl.identifier.Val());
                    }
                    if (varDecl.outerDecl != nullptr && varDecl.outerDecl->IsStructOrClassDecl()) {
                        DefaultPackageChecker(varDecl.modifiers, varDecl.identifier.Begin(), varDecl.identifier.End());
                    }
                } else {
                    if (varDecl.outerDecl != nullptr && varDecl.outerDecl->IsStructOrClassDecl()) {
                        ClassModifierChecker(varDecl);
                    }
                    if (varDecl.TestAttr(Attribute::GLOBAL)) {
                        PackageGlobalVarChecker(
                            varDecl.modifiers, varDecl.identifier.Begin(), varDecl.identifier.End());
                    }
                }

                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCON01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    SynchronizedObjectFinder(node);
}
} // namespace Codira::CodeCheck
