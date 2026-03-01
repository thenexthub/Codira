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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_SER_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_SER_H

#include "StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGSER : public StructuralRule {
public:
    explicit StructuralRuleGSER(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGSER() override = default;

protected:
    std::set<std::string> extendSers;
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override = 0;
    void FindExtendSer(Ptr<Codira::AST::Node> node);
    /*
     * Check whether the type implements the serial number interface.
     */
    template <typename T> bool IsImpSerializable(const T &decl, std::set<Ptr<AST::Decl>> declSet = {}) const
    {
        if (extendSers.find(decl.identifier) != extendSers.end()) {
            return true;
        }
        for (auto &it : decl.inheritedTypes) {
            if ((it->ToString()).find("Serializable<") != std::string::npos) {
                return true;
            }
            if (it->ty && it->ty->kind == AST::TypeKind::TYPE_CLASS) {
                auto classDecl = StaticCast<AST::ClassTy>(it->ty)->decl;
                if (declSet.count(classDecl) > 0) {
                    continue;
                }
                declSet.insert(classDecl);
                if (IsImpSerializable(*classDecl, declSet)) {
                    return true;
                }
            }
            if (it->ty && it->ty->kind == AST::TypeKind::TYPE_INTERFACE) {
                auto interfaceDecl = StaticCast<AST::InterfaceTy>(it->ty)->decl;
                if (declSet.count(interfaceDecl) > 0) {
                    continue;
                }
                declSet.insert(interfaceDecl);
                if (IsImpSerializable(*interfaceDecl, declSet)) {
                    return true;
                }
            }
        }
        return false;
    }
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_SER_H
