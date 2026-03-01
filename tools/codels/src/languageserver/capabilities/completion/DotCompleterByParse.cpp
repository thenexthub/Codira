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

#include "DotCompleterByParse.h"
#include <unordered_set>
#include <vector>
#include "Codira/AST/Node.h"
#include "Codira/Utils/CastingTemplate.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Utils;
using namespace Codira::FileUtil;

namespace {
std::string GetFullImportId(const std::vector<std::string> &prefixPaths, const std::string &id)
{
    std::string fullIdentifier;
    for (auto const &prefix : prefixPaths) {
        fullIdentifier += prefix + CONSTANTS::DOT;
    }
    return fullIdentifier + id;
}

auto CollectImportIdMap(const Ptr<File> &file)
{
    // key: import identifier, value: full identifier
    std::unordered_map<std::string, std::string> idToFullIdMap;
    for (const auto &import : file->imports) {
        if (import->IsImportAll()) {
            continue;
        }

        if (import->IsImportSingle()) {
            auto fullIdentifier = GetFullImportId(import->content.prefixPaths, import->content.identifier);
            (void)idToFullIdMap.insert_or_assign(import->content.identifier, fullIdentifier);
            continue;
        }

        if (import->IsImportAlias()) {
            auto fullIdentifier = GetFullImportId(import->content.prefixPaths, import->content.identifier);
            (void)idToFullIdMap.insert_or_assign(import->content.aliasName, fullIdentifier);
            continue;
        }
    }
    return idToFullIdMap;
}
}

namespace ark {
void DotCompleterByParse::Complete(const ArkAST &input,
                                   const Position &pos,
                                   const std::string &prefix)
{
    CompletionEnv env;
    if (!context) {
        env.OutputResult(result);
        return;
    }
    env.parserAst = &input;
    env.cache = input.semaCache;
    env.curPkgName = context->fullPackageName;
    env.SetSyscap(SplitFullPackage(env.curPkgName).first);
    // Complete SubPackage, include sourece dependency and binary dependency.
    // eg: import std.[sub_pkg].[sub_pkg]...
    for (const auto &iter : Codira::LSPCompilerInstance::codeoLibraryMap) {
        auto moduleName = SplitFullPackage(prefix).first;
        if (moduleName != iter.first) {
            continue;
        }
        for (const auto &fullPkgName : iter.second) {
            env.DotPkgName(fullPkgName, prefix);
        }
    }

    for (const auto &iter : Codira::LSPCompilerInstance::usrCodeoFileCacheMap) {
        auto curModule = SplitFullPackage(env.curPkgName).first;
        if (curModule != iter.first) {
            continue;
        }
        for (const auto &package : iter.second) {
            env.DotPkgName(package.first, prefix);
        }
    }

    // Complete accessible top-level declaration in import sepc
    // eg: import std.os.[top-level declaration]
    auto srcPkgName = input.file->curPackage->fullPackageName;
    auto idToFullIdMap = ::CollectImportIdMap(input.file->curFile);
    auto fullImportId = GetFullPrefix(input, pos, prefix);

    if (idToFullIdMap.find(prefix) != idToFullIdMap.end()) {
        fullImportId = idToFullIdMap[prefix];
    }
    auto targetPkg = importManager->GetPackageDecl(fullImportId);
    // filter root pkg symbols if cur module is combined
    std::string curModule = SplitFullPackage(srcPkgName).first;
    if (targetPkg && !CompilerCodiraProject::GetInstance()->IsCombinedSym(curModule, srcPkgName, fullImportId)) {
        auto members = importManager->GetPackageMembers(srcPkgName, targetPkg->fullPackageName);
        for (const auto &decl : members) {
            env.InvokedAccessible(decl.get(), false, false, CompletionImpl::IsPreamble(input, pos));
        }
    }

    // for packageName of Qualified_Type Node
    if (!prefix.empty()) {
        auto fullPrefix = GetFullPrefix(input, pos, prefix);
        CompleteQualifiedType(fullPrefix, env);
    }

    // Get scopeName by Parse + old Sema info, only for MemberAccess
    FuzzyDotComplete(input, pos, prefix, env);
}

void DotCompleterByParse::FuzzyDotComplete(const ArkAST &input, const Position &pos,
                                           const std::string &prefix, CompletionEnv &env)
{
    Ptr<Expr> expr;
    Ptr<Decl> topDecl = FindTopDecl(input, prefix, env, pos);
    inIfAvailable = CheckInIfAvailable(topDecl, pos);
    if (inIfAvailable) {
        topDecl = FindTopDecl(*input.semaCache, prefix, env, pos);
    }
    std::string scopeName = "a";
    bool isInclude = true;
    DeepFind(topDecl, pos, scopeName, isInclude);
    if (scopeName.empty()) {
        env.OutputResult(result);
        return;
    }
    OwnedPtr<Expr> invocationEx;
    FindExprInTopDecl(topDecl, expr, input, pos, invocationEx);
    if (!expr) {
        env.OutputResult(result);
        return;
    }
    if (expr->astKind == ASTKind::PRIMITIVE_TYPE_EXPR) {
        env.SetValue(FILTER::IS_STATIC, true);
    }
    if (CompleteEmptyPrefix(expr, env, prefix, scopeName, pos)) {
        return;
    }
    // get ty or decl info provider by GetGivenReferenceTy
    bool hasLocalDecl = CheckHasLocalDecl(prefix, scopeName, expr.get(), pos);
    Candidate declOrTy = CompilerCodiraProject::GetInstance()->GetGivenReferenceTarget(
        *context, scopeName, *expr, hasLocalDecl, packageNameForPath);

    size_t prevResSize = result.completions.size();
    CompleteCandidate(pos, prefix, env, declOrTy);
    // complete the node that depend on macro-expand
    if (result.completions.size() == prevResSize && declOrTy.tys.empty()) {
        NestedMacroComplete(input, pos, prefix, env, expr.get());
    }
}

void DotCompleterByParse::NestedMacroComplete(const ArkAST &input, const Position &pos, const std::string &prefix,
    CompletionEnv &env, Ptr<Expr> expr)
{
    if (!input.semaCache->file || !expr || input.semaCache->file->originalMacroCallNodes.empty()) {
        return;
    }
    std::string filePath = Normalize(input.semaCache->file->filePath);
    CompletionTip tipItem;
    tipItem.uri.file = URI::URIFromAbsolutePath(filePath).ToString();
    tipItem.tip = "waiting macro expand...";
    tipItem.position = pos;
    Callbacks *callback = CompilerCodiraProject::GetInstance()->GetCallback();
    if (callback) {
        // notify client need wait macro expand
        callback->PublishCompletionTip(tipItem);
    }

    std::string content = CompilerCodiraProject::GetInstance()->GetContentByFile(input.semaCache->file->filePath);
    if (content.empty()) {
        return;
    }
    std::unique_ptr<ArkAST> newSemaCache = nullptr;

    // Compile the file that use before file content of enter "."
    auto ci = CompilerCodiraProject::GetInstance()->GetCIForDotComplete(filePath, pos, content);
    if (!ci) {
        return;
    }
    if (ci->GetSourcePackages().empty()) {
        return;
    }
    auto pkg = ci->GetSourcePackages().front();
    if (!pkg) {
        return;
    }
    auto pkgInstance = std::make_unique<PackageInstance>(ci->diag, ci->importManager);
    if (!pkgInstance) {
        return;
    }
    pkgInstance->package = pkg;
    pkgInstance->ctx = nullptr;

    for (auto &file : pkg->files) {
        if (!file || file->filePath != input.semaCache->file->filePath) {
            continue;
        }
        std::pair<std::string, std::string> paths = { file->filePath, content };
        auto arkAST = std::make_unique<ArkAST>(paths, file.get(), ci->diag,
            pkgInstance.get(), &ci->GetSourceManager());
        std::string absName = FileStore::NormalizePath(file->filePath);
        int fileId = ci->GetSourceManager().GetFileID(absName);
        if (fileId >= 0) {
            arkAST->fileID = static_cast<unsigned int>(fileId);
        }
        newSemaCache = std::move(arkAST);
        break;
    }

    if (!newSemaCache || !newSemaCache->file || newSemaCache->file->originalMacroCallNodes.empty()) {
        return;
    }
    Ptr<Ty> ty;
    Ptr<NameReferenceExpr> resExpr;
    GetTyFromMacroCallNodes(expr, std::move(newSemaCache), ty, resExpr);
    if (!ty) {
        return;
    }
    if (Ty::IsTyCorrect(ty)) {
        std::unordered_set<Ptr<AST::Ty>> tys = {ty};
        Candidate candidate(tys);
        CompleteCandidate(pos, prefix, env, candidate);
        return;
    }
    if (!resExpr) {
        return;
    }
    CompleteByReferenceTarget(pos, prefix, env, expr, resExpr);
}

void DotCompleterByParse::CompleteByReferenceTarget(const Position &pos, const std::string &prefix, CompletionEnv &env,
    const Ptr<Expr> &expr, const Ptr<NameReferenceExpr> &resExpr)
{
    std::string comPrefix = prefix;
    if (comPrefix.empty()) {
        if (auto tc = DynamicCast<TrailingClosureExpr *>(expr.get())) {
            if (auto re = DynamicCast<RefExpr *>(tc->expr.get())) {
                comPrefix = re->ref.identifier;
            }
            bool hasLocalDecl = CheckHasLocalDecl(comPrefix, resExpr->scopeName, tc, pos);
            Candidate declOrTy = CompilerCodiraProject::GetInstance()->GetGivenReferenceTarget(
                *context, resExpr->scopeName, *tc, hasLocalDecl, packageNameForPath);
            CompleteCandidate(pos, prefix, env, declOrTy);
            return;
        }
    }
    bool hasLocalDecl = CheckHasLocalDecl(comPrefix, resExpr->scopeName, expr.get(), pos);
    Candidate declOrTy = CompilerCodiraProject::GetInstance()->GetGivenReferenceTarget(
        *context, resExpr->scopeName, *expr, hasLocalDecl, packageNameForPath);
    CompleteCandidate(pos, prefix, env, declOrTy);
}

void DotCompleterByParse::GetTyFromMacroCallNodes(Ptr<Expr> expr, std::unique_ptr<ArkAST> arkAst,
    Ptr<Ty> &ty, Ptr<NameReferenceExpr> &resExpr)
{
    Ptr<NameReferenceExpr> semaCacheExpr;
    auto macroCallNodes = &arkAst->file->originalMacroCallNodes;
    if (macroCallNodes->empty()) {
        return;
    }
    auto visitPre = [&expr, &semaCacheExpr](auto node) {
        if (auto ma = dynamic_cast<NameReferenceExpr *>(node.get())) {
            if (ma->begin == expr->begin && ma->end == expr->end) {
                semaCacheExpr = ma;
                return VisitAction::STOP_NOW;
            }
        }
        return VisitAction::WALK_CHILDREN;
    };
    for (const auto &item : *macroCallNodes) {
        if (expr->begin < item->begin || expr->end > item->end) {
            continue;
        }
        if (item->GetInvocation() && item->GetInvocation()->decl) {
            Walker(item->GetInvocation()->decl, visitPre).Walk();
        }
    }
    if (!semaCacheExpr) {
        return;
    }
    Position nextTokenPos = GetMacroNodeNextPosition(arkAst, semaCacheExpr);
    auto macroBeginPos = semaCacheExpr->GetMacroCallNewPos(semaCacheExpr->begin);
    auto maNextTokenPos = semaCacheExpr->GetMacroCallNewPos(nextTokenPos);
    auto searchTy = [&ty, &macroBeginPos, &maNextTokenPos, &resExpr](auto node) {
        if (auto ma = dynamic_cast<NameReferenceExpr *>(node.get())) {
            if (ma->begin == macroBeginPos && (maNextTokenPos.IsZero() || ma->end <= maNextTokenPos)) {
                ty = ma->ty;
                resExpr = ma;
                return VisitAction::STOP_NOW;
            }
        }
        return VisitAction::WALK_CHILDREN;
    };
    auto decls = &arkAst->file->decls;
    if (decls->empty()) {
        return;
    }
    for (const auto &decl : *decls) {
        if (decl) {
            Walker(decl, searchTy).Walk();
        }
    }
}

Position DotCompleterByParse::GetMacroNodeNextPosition(
    const std::unique_ptr<ArkAST> &arkAst, const Ptr<NameReferenceExpr> &semaCacheExpr) const
{
    int index = arkAst->GetCurTokenByPos(semaCacheExpr->end, 0, static_cast<int>(arkAst->tokens.size()) - 1);
    Position nextTokenPos;
    if (index != -1 && index < static_cast<int>(arkAst->tokens.size()) - 1) {
        int temp = index + 1;
        while (temp <= static_cast<int>(arkAst->tokens.size()) - 1 && arkAst->tokens[temp].kind == TokenKind::NL) {
            temp++;
        }
        if (temp <= static_cast<int>(arkAst->tokens.size()) - 1) {
            nextTokenPos = arkAst->tokens[temp].Begin();
        }
    }
    return nextTokenPos;
}

void DotCompleterByParse::CompleteCandidate(const Position &pos, const std::string &prefix,
                                            CompletionEnv &env, Candidate &declOrTy)
{
    if (declOrTy.hasDecl) {
        for (auto &decl : declOrTy.decls) {
            if (!decl || !syscap.CheckSysCap(*decl) || IsHiddenDecl(decl)) {
                continue;
            }
            if (decl->IsTypeDecl()) {
                env.SetValue(FILTER::IS_STATIC, true);
            } else {
                isEnumCtor = true;
                env.SetValue(FILTER::IS_STATIC, false);
            }
            Ptr<Ty> ty = decl->ty;
            auto vd = DynamicCast<VarDecl *>(decl);
            if (vd && vd->type) {
                ty = vd->type->ty;
            }
            CompleteFromType(decl->identifier, pos, ty, env);
        }
    } else {
        for (auto &ty : declOrTy.tys) {
            if (ty->kind == TypeKind::TYPE_ENUM) {
                isEnumCtor = IsEnumCtorTy(prefix, ty);
            }
            const std::string identifier = prefix == "this" || prefix == "super" ? prefix : "";
            CompleteFromType(identifier, pos, ty, env);
        }
    }
    env.OutputResult(result);
}

Ptr<Decl> DotCompleterByParse::FindTopDecl(const ArkAST &input, const std::string &prefix,
                                           CompletionEnv &env, const Position &pos)
{
    Ptr<Decl> topDecl = nullptr;
    std::string query = "_ = (" + std::to_string(pos.fileID) + ", " + std::to_string(pos.line) +
                        ", " + std::to_string(pos.column - 1) + ")";
    auto posSyms = SearchContext(context, query);
    if (posSyms.empty()) {
        return nullptr;
    }
    if (!input.file->curPackage || !context || !context->curPackage) {
        return nullptr;
    }
    for (auto &file : input.file->curPackage->files) {
        if (!file) {
            continue;
        }
        file->curPackage = context->curPackage;
        for (auto &decl : file->decls) {
            if (!decl) {
                continue;
            }
            if (file->fileHash == input.file->fileHash && decl->begin <= pos && pos <= decl->end) {
                topDecl = decl.get();
            }
        }
    }
    if (topDecl && topDecl->astKind == ASTKind::INVALID_DECL) {
        return nullptr;
    }

    FillingDeclsInPackageDot({prefix, env}, *posSyms[0]->node, context->curPackage->files);
    return topDecl;
}

void DotCompleterByParse::CompleteQualifiedType(const std::string &beforePrefix, CompletionEnv &env) const
{
    if (!packageNameForPath.empty() && !beforePrefix.empty()) {
        auto basicStrings = Utils::SplitQualifiedName(packageNameForPath);
        if (basicStrings.empty()) {
            return;
        }
        auto curModule = basicStrings.front();
        auto depends = ark::CompilerCodiraProject::GetInstance()->GetOneModuleDeps(curModule);
        auto strings = Utils::SplitQualifiedName(beforePrefix);
        if (strings.empty()) {
            return;
        }
        auto realModule = strings.front();
        if (curModule != realModule && depends.count(realModule) == 0) {
            return;
        }
    }

    for (auto &item : CompilerCodiraProject::GetInstance()->GetCiMapList()) {
        if (!IsFullPackageName(item)) {
            continue;
        }
        if (!CompilerCodiraProject::GetInstance()->IsVisibleForPackage(packageNameForPath, item)) {
            continue;
        }
        std::string pkgName = SplitFullPackage(item).second;
        env.DotPkgName(pkgName, SplitFullPackage(beforePrefix).second);
        env.DotPkgName(item, beforePrefix);
    }
}

bool DotCompleterByParse::CheckHasLocalDecl(const std::string &beforePrefix, const std::string &scopeName,
                                            Ptr<Expr> expr, const Position &pos) const
{
    if (scopeName == "a") { return false; }
    if (!expr) {
        return false;
    }
    std::string query = "scope_name:" + scopeName;
    auto posSyms = SearchContext(context, query);
    std::string firstVar = beforePrefix;
    auto found = beforePrefix.find_first_of('.');
    if (found != std::string::npos) {
        firstVar = beforePrefix.substr(0, found);
    }
    found = firstVar.find_last_of('?');
    if (found != std::string::npos) {
        firstVar = firstVar.substr(0, found);
    }
    for (auto const &sym : posSyms) {
        bool invalid = !sym || (sym->node && sym->node->begin > pos) || sym->name != firstVar;
        if (invalid) {
            continue;
        }
        if (sym->astKind == ASTKind::VAR_DECL) {
            invalid = sym->node && sym->node->begin <= expr->begin && sym->node->end >= expr->end;
            if (invalid) {
                return false;
            }
            return true;
        }
        if (sym->astKind == ASTKind::FUNC_PARAM) {
            return true;
        }
    }
    return false;
}

// find correct scopeName as possible
void DotCompleterByParse::FindFuncDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pFuncDecl = dynamic_cast<FuncDecl*>(node.get());
    if (!pFuncDecl) { return; }
    DeepFind(pFuncDecl->funcBody.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindFuncBody(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pFuncBody = dynamic_cast<FuncBody*>(node.get());
    if (!pFuncBody) { return; }
    if (!pFuncBody->paramLists.empty()) {
        DeepFind(pFuncBody->paramLists[0].get(), pos, scopeName, isInclude);
    }
    if (pFuncBody->body != nullptr && Contain(pFuncBody->body.get(), pos)) {
        DeepFind(pFuncBody->body.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindBlock(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto *pBlock = dynamic_cast<Block*>(node.get());
    if (!pBlock) { return; }
    if (!Contain(pBlock, pos)) {
        isInclude = false;
        return;
    }

    for (auto &iter : pBlock->body) {
        if (!Contain(iter.get(), pos)) {
            continue;
        }
        DeepFind(iter.get(), pos, scopeName, isInclude);
    }

    if (!isInclude && !pBlock->body.empty()) {
        scopeName = QueryByPos(pBlock->body[0].get(), pos);
        isInclude = true;
    }
}

void DotCompleterByParse::FindVarDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pVarDecl = dynamic_cast<VarDecl*>(node.get());
    if (!pVarDecl) { return; }
    // initializer may be lambdaExpr, matchCase
    std::set<ASTKind> keyKind = {
        ASTKind::LAMBDA_EXPR,
        ASTKind::MATCH_EXPR,
        ASTKind::TRAIL_CLOSURE_EXPR,
        ASTKind::TRY_EXPR,
        ASTKind::CALL_EXPR,
    };
    if (keyKind.find(pVarDecl->initializer->astKind) != keyKind.end() &&
        Contain(pVarDecl->initializer.get(), pos)) {
        DeepFind(pVarDecl->initializer.get(), pos, scopeName, isInclude);
    } else if (Contain(pVarDecl->initializer.get(), pos)) {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindVarWithPatternDecl(Ptr<Node> node, const Position &pos,
    std::string &scopeName, bool &isInclude)
{
    auto pVarWithPatternDecl = dynamic_cast<VarWithPatternDecl*>(node.get());
    if (!pVarWithPatternDecl) { return; }
    // initializer may be lambdaExpr, matchCase
    std::set<ASTKind> keyKind = {
        ASTKind::LAMBDA_EXPR,
        ASTKind::MATCH_EXPR,
        ASTKind::TRAIL_CLOSURE_EXPR,
        ASTKind::TRY_EXPR,
        ASTKind::CALL_EXPR
    };
    if (keyKind.find(pVarWithPatternDecl->initializer->astKind) != keyKind.end() &&
        Contain(pVarWithPatternDecl->initializer.get(), pos)) {
        DeepFind(pVarWithPatternDecl->initializer.get(), pos, scopeName, isInclude);
    } else if (Contain(pVarWithPatternDecl->initializer.get(), pos)) {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindClassDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto *pClassDecl = dynamic_cast<ClassDecl*>(node.get());
    if (!pClassDecl || !pClassDecl->body) { return; }

    if (!pClassDecl->body->decls.empty()) {
        for (auto &decl : pClassDecl->body->decls) {
            if (!Contain(decl.get(), pos)) {
                continue;
            }
            DeepFind(decl.get(), pos, scopeName, isInclude);
        }
    }

    if (!isInclude && !pClassDecl->body->decls.empty()) {
        scopeName = QueryByPos(pClassDecl->body->decls[0].get(), pos);
        isInclude = true;
    }
}

void DotCompleterByParse::FindStructDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto *pStructDecl = dynamic_cast<StructDecl*>(node.get());
    if (!pStructDecl || !pStructDecl->body) { return; }

    if (!pStructDecl->body->decls.empty()) {
        for (auto &decl : pStructDecl->body->decls) {
            if (!Contain(decl.get(), pos)) {
                continue;
            }
            DeepFind(decl.get(), pos, scopeName, isInclude);
        }
    }

    if (!isInclude && !pStructDecl->body->decls.empty()) {
        scopeName = QueryByPos(pStructDecl->body->decls[0].get(), pos);
        isInclude = true;
    }
}

void DotCompleterByParse::FindInterfaceDecl(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pInterfaceDecl = dynamic_cast<InterfaceDecl*>(node.get());
    if (!pInterfaceDecl) { return; }
    for (auto &decl : pInterfaceDecl->body->decls) {
        if (!Contain(decl.get(), pos)) {
            continue;
        }
        DeepFind(decl.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindEnumDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pEnumDecl = dynamic_cast<EnumDecl*>(node.get());
    if (!pEnumDecl) { return; }
    for (auto &decl : pEnumDecl->members) {
        if (!Contain(decl.get(), pos)) {
            continue;
        }
        DeepFind(decl.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindMainDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pMainDecl = dynamic_cast<MainDecl*>(node.get());
    if (!pMainDecl) { return; }
    if (!Contain(pMainDecl->funcBody.get(), pos)) {
        return;
    }
    DeepFind(pMainDecl->funcBody.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindExtendDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pExtendDecl = dynamic_cast<ExtendDecl*>(node.get());
    if (!pExtendDecl) { return; }
    for (auto &decl : pExtendDecl->members) {
        if (!Contain(decl.get(), pos)) {
            continue;
        }
        DeepFind(decl.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindIfExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pIfExpr = dynamic_cast<IfExpr*>(node.get());
    if (!pIfExpr) { return; }
    if (pIfExpr->condExpr && Contain(pIfExpr->condExpr.get(), pos)) {
        scopeName = QueryByPos(pIfExpr->condExpr.get(), pos);
        return;
    } else if (pIfExpr->thenBody && Contain(pIfExpr->thenBody.get(), pos)) {
        DeepFind(pIfExpr->thenBody.get(), pos, scopeName, isInclude);
    } else if (pIfExpr->elseBody && Contain(pIfExpr->elseBody.get(), pos)) {
        // elseBody can only be Block or IfExpr
        DeepFind(pIfExpr->elseBody.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pWhileExpr = dynamic_cast<WhileExpr*>(node.get());
    if (pWhileExpr == nullptr || pWhileExpr->body == nullptr) { return; }
    if (pWhileExpr->condExpr && Contain(pWhileExpr->condExpr.get(), pos)) {
        scopeName = QueryByPos(pWhileExpr->condExpr.get(), pos);
        return;
    }
    DeepFind(pWhileExpr->body.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindDoWhileExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pDoWhileExpr = dynamic_cast<DoWhileExpr*>(node.get());
    if (pDoWhileExpr == nullptr || pDoWhileExpr->body == nullptr) { return; }
    DeepFind(pDoWhileExpr->body.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindForInExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pForInExpr = dynamic_cast<ForInExpr*>(node.get());
    if (pForInExpr == nullptr || pForInExpr->body == nullptr) { return; }
    // pattern will create variable, patternGuard may not exist
    scopeName = QueryByPos(pForInExpr->pattern.get(), pos);
    DeepFind(pForInExpr->pattern.get(), pos, scopeName, isInclude);
    if (pForInExpr->patternGuard && Contain(pForInExpr->patternGuard.get(), pos)) {
        return;
    }
    DeepFind(pForInExpr->body.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindLambdaExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pLambdaExpr = dynamic_cast<LambdaExpr*>(node.get());
    if (!pLambdaExpr) { return; }
    DeepFind(pLambdaExpr->funcBody.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindMatchExpr(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pMatchExpr = dynamic_cast<MatchExpr*>(node.get());
    if (!pMatchExpr) { return; }
    // matchExpr->(selector, matchCase)
    if (pMatchExpr->selector && Contain(pMatchExpr->selector.get(), pos)) {
        DeepFind(pMatchExpr->selector.get(), pos, scopeName, isInclude);
        return;
    }
    for (auto &mc : pMatchExpr->matchCases) {
        if (mc->exprOrDecls && Contain(mc->exprOrDecls.get(), pos)) {
            DeepFind(mc->exprOrDecls.get(), pos, scopeName, isInclude);
        }
    }
}

void DotCompleterByParse::FindVarPattern(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pVarPattern = dynamic_cast<VarPattern*>(node.get());
    if (!pVarPattern) { return; }
    if (pVarPattern->varDecl) {
        std::string query = "_ = (" + std::to_string(pos.fileID) + ", " +
                            std::to_string(pVarPattern->varDecl->begin.line) + ", " +
                            std::to_string(pVarPattern->varDecl->begin.column) + ")";
        auto posSyms = SearchContext(context, query);
        if (posSyms.empty()) {
            return;
        }
        scopeName = posSyms[0]->scopeName;
        isInclude = true;
    }
}

void DotCompleterByParse::FindBinaryExpr(Ptr<Node> node, const Position &pos,
                                         std::string &scopeName, bool &isInclude)
{
    auto pBinaryExpr = dynamic_cast<BinaryExpr*>(node.get());
    if (!pBinaryExpr) { return; }
    if (Contain(pBinaryExpr->leftExpr, pos)) {
        DeepFind(pBinaryExpr->leftExpr.get(), pos, scopeName, isInclude);
    } else if (Contain(pBinaryExpr->rightExpr, pos)) {
        DeepFind(pBinaryExpr->rightExpr.get(), pos, scopeName, isInclude);
    } else {
        isInclude = false;
        return;
    }
}
void DotCompleterByParse::FindOptionalChainExpr(Ptr<Node> node, const Position &pos,
                                                std::string & /* scopeName */, bool &isInclude)
{
    auto pOptionalChainExpr = dynamic_cast<OptionalChainExpr*>(node.get());
    if (!pOptionalChainExpr) { return; }
    if (Contain(pOptionalChainExpr, pos)) {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindAssignExpr(Ptr<Node> node, const Position &pos,
                                         std::string & scopeName, bool &isInclude)
{
    auto pAssignExpr = dynamic_cast<AssignExpr*>(node.get());
    if (!pAssignExpr) { return; }
    if (Contain(pAssignExpr->leftValue, pos)) {
        DeepFind(pAssignExpr->leftValue.get(), pos, scopeName, isInclude);
    } else if (Contain(pAssignExpr->rightExpr, pos)) {
        DeepFind(pAssignExpr->rightExpr.get(), pos, scopeName, isInclude);
    } else {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindIncOrDecExpr(Ptr<Node> node, const Position &pos,
    std::string & /* scopeName */, bool &isInclude)
{
    auto incOrDecExpr = dynamic_cast<IncOrDecExpr *>(node.get());
    if (!incOrDecExpr) {
        return;
    }
    if (Contain(incOrDecExpr, pos)) {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindMemberAccess(Ptr<Node> node, const Position &pos,
                                           std::string &scopeName, bool &isInclude)
{
    auto ma = dynamic_cast<MemberAccess*>(node.get());
    if (!ma) { return; }
    if (ma->baseExpr && Contain(ma->baseExpr.get(), pos)) {
        DeepFind(ma->baseExpr.get(), pos, scopeName, isInclude);
    }
    auto curScope = QueryByPos(ma, pos);
    if (scopeName.size() < curScope.size()) {
        isInclude = false;
    }
}

void DotCompleterByParse::FindRefExpr(Ptr<Node> node, const Position &pos,
                                      std::string &scopeName, bool &isInclude)
{
    auto re = dynamic_cast<RefExpr*>(node.get());
    if (!re) { return; }
    if (re->ref.target && Contain(re->ref.target.get(), pos)) {
        DeepFind(re->ref.target.get(), pos, scopeName, isInclude);
    }

    if (scopeName == "a") {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindCallExpr(Ptr<Node> node, const Position &pos,
                                       std::string &scopeName, bool &isInclude)
{
    auto ce = DynamicCast<CallExpr *>(node.get());
    if (!ce || !Contain(ce, pos)) {
        return;
    }

    if (ce->baseFunc && Contain(ce->baseFunc.get(), pos)) {
        DeepFind(ce->baseFunc.get(), pos, scopeName, isInclude);
    }

    for (const auto &iter : ce->args) {
        if (iter->expr && Contain(iter->expr.get(), pos)) {
            DeepFind(iter->expr.get(), pos, scopeName, isInclude);
        }
    }
    if (scopeName == "a") {
        isInclude = false;
        return;
    }
}

void DotCompleterByParse::FindPrimaryCtorDecl(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto pCtorDecl = dynamic_cast<PrimaryCtorDecl*>(node.get());
    if (!pCtorDecl) { return; }
    DeepFind(pCtorDecl->funcBody.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindReturnExpr(Ptr<Node> node, const Position &pos,
                                         std::string &scopeName, bool &isInclude)
{
    auto pRE = dynamic_cast<ReturnExpr*>(node.get());
    if (!pRE) { return; }
    if (pRE->expr && Contain(pRE->expr, pos)) {
        DeepFind(pRE->expr, pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindTryExpr(Ptr<Node> node, const Position &pos, std::string & scopeName, bool &isInclude)
{
    auto pTryExpr = dynamic_cast<TryExpr*>(node.get());
    if (!pTryExpr) { return; }
    if (pTryExpr->tryBlock && Contain(pTryExpr->tryBlock.get(), pos)) {
        DeepFind(pTryExpr->tryBlock.get(), pos, scopeName, isInclude);
    }
    if (pTryExpr->catchBlocks.empty()) { return; }
    for (const auto &CB : pTryExpr->catchBlocks) {
        if (CB && Contain(CB.get(), pos)) {
            DeepFind(CB.get(), pos, scopeName, isInclude);
        }
    }
    if (pTryExpr->finallyBlock && Contain(pTryExpr->finallyBlock.get(), pos)) {
        DeepFind(pTryExpr->finallyBlock.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindMacroDecl(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto MD = dynamic_cast<MacroDecl*>(node.get());
    if (!MD) { return; }
    if (Contain(MD, pos)) {
        DeepFind(MD->funcBody.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindMacroExpandExpr(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto MEE = dynamic_cast<MacroExpandExpr*>(node.get());
    if (!MEE) { return; }
    size_t tokLen = MEE->invocation.args.size();
    if (MEE->invocation.args[0].Begin() < pos && pos <= MEE->invocation.args[tokLen - 1].Begin() + 1) {
        isInclude = false;
        return;
    }
    DeepFind(MEE->invocation.decl.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindMacroExpandDecl(
    Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    auto MED = dynamic_cast<MacroExpandDecl*>(node.get());
    if (!MED) { return; }
    DeepFind(MED->invocation.decl.get(), pos, scopeName, isInclude);
}

void DotCompleterByParse::FindSynchronizedExpr(Ptr<Codira::AST::Node> node, const Codira::Position &pos,
                                               std::string &scopeName, bool &isInclude)
{
    auto SE = dynamic_cast<SynchronizedExpr*>(node.get());
    if (!SE) { return; }
    if (SE->mutex && Contain(SE->mutex.get(), pos)) {
        DeepFind(SE->mutex.get(), pos, scopeName, isInclude);
    }
    if (SE->body && Contain(SE->body.get(), pos)) {
        DeepFind(SE->body.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindPropDecl(Ptr<Codira::AST::Node> node, const Codira::Position &pos,
                                       std::string &scopeName, bool &isInclude)
{
    auto PD = dynamic_cast<PropDecl*>(node.get());
    if (!PD) { return; }
    for (const auto &getter : PD->getters) {
        if (getter && Contain(getter.get(), pos)) {
            DeepFind(getter.get(), pos, scopeName, isInclude);
        }
    }
    for (const auto &setter : PD->setters) {
        if (setter && Contain(setter.get(), pos)) {
            DeepFind(setter.get(), pos, scopeName, isInclude);
        }
    }
}

void DotCompleterByParse::FindSpawnExpr(Ptr<Codira::AST::Node> node, const Codira::Position &pos,
                                        std::string &scopeName, bool &isInclude)
{
    auto spawnExpr = dynamic_cast<SpawnExpr*>(node.get());
    if (!spawnExpr) { return; }
    if (spawnExpr->task && Contain(spawnExpr->task.get(), pos)) {
        DeepFind(spawnExpr->task.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindTrailingClosureExpr(Ptr<Codira::AST::Node> node, const Codira::Position &pos,
                                                  std::string &scopeName, bool &isInclude)
{
    auto trailingClosureExpr = DynamicCast<TrailingClosureExpr *>(node.get());
    if (!trailingClosureExpr || !Contain(trailingClosureExpr, pos)) {
        return;
    }
    if (trailingClosureExpr->expr && Contain(trailingClosureExpr->expr, pos)) {
        DeepFind(trailingClosureExpr->expr.get(), pos, scopeName, isInclude);
    }
    if (trailingClosureExpr->lambda && Contain(trailingClosureExpr->lambda, pos)) {
        DeepFind(trailingClosureExpr->lambda.get(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::FindIfAvailableExpr(Ptr<Node> node, const Position &pos,
                                              std::string &scopeName, bool &isInclude)
{
    auto pIfAvailableExpr = dynamic_cast<IfAvailableExpr*>(node.get());
    if (!pIfAvailableExpr) { return; }
    if (Contain(pIfAvailableExpr->GetArg(), pos)) {
        DeepFind(pIfAvailableExpr->GetArg(), pos, scopeName, isInclude);
    } else if (Contain(pIfAvailableExpr->GetLambda1(), pos)) {
        DeepFind(pIfAvailableExpr->GetLambda1(), pos, scopeName, isInclude);
    } else if (Contain(pIfAvailableExpr->GetLambda2(), pos)) {
        DeepFind(pIfAvailableExpr->GetLambda2(), pos, scopeName, isInclude);
    }
}

void DotCompleterByParse::DeepFind(Ptr<Node> node, const Position &pos, std::string &scopeName, bool &isInclude)
{
    if (!node) { return; }
    auto func = DotMatcher::GetInstance().GetFunc(node->astKind);
    if (!func) { return; }
    (this->*(func))(node, pos, scopeName, isInclude);
}

void DotCompleterByParse::InitMap() const
{
    DotMatcher::GetInstance().RegFunc(ASTKind::FUNC_DECL, &ark::DotCompleterByParse::FindFuncDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::FUNC_BODY, &ark::DotCompleterByParse::FindFuncBody);
    DotMatcher::GetInstance().RegFunc(ASTKind::BLOCK, &ark::DotCompleterByParse::FindBlock);
    DotMatcher::GetInstance().RegFunc(ASTKind::VAR_DECL, &ark::DotCompleterByParse::FindVarDecl);
    DotMatcher::GetInstance().RegFunc(
        ASTKind::VAR_WITH_PATTERN_DECL, &ark::DotCompleterByParse::FindVarWithPatternDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::CLASS_DECL, &ark::DotCompleterByParse::FindClassDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::STRUCT_DECL, &ark::DotCompleterByParse::FindStructDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::INTERFACE_DECL, &ark::DotCompleterByParse::FindInterfaceDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::ENUM_DECL, &ark::DotCompleterByParse::FindEnumDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::MAIN_DECL, &ark::DotCompleterByParse::FindMainDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::EXTEND_DECL, &ark::DotCompleterByParse::FindExtendDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::IF_EXPR, &ark::DotCompleterByParse::FindIfExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::WHILE_EXPR, &ark::DotCompleterByParse::FindWhileExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::DO_WHILE_EXPR, &ark::DotCompleterByParse::FindDoWhileExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::FOR_IN_EXPR, &ark::DotCompleterByParse::FindForInExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::LAMBDA_EXPR, &ark::DotCompleterByParse::FindLambdaExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::MATCH_EXPR, &ark::DotCompleterByParse::FindMatchExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::VAR_PATTERN, &ark::DotCompleterByParse::FindVarPattern);
    DotMatcher::GetInstance().RegFunc(ASTKind::BINARY_EXPR, &ark::DotCompleterByParse::FindBinaryExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::OPTIONAL_CHAIN_EXPR, &ark::DotCompleterByParse::FindOptionalChainExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::ASSIGN_EXPR, &ark::DotCompleterByParse::FindAssignExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::MEMBER_ACCESS, &ark::DotCompleterByParse::FindMemberAccess);
    DotMatcher::GetInstance().RegFunc(ASTKind::INC_OR_DEC_EXPR, &ark::DotCompleterByParse::FindIncOrDecExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::REF_EXPR, &ark::DotCompleterByParse::FindRefExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::CALL_EXPR, &ark::DotCompleterByParse::FindCallExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::PRIMARY_CTOR_DECL, &ark::DotCompleterByParse::FindPrimaryCtorDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::RETURN_EXPR, &ark::DotCompleterByParse::FindReturnExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::TRY_EXPR, &ark::DotCompleterByParse::FindTryExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::MACRO_DECL, &ark::DotCompleterByParse::FindMacroDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::MACRO_EXPAND_DECL, &ark::DotCompleterByParse::FindMacroExpandDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::MACRO_EXPAND_EXPR, &ark::DotCompleterByParse::FindMacroExpandExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::SYNCHRONIZED_EXPR, &ark::DotCompleterByParse::FindSynchronizedExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::PROP_DECL, &ark::DotCompleterByParse::FindPropDecl);
    DotMatcher::GetInstance().RegFunc(ASTKind::SPAWN_EXPR, &ark::DotCompleterByParse::FindSpawnExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::TRAIL_CLOSURE_EXPR, &ark::DotCompleterByParse::FindTrailingClosureExpr);
    DotMatcher::GetInstance().RegFunc(ASTKind::IF_AVAILABLE_EXPR, &ark::DotCompleterByParse::FindIfAvailableExpr);
}

void DotCompleterByParse::AddExtendDeclFromIndex(Ptr<Ty> &extendTy, CompletionEnv &env, const Position &pos) const
{
    auto ast = CompilerCodiraProject::GetInstance()->GetArkAST(curFilePath);
    if (!ast || !ast->file) {
        return;
    }
    auto extendMembers = CompilerCodiraProject::GetInstance()->GetAllVisibleExtendMembers(
        extendTy, packageNameForPath, *ast->file);
    auto decl = Ty::GetDeclPtrOfTy(extendTy);
    if (!decl) {
        return;
    }
    std::unordered_set<lsp::SymbolID> visibleMembers;
    for (auto &member : extendMembers) {
        if (IsHiddenDecl(member) || IsHiddenDecl(member->outerDecl)) {
            continue;
        }
        env.DotAccessible(*member, *decl);
        auto symbol = CompletionEnv::GetDeclSymbolID(*member);
        visibleMembers.insert(symbol);
    }
    if (!ast->file->package || !ast->file->curPackage) {
        return;
    }
    if (decl->ty && decl->ty->HasGeneric()) {
        return;
    }
    auto symbolID = CompletionEnv::GetDeclSymbolID(*decl);
    auto curPkgName = ast->file->curPackage->fullPackageName;
    auto curModule = SplitFullPackage(curPkgName).first;
    // get import's pos
    int lastImportLine = 0;
    for (const auto &import : ast->file->imports) {
        if (!import) {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = ast->file->package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    Position textEditStart = {ast->fileID, lastImportLine, 0};
    Range editRange{textEditStart, textEditStart};
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    std::vector<CodeCompletion> completetions;
    index->FindExtendSymsOnCompletion(symbolID, visibleMembers, curPkgName, curModule,
        [&editRange, &completetions](const std::string &pkg, const std::string &interface,
            const lsp::Symbol &sym, const lsp::CompletionItem &completionItem) {
            CodeCompletion item;
            auto astKind = sym.kind;
            item.deprecated = sym.isDeprecated;
            item.kind = ItemResolverUtil::ResolveKindByASTKind(astKind);
            item.name = sym.name;
            item.label = sym.signature;
            item.insertText = completionItem.insertText;
            item.detail = "import " + pkg;
            ark::TextEdit textEdit;
            textEdit.range = editRange;
            textEdit.newText = "import " + pkg + "." + interface + "\n";
            item.additionalTextEdits = std::vector<TextEdit>{textEdit};
            item.sortType = SortType::AUTO_IMPORT_SYM;
            completetions.push_back(item);
        });
    for (const auto &completion : completetions) {
        env.AddCompletionItem(completion.label, completion.label, completion, false);
    }
}

bool GetDeclSetParse(const std::string &packageName, const OwnedPtr<File> &item, std::set<std::string>& declSet)
{
    if (item->package != nullptr && packageName == item->package->packageName) {
        (void)declSet.emplace("*");
        return true;
    }
    return false;
}

std::set<std::string> DotCompleterByParse::FindDeclSetByFiles(const std::string &packageName,
                                                              std::vector<OwnedPtr<File>> &files) const
{
    // add filter rules like import pkg.item
    std::set<std::string> declSet;
    for (auto &item : files) {
        // complete curPkg
        if (GetDeclSetParse(packageName, item, declSet)) {
            return declSet;
        }
    }
    return declSet;
}


void DotCompleterByParse::FillingDeclsInPackageDot(std::pair<std::string, CompletionEnv &> pkgNameAndEnv,
                                                   Codira::AST::Node &curNode,
                                                   std::vector<OwnedPtr<File>> &files,
                                                   std::set<std::string> lastDeclSet,
                                                   const std::pair<bool, bool> openFillAndIsImport)

{
    Ptr<PackageDecl> pkgDecl = importManager->GetPackageDecl(pkgNameAndEnv.first);
    if (!pkgDecl || !pkgDecl->srcPackage || pkgDecl->srcPackage->files.empty()) {
        return;
    }
    // if pkgDecl is current package complete all decl in current package
    if (!pkgDecl->identifier.Empty() && pkgDecl->fullPackageName == context->fullPackageName) {
        for (auto &file : pkgDecl->srcPackage->files) {
            for (auto &decl : file->decls) {
                pkgNameAndEnv.second.InvokedAccessible(decl.get(), false, false, openFillAndIsImport.second);
            }
        }
    }
    std::set<std::string> declsSet = FindDeclSetByFiles(pkgDecl->identifier, files);
    if (lastDeclSet.find("*") == lastDeclSet.end()) {
        declsSet = lastDeclSet;
    }
}

void DotCompleterByParse::CompleteFromType(const std::string &identifier,
                                           const Codira::Position &pos,
                                           Ty *type,
                                           CompletionEnv &env) const
{
    if (type == nullptr) {
        return;
    }
    if (typeid(*type) == typeid(ClassTy) || typeid(*type) == typeid(ClassThisTy)) {
        CompleteClassDecl(type, pos, env, identifier == "super" || identifier == "this");
    } else if (typeid(*type) == typeid(InterfaceTy)) {
        auto interfaceDecl = dynamic_cast<InterfaceTy *>(type)->decl;
        CompleteInterfaceDecl(interfaceDecl, pos, env);
    } else if (typeid(*type) == typeid(StructTy)) {
        CompleteStructDecl(type, pos, env);
    } else if (typeid(*type) == typeid(EnumTy)) {
        CompleteEnumDecl(type, pos, env);
    } else if (typeid(*type) == typeid(TypeAliasTy)) {
        auto aliasDecl = dynamic_cast<TypeAliasTy *>(type)->declPtr;
        if (!aliasDecl || !aliasDecl->type) {
            return;
        }
        CompleteFromType(identifier, pos, aliasDecl->type->ty, env);
    } else if (typeid(*type) == typeid(GenericsTy)) {
        auto genericsDecl = dynamic_cast<GenericsTy *>(type)->upperBounds;
        for (auto ty : genericsDecl) {
            CompleteFromType("", pos, ty, env);
        }
    } else if (typeid(*type) == typeid(VArrayTy)) {
        env.AddVArrayItem();
    }
    // dot completion for primitive type
    // eg: let a = Int64.parse("1")
    //     let b = a.toString()
    if (type->IsPrimitive() || type->IsPointer() || type->IsCString()) {
        CompleteBuiltInType(type, env);
    }
}

void DotCompleterByParse::CompleteClassDecl(Ptr<Ty> ty, const Codira::Position &pos,
                                            CompletionEnv &env, bool isSuperOrThis) const
{
    auto classDecl = DynamicCast<ClassDecl>(Ty::GetDeclPtrOfTy(ty));
    if (classDecl == nullptr || classDecl->body == nullptr || !syscap.CheckSysCap(*classDecl) ||
        IsHiddenDecl(classDecl)) {
        return;
    }
    if (Contain(classDecl, pos)) {
        env.AddDirectScope(classDecl->identifier, classDecl);
    }

    for (auto &decl : classDecl->body->decls) {
        env.DotAccessible(*decl, *classDecl, isSuperOrThis);
    }
    // Complete extend decl
    AddExtendDeclFromIndex(ty, env, pos);
    // Complete interfaces
    for (auto &inheritedType : classDecl->inheritedTypes) {
        if (!ark::Is<RefType>(inheritedType.get().get())) {
            continue;
        }
        auto refType = dynamic_cast<RefType*>(inheritedType.get().get());
        if (!refType || !ark::Is<InterfaceDecl>(refType->ref.target.get())) {
            continue;
        }
        CompleteInterfaceDecl(dynamic_cast<InterfaceDecl*>(refType->ref.target.get()), pos, env);
    }
    // Complete super class
    if (ark::Is<ClassDecl>(classDecl->GetSuperClassDecl().get())) {
        auto superClass = classDecl->GetSuperClassDecl();
        env.SetValue(FILTER::IS_SUPER, true);
        CompleteClassDecl(superClass->ty, pos, env, isSuperOrThis);
    }
}

void DotCompleterByParse::CompleteInterfaceDecl(Ptr<Codira::AST::InterfaceDecl> interfaceDecl,
                                                const Codira::Position &pos, CompletionEnv &env) const
{
    if (interfaceDecl == nullptr || interfaceDecl->body == nullptr || !syscap.CheckSysCap(*interfaceDecl) ||
        IsHiddenDecl(interfaceDecl.get())) {
        return;
    }
    for (auto &decl : interfaceDecl->body->decls) {
        env.DotAccessible(*decl, *interfaceDecl);
    }

    CompleteSuperInterface(interfaceDecl, pos, env);
}

void DotCompleterByParse::CompleteSuperInterface(Ptr<const InterfaceDecl> interfaceDecl, const Position &pos,
                                                 CompletionEnv &env) const
{
    for (auto &inheritedType : interfaceDecl->inheritedTypes) {
        if (!Codira::Is<RefType>(inheritedType.get())) {
            continue;
        }
        auto refType = dynamic_cast<RefType*>(inheritedType.get().get());
        if (Codira::Is<InterfaceTy>(refType->ty.get()) && !IsHiddenDecl(refType->ref.target)) {
            CompleteFromType("", pos, refType->ty, env);
        }
    }
}

void DotCompleterByParse::CompleteEnumDecl(Ptr<Ty> ty, const Codira::Position &pos, CompletionEnv &env) const
{
    // TD: Enum Type will be written later.
    auto enumDecl = DynamicCast<EnumDecl>(Ty::GetDeclPtrOfTy(ty));
    if (enumDecl == nullptr || !syscap.CheckSysCap(*enumDecl) || IsHiddenDecl(enumDecl)) {
        return;
    }
    if (!isEnumCtor) {
        for (auto &decl : enumDecl->constructors) {
            env.DotAccessible(*decl, *enumDecl);
        }
    }
    for (auto &member : enumDecl->members) {
        if (!member) {
            continue;
        }
        if (dynamic_cast<Codira::AST::VarDecl*>(member.get().get()) != nullptr) {
            env.DotAccessible(*member, *enumDecl);
            continue;
        }
        if (member->TestAttr(Codira::AST::Attribute::PRIVATE)) {
            continue;
        }
        env.DotAccessible(*member, *enumDecl);
    }
    // Complete extend decl
    AddExtendDeclFromIndex(ty, env, pos);

    CompleteEnumInterface(enumDecl, pos, env);
}

void DotCompleterByParse::CompleteEnumInterface(Ptr<EnumDecl> enumDecl, const Position &pos, CompletionEnv &env) const
{
    for (auto &superClassOrInterfaceType : enumDecl->inheritedTypes) {
        if (!superClassOrInterfaceType || !Codira::Is<RefType>(superClassOrInterfaceType.get())) {
            continue;
        }
        auto refType = dynamic_cast<RefType*>(superClassOrInterfaceType.get().get());
        if (!refType || !Codira::Is<InterfaceDecl>(refType->ref.target.get())) {
            continue;
        }
        CompleteInterfaceDecl(dynamic_cast<InterfaceDecl*>(refType->ref.target.get()), pos, env);
    }
}

void DotCompleterByParse::CompleteStructDecl(Ptr<Ty> ty, const Codira::Position &pos, CompletionEnv &env) const
{
    // TD: Struct Type will be written later.
    auto structDecl = dynamic_cast<StructTy *>(ty.get())->decl;
    if (structDecl == nullptr || structDecl->body == nullptr || !syscap.CheckSysCap(*structDecl) ||
        IsHiddenDecl(structDecl)) {
        return;
    }
    if (Contain(structDecl, pos)) {
        env.AddDirectScope(structDecl->identifier, structDecl);
    }
    for (auto &decl : structDecl->body->decls) {
        env.DotAccessible(*decl, *structDecl);
    }
    // Complete extend decl
    AddExtendDeclFromIndex(ty, env, pos);
    // Complete interface decl
    for (auto &inheritedType : structDecl->inheritedTypes) {
        if (!inheritedType || !ark::Is<RefType>(inheritedType.get().get())) {
            continue;
        }
        auto refType = dynamic_cast<RefType*>(inheritedType.get().get());
        if (!refType || !ark::Is<InterfaceDecl>(refType->ref.target.get())) {
            continue;
        }
        CompleteInterfaceDecl(dynamic_cast<InterfaceDecl*>(refType->ref.target.get()), pos, env);
    }
    // struct can not inherit
}

void DotCompleterByParse::CompleteBuiltInType(Ty *type, CompletionEnv &env) const
{
    auto ast = CompilerCodiraProject::GetInstance()->GetArkAST(curFilePath);
    if (ast == nullptr) {
        return;
    }
    auto extendDecls = CompilerCodiraProject::GetInstance()->GetAllVisibleExtendMembers(
        type, packageNameForPath, *ast->file);
    for (auto &decl : extendDecls) {
        if (!syscap.CheckSysCap(decl) || IsHiddenDecl(decl) || IsHiddenDecl(decl->outerDecl)) {
            return;
        }
        if (env.GetValue(FILTER::IS_STATIC)) {
            if (decl->TestAttr(Attribute::STATIC)) {
                env.AddBuiltInItem(*decl);
            }
        } else {
            if (!decl->TestAnyAttr(Attribute::STATIC, Attribute::OPERATOR)) {
                env.AddBuiltInItem(*decl);
            }
        }
    }
}

std::string DotCompleterByParse::QueryByPos(Ptr<Node> node, const Position pos)
{
    if (!node) { return ""; }
    std::string query = "_ = (" + std::to_string(pos.fileID) + ", "+
                        std::to_string(node->begin.line) + ", " +
                        std::to_string(node->begin.column) + ")";
    auto posSyms = SearchContext(context, query);
    if (posSyms.empty()) {
        return "";
    }
    std::string scopeName;
    if (posSyms[0]) {
        scopeName = posSyms[0]->scopeName;
    }
    auto boundary = scopeName.find('_');
    if (boundary != std::string::npos) {
        scopeName = scopeName.substr(0, boundary);
    }
    return scopeName;
}

bool DotCompleterByParse::IsEnumCtorTy(const std::string &beforePrefix, Ty *const &ty) const
{
    auto ED = dynamic_cast<EnumTy *>(ty)->decl;
    if (!ED) { return false; }
    std::string className = ED->identifier + ".";
    for (const auto &c : ED->constructors) {
        if ((c->identifier == beforePrefix || className + c->identifier == beforePrefix) &&
            !IsHiddenDecl(c)) {
            return true;
        }
    }
    return false;
}

bool DotCompleterByParse::CheckInIfAvailable(Ptr<Decl> decl, const Position &pos)
{
    if (decl == nullptr || decl->astKind != ASTKind::FUNC_DECL) {
        return false;
    }
    auto funcDecl = DynamicCast<FuncDecl*>(decl);
    if (!funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body) {
        return false;
    }
    for (auto& node : funcDecl->funcBody->body->body) {
        if (node->GetBegin() <= pos && node->GetEnd() >= pos) {
            if (node->astKind != ASTKind::MACRO_EXPAND_EXPR) {
                return false;
            }
            auto expr = DynamicCast<MacroExpandExpr*>(node.get());
            if (expr && expr->identifier == "IfAvailable") {
                return true;
            }
            return false;
        }
    }
    return false;
}

bool DotCompleterByParse::CompleteEmptyPrefix(Ptr<Expr> expr, CompletionEnv &env, const std::string &prefix,
                            const std::string &scopeName, const Position &pos)
{
    std::string comPrefix = prefix;
    if (comPrefix.empty()) {
        if (auto tc = DynamicCast<TrailingClosureExpr *>(expr.get())) {
            if (auto re = DynamicCast<RefExpr *>(tc->expr.get())) {
                comPrefix = re->ref.identifier;
            }
            bool hasLocalDecl = CheckHasLocalDecl(comPrefix, scopeName, tc, pos);
            Candidate declOrTy = CompilerCodiraProject::GetInstance()->GetGivenReferenceTarget(
                *context, scopeName, *tc, hasLocalDecl, packageNameForPath);
            CompleteCandidate(pos, prefix, env, declOrTy);
            return true;
        }
    }
    return false;
}

void DotCompleterByParse::FindExprInTopDecl(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
    const Position &pos, OwnedPtr<Expr> &invocationEx)
{
    WalkForMemberAccess(topDecl, expr, input, pos, invocationEx);
    if (inIfAvailable && !expr) {
        WalkForIfAvailable(topDecl, expr, input, pos);
    }
}

void DotCompleterByParse::WalkForMemberAccess(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
                        const Position &pos, OwnedPtr<Expr> &invocationEx)
{
    Walker(topDecl, [&](auto node) {
        if (auto ma = dynamic_cast<MemberAccess *>(node.get())) {
            if (ma->dotPos.line == pos.line && ma->dotPos.column == pos.column - 1) {
                expr = ma->baseExpr.get();
                return VisitAction::STOP_NOW;
            }
        }
        if (auto inv = node->GetInvocation()) {
            OwnedPtr<Expr> ex = Parser(inv->args, input.diag, *input.sourceManager).ParseExpr();
            AddCurFile(*ex, node->curFile);
            if (auto ma = DynamicCast<MemberAccess *>(ex.get())) {
                if (ma->dotPos.line == pos.line && ma->dotPos.column == pos.column - 1) {
                    expr = ma->baseExpr.get();
                    invocationEx = std::move(ex);
                    return VisitAction::STOP_NOW;
                }
            }
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
}

void DotCompleterByParse::WalkForIfAvailable(Ptr<Decl> topDecl, Ptr<Expr>& expr, const ArkAST &input,
                        const Position &pos)
{
    Walker(topDecl, [&](auto node) {
        if (auto refExpr = DynamicCast<RefExpr *>(node.get())) {
            if (refExpr->GetEnd().line == pos.line && refExpr->GetEnd().column == pos.column - 1) {
                expr = refExpr;
                return VisitAction::STOP_NOW;
            }
        }
        if (auto callExpr = DynamicCast<CallExpr *>(node.get())) {
            if (callExpr->GetEnd().line == pos.line && callExpr->GetEnd().column == pos.column - 1) {
                expr = callExpr;
                return VisitAction::STOP_NOW;
            }
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
}

// check if in multiimport complete,
// if true, return prefix with organization name.
std::string DotCompleterByParse::GetFullPrefix(const ArkAST &input, const Position &pos, const std::string &prefix)
{
    std::string name = prefix;
    for (const auto &im : input.file->imports) {
        if (pos > im->end) {
            continue;
        }
        bool multiImport = im->content.kind == Codira::AST::ImportKind::IMPORT_MULTI
                && im->content.hasDoubleColon && pos > im->content.leftCurlPos;
        if (multiImport && !im->content.prefixPaths.empty()) {
            name = im->content.prefixPaths[0] + "::" + prefix;
        }
        return name;
    }
    return name;
}
}
