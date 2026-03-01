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

#include <string>
#include "Codira/Basic/Match.h"
#include "../logger/Logger.h"
#include "Utils.h"
#include "ItemResolverUtil.h"

using namespace Codira;
using namespace Codira::AST;

namespace ark {
std::unordered_map<std::string, int> InitKeyMap()
{
    std::unordered_map<std::string, int> keyMap = {};
    for (auto &i: TOKENS) {
        if (IsExperimental(i)) {
            continue;
        }
        std::string token(i);
        if (token.empty() || !isalpha(token[0])) { continue; }
        keyMap[token] = 1;
    }
    return keyMap;
}

const std::unordered_map<std::string, int> keyMap = InitKeyMap();
const int ATTRIBUTE_MACRO_PARAM_NUM = MACRO_ATTR_ARGS;
const int ATTRIBUTE_MACRO_PARAM_POSITION = 0;

std::string ItemResolverUtil::ResolveNameByNode(Codira::AST::Node &node)
{
    if (ark::Is<MacroExpandDecl>(&node)) {
        auto decl = dynamic_cast<MacroExpandDecl *>(&node);
        if (decl->invocation.decl != nullptr) {
            return ResolveNameByNode(*decl->invocation.decl);
        }
        return "";
    } else if (ark::Is<Decl>(&node)) {
        auto decl = dynamic_cast<Decl *>(&node);
        return decl->identifier;
    } else if (ark::Is<Package>(&node)) {
        auto pkg = dynamic_cast<Package *>(&node);
        return pkg->fullPackageName;
    } else {
        return "";
    }
}

CompletionItemKind ItemResolverUtil::ResolveKindByNode(Codira::AST::Node &node)
{
    return Meta::match(node)(
        [](const Codira::AST::VarDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](const Codira::AST::FuncDecl &decl) {
            if (decl.TestAttr(Codira::AST::Attribute::CONSTRUCTOR)) {
                return CompletionItemKind::CIK_CONSTRUCTOR;
            }
            return CompletionItemKind::CIK_METHOD;
        },
        [](const Codira::AST::MacroDecl & /* decl */) {
            return CompletionItemKind::CIK_METHOD;
        },
        [](Codira::AST::ClassDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Codira::AST::InterfaceDecl & /* decl */) {
            return ark::CompletionItemKind::CIK_INTERFACE;
        },
        [](Codira::AST::EnumDecl & /* decl */) {
            return CompletionItemKind::CIK_ENUM;
        },
        [](Codira::AST::GenericParamDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](Codira::AST::StructDecl & /* decl */) {
            return CompletionItemKind::CIK_STRUCT;
        },
        [](Codira::AST::TypeAliasDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Codira::AST::BuiltInDecl & /* decl */) {
            return CompletionItemKind::CIK_CLASS;
        },
        [](Codira::AST::PrimaryCtorDecl & /* decl */) {
            return CompletionItemKind::CIK_METHOD;
        },
        [](Codira::AST::VarWithPatternDecl & /* decl */) {
            return CompletionItemKind::CIK_VARIABLE;
        },
        [](const Codira::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return CompletionItemKind::CIK_MISSING; }
            return ResolveKindByNode(*decl.invocation.decl);
        },
        // ----------- Match nothing --------------------
        []() {
            return CompletionItemKind::CIK_MISSING;
        });
}

CompletionItemKind ItemResolverUtil::ResolveKindByASTKind(Codira::AST::ASTKind &astKind)
{
    switch (astKind) {
        case Codira::AST::ASTKind::VAR_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Codira::AST::ASTKind::FUNC_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Codira::AST::ASTKind::MACRO_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Codira::AST::ASTKind::CLASS_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Codira::AST::ASTKind::INTERFACE_DECL:
            return ark::CompletionItemKind::CIK_INTERFACE;
        case Codira::AST::ASTKind::ENUM_DECL:
            return CompletionItemKind::CIK_ENUM;
        case Codira::AST::ASTKind::GENERIC_PARAM_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Codira::AST::ASTKind::STRUCT_DECL:
            return CompletionItemKind::CIK_STRUCT;
        case Codira::AST::ASTKind::TYPE_ALIAS_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Codira::AST::ASTKind::BUILTIN_DECL:
            return CompletionItemKind::CIK_CLASS;
        case Codira::AST::ASTKind::PRIMARY_CTOR_DECL:
            return CompletionItemKind::CIK_METHOD;
        case Codira::AST::ASTKind::VAR_WITH_PATTERN_DECL:
            return CompletionItemKind::CIK_VARIABLE;
        case Codira::AST::ASTKind::MACRO_EXPAND_DECL:
            return CompletionItemKind::CIK_METHOD;
        default:
            return CompletionItemKind::CIK_MISSING;
    }
}

std::string ItemResolverUtil::ResolveDetailByNode(Codira::AST::Node &node, SourceManager *sourceManager)
{
    std::string detail;
    return Meta::match(node)(
        [&detail, sourceManager](const Codira::AST::VarDecl &decl) {
            ResolveVarDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::FuncDecl &decl) {
            ResolveFuncDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::MacroDecl &decl) {
            ResolveMacroDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](Codira::AST::ClassDecl &decl) {
            ResolveClassDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](Codira::AST::InterfaceDecl &decl) {
            ResolveInterfaceDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::EnumDecl &decl) {
            ResolveEnumDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail](const Codira::AST::StructDecl &decl) {
            ResolveStructDeclDetail(detail, decl);
            return detail;
        },
        [&detail](const Codira::AST::GenericParamDecl &decl) {
            ResolveGenericParamDeclDetail(detail, decl);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::TypeAliasDecl &decl) {
            ResolveTypeAliasDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail](const Codira::AST::BuiltInDecl &decl) {
            ResolveBuiltInDeclDetail(detail, decl);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::PrimaryCtorDecl &decl) {
            ResolvePrimaryCtorDeclDetail(detail, decl, sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::VarWithPatternDecl &decl) {
            ResolvePatternDetail(detail, decl.irrefutablePattern.get(), sourceManager);
            return detail;
        },
        [&detail, sourceManager](const Codira::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return detail; }
            detail = ResolveDetailByNode(*decl.invocation.decl, sourceManager);
            return detail;
        },
        // ----------- Match nothing --------------------
        [&detail]() { return detail; });
}

std::string ItemResolverUtil::ResolveSignatureByNode(const Node &node, Codira::SourceManager *sourceManager,
                                                     bool isCompletionInsert, bool isAfterAT)
{
    std::string signature;
    return Meta::match(node)(
        [&signature](const Codira::AST::VarDecl &decl) {
            if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR) && decl.outerDecl) {
                return signature + decl.outerDecl->identifier + GetGenericParamByDecl(decl.outerDecl->GetGeneric()) +
                       CONSTANTS::DOT + decl.identifier;
            }
            return signature + decl.identifier;
        },
        [&signature, sourceManager, isCompletionInsert, isAfterAT](const Codira::AST::FuncDecl &decl) {
            ResolveFuncDeclSignature(signature, decl, sourceManager, isCompletionInsert, isAfterAT);
            // case: sth...0(), doesn't need to be shown
            return decl.TestAttr(Codira::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature](const Codira::AST::MainDecl &decl) {
            signature += decl.identifier + "()";
            return signature;
        },
        [&signature, sourceManager](const Codira::AST::MacroDecl &decl) {
            ResolveMacroDeclSignature(signature, decl, sourceManager);
            return decl.TestAttr(Codira::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature](const Codira::AST::ClassDecl &classDecl) {
            return signature + classDecl.identifier + GetGenericParamByDecl(classDecl.generic.get());
        },
        [&signature](const Codira::AST::InterfaceDecl &interfaceDecl) {
            return signature + interfaceDecl.identifier + GetGenericParamByDecl(interfaceDecl.generic.get());
        },
        [&signature](const Codira::AST::EnumDecl &enumDecl) {
            return signature + enumDecl.identifier  + GetGenericParamByDecl(enumDecl.generic.get());
        },
        [&signature](const Codira::AST::GenericParamDecl &genericParamDecl) {
            return signature + genericParamDecl.identifier;
        },
        [&signature](const Codira::AST::StructDecl &structDecl) {
            return signature + structDecl.identifier + GetGenericParamByDecl(structDecl.generic.get());
        },
        [&signature](const Codira::AST::Package &decl) {
            return signature + decl.fullPackageName;
        },
        [&signature](const Codira::AST::TypeAliasDecl &decl) {
            return signature + decl.identifier + GetGenericParamByDecl(decl.generic.get());
        },
        [&signature](const Codira::AST::BuiltInDecl &decl) {
            return signature + decl.identifier + GetGenericParamByDecl(decl.generic.get());
        },
        [&signature, sourceManager, isCompletionInsert, isAfterAT](const Codira::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return signature; }
            signature = ResolveSignatureByNode(*decl.invocation.decl, sourceManager, isCompletionInsert, isAfterAT);
            return signature;
        },
        [&signature, sourceManager](const Codira::AST::VarWithPatternDecl &decl) {
            ResolvePatternSignature(signature, decl.irrefutablePattern.get(), sourceManager);
            return signature;
        },
        [&signature, sourceManager, isAfterAT](const Codira::AST::PrimaryCtorDecl &decl) {
            ResolvePrimaryCtorDeclSignature(signature, decl, sourceManager, isAfterAT);
            return signature;
        },
        // ----------- Match nothing --------------------
        [&signature]() {
            return signature;
        });
}

std::string ItemResolverUtil::ResolveSourceByNode(Ptr<Codira::AST::Decl> decl, std::string path)
{
    if (!decl) {
        return "";
    }
    std::string source = "Declared in: ";

    if (decl->curFile) {
        std::string fileName = decl->curFile->fileName;

#ifdef _WIN32
        std::optional<std::string> fileNameOpt =
            Codira::StringConvertor::NormalizeStringToUTF8(fileName);
        if (fileNameOpt.has_value()) {
            fileName = fileNameOpt.value();
        }
        GetRealFileName(fileName, path);
#endif
        source += fileName;
    }

    source += CONSTANTS::BLANK;
    source += "Package info: ";
    source += decl->fullPackageName;

    return source;
}

std::string ItemResolverUtil::GetGenericParamByDecl(Ptr<Codira::AST::Generic> genericDecl)
{
    if (genericDecl == nullptr) { return ""; }
    std::string genericString = "<";
    bool first = true;
    for (auto &it:genericDecl->typeParameters) {
        if (!first) { genericString += ", "; }
        genericString += it->identifier;
        first = false;
    }
    genericString += ">";
    return genericString;
}

int ItemResolverUtil::AddGenericInsertByDecl(std::string &detail, Ptr<Codira::AST::Generic> genericDecl)
{
    int numParm = 0;
    if (genericDecl == nullptr) { return numParm; }
    // if Generic-T only one let it be the last one $0 because $0 can use code completion
    numParm = 1;
    detail += "<";
    bool first = true;
    for (auto &it:genericDecl->typeParameters) {
        if (!first) { detail += ", "; }
        detail += "${" + std::to_string(numParm) + ":" + it->identifier + "}";
        numParm++;
        first = false;
    }
    detail += ">";
    return numParm;
}

std::string ItemResolverUtil::GetGenericInsertByDecl(Ptr<Codira::AST::Generic> genericDecl)
{
    if (genericDecl == nullptr) { return ""; }
    std::string genericInsert = "<";
    bool first = true;
    // if Generic-T only one let it be the last one $0 because $0 can use code completion
    int numParm = 1;
    for (auto &it : genericDecl->typeParameters) {
        if (!it) { continue; }
        if (!first) { genericInsert += ", "; }
        genericInsert += "${" + std::to_string(numParm) + ":" + it->identifier + "}";
        numParm++;
        first = false;
    }
    genericInsert += ">";
    return genericInsert;
}

Ptr<Decl> ItemResolverUtil::GetDeclByTy(Codira::AST::Ty *ty)
{
    Ptr<Decl> decl = nullptr;
    if (ty == nullptr) { return nullptr; }
    return Meta::match(*ty)(
        [&](const ClassTy &classTy) {
            Ptr<Decl> decl = classTy.decl;
            return decl;
        },
        [&](const InterfaceTy &interfaceTy) {
            Ptr<Decl> decl = interfaceTy.decl;
            return decl;
        },
        [&](const StructTy &structTy) {
            Ptr<Decl> decl = structTy.decl;
            return decl;
        },
        [&](const EnumTy &enumTy) {
            Ptr<Decl> decl = enumTy.decl;
            return decl;
        },
        [&]() {
            return decl;
        });
}

std::string ItemResolverUtil::ResolveInsertByNode(const Codira::AST::Node &node, Codira::SourceManager *sourceManager,
                                                  bool isAfterAT)
{
    std::string signature;
    return Meta::match(node)(
        [&signature](const Codira::AST::VarDecl &varDecl) {
            return signature + varDecl.identifier;
        },
        [&signature, sourceManager, isAfterAT](const Codira::AST::FuncDecl &funcDecl) {
            ResolveFuncLikeDeclInsert(signature, funcDecl, sourceManager, isAfterAT);
            // case: sth...0(), doesn't need to be shown
            return funcDecl.TestAttr(Codira::AST::Attribute::HAS_INITIAL) ? "" : signature;
        },
        [&signature, sourceManager, isAfterAT](const Codira::AST::PrimaryCtorDecl &primaryCtorDecl) {
            ResolveFuncLikeDeclInsert(signature, primaryCtorDecl, sourceManager, isAfterAT);
            return signature;
        },
        [&signature, sourceManager](const Codira::AST::MacroDecl &macroDecl) {
            if (macroDecl.desugarDecl && macroDecl.desugarDecl->funcBody) {
                ResolveFuncLikeDeclInsert(signature, *macroDecl.desugarDecl.get(), sourceManager);
            }
            return signature.empty() ? macroDecl.identifier : signature;
        },
        [&signature](const Codira::AST::ClassDecl &classDecl) {
            return signature + classDecl.identifier + GetGenericInsertByDecl(classDecl.GetGeneric());
        },
        [&signature](const Codira::AST::InterfaceDecl &interfaceDecl) {
            return signature + interfaceDecl.identifier + GetGenericInsertByDecl(interfaceDecl.GetGeneric());
        },
        [&signature](const Codira::AST::EnumDecl &enumDecl) {
            return signature + enumDecl.identifier + GetGenericInsertByDecl(enumDecl.GetGeneric());
        },
        [&signature](const Codira::AST::GenericParamDecl &genericParamDecl) {
            return signature + genericParamDecl.identifier;
        },
        [&signature](const Codira::AST::StructDecl &structDecl) {
            return signature + structDecl.identifier + GetGenericInsertByDecl(structDecl.GetGeneric());
        },
        [&signature](const Codira::AST::Package &packageDecl) {
            return signature + packageDecl.fullPackageName;
        },
        [&signature](const Codira::AST::TypeAliasDecl &typeAliasDecl) {
            return signature + typeAliasDecl.identifier + GetGenericInsertByDecl(typeAliasDecl.GetGeneric());
        },
        [&signature](const Codira::AST::BuiltInDecl &builtInDecl) {
            return signature + builtInDecl.identifier + GetGenericInsertByDecl(builtInDecl.GetGeneric());
        },
        [&signature, sourceManager, isAfterAT](const Codira::AST::MacroExpandDecl &decl) {
            if (!decl.invocation.decl) { return signature; }
            signature = ResolveInsertByNode(*decl.invocation.decl, sourceManager, isAfterAT);
            return signature;
        },
        // ----------- Match nothing --------------------
        [&signature]() {
            return signature;
        });
}

std::string ItemResolverUtil::FetchTypeString(const Codira::AST::Type &type)
{
    if (type.ty == nullptr) {
        return "";
    }

    std::string identifier;
    return Meta::match(*(type.ty))(
        [](const ClassTy &ty) {
            return GetString(ty);
        },
        [](const InterfaceTy &ty) {
            return GetGenericString(ty);
        },
        [](const EnumTy &ty) {
            return GetString(ty);
        },
        [](const StructTy &ty) {
            return GetString(ty);
        },
        [](const TypeAliasTy &ty) {
            return ty.declPtr->identifier.Val();
        },
        [](const GenericsTy &ty) {
            return ty.decl->identifier.Val();
        },
        [](const Ty &ty) {
            return ty.String();
        },
        [identifier]() {
            return identifier;
        });
}

template <typename T>
std::string ItemResolverUtil::GetGenericString(const T &t)
{
    std::string typeStr = t.decl->identifier;
    if (t.decl->generic) {
        typeStr += "<";
        for (const auto &parameter : t.decl->generic->typeParameters) {
            typeStr += parameter->identifier;
            typeStr += ",";
        }
        typeStr = typeStr.substr(0, typeStr.size() - 1);
        typeStr += ">";
    }
    return typeStr;
}

std::string GetModifierByNode(const Codira::AST::Node &node)
{
    std::string modifier;
    if (node.TestAttr(Attribute::PUBLIC)) {
        modifier += "public ";
    } else if (node.TestAttr(Attribute::PRIVATE)) {
        modifier += "private ";
    } else if (node.TestAttr(Attribute::INTERNAL)) {
        modifier += "internal ";
    } else if (node.TestAttr(Attribute::PROTECTED)) {
        modifier += "protected ";
    }

    if (node.TestAttr(Attribute::SEALED)) {
        modifier += "sealed ";
    }

    if (node.TestAttr(Attribute::STATIC)) {
        modifier += "static ";
    }

    if (node.TestAttr(Attribute::UNSAFE)) {
        modifier += "unsafe ";
    }

    if (node.TestAttr(Attribute::MUT)) {
        modifier += "mut ";
    }

    // spec: 'open' cannot modify 'abstract class'
    if (node.TestAttr(Attribute::ABSTRACT)) {
        modifier += "abstract ";
    } else if (node.TestAttr(Attribute::OPEN)) {
        modifier += "open ";
    }
    return modifier;
}

void ItemResolverUtil::AddTypeByNodeAndType(std::string &detail, const std::string filePath,
    Ptr<Codira::AST::Node> type, Codira::SourceManager *sourceManager)
{
    if (type == nullptr) { return; }
    std::string result;
    if (!filePath.empty() && sourceManager) {
        int fileId = sourceManager->GetFileID(filePath);
        if (fileId >= 0) {
            result = sourceManager->GetContentBetween(static_cast<unsigned int>(fileId),
                                                      {type->GetBegin().line, type->GetBegin().column},
                                                      {type->GetEnd().line, type->GetEnd().column});
        }
    }
    try {
        if (result.empty() && !type->ToString().empty()) {
            std::string temp = type->ToString();
            Utils::TrimString(temp);
            result += temp;
        }
    } catch (NullPointerException &e) {
        Trace::Log("Invoke compiler api ToString() catch a NullPointerException");
    }
    detail += result;
}

void ItemResolverUtil::ResolveVarDeclDetail(std::string &detail, const Codira::AST::VarDecl &decl,
                                            SourceManager *sourceManager)
{
    if (decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail = "(enum_construct) ";
        detail += ResolveSignatureByNode(decl);
        return;
    }
    detail = "(variable) ";
    detail += GetModifierByNode(decl);
    if (decl.isConst) {
        detail += "const ";
    } else if (decl.isVar) {
        detail += "var ";
    } else {
        detail += "let ";
    }
    // append identifier
    detail += ResolveSignatureByNode(decl);
    if (decl.ty != nullptr && decl.ty->kind == TypeKind::TYPE_FUNC) {
        GetDetailByTy(decl.ty, detail);
        return;
    }
    if (decl.ty == nullptr) { return; }

    // for parse info
    if (GetString(*decl.ty) == "UnknownType") {
        GetInitializerInfo(detail, decl, sourceManager, false);
        return;
    }
    std::string type = GetString(*decl.ty);
    bool hasType = false;
    if (!type.empty()) {
        detail += ": ";
        detail += GetString(*decl.ty);
        hasType = true;
    }
    GetInitializerInfo(detail, decl, sourceManager, hasType);
}

void ItemResolverUtil::GetInitializerInfo(std::string &detail, const Codira::AST::VarDecl &decl,
                                          SourceManager *sourceManager, bool hasType)
{
    if (decl.type != nullptr && decl.curFile != nullptr && !hasType) {
        detail += ": ";
        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.type.get(), sourceManager);
    }
    if (decl.initializer != nullptr && decl.curFile != nullptr) {
        detail += " = ";

        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.initializer.get(), sourceManager);
    }
    if (detail.length() > detailMaxLen) {
        detail = detail.substr(0, detailMaxLen);
        detail += "...";
    }
}

void ItemResolverUtil::GetDetailByTy(const Ty *ty, std::string &detail, bool isLambda)
{
    detail += "(";
    auto *ft = dynamic_cast<const FuncTy*>(ty);
    if (ft == nullptr) { return; }
    bool firstParams = true;
    for (auto &param : ft->paramTys) {
        if (!firstParams) {
            detail += ", ";
        }
        if (param != nullptr) {
            detail += GetString(*param);
        }
        firstParams = false;
    }
    detail += ")";
    if (ft->retTy != nullptr) {
        detail += isLambda ? "->" : ":";
        detail += ft->retTy->String();
    }
}

void ItemResolverUtil::ResolveFuncDeclQuickLook(std::string &quickLook, const Codira::AST::FuncDecl &decl,
                                                Codira::SourceManager *sourceManager)
{
    ResolveFuncDeclSignature(quickLook, decl, sourceManager);
    // Enhancing null-pointer judgment in release version
    if (decl.funcBody == nullptr || decl.funcBody->retType == nullptr || decl.identifier == "init") {
        return;
    }
    // append return type
    if (!FetchTypeString(*decl.funcBody->retType).empty() &&
        !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)
        && decl.funcBody->retType != nullptr) {
        std::string myFilePath = "";
        if (decl.outerDecl != nullptr &&
            decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
        if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
        if (myFilePath.empty()) { return; }
        quickLook += ": ";
        DealTypeDetail(quickLook, decl.funcBody->retType.get(), myFilePath, sourceManager);
    }
}

void ItemResolverUtil::ResolveMacroDeclDetail(std::string &detail, const Codira::AST::MacroDecl &decl,
                                              Codira::SourceManager *sourceManager)
{
    if (decl.desugarDecl == nullptr) {
        return;
    }
    detail = "(macro) ";
    detail += GetModifierByNode(decl);
    detail += "macro ";
    ResolveFuncDeclQuickLook(detail, *decl.desugarDecl.get(), sourceManager);
}

void ItemResolverUtil::ResolveFuncDeclDetail(std::string &detail, const Codira::AST::FuncDecl &decl,
                                             Codira::SourceManager *sourceManager)
{
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        detail = "(macro) ";
        detail += GetModifierByNode(decl);
        detail += "macro ";
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
    } else if (decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail = "(enum_construct) ";
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
    } else {
        detail = "(function) ";
        detail += GetModifierByNode(decl);
        if (decl.propDecl) {
            detail += "prop ";
        } else {
            detail += "func ";
        }
        ResolveFuncDeclQuickLook(detail, decl, sourceManager);
        if (decl.propDecl) {
            std::string propIdentifier = decl.propDecl->identifier;
            std::string desugar = "$" + propIdentifier;
            size_t pos = detail.find(desugar);
            if (detail.find(desugar) == std::string::npos) {
                return;
            }
            detail.replace(pos, desugar.size(), propIdentifier + ".");
        }
    }
}

void ItemResolverUtil::ResolvePrimaryCtorDeclDetail(std::string &detail, const Codira::AST::PrimaryCtorDecl &decl,
                                                    Codira::SourceManager *sourceManager)
{
    detail = "(function) ";
    detail += GetModifierByNode(decl);
    detail += "func ";
    ResolvePrimaryCtorDeclSignature(detail, decl, sourceManager);

    if (decl.funcBody == nullptr || decl.funcBody->retType == nullptr) { return; }
    // append return type
    if (!FetchTypeString(*decl.funcBody->retType).empty() &&
        !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR) && decl.funcBody->retType != nullptr) {
        std::string myFilePath = "";
        if (decl.outerDecl != nullptr &&
            decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
        if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
        if (myFilePath.empty()) { return; }
        detail += ": ";
        DealTypeDetail(detail, decl.funcBody->retType.get(), myFilePath, sourceManager);
    }
}

void ItemResolverUtil::ResolvePrimaryCtorDeclSignature(std::string &detail,
                                                       const Codira::AST::PrimaryCtorDecl &decl,
                                                       Codira::SourceManager *sourceManager, bool isAfterAT)
{
    std::string myFilePath = "";
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    if (decl.funcBody == nullptr) { return; }
    detail += decl.identifier;
    if (decl.funcBody != nullptr && decl.funcBody->generic != nullptr &&
        !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail += GetGenericParamByDecl(decl.funcBody->generic.get());
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "[";
    } else {
        detail += "(";
    }
    ResolveFuncParams(detail, decl.funcBody->paramLists,
                      decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR), sourceManager, myFilePath);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolvePatternSignature(std::string &signature, Ptr<Codira::AST::Pattern> pattern,
                                               Codira::SourceManager *sourceManager)
{
    if (!pattern) { return; }
    if (pattern->astKind == ASTKind::ENUM_PATTERN) {
        auto pEnumPattern = dynamic_cast<EnumPattern*>(pattern.get());
        if (!pEnumPattern) { return; }
        for (auto &ptn : pEnumPattern->patterns) {
            ResolvePatternSignature(signature, ptn.get(), sourceManager);
        }
        return;
    }
    if (pattern->astKind == ASTKind::VAR_PATTERN) {
        auto pVarPattern = dynamic_cast<VarPattern*>(pattern.get());
        if (!pVarPattern || !pVarPattern->varDecl) { return; }
        signature = pVarPattern->varDecl->identifier;
    }
}

void ItemResolverUtil::ResolvePatternDetail(std::string &detail, Ptr<Codira::AST::Pattern> pattern,
                                            Codira::SourceManager *sourceManager)
{
    if (!pattern) { return; }
    if (pattern->astKind == ASTKind::ENUM_PATTERN) {
        auto pEnumPattern = dynamic_cast<EnumPattern*>(pattern.get());
        if (!pEnumPattern) { return; }
        for (auto &ptn : pEnumPattern->patterns) {
            ResolvePatternDetail(detail, ptn.get(), sourceManager);
        }
        return;
    }
    if (pattern->astKind == ASTKind::VAR_PATTERN) {
        auto pVarPattern = dynamic_cast<VarPattern*>(pattern.get());
        if (!pVarPattern || !pVarPattern->varDecl) { return; }
        ResolveVarDeclDetail(detail, *pVarPattern->varDecl.get(), sourceManager);
    }
}

void ItemResolverUtil::ResolveFuncParams(std::string &detail,
                                         const std::vector<OwnedPtr<FuncParamList>> &paramLists,
                                         bool isEnumConstruct,
                                         Codira::SourceManager *sourceManager,
                                         const std::string &filePath,
                                         bool needLastParam)
{
    if (paramLists.empty()) {
        return;
    }
    bool firstParams = true;
    // no Curring, so just the first paramLists
    auto &paramList = paramLists.front();
    size_t paramCount = paramList->params.size();
    if (!needLastParam && paramCount > 0) {
        paramCount--;
    }

    for (size_t i = 0; i < paramCount; i++) {
        auto &param = paramList->params[i];
        if (param == nullptr) {
            continue;
        }
        std::string paramName = param->identifier.GetRawText();
        if (keyMap.find(paramName)!= keyMap.end()) {
            paramName = "`" + paramName + "`";
        }
        if ((param->TestAttr(Codira::AST::Attribute::COMPILER_ADD) &&
                paramName == "macroCallPtr")) { return; }
        if (!firstParams) {
            detail += ", ";
        }
        firstParams = false;
        if (!paramName.empty() && !isEnumConstruct) {
            detail += paramName;
            detail += param->isNamedParam ? "!: " : ": ";
        }
        if (param->ty == nullptr) {
            continue;
        }
        bool getTypeByNodeAndType = param->type != nullptr &&
                                    (GetString(*param->ty) == "UnknownType" ||
                                        (sourceManager && param->type->astKind == Codira::AST::ASTKind::FUNC_TYPE));
        if (getTypeByNodeAndType) {
            AddTypeByNodeAndType(detail, filePath, param->type.get(), sourceManager);
        } else {
            detail += GetString(*param->ty);
        }
        GetFuncNamedParam(detail, sourceManager, filePath, param);
    }
    size_t variadicIndex = paramList->variadicArgIndex;
    if (variadicIndex > 0) {
        detail += ", ...";
    }
}

void ItemResolverUtil::GetFuncNamedParam(std::string &detail, Codira::SourceManager *sourceManager,
    const std::string &filePath, const OwnedPtr<Codira::AST::FuncParam> &param)
{
    try {
        auto assignExpr = param->assignment.get();
        if (assignExpr && assignExpr->desugarExpr) {
            assignExpr = assignExpr->desugarExpr;
        }
        if (assignExpr && !assignExpr->ToString().empty()) {
            detail += " = ";
            AddTypeByNodeAndType(detail, filePath, assignExpr, sourceManager);
        }
    } catch (NullPointerException &e) {
        Trace::Log("Invoke compiler api catch a NullPointerException");
    }
}

void ItemResolverUtil::ResolveMacroParams(std::string &detail,
                                          const std::vector<OwnedPtr<FuncParamList>> &paramLists)
{
    for (auto &paramList : paramLists) {
        if (paramList && paramList->params.size() == ATTRIBUTE_MACRO_PARAM_NUM &&
            paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]) {
            detail = detail + "[" + paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]->identifier + ": Tokens]";
        }
    }
}

void ItemResolverUtil::ResolveMacroParamsInsert(std::string &detail,
                                                const std::vector<OwnedPtr<FuncParamList>> &paramLists)
{
    for (auto &paramList : paramLists) {
        if (paramList && paramList->params.size() == ATTRIBUTE_MACRO_PARAM_NUM &&
            paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]) {
            detail = detail + "[${1:" + paramList->params[ATTRIBUTE_MACRO_PARAM_POSITION]->identifier + ": Tokens}]";
        }
    }
}

void ItemResolverUtil::ResolveFuncDeclSignature(std::string &detail, const Codira::AST::FuncDecl &decl,
                                                Codira::SourceManager *sourceManager, bool isCompletionInsert,
                                                bool isAfterAT)
{
    std::string myFilePath = "";
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    if (decl.funcBody == nullptr) { return; }
    if (decl.identifier.Val() == "arrayInit1" || decl.identifier.Val() == "arrayInit2" &&
        decl.fullPackageName == "std.core") {
        return;
    }
    if (decl.TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        if (decl.outerDecl) {
            detail += decl.outerDecl->identifier + GetGenericParamByDecl(decl.outerDecl->GetGeneric()) + ".";
        }
    }
    detail += decl.identifier;
    if (isCompletionInsert && decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        ResolveMacroParams(detail, decl.funcBody->paramLists);
        return;
    }
    if (decl.funcBody->generic != nullptr && !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        detail += GetGenericParamByDecl(decl.funcBody->generic.get());
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "[";
    } else {
        detail += "(";
    }
    ResolveFuncParams(detail, decl.funcBody->paramLists,
                      decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR),
                      sourceManager, myFilePath);
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolveMacroDeclSignature(std::string &detail, const Codira::AST::MacroDecl &decl,
                                                 Codira::SourceManager *sourceManager)
{
    if (decl.desugarDecl == nullptr || decl.desugarDecl->funcBody == nullptr || decl.curFile == nullptr) {
        return;
    }
    detail += decl.identifier;
    if (sourceManager && decl.desugarDecl && decl.desugarDecl.get()->funcBody) {
        ResolveMacroParams(detail, decl.desugarDecl.get()->funcBody->paramLists);
        return;
    }
    detail += "(";
    ResolveFuncParams(detail, decl.desugarDecl->funcBody->paramLists, false, sourceManager,
                      decl.curFile->filePath);
    detail += ")";
}

void ItemResolverUtil::ResolveFuncTypeParamSignature(std::string &detail,
                                                     const std::vector<OwnedPtr<Codira::AST::Type>> &paramTypes,
                                                     Codira::SourceManager *sourceManager,
                                                     const std::string &filePath,
                                                     bool needLastParam)
{
    if (paramTypes.empty()) {
        return;
    }
    bool firstParams = true;
    size_t index = 0;
    size_t paramCount = paramTypes.size();
    if (!needLastParam && paramCount > 0) {
        paramCount--;
    }
    for (size_t i = 0; i < paramCount; i++) {
        auto &paramType = paramTypes[i];
        if (!paramType) {
            continue;
        }
        if (!firstParams) {
            detail += ", ";
        }
        firstParams = false;
        bool getTypeByNodeAndType = GetString(*paramType->ty) == "UnknownType"
                                        || (sourceManager && paramType->astKind == Codira::AST::ASTKind::FUNC_TYPE);
        if (getTypeByNodeAndType) {
            ItemResolverUtil::AddTypeByNodeAndType(detail, filePath, paramType.get(), sourceManager);
        } else {
            if (!paramType->typeParameterName.empty()) {
                detail += paramType->typeParameterName + ": ";
            }
            detail += GetString(*paramType->ty);
        }
    }
}

void ItemResolverUtil::ResolveFuncTypeParamInsert(std::string &detail,
                                                  const std::vector<OwnedPtr<Codira::AST::Type>> &paramTypes,
                                                  Codira::SourceManager *sourceManager,
                                                  const std::string &filePath,
                                                  int &numParm,
                                                  bool needLastParam,
                                                  bool needDefaultParamName)
{
    if (paramTypes.empty()) {
        return;
    }
    bool firstParams = true;
    size_t paramCount = paramTypes.size();
    if (!needLastParam && paramCount > 0) {
        paramCount--;
    }
    std::unordered_set<std::string> parameterNameSet;
    size_t parameterNum = 1;
    std::string paramInsert;
    for (size_t i = 0; i < paramCount; i++) {
        auto &paramType = paramTypes[i];
        if (!paramType) {
            continue;
        }
        if (!firstParams) {
            detail += ", ";
        }
        if (numParm >= 0) {
            detail += "${" + std::to_string(numParm) + ":";
            numParm++;
        }
        if (paramType->typeParameterName.empty() && needDefaultParamName) {
            std::string defaultName = "arg" + std::to_string(parameterNum);
            while (parameterNameSet.find(defaultName) != parameterNameSet.end()) {
                parameterNum++;
                defaultName = "arg" + std::to_string(parameterNum);
            }
            detail += defaultName + ": ";
            parameterNameSet.insert(defaultName);
        }
        bool getTypeByNodeAndType = GetString(*paramType->ty) == "UnknownType"
                                    || (sourceManager && paramType->astKind == Codira::AST::ASTKind::FUNC_TYPE);
        if (getTypeByNodeAndType) {
            ItemResolverUtil::AddTypeByNodeAndType(detail, filePath, paramType.get(), sourceManager);
        } else {
            if (!paramType->typeParameterName.empty()) {
                detail += paramType->typeParameterName + ": ";
                parameterNameSet.insert(paramType->typeParameterName);
            }
            detail += GetString(*paramType->ty);
        }
        firstParams = false;
        if (numParm >= 0) {
            detail += "}";
        }
    }
}

int ItemResolverUtil::ResolveFuncParamInsert(std::string &detail, const std::string myFilePath,
    Ptr<Codira::AST::FuncParam> param, int numParm, Codira::SourceManager *sourceManager, bool isEnumConstruct)
{
    std::string paramName = isEnumConstruct ? "": param->identifier.GetRawText();
    if (keyMap.find(paramName)!= keyMap.end()) {
        paramName = "`" + paramName + "`";
    }
    if (param->isNamedParam) {
        detail += (paramName.empty() ? "" : (paramName + ": "));
        detail += "${" + std::to_string(numParm) + ":";
        numParm++;
        auto assignExpr = param->assignment.get();
        if (assignExpr && assignExpr->desugarExpr) {
            assignExpr = assignExpr->desugarExpr;
        }
        bool flag = param->ty && assignExpr && !assignExpr->ToString().empty();
        bool getTypeByNodeAndType = param->type != nullptr &&
                                    (GetString(*param->ty) == "UnknownType" ||
                                    (sourceManager && param->type->astKind == Codira::AST::ASTKind::FUNC_TYPE));
        if (getTypeByNodeAndType) {
            ItemResolverUtil::AddTypeByNodeAndType(detail, myFilePath, param->type.get(), sourceManager);
            detail += flag ? " = " : "";
            ItemResolverUtil::AddTypeByNodeAndType(detail, myFilePath, assignExpr, sourceManager);
        } else {
            detail += param->ty ? GetString(*param->ty) : "";
            detail += flag ? (" = " + assignExpr->ToString()) : "";
        }
    } else {
        detail += "${" + std::to_string(numParm) + ":";
        numParm++;
        detail += (paramName.empty() ? "" : (paramName + ": "));
        bool getTypeByNodeAndType = param->type != nullptr &&
                                    (GetString(*param->ty) == "UnknownType" ||
                                    (sourceManager && param->type->astKind == Codira::AST::ASTKind::FUNC_TYPE));
        if (getTypeByNodeAndType) {
            ItemResolverUtil::AddTypeByNodeAndType(detail, myFilePath, param->type.get(), sourceManager);
        } else {
            detail += (param->ty ? GetString(*param->ty) : "");
        }
    }
    return numParm;
}

std::string ItemResolverUtil::ResolveFollowLambdaSignature(const Codira::AST::Node &node,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    std::string signature;
    return Meta::match(node)(
        [&signature, sourceManager, initFuncReplace](const Codira::AST::FuncDecl &decl) {
            ResolveFollowLambdaFuncSignature(signature, decl, sourceManager, initFuncReplace);
            return signature;
        },
        [&signature, sourceManager, initFuncReplace](const Codira::AST::PrimaryCtorDecl &decl) {
            ResolveFollowLambdaFuncSignature(signature, decl, sourceManager, initFuncReplace);
            return signature;
        },
        [&signature, sourceManager, initFuncReplace](const Codira::AST::VarDecl &decl) {
            ResolveFollowLambdaVarSignature(signature, decl, sourceManager, initFuncReplace);
            return signature;
        },
        [&signature]() { return signature; });
}

std::string ItemResolverUtil::ResolveFollowLambdaInsert(const Codira::AST::Node &node,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    std::string insert;
    return Meta::match(node)(
        [&insert, sourceManager, initFuncReplace](const Codira::AST::FuncDecl &decl) {
            ResolveFollowLambdaFuncInsert(insert, decl, sourceManager, initFuncReplace);
            return insert;
        },
        [&insert, sourceManager, initFuncReplace](const Codira::AST::PrimaryCtorDecl &decl) {
            ResolveFollowLambdaFuncInsert(insert, decl, sourceManager, initFuncReplace);
            return insert;
        },
        [&insert, sourceManager, initFuncReplace](const Codira::AST::VarDecl &decl) {
            ResolveFollowLambdaVarInsert(insert, decl, sourceManager, initFuncReplace);
            return insert;
        },
        [&insert]() { return insert; });
}

template<typename T>
void ItemResolverUtil::ResolveFollowLambdaFuncSignature(std::string &detail, const T &decl,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC) || !decl.funcBody
        || decl.funcBody->paramLists.empty()) {
        return;
    }
    // no Curring, so just the first paramLists
    auto &paramList = decl.funcBody->paramLists.front();
    if (!paramList || paramList->params.empty()) {
        return;
    }
    auto &lastParam = paramList->params.back();
    if (!lastParam) {
        return;
    }
    if (!lastParam->type || lastParam->type->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    auto funcType = DynamicCast<FuncType *>(lastParam->type.get());
    if (!funcType) {
        return;
    }
    std::string signature;
    std::string myFilePath;
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    bool isInvalid =
        decl.astKind == ASTKind::FUNC_DECL
        && (decl.identifier.Val().compare("arrayInit1") == 0 || decl.identifier.Val().compare("arrayInit2") == 0)
        && decl.fullPackageName.compare("std.core") == 0;
    if (isInvalid) {
        return;
    }
    signature += decl.identifier;
    if (decl.funcBody->generic != nullptr && !decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        signature += ItemResolverUtil::GetGenericParamByDecl(decl.funcBody->generic.get());
    }
    bool isCustomAnnotationFlag = ItemResolverUtil::IsCustomAnnotation(decl);
    DealEmptyParamFollowLambda(decl, sourceManager, paramList, signature, myFilePath);
    ItemResolverUtil::ResolveFuncTypeParamSignature(signature, funcType->paramTypes,
        sourceManager, myFilePath);
    signature += " => ";
    if (funcType->retType && !ItemResolverUtil::FetchTypeString(*funcType->retType).empty()) {
        ItemResolverUtil::DealTypeDetail(signature, funcType->retType.get(),
            myFilePath, sourceManager);
    }
    signature += " }";
    if (!initFuncReplace.empty()) {
        auto len = static_cast<long long>(decl.identifier.Val().size());
        signature = signature.replace(signature.begin(), signature.begin() + len, initFuncReplace);
    }
    detail = signature;
}

template <typename T>
void ItemResolverUtil::DealEmptyParamFollowLambda(const T &decl, Codira::SourceManager *sourceManager,
    OwnedPtr<Codira::AST::FuncParamList> &paramList, std::string &signature, const std::string &myFilePath)
{
    if (paramList->params.size() == 1) {
        signature += " { ";
    } else {
        signature += "(";
        ResolveFuncParams(signature, decl.funcBody->paramLists,
            decl.TestAttr(Attribute::ENUM_CONSTRUCTOR),
            sourceManager, myFilePath, false);
        signature += ") { ";
    }
}

void ItemResolverUtil::ResolveFollowLambdaVarSignature(std::string &detail, const Codira::AST::VarDecl &decl,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    if (!decl.type || decl.type->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    std::string signature = decl.identifier;
    std::string myFilePath;
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    auto funcType = DynamicCast<FuncType *>(decl.type.get());
    if (!funcType) {
        return;
    }
    if (funcType->paramTypes.empty()) {
        return;
    }
    auto lastType = funcType->paramTypes.back().get();
    if (!lastType || lastType->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    auto lastFuncType = DynamicCast<FuncType *>(lastType.get());
    if (!lastFuncType) {
        return;
    }

    std::string flSignature = decl.identifier;
    if (funcType->paramTypes.size() == 1) {
        flSignature += " { ";
    } else {
        flSignature += "(";
        ItemResolverUtil::ResolveFuncTypeParamSignature(flSignature, funcType->paramTypes,
            sourceManager, myFilePath, false);
        flSignature += ") { ";
    }
    ItemResolverUtil::ResolveFuncTypeParamSignature(flSignature, lastFuncType->paramTypes,
        sourceManager, myFilePath);
    flSignature += " => ";
    if (lastFuncType->retType && !ItemResolverUtil::FetchTypeString(*lastFuncType->retType).empty()) {
        ItemResolverUtil::DealTypeDetail(flSignature, lastFuncType->retType.get(),
            myFilePath, sourceManager);
    }
    flSignature += " }";
    detail = flSignature;
}

template<typename T>
void ItemResolverUtil::ResolveFollowLambdaFuncInsert(std::string &detail, const T &decl,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC) || !decl.funcBody
        || decl.funcBody->paramLists.empty()) {
        return;
    }
    // no Curring, so just the first paramLists
    auto &paramList = decl.funcBody->paramLists.front();
    if (!paramList || paramList->params.empty()) {
        return;
    }
    auto &lastParam = paramList->params.back();
    if (!lastParam) {
        return;
    }
    if (!lastParam->type || lastParam->type->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    auto funcType = DynamicCast<FuncType *>(lastParam->type.get());
    if (!funcType) {
        return;
    }
    std::string myFilePath;
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    myFilePath = (decl.curFile != nullptr) ? decl.curFile->filePath : myFilePath;
    bool isInvalid =
        decl.astKind == ASTKind::FUNC_DECL
        && (decl.identifier.Val().compare("arrayInit1") == 0 || decl.identifier.Val().compare("arrayInit2") == 0)
        && decl.fullPackageName.compare("std.core") == 0;
    if (isInvalid) {
        return;
    }
    std::string insertText = decl.identifier;
    int numParm = BuildLambdaFuncPreParamInsert(decl, sourceManager, paramList, myFilePath, insertText);
    int temp = -1;
    ItemResolverUtil::ResolveFuncTypeParamInsert(insertText, funcType->paramTypes, sourceManager,
        myFilePath, temp, true, true);
    insertText += " => ";
    if (funcType->retType && !ItemResolverUtil::FetchTypeString(*funcType->retType).empty()) {
        insertText += "${" + std::to_string(numParm) + ":";
        ItemResolverUtil::DealTypeDetail(insertText, funcType->retType.get(),
            myFilePath, sourceManager);
        insertText += "}";
    }
    insertText += " }";
    if (!initFuncReplace.empty()) {
        auto len = static_cast<long long>(decl.identifier.Val().size());
        insertText = insertText.replace(insertText.begin(), insertText.begin() + len, initFuncReplace);
    }
    detail = insertText;
}

template <typename T>
int ItemResolverUtil::BuildLambdaFuncPreParamInsert(const T &decl, Codira::SourceManager *sourceManager,
    OwnedPtr<Codira::AST::FuncParamList> &paramList, const std::string &myFilePath, std::string &insertText)
{
    int numParm = 1;
    bool isEnumConstruct = false;
    if (!decl.TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
        numParm = AddGenericInsertByDecl(insertText, decl.GetGeneric()) + 1;
        if ((decl.identifier.Val().compare("init") == 0 && decl.astKind == ASTKind::FUNC_DECL ||
                decl.astKind == ASTKind::PRIMARY_CTOR_DECL) && decl.outerDecl != nullptr) {
            std::string temp = "";
            numParm = AddGenericInsertByDecl(temp, decl.outerDecl->GetGeneric()) + 1;
        }
    } else {
        isEnumConstruct = true;
    }
    size_t paramCount = paramList->params.size();
    if (paramCount > 0) {
        paramCount--;
    }
    if (paramList->params.size() == 1) {
        insertText += " { ";
    } else {
        insertText += "(";
        bool firstParams = true;
        for (size_t i = 0; i < paramCount; i++) {
            auto &param = paramList->params[i];
            if (param == nullptr) {
                continue;
            }
            if (!firstParams) {
                insertText += ", ";
            }
            numParm = ResolveFuncParamInsert(insertText, myFilePath, param.get(),
                numParm, sourceManager, isEnumConstruct);
            firstParams = false;
            insertText += "}";
        }
        insertText += ") { ";
    }

    return numParm;
}

void ItemResolverUtil::ResolveFollowLambdaVarInsert(std::string &detail, const Codira::AST::VarDecl &decl,
    Codira::SourceManager *sourceManager, const std::string &initFuncReplace)
{
    if (!decl.type || decl.type->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    std::string signature = decl.identifier;
    std::string myFilePath;
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) { myFilePath = decl.outerDecl->curFile->filePath; }
    if (decl.curFile != nullptr) { myFilePath = decl.curFile->filePath; }
    auto funcType = DynamicCast<FuncType *>(decl.type.get());
    if (!funcType) {
        return;
    }
    if (funcType->paramTypes.empty()) {
        return;
    }
    auto lastType = funcType->paramTypes.back().get();
    if (!lastType || lastType->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    auto lastFuncType = DynamicCast<FuncType *>(lastType.get());
    if (!lastFuncType) {
        return;
    }
    std::string flInsertText = decl.identifier;
    int flNumParm = 1;
    if (funcType->paramTypes.size() == 1) {
        flInsertText += " { ";
    } else {
        flInsertText += "(";
        ItemResolverUtil::ResolveFuncTypeParamInsert(flInsertText, funcType->paramTypes,
            sourceManager, myFilePath, flNumParm, false);
        flInsertText += ") { ";
    }
    int temp = -1;
    ItemResolverUtil::ResolveFuncTypeParamInsert(flInsertText, lastFuncType->paramTypes,
        sourceManager, myFilePath, temp, true, true);
    flInsertText += " => ";
    if (lastFuncType->retType && !ItemResolverUtil::FetchTypeString(*lastFuncType->retType).empty()) {
        flInsertText += "${" + std::to_string(flNumParm) + ":";
        ItemResolverUtil::DealTypeDetail(flInsertText, lastFuncType->retType.get(),
            myFilePath, sourceManager);
        flInsertText += "}";
    }
    flInsertText += " }";
    detail = flInsertText;
}

void ItemResolverUtil::ResolveParamListFuncTypeVarDecl(const Codira::AST::Node &node, std::string &label,
    std::string &insertText, SourceManager *sourceManager)
{
    auto decl = dynamic_cast<const Codira::AST::VarDecl*>(&node);
    if (!decl || !decl->type || decl->type->astKind != ASTKind::FUNC_TYPE) {
        return;
    }
    std::string signature = decl->identifier;
    std::string myFilePath;
    if (decl->outerDecl != nullptr &&
        decl->outerDecl->curFile != nullptr) { myFilePath = decl->outerDecl->curFile->filePath; }
    if (decl->curFile != nullptr) { myFilePath = decl->curFile->filePath; }
    auto funcType = DynamicCast<FuncType *>(decl->type.get());
    if (!funcType) {
        return;
    }
    signature += "(";
    ItemResolverUtil::ResolveFuncTypeParamSignature(signature, funcType->paramTypes,
        sourceManager, myFilePath);
    signature += ")";
    label = signature;
    insertText = decl->identifier;
    insertText += "(";
    int numParm = 1;
    ItemResolverUtil::ResolveFuncTypeParamInsert(insertText, funcType->paramTypes, sourceManager,
        myFilePath, numParm);
    insertText += ")";
}

template<typename T>
void GetFilePath(std::string &myFilePath, const T &decl)
{
    if (decl.outerDecl != nullptr &&
        decl.outerDecl->curFile != nullptr) {
        myFilePath = decl.outerDecl->curFile->filePath; 
    }
    if (decl.curFile != nullptr) { 
        myFilePath = decl.curFile->filePath; 
    }
}

template<typename T>
void ItemResolverUtil::ResolveFuncLikeDeclInsert(std::string &detail, const T &decl,
                                                 Codira::SourceManager *sourceManager, bool isAfterAT)
{
    std::string myFilePath = "";
    GetFilePath(myFilePath, decl);
    detail += decl.identifier;
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC)) {
        ResolveMacroParamsInsert(detail, decl.funcBody->paramLists);
        return;
    }
    int numParm = 1;
    bool isEnumConstruct = false;
    if (!decl.TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR)) {
        numParm = AddGenericInsertByDecl(detail, decl.GetGeneric()) + 1;
        if ((decl.identifier.Val().compare("init") == 0 && decl.astKind == ASTKind::FUNC_DECL ||
             decl.astKind == ASTKind::PRIMARY_CTOR_DECL) && decl.outerDecl != nullptr) {
            std::string temp = "";
            numParm = AddGenericInsertByDecl(temp, decl.outerDecl->GetGeneric()) + 1;
        }
    } else {
        isEnumConstruct = true;
    }
    bool isCustomAnnotationFlag = IsCustomAnnotation(decl);
    detail += ((isAfterAT && isCustomAnnotationFlag) ? "[" : "(");
    // append params
    bool firstParams = true;
    if (decl.funcBody == nullptr || decl.funcBody->paramLists.empty()) { return; }
    auto paramList = decl.funcBody->paramLists[0].get();
    for (auto &param : paramList->params) {
        if (!firstParams) {
            detail += ", ";
        }
        if (param) {
            numParm = ResolveFuncParamInsert(detail, myFilePath, param.get(), numParm, sourceManager, isEnumConstruct);
        }
        firstParams = false;
        detail += "}";
    }
    if (isAfterAT && isCustomAnnotationFlag) {
        detail += "]";
    } else {
        detail += ")";
    }
}

void ItemResolverUtil::ResolveClassDeclDetail(std::string &detail, Codira::AST::ClassDecl &decl,
                                              Codira::SourceManager *sourceManager)
{
    detail = "(class) ";
    detail += GetModifierByNode(decl);
    detail += "class ";
    detail += ResolveSignatureByNode(decl);
    // Do not display basic type -- Object.
    if (decl.inheritedTypes.size() == 0) {
        return;
    }
    if (decl.inheritedTypes.size() == 1 && decl.inheritedTypes[0]->ty->IsObject()) {
        return;
    }
    detail += " <: ";
    bool isFirst = true;
    for (auto &iterFace : decl.inheritedTypes) {
        if (iterFace == nullptr || decl.curFile == nullptr || iterFace->ty->IsObject()) {
            continue;
        }
        if (!isFirst) {
            detail += " & ";
        }
        DealTypeDetail(detail, iterFace.get(), decl.curFile->filePath, sourceManager);
        isFirst = false;
    }
}

void ItemResolverUtil::ResolveInterfaceDeclDetail(std::string &detail, Codira::AST::InterfaceDecl &decl,
                                                  Codira::SourceManager *sourceManager)
{
    detail = "(interface) ";
    detail += GetModifierByNode(decl);
    detail += "interface ";
    detail += ResolveSignatureByNode(decl);
    if (decl.inheritedTypes.empty()) {
        return;
    }
    detail += " extends ";
    bool firstInterface = true;
    for (auto &iterFace : decl.inheritedTypes) {
        if (iterFace == nullptr || decl.curFile == nullptr) { continue; }
        if (!firstInterface) {
            detail += ", ";
        }
        DealTypeDetail(detail, iterFace.get(), decl.curFile->filePath, sourceManager);
        firstInterface = false;
    }
}

void ItemResolverUtil::ResolveEnumDeclDetail(std::string &detail, const Codira::AST::EnumDecl &decl,
                                             Codira::SourceManager *sourceManager)
{
    detail = "(enum) ";
    detail += GetModifierByNode(decl);
    detail += "enum ";
    detail += ResolveSignatureByNode(decl);
    if (decl.inheritedTypes.size() == 0) {
        return;
    }
    detail += " <: ";
    bool isFirst = true;
    for (auto &type : decl.inheritedTypes) {
        if (type == nullptr || decl.curFile == nullptr) { continue; }
        if (!isFirst) {
            detail += " & ";
        }
        DealTypeDetail(detail, type.get(), decl.curFile->filePath, sourceManager);
        isFirst = false;
    }
}

void ItemResolverUtil::ResolveStructDeclDetail(std::string &detail, const Codira::AST::StructDecl &decl)
{
    detail = "(struct) ";
    detail += GetModifierByNode(decl);
    detail += "struct ";
    detail += ResolveSignatureByNode(decl);
}

void ItemResolverUtil::ResolveGenericParamDeclDetail(std::string &detail, const Codira::AST::GenericParamDecl &decl)
{
    detail += "genericParam ";
    detail += ResolveSignatureByNode(decl);
}

void ItemResolverUtil::ResolveTypeAliasDetail(std::string &detail, const Codira::AST::TypeAliasDecl &decl,
                                              Codira::SourceManager *sourceManager)
{
    detail += GetModifierByNode(decl);
    detail += "type ";
    detail += ResolveSignatureByNode(decl);
    if (decl.type == nullptr) {
        return;
    }
    detail += " = ";
    if (decl.curFile != nullptr && FetchTypeString(*decl.type.get()) == "UnknownType") {
        AddTypeByNodeAndType(detail, decl.curFile->filePath, decl.type.get(), sourceManager);
        return;
    }

    detail += FetchTypeString(*decl.type);
}

void ItemResolverUtil::DealTypeDetail(std::string &detail, Ptr<Codira::AST::Type> type,
                                      const std::string &filePath, Codira::SourceManager *sourceManager)
{
    if (!type) { return; }
    if (FetchTypeString(*type) == "UnknownType") {
        AddTypeByNodeAndType(detail, filePath, type, sourceManager);
        return;
    }
    detail += FetchTypeString(*type);
}

void ItemResolverUtil::ResolveBuiltInDeclDetail(std::string &detail, const BuiltInDecl &decl)
{
    detail = "(BuiltInType) ";
    detail += GetModifierByNode(decl);
    detail += "Type ";
    detail += ResolveSignatureByNode(decl);
}

bool ItemResolverUtil::IsCustomAnnotation(const Codira::AST::Decl &decl)
{
    if (!decl.outerDecl) {
        return false;
    }
    auto outerDecl = decl.outerDecl;
    for (auto &annotation : outerDecl->annotations) {
        if (!annotation) { continue; }
        if (annotation->astKind == ASTKind::ANNOTATION) { return true; }
    }
    return false;
}
} // namespace ark
