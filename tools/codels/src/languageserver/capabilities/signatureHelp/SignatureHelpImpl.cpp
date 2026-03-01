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

#include "SignatureHelpImpl.h"
#include <regex>
#include "../../CompilerCodiraProject.h"

namespace ark {
using namespace Codira;
using namespace Codira::AST;

SignatureHelpImpl::SignatureHelpImpl(const ArkAST &ast,
                                     SignatureHelp &result,
                                     Codira::ImportManager &importManager,
                                     const SignatureHelpParams &params,
                                     const Codira::Position &pos)
    : ast(&ast), result(&result), importManager(&importManager), params(params), pos(pos)
{
    hasNameParam = false;
    nameParamPos = -1;
    leftQuoteIndex = -1;
    funcNameIndex = -1;
    visitedFunc = {};
    signatureLabel = {};
    if (ast.semaCache->packageInstance && ast.semaCache->packageInstance->ctx &&
        ast.semaCache->packageInstance->ctx->curPackage &&
        !ast.semaCache->packageInstance->ctx->curPackage->files.empty()) {
        packageNameForPath = GetPkgNameFromNode(ast.semaCache->packageInstance->ctx->curPackage->files[0].get());
    }
}

void SignatureHelpImpl::FindSignatureHelp()
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "SignatureHelpImpl::FindSignatureHelp in.");
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast->tokens, pos, *ast->file);
    SetRealTokensAndIndex();
    leftQuoteIndex = GetDotIndex();
    if (leftQuoteIndex < 1) { return; }
    funcNameIndex = GetFuncNameIndex();
    if (funcNameIndex < 0) { return; }
    int activeParameter = CalActiveParaAndParamPos();
    if (activeParameter == -1) { return; }
    if (!result) {
        return;
    }
    result->activeParameter = static_cast<unsigned int>(activeParameter);
    if (params.context.isRetrigger) {
        DealRetrigger();
        return;
    }
    if (!MemberFuncSignatureHelp()) {
        NormalFuncSignatureHelp();
    }
    FindRealActiveParamPos();
}

void SignatureHelpImpl::SetRealTokensAndIndex()
{
    auto realTokens = &realTokensAndIndex.first;
    int index = ast->GetCurTokenSkipSpace(pos, 0, static_cast<int>(ast->tokens.size()) - 1,
                                          static_cast<int>(ast->tokens.size()) - 1);
    if (ast->tokens[index].kind != TokenKind::STRING_LITERAL &&
        ast->tokens[index].kind != TokenKind::MULTILINE_STRING) {
        realTokens->insert(realTokens->end(), ast->tokens.begin(), ast->tokens.end());
        realTokensAndIndex.second = index;
        return;
    }
    Lexer lexer = Lexer(ast->tokens, ast->packageInstance->diag, *ast->sourceManager);
    auto stringParts = lexer.GetStrParts(ast->tokens[index]);
    for (auto &strP : stringParts) {
        if (strP.strKind != StringPart::EXPR) {
            continue;
        }
        Lexer lexerStr(strP.begin.fileID, strP.value, ast->packageInstance->diag, *ast->sourceManager, strP.begin);
        std::vector<Token> tks = lexerStr.GetTokens();
        realTokens->insert(realTokens->end(), tks.begin(), tks.end());
    }
    realTokensAndIndex.second = GetCurTokenInTargetTokens(pos, *realTokens, 0,
                                          static_cast<int>(realTokens->size()) - 1);
}

void SignatureHelpImpl::DealRetrigger()
{
    if (!ast || !result) {
        return;
    }
    auto index = realTokensAndIndex.second;
    if (index < 0) { return; }
    result->activeSignature = params.context.activeSignatureHelp.activeSignature;
    result->signatures = params.context.activeSignatureHelp.signatures;
    result->signatures = {};
    if (!MemberFuncSignatureHelp()) {
        NormalFuncSignatureHelp();
    }
    FindRealActiveParamPos();
}

bool SignatureHelpImpl::checkModifier(const std::string curPkg, Ptr<const Codira::AST::Decl> decl) const
{
    if (!decl) { return false; }
    return decl->identifier == "init" && decl->astKind == Codira::AST::ASTKind::FUNC_DECL &&
           ((curPkg == decl->fullPackageName
             && !decl->TestAttr(Codira::AST::Attribute::PRIVATE)) ||
            (curPkg != decl->fullPackageName
             && decl->TestAttr(Codira::AST::Attribute::PUBLIC)));
}

void SignatureHelpImpl::FindRealActiveParamPos()
{
    if (!result || !ast) {
        return;
    }
    std::sort(result->signatures.begin(), result->signatures.end());
    if (offset <= result->activeParameter) {
        result->activeParameter -= offset;
    } else {
        result->activeParameter = static_cast<unsigned int>(meanNoMatchParameter);
    }
    if (nameParamPos < 1) {
        if (hasNameParam) {
            result->activeParameter = static_cast<unsigned int>(meanNoMatchParameter);
        }
        return;
    }
    auto realTokens = realTokensAndIndex.first;
    // resolve there are namePram in func( )
    std::string namePram = realTokens[nameParamPos - 1].Value();
    // namePram is in current activeSignature
    auto nowActiveSig = result->activeSignature;
    if (result->signatures.size() <= nowActiveSig) { return; }
    for (size_t j = 0; j < result->signatures[nowActiveSig].parameters.size(); j++) {
        std::string basicString = result->signatures[nowActiveSig].parameters[j];
        std::regex wsRegex("!");
        std::vector<std::string> vectorString(
            std::sregex_token_iterator(basicString.begin(), basicString.end(), wsRegex, -1),
            std::sregex_token_iterator());
        // if the parameter is named, the length of the vector is greater than 1. like  name ! :200
        if (vectorString.size() < 2 || vectorString[0] != namePram) {
            continue;
        }
        result->activeParameter = static_cast<unsigned int>(j);
        return;
    }
    // according to current namePram find the right activeSignatures
    for (size_t i = 0; i < result->signatures.size(); i++) {
        for (size_t j = 0; j < result->signatures[i].parameters.size(); j++) {
            std::string basicString = result->signatures[i].parameters[j];
            std::regex ws("!");
            std::vector<std::string> vectorString(
                std::sregex_token_iterator(basicString.begin(), basicString.end(), ws, -1),
                std::sregex_token_iterator());
            // if the parameter is named, the length of the vector is greater than 1. like  name ! :200
            if (vectorString.size() <= 1 || vectorString[0] != namePram) {
                continue;
            }
            result->activeParameter = static_cast<unsigned int>(j);
            result->activeSignature = static_cast<unsigned int>(i);
            return;
        }
    }
    result->activeParameter = static_cast<unsigned int>(meanNoMatchParameter);
}

void SignatureHelpImpl::FindFunDeclByNode(Codira::AST::Node &node)
{
    if (node.astKind == Codira::AST::ASTKind::FUNC_DECL) {
        auto *funcDecl = dynamic_cast<Codira::AST::FuncDecl*>(&node);
        if (funcDecl == nullptr) { return; }
        ResolveFuncDecl(*funcDecl);
        return;
    }
    if (node.astKind == Codira::AST::ASTKind::CLASS_DECL) {
        ResolveClassDecl(node);
        return;
    }
    if (!ast || !ast->semaCache ||
        !ast->semaCache->packageInstance || !ast->semaCache->packageInstance->ctx) {
        return;
    }
    if (node.astKind == Codira::AST::ASTKind::STRUCT_DECL) {
        auto *structDecl = dynamic_cast<Codira::AST::StructDecl*>(&node);
        bool invalid = structDecl == nullptr || structDecl->body == nullptr || structDecl->body.get() == nullptr ||
                       structDecl->body.get()->decls.empty();
        if (invalid) {
            return;
        }
        for (auto &it: structDecl->body.get()->decls) {
            if (checkModifier(ast->semaCache->packageInstance->ctx->fullPackageName, it.get())) {
                ResolveFuncDecl(*it);
            }
        }
        return;
    }
    if (node.astKind == Codira::AST::ASTKind::VAR_DECL) {
        auto *varDecl = dynamic_cast<VarDecl*>(&node);
        bool invalid = varDecl != nullptr && varDecl->initializer != nullptr &&
                       varDecl->initializer->astKind == Codira::AST::ASTKind::LAMBDA_EXPR;
        if (invalid) {
            auto lambdaExpr = dynamic_cast<LambdaExpr*>(varDecl->initializer.get().get());
            invalid = lambdaExpr == nullptr || lambdaExpr->funcBody == nullptr;
            if (invalid) {
                return;
            }
            std::vector<OwnedPtr<FuncParamList>> &paramLists = lambdaExpr->funcBody->paramLists;
            Signatures signatures = {};
            std::string detail = "lambda(";
            bool firstParams = true;
            for (OwnedPtr<FuncParamList> &paramList : paramLists) {
                for (auto &param : paramList->params) {
                    ResolveParameter(detail, firstParams, param, signatures);
                }
            }
            detail += ")";
            signatures.label = detail;
            if (!result) {
                return;
            }
            result->signatures.push_back(signatures);
        }
        return;
    }
}

void SignatureHelpImpl::CalBackParamPos(const int &index)
{
    if (!ast) {
        return;
    }
    int flag = 0;
    int commaCount = 0;
    auto realTokens = realTokensAndIndex.first;
    for (size_t i = static_cast<size_t>(index) + 1; i < realTokens.size(); ++i) {
        if (realTokens[i] == ")") {
            if (flag == 0) { return; }
            flag--;
        } else if (realTokens[i] == "(") {
            flag++;
        } else if (flag == 0 && realTokens[i] == ",") {
            commaCount++;
        } else if (flag == 0 && realTokens[i] == ":") {
            if (commaCount == 0) {
                nameParamPos = static_cast<int>(i);
            }
            return;
        }
    }
}

int SignatureHelpImpl::CalActiveParaAndParamPos()
{
    int flag = 0;
    int commaCount = 0;
    if (!ast) {
        return commaCount;
    }
    auto realTokens = realTokensAndIndex.first;
    int index = realTokensAndIndex.second;
    if (index == -1) { return -1; }
    if (leftQuoteIndex < 0 || leftQuoteIndex == index) {
        return commaCount;
    }
    for (int i = leftQuoteIndex + 1; i <= index; i++) {
        std::string value = realTokens[i].Value();
        if (value == "(" || value == "[" || value == "{") {
            flag++;
            continue;
        } else if (value == ")" || value == "]" || value == "}") {
            flag--;
            continue;
        }
        if (flag == 0 && value == ":") {
            hasNameParam = true;
            if (commaCount != 0) {
                nameParamPos = i;
            }
        } else if (flag == 0 && value == ",") {
            commaCount++;
            nameParamPos = -1;
        }
    }
    if (flag == 0) {
        CalBackParamPos(index);
    }
    return commaCount;
}

void SignatureHelpImpl::ResolveParameter(std::string &detail, bool &firstParams,
                                         const OwnedPtr<FuncParam> &paramPtr,
                                         Signatures &signatures)
{
    std::string parameter = "";
    if (!firstParams) {
        detail += ", ";
    }
    if (!paramPtr->identifier.Empty()) {
        parameter += paramPtr->identifier.Val();
        if (paramPtr->isNamedParam) {
            parameter += "!";
        }
        parameter += ": ";
    }
    if (paramPtr->ty != nullptr) {
        parameter += GetString(*paramPtr->ty);
        auto assignExpr = paramPtr->assignment.get();
        if (assignExpr && assignExpr->desugarExpr) {
            assignExpr = assignExpr->desugarExpr;
        }
        if (assignExpr && !assignExpr->ToString().empty()) {
            parameter += " = " + assignExpr->ToString();
        }
        detail += parameter;
        signatures.parameters.push_back(parameter);
    }
    firstParams = false;
}

void SignatureHelpImpl::ResolveFuncDecl(Codira::AST::Decl &decl)
{
    if (!ast || IsHiddenDecl(&decl) || IsHiddenDecl(decl.outerDecl)) {
        return;
    }
    auto realTokens = realTokensAndIndex.first;
    // If the current position is the declared position return
    if (leftQuoteIndex > 1 && decl.identifier.Begin() == realTokens[leftQuoteIndex - 1].Begin()) { return; }
    auto *funcDecl = dynamic_cast<Codira::AST::FuncDecl*>(&decl);
    if (!IsFuncDeclValid(funcDecl)) { return; }
    std::string detail = funcDecl->identifier;
    detail += "(";
    Signatures signatures = {};
    // append params
    bool firstParams = true;
    for (auto &paramList : funcDecl->funcBody->paramLists) {
        if (!paramList) { continue; }
        for (auto &param : paramList->params) {
            ResolveParameter(detail, firstParams, param, signatures);
        }
    }
    detail += ")";
    if (!funcDecl->TestAttr(Codira::AST::Attribute::ENUM_CONSTRUCTOR) &&
        funcDecl->identifier != "init" &&
        funcDecl->funcBody->retType != nullptr &&
        funcDecl->funcBody->retType->ty != nullptr) {
        detail += " -> " + GetString(*funcDecl->funcBody->retType->ty);
    }
    signatures.label = detail;
    if (signatureLabel.find(detail) == signatureLabel.end()) {
        result->signatures.push_back(signatures);
        (void) signatureLabel.insert(detail);
    }
}

void SignatureHelpImpl::ResolveClassDecl(Codira::AST::Node &node)
{
    auto *classDecl = dynamic_cast<Codira::AST::ClassDecl*>(&node);
    if (classDecl == nullptr || classDecl->body == nullptr || classDecl->body.get() == nullptr ||
    classDecl->body.get()->decls.empty()) {
        return;
    }
    for (auto &it: classDecl->body.get()->decls) {
        if (checkModifier(ast->semaCache->packageInstance->ctx->fullPackageName, it.get())) {
            ResolveFuncDecl(*it);
        }
    }
}

void SignatureHelpImpl::NormalFuncSignatureHelp()
{
    if (leftQuoteIndex < 1) { return; }
    std::string funcName = ResolveFuncName();
    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                        + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";
    if (!ast || !ast->semaCache || !ast->semaCache->packageInstance || !ast->semaCache->packageInstance->ctx) {
        return;
    }
    auto posSyms = SearchContext(ast->semaCache->packageInstance->ctx, query);
    if (posSyms.empty() || funcName.empty()) {
        return;
    }
    if (std::equal(funcName.begin(), funcName.end(), "super")) {
        FindSuperClassInit(posSyms);
        return;
    }
    std::string curScopeName = ScopeManagerApi::GetScopeNameWithoutTail(posSyms[0]->scopeName);
    for (; !curScopeName.empty(); curScopeName = ScopeManagerApi::GetParentScopeName(curScopeName)) {
        query = "scope_name: " + curScopeName + "&& name: " + funcName;
        auto syms = SearchContext(ast->semaCache->packageInstance->ctx, query);
        for (auto it:syms) {
            if (it && !it->name.empty() && it->name == funcName) {
                FindFunDeclByNode(*it->node);
            }
        }
    }
    if (ast->semaCache->packageInstance->ctx->curPackage == nullptr ||
        ast->semaCache->packageInstance->ctx->curPackage->files.empty()) {
        return;
    }
    auto matchDecls = importManager->GetImportedDeclsByName(*ast->file, funcName);
    for (const auto &decl : matchDecls) {
        if (decl == nullptr) {
            continue;
        }
        FindFunDeclByNode(*decl);
    }
}

std::string SignatureHelpImpl::ResolveFuncName()
{
    if (!ast || leftQuoteIndex < 0) {
        return "";
    }
    auto realTokens = realTokensAndIndex.first;
    std::string funcName = realTokens[leftQuoteIndex - 1].Value();
    // deal signature for  "@ValWithGrad(product, 2.0,3.0, true)"
    // ensure like "@ValWithGrad(product,"
    // offset -2 is "@" && offset -1 is (VJP|Grad|ValWithGrad) && offset +1 is funcName &&
    // then is need signatureHelp
    if (leftQuoteIndex > 1 && realTokens[leftQuoteIndex - CONSTANTS::AD_OFFSET] == "@" &&
        (funcName == "ValWithGrad" || funcName == "Grad" || funcName == "VJP") &&
        static_cast<unsigned long>(leftQuoteIndex + 1) < realTokens.size() &&
        !realTokens[leftQuoteIndex + 1].Value().empty()) {
        funcName = realTokens[leftQuoteIndex + 1].Value();
        offset = 1;
    } else if (funcNameIndex >= 0) {
        funcName = realTokens[funcNameIndex].Value();
    }
    return funcName;
}

bool SignatureHelpImpl::IsFuncDeclValid(Ptr<Codira::AST::FuncDecl> funcDecl)
{
    if (funcDecl == nullptr || !funcDecl->funcBody ||
        (funcDecl->identifier.Begin().line == 0 &&
         funcDecl->identifier.Begin().column == 0)) {
        return false;
    }
    if (visitedFunc.find(funcDecl->identifier.Begin()) != visitedFunc.end()) { return false; }
    (void)visitedFunc.insert(funcDecl->identifier.Begin());
    return true;
}

void SignatureHelpImpl::FindSuperClassInit(const std::vector<Symbol*>& symbols)
{
    for (auto symbol: symbols) {
        if (symbol == nullptr || symbol->node == nullptr) {
            return;
        }
        if (symbol->node->IsClassLikeDecl()) {
            auto decl = dynamic_cast<ClassLikeDecl*>(symbol->node.get());
            for (auto &type : decl->inheritedTypes) {
                if (type != nullptr) {
                    fatherCLassName = type.get()->ty->name;
                    FindFunDeclByType(*type->ty, "init");
                    break;
                }
            }
            break;
        }
    }
}

int SignatureHelpImpl::GetDotIndex() const
{
    if (!ast) {
        return -1;
    }
    auto realTokens = realTokensAndIndex.first;
    int index = realTokensAndIndex.second;
    if (index < 0) {
        return -1;
    }
    int flag = 0;
    for (int i = index; i > 0; --i) {
        if (realTokens[i] == "(") {
            if (flag != 0) {
                flag--;
                continue;
            }
            if (realTokens[i - 1].kind == Codira::TokenKind::IDENTIFIER ||
                realTokens[i - 1].kind == Codira::TokenKind::GT) {
                return i;
            }
        } else if (realTokens[i] == ")") {
            flag++;
        }
    }
    return -1;
}

int SignatureHelpImpl::GetFuncNameIndex() const
{
    auto realTokens = realTokensAndIndex.first;
    if (!ast || leftQuoteIndex < 1 || static_cast<size_t>(leftQuoteIndex) > realTokens.size()) { return -1; }
    if (realTokens[leftQuoteIndex - 1] != ">") {
        return leftQuoteIndex - 1;
    }
    int flag = 0;
    for (int i = leftQuoteIndex - 1; i >= 0; --i) {
        if (realTokens[i] == "<") {
            if (flag == 1) { return i - 1; }
            flag--;
        } else if (realTokens[i] == ">") {
            flag++;
        }
    }
    return -1;
}

void SignatureHelpImpl::FindFuncDeclByDeclType(Ptr<Ty> declTy, const std::string& funcName)
{
    auto id = Ty::GetDeclPtrOfTy<InheritableDecl>(declTy);
    if (!id) { return; }
    bool check = !ast || !ast->semaCache || !ast->semaCache->packageInstance || !ast->semaCache->packageInstance->ctx;
    if (check) {
        return;
    }
    for (auto &decl : id->GetMemberDecls()) {
        if (!decl || decl->identifier != funcName || decl->astKind != Codira::AST::ASTKind::FUNC_DECL) {
            continue;
        }
        if (checkAccess(ast->semaCache->packageInstance->ctx->fullPackageName, *decl)) {
            ResolveFuncDecl(*decl);
        }
    }
    auto inheritableDecl = DynamicCast<InheritableDecl>(id);
    if (!inheritableDecl) { return; }
    auto extendMembers = CompilerCodiraProject::GetInstance()->GetAllVisibleExtendMembers(
        declTy, packageNameForPath, *ast->file);
    for (auto &decl : extendMembers) {
        // Make sure extend has access
        if (!decl || decl->fullPackageName != ast->semaCache->packageInstance->ctx->curPackage->fullPackageName &&
                     decl->fullPackageName != id->fullPackageName && !decl->TestAttr(Attribute::PUBLIC)) {
            continue;
        }
        if (!decl || decl->identifier != funcName ||
            decl->astKind != Codira::AST::ASTKind::FUNC_DECL) {
            continue;
        }
        if (!isThis && decl->TestAttr(Attribute::PRIVATE)) {
            continue;
        }
        ResolveFuncDecl(*decl);
    }
}

bool SignatureHelpImpl::checkAccess(const std::string curPkg, const Codira::AST::Decl &decl) const
{
    if (curPkg != decl.fullPackageName && !decl.TestAttr(Codira::AST::Attribute::PUBLIC)) {
        return false;
    }
    // about static
    if (decl.TestAttr(Codira::AST::Attribute::STATIC)) {
        if (decl.TestAttr(Codira::AST::Attribute::PRIVATE)) {
            return false;
        }
        return true;
    }
    if (decl.TestAttr(Codira::AST::Attribute::PRIVATE)) {
        return false;
    }
    return true;
}

// Complement function name based on the actual variable type.
void SignatureHelpImpl::FindFunDeclByType(Codira::AST::Ty &nodeTy, const std::string funcName)
{
    std::vector<OwnedPtr<Type>>::iterator begin;
    std::vector<OwnedPtr<Type>>::iterator end;
    Meta::match(nodeTy)(
        [this, &begin, &end, &funcName](ClassTy& classTy) {
            bool invalid = !classTy.decl || (classTy.decl->identifier != fatherCLassName && funcName == "init");
            if (invalid) {
                return;
            }
            FindFuncDeclByDeclType(&classTy, funcName);
            begin = classTy.decl->inheritedTypes.begin();
            end = classTy.decl->inheritedTypes.end();
        },
        [this, &begin, &end, &funcName](InterfaceTy& interfaceTy) {
            if (interfaceTy.decl) {
                FindFuncDeclByDeclType(&interfaceTy, funcName);
                begin = interfaceTy.decl->inheritedTypes.begin();
                end = interfaceTy.decl->inheritedTypes.end();
            }
        },
        [this, &begin, &end, &funcName](StructTy& structTy) {
            if (structTy.decl) {
                FindFuncDeclByDeclType(&structTy, funcName);
                begin = structTy.decl->inheritedTypes.begin();
                end = structTy.decl->inheritedTypes.end();
            }
        },
        [&begin, &end, &funcName, this](const EnumTy& enumTy) {
            if (enumTy.decl == nullptr) {
                return;
            }
            for (auto &decl : enumTy.decl->members) {
                if (decl->identifier == funcName) {
                    ResolveFuncDecl(*decl);
                }
            }
            for (auto &decl : enumTy.decl->constructors) {
                if (decl->identifier == funcName) {
                    ResolveFuncDecl(*decl);
                }
            }
            begin = enumTy.decl->inheritedTypes.begin();
            end = enumTy.decl->inheritedTypes.end();
        },
        [&funcName, this](GenericsTy& gnericsTy) {
            for (auto ty : gnericsTy.upperBounds) {
                FindFunDeclByType(*ty, funcName);
            }
        });
    for (; begin != end; ++begin) {
        if (begin->get() && begin->get()->ty) {
            FindFunDeclByType(*begin->get()->ty, funcName);
        }
    }
}

void SignatureHelpImpl::FillingDeclsInPackage(std::string &packageName, const std::string &funcName,
                                              const Codira::AST::Node &curNode)
{
    if (!importManager) {
        return;
    }
    bool check = !ast || !ast->semaCache || !ast->semaCache->packageInstance || !ast->semaCache->packageInstance->ctx;
    if (check) {
        return;
    }
    // filter import packageName
    Ptr<PackageDecl> pkgDecl = importManager->GetImportedPackageDecl(&curNode, packageName).first;
    // deal external import pkgDecl
    if (pkgDecl == nullptr) { return; }
    if (pkgDecl->srcPackage == nullptr) {
        return;
    }
    // resolve curPkg AliasName like import pkgA.classA as newName   pkgA.newName
    if (pkgDecl->identifier.Empty() || pkgDecl->identifier != ast->semaCache->packageInstance->ctx->fullPackageName) {
        if (ast->semaCache->packageInstance->ctx->curPackage == nullptr) { return; }
        auto matchDecl = importManager->GetPackageMembersByName(
            *ast->semaCache->packageInstance->ctx->curPackage, funcName);
        for (auto item : matchDecl) {
            if (item == nullptr) { continue; }
            if (item->fullPackageName == pkgDecl->fullPackageName) {
                FindFunDeclByNode(*item);
            }
        }
        // resolve external import pkgName.*
        matchDecl = importManager->GetPackageMembersByName(*pkgDecl->srcPackage, funcName);
        for (auto item : matchDecl) {
            if (item == nullptr) { continue; }
            FindFunDeclByNode(*item);
        }
        return;
    }
    // if pkgDecl is current package complete all decl in current package
    for (auto &item : pkgDecl->srcPackage->files) {
        if (!item) {
            continue;
        }
        for (auto &it : item->decls) {
            if (it && funcName == it->identifier) {
                ResolveFuncDecl(*it);
            }
        }
    }
}

bool SignatureHelpImpl::MemberFuncSignatureHelp()
{
    int targetOffset = 2;
    auto realTokens = realTokensAndIndex.first;
    bool invalid = funcNameIndex < targetOffset || realTokens[funcNameIndex - 1].Value() != ".";
    if (invalid) {
        return false;
    }
    bool astInvalid = !ast || !ast->semaCache || !ast->semaCache->packageInstance ||
        !ast->semaCache->packageInstance->ctx;
    if (astInvalid) {
        return false;
    }
    std::string funcName = realTokens[funcNameIndex].Value();
    std::string packageName = realTokens[funcNameIndex - targetOffset].Value();
    Position posOfMember = realTokens[funcNameIndex].Begin();
    std::string query = "_ = (" + std::to_string(posOfMember.fileID) + ", "
                        + std::to_string(posOfMember.line) + ", " + std::to_string(posOfMember.column) + ")";
    auto posSyms = SearchContext(ast->semaCache->packageInstance->ctx, query);
    if (posSyms.empty()) {
        return false;
    }
    auto dotSymbol = posSyms[0];
    if (dotSymbol && dotSymbol->astKind == ASTKind::RETURN_EXPR) {
        bool nodeInvalid = !dynamic_cast<ReturnExpr*>(dotSymbol->node.get()) ||
            !dynamic_cast<ReturnExpr*>(dotSymbol->node.get())->expr;
        if (nodeInvalid) {
            return false;
        }
        dotSymbol = dynamic_cast<ReturnExpr*>(dotSymbol->node.get())->expr->symbol;
        if (dotSymbol == nullptr) {
            return false;
        }
    }
    if (dotSymbol && dotSymbol->astKind == ASTKind::MEMBER_ACCESS) {
        bool nodeInvalid = !dynamic_cast<MemberAccess*>(dotSymbol->node.get()) ||
            !dynamic_cast<MemberAccess*>(dotSymbol->node.get())->baseExpr;
        if (nodeInvalid) {
            return false;
        }
        auto node = dynamic_cast<MemberAccess*>(dotSymbol->node.get())->baseExpr.get();
        if (node == nullptr) {
            return false;
        }
        auto nodeTy = (node->symbol && node->symbol->target) ? node->symbol->target->ty : node->ty;
        Logger &logger = Logger::Instance();
        if (!Ty::IsTyCorrect(nodeTy)) {
            auto realPos = node->GetMacroCallNewPos(posOfMember);
            std::string realQuery = "_ = (" + std::to_string(realPos.fileID) + ", "
                        + std::to_string(realPos.line) + ", " + std::to_string(realPos.column) + ")";
            auto realSyms = SearchContext(ast->semaCache->packageInstance->ctx, realQuery);
            if (realSyms.empty()) {
                return false;
            }
            auto realSymbol = realSyms[0];
            if (!realSymbol) {
                return false;
            }
            bool realNodeInvalid = !dynamic_cast<MemberAccess*>(realSymbol->node.get()) ||
                !dynamic_cast<MemberAccess*>(realSymbol->node.get())->baseExpr;
            if (realNodeInvalid) {
                return false;
            }
            node = dynamic_cast<MemberAccess*>(realSymbol->node.get())->baseExpr.get();
            if (node == nullptr) {
                return false;
            }
            nodeTy = (node->symbol && node->symbol->target) ? node->symbol->target->ty : node->ty;
        }
        bool check = nodeTy != nullptr && (Is<ClassTy>(nodeTy.get()) || Is<EnumTy>(nodeTy.get()) ||
                                           Is<InterfaceTy>(nodeTy.get()) || Is<StructTy>(nodeTy.get()) ||
                                           Is<GenericsTy>(nodeTy.get()));
        if (check) {
            auto re = DynamicCast<RefExpr>(node);
            isThis = re && re->isThis;
            FindFunDeclByType(*nodeTy, funcName);
            isThis = false;
            return true;
        }
        FillingDeclsInPackage(packageName, funcName, *node);
    }
    return true;
}
} // namespace ark
