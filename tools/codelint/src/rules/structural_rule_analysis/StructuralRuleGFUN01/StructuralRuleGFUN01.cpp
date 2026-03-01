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

#include "StructuralRuleGFUN01.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

static const size_t MAX_DEPTH_LEVEL = 4;
static const size_t MAX_LINE_NUM = 50;
static const size_t MAX_FUNC_ARGS = 5;

static bool IsCommentStart(const std::string& line)
{
    size_t pos = line.find("//");
    if (pos != std::string::npos) {
        return true; // Single-line comment found
    }
    pos = line.find("/*");
    if (pos != std::string::npos) {
        return true; // Start of block comment found
    }
    return false;
}

static bool IsCommentEnd(const std::string& line, size_t& endPos)
{
    endPos = line.rfind("*/");
    if (endPos != std::string::npos) {
        return true; // End of block comment found
    }
    return false;
}
static void CountFunctionLinesHelper(std::string& line, size_t& endPos, size_t& lineCount, bool& inBlockComment)
{
    if (IsCommentEnd(line, endPos)) {
        inBlockComment = false;
        // Check if there's code after the end of a block comment on the same line
        // 2 is the size of "*/"
        if (line.find_first_not_of(" \t", endPos + 2) != std::string::npos) {
            ++lineCount;
        }
    }
}
static bool IsInQuotes(const std::string line, size_t pos)
{
    auto str = line.substr(0, pos);
    auto singleQuotesCount = std::count(str.begin(), str.end(), '\'');
    auto doubleQuotesCount = std::count(str.begin(), str.end(), '"');
    // Determine whether the number is even by dividing by 2, that is, whether the symbol is in quotation marks.
    return singleQuotesCount % 2 == 0 && doubleQuotesCount % 2 == 0;
}
static void CountFunctionLinesHelper02(std::string& line, size_t& endPos, size_t& lineCount, bool& inBlockComment)
{
    // Check for block comment start
    size_t startPos = line.find("/*");
    if (startPos != std::string::npos) {
        if (!IsInQuotes(line, startPos)) {
            lineCount++;
            return;
        }
        inBlockComment = true;
        // Check if there's code before the start of a block comment on the same line
        if (line.find_first_not_of(" \t") < startPos) {
            ++lineCount;
        }
        // Check if block comment ends on the same line
        if (IsCommentEnd(line, endPos) && endPos > startPos) {
            inBlockComment = false;
            // Check for code after block comment on the same line
            // 2 is the size of "*/"
            if (line.find_first_not_of(" \t", endPos + 2) != std::string::npos) {
                ++lineCount;
            }
        }
    } else {
        // It's a single-line comment, check for code before it
        if (line.find_first_not_of(" \t") < line.find("//")) {
            ++lineCount;
        }
    }
}
static size_t CountFunctionLines(std::stringstream& ss)
{
    std::string line;
    size_t lineCount = 0;
    bool inBlockComment = false;

    while (getline(ss, line, '\n')) {
        size_t endPos = 0;
        // Skip empty lines
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
            continue;
        }

        if (inBlockComment) {
            CountFunctionLinesHelper(line, endPos, lineCount, inBlockComment);
            // Continue to next iteration if still inside a block comment
            continue;
        }

        if (IsCommentStart(line)) {
            CountFunctionLinesHelper02(line, endPos, lineCount, inBlockComment);
        } else {
            ++lineCount; // Count non-comment lines
        }
    }
    return lineCount;
}

static inline bool IsControlExpr(const AST::Node& node)
{
    return node.astKind == ASTKind::IF_EXPR || node.astKind == ASTKind::WHILE_EXPR ||
        node.astKind == ASTKind::DO_WHILE_EXPR || node.astKind == ASTKind::MATCH_EXPR ||
        node.astKind == ASTKind::FOR_IN_EXPR;
}

void StructuralRuleGFUN01::CheckFuncHelper(const AST::FuncDecl& funcDecl)
{
    if (!funcDecl.TestAttr(Attribute::CONSTRUCTOR) && funcDecl.funcBody && !funcDecl.funcBody->paramLists.empty() &&
        funcDecl.funcBody->paramLists[0]->params.size() > MAX_FUNC_ARGS) {
        Diagnose(funcDecl.begin, funcDecl.end, CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_0,
            funcDecl.identifier.Val());
    }

    // Check the nesting depth of the code
    Walker walker(const_cast<AST::FuncDecl*>(&funcDecl), [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const MatchExpr& matchExpr) {
                CheckControlBlock(matchExpr, ASTKind::FUNC_DECL);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const IfExpr& ifExpr) {
                CheckControlBlock(ifExpr, ASTKind::FUNC_DECL);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const WhileExpr& whileExpr) {
                CheckControlBlock(whileExpr, ASTKind::FUNC_DECL);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ForInExpr& forInExpr) {
                CheckControlBlock(forInExpr, ASTKind::FUNC_DECL);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const DoWhileExpr& doWhileExpr) {
                CheckControlBlock(doWhileExpr, ASTKind::FUNC_DECL);
                return VisitAction::SKIP_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();

    if (funcDecl.end.line - funcDecl.begin.line + 1 <= MAX_LINE_NUM) {
        return;
    }
    std::string content =
        diagEngine->GetSourceManager().GetContentBetween(funcDecl.begin.fileID, funcDecl.begin, funcDecl.end);
    std::stringstream ss(content);
    std::string line;
    size_t lineNum = CountFunctionLines(ss);
    if (lineNum > MAX_LINE_NUM) {
        Diagnose(funcDecl.begin, funcDecl.end, CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_1,
            funcDecl.identifier.Val());
    }
}

void StructuralRuleGFUN01::CheckLambdaExprHelper(const AST::LambdaExpr& lambdaExpr)
{
    if (lambdaExpr.funcBody && !lambdaExpr.funcBody->paramLists.empty() &&
        lambdaExpr.funcBody->paramLists[0]->params.size() > MAX_FUNC_ARGS) {
        Diagnose(lambdaExpr.begin, lambdaExpr.end,
            CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_3, "");
    }
    if (lambdaExpr.funcBody && lambdaExpr.funcBody->body) {
        for (auto& expr : lambdaExpr.funcBody->body->body) {
            if (IsControlExpr(*expr.get())) {
                CheckControlBlock(*static_cast<AST::Expr*>(expr.get().get()), ASTKind::LAMBDA_EXPR);
            }
        }
    }
    if (lambdaExpr.end.line - lambdaExpr.begin.line + 1 <= MAX_LINE_NUM) {
        return;
    }
    std::string content =
        diagEngine->GetSourceManager().GetContentBetween(lambdaExpr.begin.fileID, lambdaExpr.begin, lambdaExpr.end);
    std::stringstream ss(content);
    std::string line;
    size_t lineNum = CountFunctionLines(ss);
    if (lineNum > MAX_LINE_NUM) {
        Diagnose(lambdaExpr.begin, lambdaExpr.end,
            CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_4, "");
    }
}

void StructuralRuleGFUN01::CheckControlExprHelper(std::vector<OwnedPtr<AST::Node>>& body, size_t& depth)
{
    size_t maxLocalDepth = 0;
    for (auto& newExpr : body) {
        size_t localDepth = 0;
        if (IsControlExpr(*newExpr.get())) {
            CheckControlExpr(*static_cast<Expr*>(newExpr.get().get()), localDepth);
        }
        maxLocalDepth = maxLocalDepth > localDepth ? maxLocalDepth : localDepth;
    }
    depth += maxLocalDepth;
}

void StructuralRuleGFUN01::CheckIfExprHelper(const AST::Expr& expr, size_t& depth)
{
    auto ifExpr = dynamic_cast<const IfExpr*>(&expr);
    if (!ifExpr) {
        return;
    }
    if (ifExpr->thenBody) {
        CheckControlExprHelper(ifExpr->thenBody->body, depth);
    }
    if (ifExpr->elseBody && ifExpr->elseBody->astKind == AST::ASTKind::BLOCK) {
        CheckControlExprHelper(static_cast<Block*>(ifExpr->elseBody.get().get())->body, depth);
    }
}

void StructuralRuleGFUN01::CheckWhileExpr(const AST::Expr& expr, size_t& depth)
{
    auto whileExpr = dynamic_cast<const WhileExpr*>(&expr);
    if (!whileExpr) {
        return;
    }
    if (whileExpr->body) {
        CheckControlExprHelper(whileExpr->body->body, depth);
    }
}

void StructuralRuleGFUN01::CheckDoWhileExpr(const AST::Expr& expr, size_t& depth)
{
    auto doWhileExpr = dynamic_cast<const DoWhileExpr*>(&expr);
    if (!doWhileExpr) {
        return;
    }
    if (doWhileExpr->body) {
        CheckControlExprHelper(doWhileExpr->body->body, depth);
    }
}

void StructuralRuleGFUN01::CheckMatchExpr(const AST::Expr& expr, size_t& depth)
{
    auto matchExpr = dynamic_cast<const MatchExpr*>(&expr);
    if (!matchExpr) {
        return;
    }
    size_t maxLocalDepth = 0;
    for (auto& matchCase : matchExpr->matchCases) {
        size_t localDepth = 0;
        if (!matchCase->exprOrDecls) {
            continue;
        }
        CheckControlExprHelper(matchCase->exprOrDecls->body, localDepth);
        maxLocalDepth = maxLocalDepth > localDepth ? maxLocalDepth : localDepth;
    }
    for (auto& matchCase : matchExpr->matchCaseOthers) {
        size_t localDepth = 0;
        if (!matchCase->exprOrDecls) {
            continue;
        }
        CheckControlExprHelper(matchCase->exprOrDecls->body, localDepth);
        maxLocalDepth = maxLocalDepth > localDepth ? maxLocalDepth : localDepth;
    }
    depth += maxLocalDepth;
}

void StructuralRuleGFUN01::CheckForInExpr(const AST::Expr& expr, size_t& depth)
{
    auto forInExpr = dynamic_cast<const ForInExpr*>(&expr);
    if (!forInExpr) {
        return;
    }
    if (forInExpr->body) {
        CheckControlExprHelper(forInExpr->body->body, depth);
    }
}

void StructuralRuleGFUN01::CheckControlExpr(const AST::Expr& expr, size_t& depth)
{
    if (depth > MAX_DEPTH_LEVEL) {
        return;
    }
    switch (expr.astKind) {
        case ASTKind::IF_EXPR: {
            depth++;
            CheckIfExprHelper(expr, depth);
            break;
        }
        case ASTKind::WHILE_EXPR: {
            depth++;
            CheckWhileExpr(expr, depth);
            break;
        }
        case ASTKind::DO_WHILE_EXPR: {
            depth++;
            CheckDoWhileExpr(expr, depth);
            break;
        }
        case ASTKind::MATCH_EXPR: {
            depth++;
            CheckMatchExpr(expr, depth);
            break;
        }
        case ASTKind::FOR_IN_EXPR: {
            depth++;
            CheckForInExpr(expr, depth);
            break;
        }
        default:
            break;
    }
}

void StructuralRuleGFUN01::CheckControlBlock(const AST::Expr& expr, AST::ASTKind astKind)
{
    size_t depth = 0;
    CheckControlExpr(expr, depth);
    if (depth > MAX_DEPTH_LEVEL) {
        if (astKind == AST::ASTKind::FUNC_DECL) {
            Diagnose(
                expr.begin, expr.end, CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_2, "");
        }
        if (astKind == AST::ASTKind::LAMBDA_EXPR) {
            Diagnose(
                expr.begin, expr.end, CodeCheckDiagKind::G_FUN_01_function_single_responsibility_information_5, "");
        }
    }
}

void StructuralRuleGFUN01::FuncAndCodeControlBlockFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                CheckFuncHelper(funcDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const LambdaExpr& lambdaExpr) {
                CheckLambdaExprHelper(lambdaExpr);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGFUN01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FuncAndCodeControlBlockFinder(node);
}
} // namespace Codira::CodeCheck
