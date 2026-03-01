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

#include "FindOverrideMethodsImpl.h"
#include "FindOverrideMethodsUtils.h"
#include "Codira/Lex/Token.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Modules/ModulesUtils.h"
#include "../../common/Utils.h"
#include "../../CompilerCodiraProject.h"
#include "../../common/ItemResolverUtil.h"

namespace ark {
std::string FindOverrideMethodsImpl::curFilePath;
std::string FindOverrideMethodsImpl::fullPkgName;
std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>>
    FindOverrideMethodsImpl::genericReplaceMap;
const std::string TAB = "    ";
const std::string NEWLINE = "\n";
const std::string SPACE = " ";

const std::unordered_map<std::string, int> keyMap = InitKeyMap();

const std::unordered_set<TokenKind> OPERATOR_TO_OVERLOAD = {
    TokenKind::LSQUARE, TokenKind::RSQUARE, TokenKind::NOT,
    TokenKind::EXP, TokenKind::MUL, TokenKind::MOD,
    TokenKind::DIV, TokenKind::ADD, TokenKind::SUB,
    TokenKind::LSHIFT, TokenKind::RSHIFT, TokenKind::LT,
    TokenKind::LE, TokenKind::GT, TokenKind::GE,
    TokenKind::EQUAL, TokenKind::NOTEQ, TokenKind::BITAND,
    TokenKind::BITXOR, TokenKind::BITOR};


std::string FilterModifiers(Ptr<Decl> decl, const std::vector<std::string>& modifiers)
{
    std::unordered_set<std::string> filterItems = {"abstract"};
    if (!decl->TestAnyAttr(Attribute::OPEN, Attribute::SEALED)) {
        filterItems.insert("open");
    }

    if (decl->astKind == ASTKind::INTERFACE_DECL) {
        filterItems.insert("public");
    }

    if (decl->astKind == ASTKind::EXTEND_DECL) {
        filterItems.insert("open");
        filterItems.insert("override");
        filterItems.insert("redef");
    }

    std::string result{};
    for (const auto& modifier: modifiers) {
        if (filterItems.count(modifier)) {
            continue;
        }
        result += modifier + SPACE;
    }
    return result;
}

std::string GetSuperFuncCall(const Ptr<InheritableDecl>& owner, FuncDecl* funcDecl,
                             const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    // if lack of function body, don't add super call
    bool isCanSuperCall =
        std::any_of(canSuperCall.begin(), canSuperCall.end(), [&owner](const Ptr<ClassLikeDecl>& decl) {
            return owner->curFile == decl->curFile && owner->begin == decl->begin && owner->end == decl->end;
        });
    if (funcDecl->TestAttr(Attribute::ABSTRACT) || (!funcDecl->TestAttr(Attribute::STATIC) && !isCanSuperCall) ||
        IsHiddenDecl(funcDecl) || IsHiddenDecl(owner)) {
        return TAB + "throw Exception(\"Function not implemented.\")\n";
    }

    std::string superPrefix = "super.";
    if (funcDecl->TestAttr(Attribute::STATIC) && !isCanSuperCall) {
        superPrefix = owner->identifier.Val() + ".";
    }

    bool paramFirst = true;
    std::string paramsText;
    for (auto& paramList: funcDecl->funcBody->paramLists) {
        for (auto& param: paramList->params) {
            std::string paramName = param->identifier.GetRawText();
            if (keyMap.find(paramName)!= keyMap.end()) {
                paramName = "`" + paramName + "`";
            }
            if (param == nullptr ||
                (param->TestAttr(Codira::AST::Attribute::COMPILER_ADD) &&
                 paramName == "macroCallPtr")) { continue; }
            if (paramName.empty()) {
                continue;
            }
            if (!paramFirst) {
                paramsText += ", ";
            }
            if (param->isNamedParam) {
                paramsText += paramName + ": ";
            }
            paramsText += paramName;
            paramFirst = false;
        }
    }
    return TAB + superPrefix + funcDecl->identifier +"(" + paramsText + ")" + NEWLINE;
}

void ApplyReplace(std::unique_ptr<TypeDetail>& detail, const std::unordered_map<std::string, std::string>& replace)
{
    for (auto& item: replace) {
        detail->SetIdentifier(item.first, item.second);
    }
}

bool IsOverrideable(const Ptr<ClassLikeDecl> owner, const Ptr<Decl> member)
{
    // make sure member is member of owner before call this function
    if (auto funcDecl = DynamicCast<FuncDecl>(member)) {
        if ((funcDecl->TestAttr(Attribute::OPEN) ||
            (funcDecl->TestAttr(Attribute::ABSTRACT) && owner->TestAttr(Attribute::ABSTRACT)) &&
            !funcDecl->TestAttr(Attribute::CONSTRUCTOR))) {
                return true;
            }
    }

    if (auto propDecl = DynamicCast<PropDecl>(member)) {
        if (propDecl->TestAttr(Attribute::OPEN) ||
            (propDecl->TestAttr(Attribute::ABSTRACT) && owner->TestAttr(Attribute::ABSTRACT))) {
                return true;
            }
    }

    return false;
}

void FindOverrideMethodsImpl::AddFuncItemsToResult(const Ptr<Decl>& decl, const Ptr<InheritableDecl>& owner,
                          OverrideMethodsItem& item, const std::vector<FuncDecl*>& overridableMethods,
                          const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    for (auto method: overridableMethods) {
        OverrideMethodInfo info;
        std::string superCallText = GetSuperFuncCall(owner, method, canSuperCall);
        auto funcDetail = ResolveFuncDetail(method);
        if (auto inheritableDecl = DynamicCast<InheritableDecl>(owner)) {
            if (genericReplaceMap.find(inheritableDecl) != genericReplaceMap.end()) {
                const auto& replace = genericReplaceMap.at(inheritableDecl);
                for (auto& rule: replace) {
                    funcDetail.SetGenericType(rule.first, rule.second);
                }
            }
        }
        auto cleanDetail = funcDetail.ToString();
        info.deprecated = method->HasAnno(AnnotationKind::DEPRECATED);
        auto funcTextPos = cleanDetail.find(" func ");
        auto modifiers = FilterModifiers(decl, funcDetail.modifiers);
        info.signatureWithRet = cleanDetail.substr(funcTextPos+ strlen(" func "));
        if (OPERATOR_TO_OVERLOAD.count(method->op)) {
            info.insertText = modifiers + "operator func " + info.signatureWithRet + " {\n" + superCallText + "}";
        } else {
            info.insertText = modifiers + "func " + info.signatureWithRet + " {\n" + superCallText + "}";
        }
        item.overrideMethodInfos.emplace_back(info);
    }
}

void FindOverrideMethodsImpl::AddPropItemsToResult(const Ptr<Decl> decl, const Ptr<InheritableDecl> owner,
                                                   OverrideMethodsItem& item,
                                                   const std::vector<PropDecl*>& overridableProps,
                                                   const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    for (auto prop: overridableProps) {
        auto propDetail = ResolvePropDetail(prop);
        if (auto inheritableDecl = DynamicCast<InheritableDecl>(decl)) {
            if (genericReplaceMap.find(inheritableDecl) != genericReplaceMap.end()) {
                const auto& replace = genericReplaceMap.at(inheritableDecl);
                for (auto& rule: replace) {
                    propDetail.SetGenericType(rule.first, rule.second);
                }
            }
        }
        auto modifiers = FilterModifiers(decl, propDetail.modifiers);
        auto identifier = propDetail.identifier;
        auto propTy = propDetail.type->ToString();
        std::string superCall = TAB + TAB + "super." + identifier + NEWLINE;
        bool isCanSuperCall =
            std::any_of(canSuperCall.begin(), canSuperCall.end(), [&owner](const Ptr<ClassLikeDecl>& decl) {
                return decl->curFile == owner->curFile && decl->begin == owner->begin && decl->end == owner->end;
            });
        if (prop->TestAttr(Attribute::ABSTRACT) || (!prop->TestAttr(Attribute::STATIC) &&!isCanSuperCall)) {
            superCall = TAB + TAB + "throw Exception(\"Prop not implemented.\")\n";
        }

        if (prop->TestAttr(Attribute::STATIC) && !isCanSuperCall) {
            superCall = TAB + TAB + owner->identifier.Val() + "." + identifier + NEWLINE;
        }
        std::string getter = "get() {\n" + superCall + TAB + "}";
        std::string setter;
        if (modifiers.find("mut") != std::string::npos) {
            setter = "set(v) {\n" + TAB + "}";
        }
        OverrideMethodInfo info;
        info.isProp = true;
        info.deprecated = prop->HasAnno(AnnotationKind::DEPRECATED);
        info.signatureWithRet = identifier + ": " + propTy;
        info.insertText = modifiers + "prop " + info.signatureWithRet + " {\n" + TAB + getter;
        if (!setter.empty()) {
            info.insertText += NEWLINE + TAB + setter;
        }
        info.insertText += "\n}";
        item.overrideMethodInfos.emplace_back(info);
    }
}

std::vector<Ptr<ClassLikeDecl>> GetCanSuperCallDecls(const Ptr<Decl>& decl)
{
    if (auto classDecl = DynamicCast<ClassDecl*>(decl)) {
        if (auto superClass = classDecl->GetSuperClassDecl()) {
            return superClass->GetAllSuperDecls();
        }
    }
    return {};
}

template <class T>
void FilterMemberDecls(Ptr<T> decl, std::vector<FuncDecl*>& implementedMethods,
                       std::vector<PropDecl*>& implementedDecls)
{
    for (auto& memberDecl: decl->GetMemberDecls()) {
        if (FuncDecl* funcDecl = DynamicCast<FuncDecl*>(memberDecl.get())) {
            implementedMethods.emplace_back(funcDecl);
            continue;
        }

        if (PropDecl* propDecl = DynamicCast<PropDecl*>(memberDecl.get())) {
            implementedDecls.emplace_back(propDecl);
            continue;
        }
    }
}

void ExtractReplace(const Ptr<InheritableDecl>& decl, std::unordered_map<Ptr<InheritableDecl>,
                    std::unordered_map<std::string, std::string>>& genericReplaceMap)
{
    if (auto inheritableDecl = DynamicCast<InheritableDecl>(decl)) {
        const auto& inheritedTypes = inheritableDecl->inheritedTypes;
        std::unordered_map<std::string, std::string> replace{};
        if (genericReplaceMap.find(decl) != genericReplaceMap.end()) {
            replace = genericReplaceMap[decl];
        }
        for (const auto& inheritedType: inheritedTypes) {
            Ptr<ClassLikeDecl> inheritedDecl = nullptr;
            if (auto clsTy = DynamicCast<ClassTy*>(inheritedType->ty)) {
                inheritedDecl = clsTy->declPtr;
            } else if (auto ifTy = DynamicCast<InterfaceTy*>(inheritedType->ty)) {
                inheritedDecl = ifTy->declPtr;
            }
            if (inheritedDecl && inheritedType->ty) {
                auto originalDetail = ResolveType(inheritedDecl->ty);
                auto newDetail = ResolveType(inheritedType->ty);
                ApplyReplace(newDetail, replace);
                std::unordered_map<std::string, std::string> myReplace;
                originalDetail->Diff(newDetail, myReplace);
                genericReplaceMap[inheritedDecl] = myReplace;
            }
        }
    }
}

void FindOverrideMethodsImpl::FindOverrideMethods(const ArkAST &ast, FindOverrideMethodResult &result,
                                                  Codira::Position pos, bool isExtend)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "FindOverrideMethodsImpl::FindOverrideMethods in.");
    // update pos fileID
    pos.fileID = ast.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);
    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    LowFileName(curFilePath);
    // check current token is the kind required in function CheckTokenKind(TokenKind)
    std::vector<Symbol *> syms;
    std::vector<Ptr<Codira::AST::Decl> > decls;
    Ptr<Decl> decl = ast.GetDeclByPosition(pos, syms, decls, {true, false});
    if (!decl && !isExtend) { return; }

    if (isExtend && syms.empty()) {
        // in extended type is basic type case:
        //      try to obtain extend decl, pos begin of extended type identifier before macro expand
        std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                            + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";

        syms = SearchContext(ast.packageInstance->ctx, query);
    }

    if (syms.empty() || !syms[0] || (!isExtend && !DynamicCast<InheritableDecl*>(decl))) {
        return;
    }

    // in extend case, identifier is a class decl or others,
    // discard the search result and get the ExtendDecl by found symbols
    if (isExtend) {
        if (syms.size() <= 1 || !syms[1]) {
            return;
        }
        if (auto extendDecl = DynamicCast<ExtendDecl*>(syms[1]->node)) {
            decl = extendDecl;
        } else {
            return;
        }
    }

    if (!decl) {
        return;
    }
    
    if (auto inheritableDecl = DynamicCast<InheritableDecl *>(decl)) {
        std::vector<FuncDecl *> implementedMethods;
        std::vector<PropDecl *> implementedDecls;
        genericReplaceMap.clear();
        GetImplementedMethodsAndProps(ast, inheritableDecl, implementedMethods, implementedDecls);
        std::vector<Ptr<ClassLikeDecl>> canSuperCall = GetCanSuperCallDecls(decl);
        OverridableFuncAndPropMap overrideableMethodsAndPropMap;
        GetOverridableMethodsAndPropsMap(
            inheritableDecl, overrideableMethodsAndPropMap, implementedMethods, implementedDecls);

        for (const auto &mapItem : overrideableMethodsAndPropMap) {
            const Ptr<InheritableDecl> owner = mapItem.first;
            OverrideMethodsItem item;
            item.package = GetPkgNameFromNode(mapItem.first);
            item.identifier = mapItem.first->identifier;
            item.kind = mapItem.first->astKind == ASTKind::INTERFACE_DECL ? "interface"
                        : mapItem.first->astKind == ASTKind::CLASS_DECL   ? "class"
                                                                          : "others";
            AddFuncItemsToResult(decl, owner, item, mapItem.second.first, canSuperCall);
            AddPropItemsToResult(decl, owner, item, mapItem.second.second, canSuperCall);
            result.overrideMethods.emplace_back(item);
        }
    }
}

void FindOverrideMethodsImpl::GetImplementedMethodsAndProps(const ArkAST &ast, Ptr<InheritableDecl> decl,
    std::vector<FuncDecl*>& implementedMethods, std::vector<PropDecl*>& implementedDecls)
{
    if (auto classDecl = DynamicCast<ClassLikeDecl*>(decl)) {
        FilterMemberDecls<ClassLikeDecl>(classDecl, implementedMethods, implementedDecls);
    }

    if (auto structDecl = DynamicCast<StructDecl*>(decl)) {
        FilterMemberDecls<StructDecl>(structDecl, implementedMethods, implementedDecls);
    }

    if (auto enumDecl = DynamicCast<EnumDecl*>(decl)) {
        FilterMemberDecls<EnumDecl>(enumDecl, implementedMethods, implementedDecls);
    }

    if (auto* extendDecl = DynamicCast<ExtendDecl*>(decl)) {
        FilterMemberDecls<ExtendDecl>(extendDecl, implementedMethods, implementedDecls);
        auto extendTyDecl =  ast.GetDelFromType(extendDecl->extendedType);
        if (auto originDecl = DynamicCast<InheritableDecl*>(extendTyDecl)) {
            auto superTys = originDecl->GetAllSuperDecls();
            for (auto superTy: superTys) {
                FilterMemberDecls<ClassLikeDecl>(superTy, implementedMethods, implementedDecls);
            }
        }
    }
}

void FindOverrideMethodsImpl::GetOverridableMethodsAndPropsMap(Ptr<InheritableDecl> decl,
                                                               OverridableFuncAndPropMap& overrideableMethodsAndPropMap,
                                                               const std::vector<FuncDecl*>& implementedMethods,
                                                               const std::vector<PropDecl*>& implementedProps)
{
    std::vector<FuncDecl*> cachedFuncResult = implementedMethods;
    std::vector<PropDecl*> cachedPropResult = implementedProps;
    // check identical func decl in different level
    auto checkRepeatFunc = [&cachedFuncResult](FuncDecl* func) -> bool {
        return std::any_of(cachedFuncResult.begin(), cachedFuncResult.end(),
                           [func](const FuncDecl* cmpDecl)-> bool {
            // this function can‘t cover all cases
            return IsFuncSignatureIdentical(*cmpDecl, *func);
        });
    };

    auto checkRepeatProp = [&cachedPropResult](PropDecl* prop) -> bool {
        return std::any_of(cachedPropResult.begin(), cachedPropResult.end(),
                           [prop](const PropDecl* cmpDecl)-> bool {
            return prop->identifier == cmpDecl->identifier;
        });
    };

    ExtractReplace(decl, genericReplaceMap);

    auto superTys = decl->GetAllSuperDecls();
    for (auto superTy: superTys) {
        ExtractReplace(superTy, genericReplaceMap);
        auto& memberDecls = superTy->GetMemberDecls();
        for (auto& memberDecl: memberDecls) {
            if (auto funcDecl = DynamicCast<FuncDecl*>(memberDecl.get())) {
                if (IsOverrideable(superTy, funcDecl) && !checkRepeatFunc(funcDecl)) {
                    overrideableMethodsAndPropMap[superTy].first.emplace_back(funcDecl);
                }
                cachedFuncResult.emplace_back(funcDecl);
            }

            if (auto propDecl = DynamicCast<PropDecl*>(memberDecl.get())) {
                if (IsOverrideable(superTy, propDecl) && !checkRepeatProp(propDecl)) {
                    overrideableMethodsAndPropMap[superTy].second.emplace_back(propDecl);
                }
                cachedPropResult.emplace_back(propDecl);
            }
        }
    }
}
}
