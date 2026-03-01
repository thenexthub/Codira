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

#include "StructuralRuleGDCL01.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

const std::string VAR_DECL = "varDecl";
const std::string FUNC_DECL = "funcDecl";

/*
 * Generate key, Ensure that the same function generates the same key.
 */
std::string GenerateKey(FuncDecl& funcDecl, FuncTy* types)
{
    std::string key;
    key.append(funcDecl.identifier.GetRawText());
    key.append("/");
    if (funcDecl.funcBody->generic) {
        auto genericTypeNum = funcDecl.funcBody->generic->typeParameters.size();
        key.append(std::to_string(genericTypeNum));
        key.append("/");
    }
    for (auto& param : types->paramTys) {
        key.append(param->String());
        key.append("/");
    }
    return key;
}

void StructuralRuleGDCL01::VarDeclProcessor(VarDecl& varDecl, std::map<std::string, std::stack<Decl*>>& varContainer,
    std::map<std::string, std::vector<Decl*>>& varTmpContainer, bool isStaticFunc)
{
    auto varId = varDecl.identifier.GetRawText();
    auto it = varContainer.find(varId);
    if (it != varContainer.end()) {
        // while varDecl's pos more forward than top item of stack
        // add top item to varTmpContainer, and pop it from stack, we need it later.
        while (!varContainer[varId].empty() &&
            varContainer[varId].top()->identifier.GetRawPos() > varDecl.identifier.GetRawPos()) {
            varTmpContainer[varId].insert(varTmpContainer[varId].begin(), varContainer[varId].top());
            varContainer[varId].pop();
        }
        // find varId in map, and map value is not empty, there is shadow, warning.
        if (!varContainer[varId].empty()) {
            if (varContainer[varId].top()->IsStaticOrGlobal() || !isStaticFunc) {
                Diagnose(varDecl.identifier.GetRawPos(), varDecl.identifier.GetRawEndPos(),
                    CodeCheckDiagKind::G_DCL_01_avoid_shadow, varId,
                    varContainer[varId].top()->identifier.GetRawPos().line, varId, varDecl.identifier.GetRawPos().line);
            }
        }
        varContainer[varId].push(&varDecl);
    } else {
        // If varId not in map, insert it.
        std::stack<Decl*> declStack;
        declStack.push(&varDecl);
        varContainer.insert(std::make_pair(varId, declStack));
    }
}

void StructuralRuleGDCL01::FuncDeclProcessor(
    FuncDecl& funcDecl, std::map<std::string, std::stack<Decl*>>& funcContainer)
{
    // get func param types
    auto types = DynamicCast<FuncTy*>(funcDecl.ty);
    if (!types) {
        return;
    }
    if (!funcDecl.funcBody) {
        return;
    }

    // Generate key for funcContainer
    auto key = GenerateKey(funcDecl, types);
    auto it = funcContainer.find(key);
    // find key in map, it means, there is shadow, warning.
    if (it != funcContainer.end()) {
        if (!funcContainer[key].empty()) {
            Diagnose(funcDecl.identifier.GetRawPos(), funcDecl.identifier.GetRawEndPos(),
                CodeCheckDiagKind::G_DCL_01_avoid_shadow, funcDecl.identifier.GetRawText(),
                funcContainer[key].top()->identifier.GetRawPos().line, funcDecl.identifier.GetRawText(),
                funcDecl.identifier.GetRawPos().line);
        }
        funcContainer[key].push(&funcDecl);
    } else {
        // If key not in map, insert it.
        std::stack<Decl*> declStack;
        declStack.push(&funcDecl);
        funcContainer.insert(std::make_pair(key, declStack));
    }
}

void StructuralRuleGDCL01::PostVarDeclProcessor(
    VarDecl& varDecl, std::map<std::string, std::stack<Decl*>>& varContainer, std::set<std::string>& varSet)
{
    auto varId = varDecl.identifier.GetRawText();
    varSet.insert(varId);
    if (!varContainer[varId].empty()) {
        varContainer[varId].pop();
    }

    if (varContainer[varId].empty()) {
        varContainer.erase(varId);
    }
}

void StructuralRuleGDCL01::PostFuncDeclProcessor(
    FuncDecl& funcDecl, std::map<std::string, std::stack<Decl*>>& funcContainer)
{
    auto types = DynamicCast<FuncTy*>(funcDecl.ty);
    if (!types) {
        return;
    }
    if (!funcDecl.funcBody) {
        return;
    }

    auto key = GenerateKey(funcDecl, types);
    if (!funcContainer[key].empty()) {
        funcContainer[key].pop();
    }

    if (funcContainer[key].empty()) {
        funcContainer.erase(key);
    }
}

template <typename T>
void StructuralRuleGDCL01::TraversingNode(std::vector<OwnedPtr<T>>& node,
    std::map<std::string, std::map<std::string, std::stack<Decl*>>>& container,
    std::map<std::string, std::map<std::string, std::vector<Decl*>>>& tmpContainer, bool isStaticFunc)
{
    for (auto& decl : node) {
        if (decl->astKind == ASTKind::VAR_DECL) {
            auto pVarDecl = As<ASTKind::VAR_DECL>(decl.get());
            // collect varDecl info
            VarDeclProcessor(*pVarDecl, container[VAR_DECL], tmpContainer[VAR_DECL], isStaticFunc);
        } else if (decl->astKind == ASTKind::FUNC_DECL) {
            auto pFuncDecl = As<ASTKind::FUNC_DECL>(decl.get());
            // collect funcDecl info
            FuncDeclProcessor(*pFuncDecl, container[FUNC_DECL]);
        }
    }
}

template <typename T>
void StructuralRuleGDCL01::PostBlockAndClassBodyProcessor(std::vector<OwnedPtr<T>>& nodes,
    std::map<std::string, std::map<std::string, std::stack<Decl*>>>& container,
    std::map<std::string, std::map<std::string, std::vector<Decl*>>>& tmpContainer)
{
    std::set<std::string> varSet;
    for (auto& decl : nodes) {
        if (decl->astKind == ASTKind::VAR_DECL) {
            auto pVarDecl = As<ASTKind::VAR_DECL>(decl.get());
            PostVarDeclProcessor(*pVarDecl, container[VAR_DECL], varSet);
        } else if (decl->astKind == ASTKind::FUNC_DECL) {
            auto pFuncDecl = As<ASTKind::FUNC_DECL>(decl.get());
            PostFuncDeclProcessor(*pFuncDecl, container[FUNC_DECL]);
        }
    }
    for (auto& var : varSet) {
        for (auto& pos : tmpContainer[VAR_DECL][var]) {
            container[VAR_DECL][var].push(pos);
        }
    }
}

void StructuralRuleGDCL01::FindShadowNode(Ptr<Codira::AST::Node>& node)
{
    if (!node) {
        return;
    }
    std::map<std::string, std::map<std::string, std::stack<Decl*>>> container;
    std::map<std::string, std::map<std::string, std::vector<Decl*>>> tmpContainer;
    container[VAR_DECL] = std::map<std::string, std::stack<Decl*>>();
    container[FUNC_DECL] = std::map<std::string, std::stack<Decl*>>();
    tmpContainer[VAR_DECL] = std::map<std::string, std::vector<Decl*>>();

    // pre-action before walk node children;
    auto preVist = [&container, &tmpContainer, this](Ptr<Node> node) -> VisitAction {
        switch (node->astKind) {
            case ASTKind::FILE: {
                auto pFILE = As<ASTKind::FILE>(node.get());
                container.clear();
                tmpContainer.clear();
                // collect top level elements info
                TraversingNode(pFILE->decls, container, tmpContainer);
                break;
            }
            case ASTKind::FUNC_BODY: {
                auto pFuncBody = As<ASTKind::FUNC_BODY>(node.get());
                bool isStaticFunc = false;
                if (pFuncBody->funcDecl) {
                    isStaticFunc = pFuncBody->funcDecl->TestAttr(Attribute::STATIC);
                }
                // collect elements info in block
                if (pFuncBody->body) {
                    TraversingNode(pFuncBody->body->body, container, tmpContainer, isStaticFunc);
                }
                break;
            }
            case ASTKind::CLASS_BODY: {
                auto pClassBody = As<ASTKind::CLASS_BODY>(node.get());
                // collect elements info in block
                TraversingNode(pClassBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::INTERFACE_BODY: {
                auto pInterfaceBody = As<ASTKind::INTERFACE_BODY>(node.get());
                // collect elements info in block
                TraversingNode(pInterfaceBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::STRUCT_BODY: {
                auto pStructBody = As<ASTKind::STRUCT_BODY>(node.get());
                // collect elements info in block
                TraversingNode(pStructBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::ENUM_DECL: {
                auto pEnumDecl = As<ASTKind::ENUM_DECL>(node.get());
                // collect elements info in block
                TraversingNode(pEnumDecl->members, container, tmpContainer);
                break;
            }
            case ASTKind::EXTEND_DECL: {
                auto pExtendDecl = As<ASTKind::EXTEND_DECL>(node.get());
                // collect elements info in block
                TraversingNode(pExtendDecl->members, container, tmpContainer);
                break;
            }
            default:
                break;
        }
        return VisitAction::WALK_CHILDREN;
    };

    // post-action after walk node children;
    auto postVisit = [&container, &tmpContainer](Ptr<Node> node) -> VisitAction {
        switch (node->astKind) {
            case ASTKind::FUNC_BODY: {
                auto pFuncBody = As<ASTKind::FUNC_BODY>(node.get());
                if (pFuncBody->body) {
                    PostBlockAndClassBodyProcessor(pFuncBody->body->body, container, tmpContainer);
                }
                break;
            }
            case ASTKind::CLASS_BODY: {
                auto pClassBody = As<ASTKind::CLASS_BODY>(node.get());
                PostBlockAndClassBodyProcessor(pClassBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::INTERFACE_BODY: {
                auto pInterfaceBody = As<ASTKind::INTERFACE_BODY>(node.get());
                // collect elements info in block
                PostBlockAndClassBodyProcessor(pInterfaceBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::STRUCT_BODY: {
                auto pStructBody = As<ASTKind::STRUCT_BODY>(node.get());
                // collect elements info in block
                PostBlockAndClassBodyProcessor(pStructBody->decls, container, tmpContainer);
                break;
            }
            case ASTKind::ENUM_DECL: {
                auto pEnumDecl = As<ASTKind::ENUM_DECL>(node.get());
                // collect elements info in block
                PostBlockAndClassBodyProcessor(pEnumDecl->members, container, tmpContainer);
                break;
            }
            case ASTKind::EXTEND_DECL: {
                auto pExtendDecl = As<ASTKind::EXTEND_DECL>(node.get());
                // collect elements info in block
                PostBlockAndClassBodyProcessor(pExtendDecl->members, container, tmpContainer);
                break;
            }
            default:
                break;
        }
        return VisitAction::WALK_CHILDREN;
    };

    Walker walker(node, preVist, postVisit);
    walker.Walk();
}

template <typename T>
void StructuralRuleGDCL01::MemberDeclProcessor(
    const std::vector<OwnedPtr<T>>& members, std::map<std::string, Decl*>& genericContainer)
{
    for (auto& memberDecl : members) {
        if (memberDecl->astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        auto pFuncDecl = As<ASTKind::FUNC_DECL>(memberDecl.get());
        if (!pFuncDecl->funcBody || !pFuncDecl->funcBody->generic) {
            continue;
        }
        for (auto& genericParam : pFuncDecl->funcBody->generic->typeParameters) {
            auto genericParamID = genericParam->identifier.GetRawText();
            auto it = genericContainer.find(genericParamID);
            if (it != genericContainer.end()) {
                Diagnose(genericParam->identifier.GetRawPos(), genericParam->identifier.GetRawEndPos(),
                    CodeCheckDiagKind::G_DCL_01_avoid_shadow, genericParamID,
                    genericContainer[genericParamID]->identifier.GetRawPos().line, genericParamID,
                    genericParam->identifier.GetRawPos().line);
            }
        }
    }
}

void SetGenericContainer(
    std::vector<OwnedPtr<GenericParamDecl>>& typeParameters, std::map<std::string, Decl*>& genericContainer)
{
    for (auto& genericParam : typeParameters) {
        genericContainer[genericParam->identifier.GetRawText()] = genericParam.get();
    }
}

void StructuralRuleGDCL01::FindGenericShadowNode(Ptr<Codira::AST::Node>& node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        switch (node->astKind) {
            case ASTKind::ENUM_DECL:
            case ASTKind::EXTEND_DECL:
            case ASTKind::STRUCT_DECL:
            case ASTKind::INTERFACE_DECL:
            case ASTKind::CLASS_DECL: {
                auto pDecl = As<ASTKind::DECL>(node.get());
                if (!pDecl->generic) {
                    break;
                }
                std::map<std::string, Decl*> genericContainer;
                SetGenericContainer(pDecl->generic->typeParameters, genericContainer);
                MemberDeclProcessor(pDecl->GetMemberDecls(), genericContainer);
                break;
            }
            case ASTKind::FUNC_DECL: {
                auto pFuncDecl = As<ASTKind::FUNC_DECL>(node.get());
                if (!pFuncDecl->funcBody || !pFuncDecl->funcBody->generic || !pFuncDecl->funcBody->body) {
                    break;
                }
                std::map<std::string, Decl*> genericContainer;
                SetGenericContainer(pFuncDecl->funcBody->generic->typeParameters, genericContainer);
                MemberDeclProcessor(pFuncDecl->funcBody->body->body, genericContainer);
                break;
            }
            default:
                break;
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGDCL01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    // check shadow: param shadow, function shadow
    FindShadowNode(node);
    // check Generic param shadow
    FindGenericShadowNode(node);
}
} // namespace Codira::CodeCheck
