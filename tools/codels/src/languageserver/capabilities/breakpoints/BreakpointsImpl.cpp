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

#include "BreakpointsImpl.h"

namespace ark {
std::string BreakpointsImpl::curFilePath = "";

void HandlePos(std::set<BreakpointLocation> &result, const ArkAST &ast, Position pos, const Node &token)
{
    const std::string uri = URI::URIFromAbsolutePath(BreakpointsImpl::curFilePath).ToString();
    PositionUTF8ToIDE(ast.tokens, pos, token);
    const Range range = {{pos.fileID, pos.line - 1, 0}, {pos.fileID, pos.line - 1, 0}};
    (void) result.insert({uri, range});
}

bool containExpr(const Block &funcBody)
{
    for (auto &member : funcBody.body) {
        bool isValidStatement = member->astKind != ASTKind::RETURN_EXPR && member->astKind != ASTKind::LIT_CONST_EXPR &&
                                member->astKind != ASTKind::FUNC_DECL;
        if (isValidStatement) {
            return true;
        }
    }
    return false;
}

void HandleReturnExpr(std::set<BreakpointLocation> &result, const ArkAST &ast)
{
    if (!ast.packageInstance->ctx || !ast.packageInstance->ctx->searcher) {
        return;
    }
    const std::string returnQuery = "ast_kind:return_expr";
    const auto returnSymbols = ast.packageInstance->ctx->searcher->Search(*ast.packageInstance->ctx,
                                                                    returnQuery, Sort::posDesc, {ast.file->fileHash});
    for (const auto symbol : returnSymbols) {
        if (!symbol->node) {
            continue;
        }
        auto *retExpr = dynamic_cast<const ReturnExpr*>(symbol->node.get());
        if (!retExpr) { continue; }
        if (retExpr->refFuncBody && containExpr(*retExpr->refFuncBody->body)) {
            HandlePos(result, ast, symbol->node->begin, *symbol->node);
        }
    }
}

void HandleIfExpr(std::set<BreakpointLocation> &result, const Node &expr)
{
    auto *ifExpr = dynamic_cast<const IfExpr*>(&expr);
    if (!ifExpr) { return; }
    BreakpointsImpl::HandleBlockExit(result, ifExpr->thenBody.get());
    auto elseExpr = ifExpr->elseBody.get();
    if (!elseExpr) { return; }
    if (elseExpr->astKind == ASTKind::IF_EXPR) {
        HandleIfExpr(result, *elseExpr);
    }
    if (elseExpr->astKind == ASTKind::BLOCK) {
        auto *elseBlock = dynamic_cast<const Block*>(elseExpr.get());
        if (!elseBlock) { return; }
        BreakpointsImpl::HandleBlockExit(result, elseBlock);
    }
}

void HandleMatchExpr(std::set<BreakpointLocation> &result, const Node &expr)
{
    auto *matchExpr = dynamic_cast<const MatchExpr*>(&expr);
    if (!matchExpr) { return; }
    for (auto &matchCase : matchExpr->matchCases) {
        if (!matchCase) { continue; }
        auto matchBlock = matchCase->exprOrDecls.get();
        BreakpointsImpl::HandleBlockExit(result, matchBlock);
    }
    for (auto &matchCaseOther : matchExpr->matchCaseOthers) {
        if (!matchCaseOther) { continue; }
        auto matchOtherBlock = matchCaseOther->exprOrDecls.get();
        BreakpointsImpl::HandleBlockExit(result, matchOtherBlock);
        if (matchCaseOther->matchExpr) {
            HandleMatchExpr(result, *matchCaseOther->matchExpr);
        }
    }
}

void HandleTryExpr(std::set<BreakpointLocation> &result, const Node &expr)
{
    auto *tryExpr = dynamic_cast<const TryExpr*>(&expr);
    if (!tryExpr) { return; }
    BreakpointsImpl::HandleBlockExit(result, tryExpr->tryBlock.get());
    for (auto &catchBlock : tryExpr->catchBlocks) {
        if (!catchBlock) { continue; }
        BreakpointsImpl::HandleBlockExit(result, catchBlock.get());
    }
    BreakpointsImpl::HandleBlockExit(result, tryExpr->finallyBlock.get());
}

void HandleSynchronizedExpr(std::set<BreakpointLocation> &result, const Node &expr)
{
    auto *synchronizedExpr = dynamic_cast<const SynchronizedExpr*>(&expr);
    if (!synchronizedExpr || !synchronizedExpr->desugarExpr) { return; }
    const auto deSugarExpr = dynamic_cast<const Block*>(synchronizedExpr->desugarExpr.get().get());
    if (!deSugarExpr) { return; }
    const auto deSugarBlock = dynamic_cast<const TryExpr*>(deSugarExpr->GetLastExprOrDecl().get());
    if (!deSugarBlock) { return; }
    BreakpointsImpl::HandleBlockExit(result, deSugarBlock->tryBlock.get());
}

void HandleFuncBodyEntryAndExit(std::set<BreakpointLocation> &result, const Block &funcBody, const ArkAST &ast,
                                const Type &retType)
{
    bool firstExpr = false;
    size_t implicitSuperNum = 0;
    for (const auto &member : funcBody.body) {
        if (!member) { continue; }
        // skip desugar
        bool isDeSugar = member->TestAttr(Attribute::COMPILER_ADD) &&
                !member->TestAttr(Attribute::IS_CLONED_SOURCE_CODE);
        if (isDeSugar) {
            implicitSuperNum ++;
            continue;
        }
        bool isFirstExpr = !firstExpr && member->astKind != ASTKind::RETURN_EXPR &&
                member->astKind != ASTKind::LIT_CONST_EXPR && member->astKind != ASTKind::FUNC_DECL;
        if (isFirstExpr) {
            firstExpr = true;
            break;
        }
    }
    if (!firstExpr) {
        return;
    }
    // handle func entry
    HandlePos(result, ast, funcBody.body[implicitSuperNum]->begin, *funcBody.body[implicitSuperNum]);
    // handle func exit
    if (retType.ty && retType.ty->kind == TypeKind::TYPE_UNIT) {
        HandlePos(result, ast, funcBody.end, funcBody);
        return;
    }
    BreakpointsImpl::HandleBlockExit(result, &funcBody);
}

void HandleLambda(std::set<BreakpointLocation> &result, const ArkAST &ast)
{
    if (!ast.packageInstance->ctx || !ast.packageInstance->ctx->searcher) {
        return;
    }
    const std::string funcQuery = "ast_kind:lambda_expr";
    const auto lambdaSymbols = ast.packageInstance->ctx->searcher->Search(*ast.packageInstance->ctx,
                                                                    funcQuery, Sort::posDesc, {ast.file->fileHash});
    for (const auto *item : lambdaSymbols) {
        if (!item) { continue; }
        auto *lambdaExpr = dynamic_cast<const LambdaExpr*>(item->node.get());
        bool bodyEmpty = !lambdaExpr || !lambdaExpr->funcBody || !lambdaExpr->funcBody->body ||
                lambdaExpr->funcBody->body->body.empty();
        if (bodyEmpty) {
            continue;
        }
        HandleFuncBodyEntryAndExit(result, *lambdaExpr->funcBody->body, ast,
                                   *lambdaExpr->funcBody->retType);
    }
}

void HandleFuncDecl(std::set<BreakpointLocation> &result, const ArkAST &ast)
{
    if (!ast.packageInstance->ctx || !ast.packageInstance->ctx->searcher) {
        return;
    }
    const std::string funcQuery = "ast_kind:func_decl";
    const auto funcSymbols = ast.packageInstance->ctx->searcher->Search(*ast.packageInstance->ctx,
                                                                  funcQuery, Sort::posDesc, {ast.file->fileHash});
    for (auto item : funcSymbols) {
        if (!item) { continue; }
        auto *funcDecl = dynamic_cast<const FuncDecl*>(item->node.get());
        bool bodyEmpty = !funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body ||
                funcDecl->funcBody->body->body.empty();
        if (bodyEmpty) {
            continue;
        }
        // skip Implicit init
        if (funcDecl->identifier == "init" && funcDecl->outerDecl &&
            funcDecl->outerDecl->identifier.Begin() == funcDecl->begin) {
            continue;
        }
        HandleFuncBodyEntryAndExit(result, *funcDecl->funcBody->body, ast, *funcDecl->funcBody->retType);
    }
}

void BreakpointsImpl::HandleBlockExit(std::set<BreakpointLocation> &result, Ptr<const Block> block)
{
    if (!block) { return; }
    const auto lastMember = block->GetLastExprOrDecl();
    if (!lastMember) { return; }
    if (lastMember->astKind == ASTKind::IF_EXPR) {
        HandleIfExpr(result, *lastMember);
        return;
    }
    if (lastMember->astKind == ASTKind::MATCH_EXPR) {
        HandleMatchExpr(result, *lastMember);
        return;
    }
    if (lastMember->astKind == ASTKind::TRY_EXPR) {
        HandleTryExpr(result, *lastMember);
        return;
    }
    if (lastMember->astKind == ASTKind::SYNCHRONIZED_EXPR) {
        HandleSynchronizedExpr(result, *lastMember);
        return;
    }
    if (lastMember->astKind == ASTKind::RETURN_EXPR) {
        return;
    }
    const ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(curFilePath);
    if (!arkAst) {
        return;
    }
    HandlePos(result, *arkAst, lastMember->begin, *lastMember);
}

void BreakpointsImpl::Breakpoints(const ArkAST &ast, std::set<BreakpointLocation> &result)
{
    bool invalid = !ast.file || !ast.packageInstance;
    if (invalid) {
        return;
    }
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "BreakpointsImpl::Breakpoints in.");
    curFilePath = ast.file->filePath;
    HandleReturnExpr(result, ast);
    HandleFuncDecl(result, ast);
    HandleLambda(result, ast);
}
}
