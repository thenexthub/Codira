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

#include "DocumentSymbolImpl.h"
#include <cangjie/AST/Node.h>
#include "../../common/Utils.h"

using namespace Codira;
using namespace Codira::AST;
namespace ark {
std::map<Attribute, std::string>
    DocumentSymbolImpl::supportAttribute = {{Attribute::PUBLIC,         "public"},
                                            {Attribute::CONSTRUCTOR,      "constructor"},
                                            {Attribute::ENUM_CONSTRUCTOR, "constructor"},
                                            {Attribute::STATIC,           "static"},
                                            {Attribute::PRIVATE,          "private"},
                                            {Attribute::PROTECTED,        "protected"},
                                            {Attribute::ABSTRACT,         "abstract"}};

std::unordered_set<ASTKind>
    DocumentSymbolImpl::supportSymbolKind = {ASTKind::EXTEND_DECL, ASTKind::VAR_DECL,
                                             ASTKind::CLASS_DECL, ASTKind::INTERFACE_DECL,
                                             ASTKind::ENUM_DECL, ASTKind::FUNC_DECL,
                                             ASTKind::STRUCT_DECL, ASTKind::TYPE_ALIAS_DECL,
                                             ASTKind::MACRO_DECL, ASTKind::MAIN_DECL};

std::string GetDocumentSymbolDetail(Ptr<const Decl> decl)
{
    std::string detail = "";
    std::map<ASTKind, std::string> astToString;
    (void) astToString.emplace(ASTKind::CLASS_DECL, "class");
    (void) astToString.emplace(ASTKind::STRUCT_DECL, "struct");
    (void) astToString.emplace(ASTKind::EXTEND_DECL, "extend");
    (void) astToString.emplace(ASTKind::TYPE_ALIAS_DECL, "type alias");
    if (astToString.find(decl->astKind) != astToString.end()) {
        detail = astToString.find(decl->astKind)->second;
    }
    if (decl->astKind == ASTKind::VAR_DECL) {
        if (!decl->TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
            detail = GetVarDeclType(dynamic_cast<const VarDecl*>(decl.get()));
        }
    } else if (decl->astKind == ASTKind::PROP_DECL) {
        auto propDecl = dynamic_cast<const PropDecl*>(decl.get());
        if (propDecl) {
            if (propDecl->isVar) {
                detail = "getter and setter";
            } else {
                detail = "only getter";
            }
        }
    }
    for (auto &attribute: DocumentSymbolImpl::GetSupportAttribute()) {
        if (decl->TestAttr(attribute.first)) {
            if (!detail.empty()) {
                detail.append(", ");
            }
            detail.append(attribute.second);
        }
    }
    return detail;
}

void GetDocumentSymbolRange(const ArkAST &ast, const Decl &decl, const std::string &resultName, Range &range,
                            Range &selectionRange)
{
    auto begin = decl.begin;
    auto end = decl.end;

    PositionUTF8ToIDE(ast.tokens, begin, decl);
    PositionUTF8ToIDE(ast.tokens, end, decl);
    range = {begin, end};
    range = TransformFromChar2IDE(range);

    selectionRange = GetDeclRange(decl, static_cast<int>(CountUnicodeCharacters(resultName)));
    UpdateRange(ast.tokens, selectionRange, decl);
    selectionRange = TransformFromChar2IDE(selectionRange);
    if (!(range.start <= selectionRange.start && selectionRange.end <= range.end)) {
        range = selectionRange;
    }
    if (decl.astKind == ASTKind::TYPE_ALIAS_DECL || decl.astKind == ASTKind::VAR_DECL) {
        range = selectionRange;
    }
}

template<class T>
std::string GetGenericList(T *decl)
{
    std::string genericList;
    if (decl == nullptr || decl->generic == nullptr || decl->generic->typeParameters.empty()) {
        return "";
    }
    bool firstGenericParam = true;
    genericList += "<";
    for (auto &generic : decl->generic->typeParameters) {
        if (!firstGenericParam) {
            genericList += ", ";
        }
        if (generic->identifier.Empty()) {
            continue;
        }
        genericList += generic->identifier;
        firstGenericParam = false;
    }
    genericList += ">";
    return genericList;
}

std::string GetDocumentSymbolNameByFuncDecl(const FuncDecl &decl, bool isMain)
{
    std::string detail;
    std::string name = decl.TestAttr(Codira::AST::Attribute::COMPILER_ADD) ? decl.identifierForLsp : decl.identifier;
    detail += name + GetGenericList(decl.funcBody.get().get());
    if (decl.funcBody == nullptr) {
        return detail;
    }
    detail += "(";
    // append params
    const std::vector<std::string> &funcParamsTypeName = GetFuncParamsTypeName(&decl);
    for (decltype(funcParamsTypeName.size()) i = 0; i < funcParamsTypeName.size(); ++i) {
        if (i != 0) {
            detail += ", ";
        }
        detail += funcParamsTypeName[i];
    }
    detail += ")";
    if (isMain) {
        detail += ": Int64";
        return detail;
    }
    Ty *retTy = decl.funcBody->retType ? decl.funcBody->retType->ty : nullptr;
    std::string retTyStr;
    if (retTy) {
        retTyStr = GetString(*retTy);
    }
    if (!retTyStr.empty() &&
        !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR) &&
        retTyStr != "UnknownType") {
        detail += ": " + retTyStr;
    }
    return detail;
}

void GetDocumentSymbolName(std::string &identifierName, std::string &name, Ptr<Decl> decl)
{
    if (!decl) {
        return;
    }
    if (decl->astKind == ASTKind::FUNC_DECL) {
        identifierName = decl->TestAttr(Codira::AST::Attribute::COMPILER_ADD) ? decl->identifierForLsp
                                                                               : decl->identifier;
        auto *funcDecl = dynamic_cast<const FuncDecl*>(decl.get());
        if (!funcDecl) {
            return;
        }
        name = GetDocumentSymbolNameByFuncDecl(*funcDecl, false);
        return;
    } else if (decl->astKind == ASTKind::MAIN_DECL) {
        identifierName = decl->TestAttr(Codira::AST::Attribute::COMPILER_ADD) ? decl->identifierForLsp
                                                                               : decl->identifier;
        auto *mainDecl = dynamic_cast<MainDecl*>(decl.get());
        if (!mainDecl || mainDecl->desugarDecl == nullptr) {
            return;
        }
        name = GetDocumentSymbolNameByFuncDecl(*mainDecl->desugarDecl, true);
        return;
    } else if (decl->astKind == ASTKind::EXTEND_DECL) {
        auto *extendDecl = dynamic_cast<ExtendDecl*>(decl.get());
        if (extendDecl && extendDecl->extendedType && extendDecl->extendedType->ty) {
            // extend primitive type, including char, bool, Int32, etc.
            auto *primitiveType = dynamic_cast<PrimitiveType*>(extendDecl->extendedType.get().get());
            if (primitiveType == nullptr) {
                identifierName = extendDecl->extendedType->ty->name;
                name = identifierName + GetGenericList(decl.get());
            } else {
                identifierName = primitiveType->str;
                name = identifierName;
            }
        }
        return;
    }
    identifierName = decl->identifier;
    name = identifierName + GetGenericList(decl.get());
}

DocumentSymbol GetDocumentSymbolByDecl(const ArkAST &ast, Ptr<Decl> decl)
{
    DocumentSymbol result;
    if (decl == nullptr || InValidDecl(decl) || decl->TestAttr(Attribute::MACRO_EXPANDED_NODE)) {
        return result;
    }
    // real decl name
    std::string identifierName;
    // get DocumentSymbol.name
    GetDocumentSymbolName(identifierName, result.name, decl);
    if (result.name.find("<invalid identifier>") != std::string::npos || result.name.empty()) {
        return DocumentSymbol{};
    }
    // get DocumentSymbol.detail, kind, range, selectionRange
    result.detail = GetDocumentSymbolDetail(decl);
    // get DocumentSymbol.kind
    result.kind = GetSymbolKind(decl->astKind);
    // get DocumentSymbol.range, DocumentSymbol.selectionRange
    GetDocumentSymbolRange(ast, *decl, identifierName, result.range, result.selectionRange);
    // get DocumentSymbol.children
    for (auto &member: decl->GetMemberDecls()) {
        if (InValidDecl(member.get())) {
            continue;
        }
        DocumentSymbol documentSymbol = GetDocumentSymbolByDecl(ast, member.get());
        if (!documentSymbol.name.empty()) {
            result.children.emplace_back(documentSymbol);
        }
    }
    return result;
}

DocumentSymbol GetDocumentSymbolByEnumDecl(const ArkAST &ast, Ptr<Decl> decl)
{
    DocumentSymbol result;
    auto *enumDecl = dynamic_cast<EnumDecl*>(decl.get());
    if (enumDecl == nullptr || enumDecl->TestAttr(Attribute::MACRO_EXPANDED_NODE)) {
        return result;
    }
    result = GetDocumentSymbolByDecl(ast, enumDecl);
    for (auto &enumMember : enumDecl->constructors) {
        DocumentSymbol documentSymbol = GetDocumentSymbolByDecl(ast, enumMember.get());
        if (!documentSymbol.name.empty()) {
            result.children.emplace_back(documentSymbol);
        }
    }
    return result;
}

std::map<Attribute, std::string> DocumentSymbolImpl::GetSupportAttribute()
{
    return supportAttribute;
}

void DocumentSymbolImpl::FindDocumentSymbols(const ArkAST &ast, std::vector<DocumentSymbol> &result)
{
    if (ast.file == nullptr || ast.file->decls.empty()) {
        return;
    }
    for (auto &toplevelDecl : ast.file->decls) {
        if (!toplevelDecl) {
            continue;
        }
        auto aimDecl = toplevelDecl.get();
        auto astKind = toplevelDecl->astKind;
        // just need no macroExpand Node
        if (astKind == ASTKind::MACRO_EXPAND_DECL) {
            auto macroExpandDecl = dynamic_cast< MacroExpandDecl *>(toplevelDecl.get().get());
            if (macroExpandDecl && macroExpandDecl->invocation.decl) {
                aimDecl = macroExpandDecl->invocation.decl.get();
                astKind = aimDecl->astKind;
            }
        } else if (toplevelDecl->TestAttr(Attribute::MACRO_EXPANDED_NODE) && toplevelDecl->curMacroCall) {
            auto *macroExpandDecl = dynamic_cast<MacroExpandDecl *>(toplevelDecl->curMacroCall.get());
            if (macroExpandDecl && macroExpandDecl->invocation.decl) {
                aimDecl = macroExpandDecl->invocation.decl.get();
                astKind = aimDecl->astKind;
            }
        }
        if (!supportSymbolKind.count(astKind)) {
            continue;
        }
        DocumentSymbol documentSymbol;
        if (astKind == ASTKind::ENUM_DECL) {
            documentSymbol = GetDocumentSymbolByEnumDecl(ast, aimDecl);
        } else {
            documentSymbol = GetDocumentSymbolByDecl(ast, aimDecl);
        }
        if (!documentSymbol.name.empty()) {
            result.emplace_back(documentSymbol);
        }
    }
}
}
