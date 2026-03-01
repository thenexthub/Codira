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

#include "Format/NodeFormatter/Expr/LitConstExprFormatter.h"
#include <regex>
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

std::string LitConstExprFormatter::CleanString(const std::string& input)
{
    std::string result = std::regex_replace(input, std::regex(R"()" + options.newLine + R"()"), "");
    result = std::regex_replace(result, std::regex(R"(\s+\.)"), ".");
    result = std::regex_replace(result, std::regex(R"(\s+)"), " ");
    result = std::regex_replace(result, std::regex(R"(^\s+|\s+$)"), "");

    return result;
}

void LitConstExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto expr = As<ASTKind::LIT_CONST_EXPR>(node);
    AddLitConstExpr(doc, *expr, level);
}

void LitConstExprFormatter::AddLitConstExpr(Doc& doc, const Codira::AST::LitConstExpr& litConstExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    std::string strValue;
    std::string quote = litConstExpr.isSingleQuote ? "'" : "\"";
    if (litConstExpr.kind == LitConstKind::STRING || litConstExpr.kind == LitConstKind::JSTRING) {
        if (litConstExpr.stringKind == StringKind::MULTILINE_RAW) {
            AddMultiLineRaw(doc, litConstExpr, quote, strValue, level);
        } else if (litConstExpr.stringKind == StringKind::MULTILINE) {
            AddMultiLine(doc, litConstExpr, quote, strValue, level);
        } else if (litConstExpr.stringKind == StringKind::JSTRING) {
            strValue += "J" + quote;
            AddSiPartExprs(doc, litConstExpr, strValue, level, true);
            strValue += quote;
        } else {
            strValue += quote;
            AddSiPartExprs(doc, litConstExpr, strValue, level, true);
            strValue += quote;
        }
    } else if (litConstExpr.kind == LitConstKind::RUNE) {
        std::string runeSymbol = "r";
        strValue = runeSymbol + quote;
        AddSiPartExprs(doc, litConstExpr, strValue, level);
        strValue += quote;
    } else if (litConstExpr.kind == LitConstKind::UNIT) {
        if (!litConstExpr.TestAttr(Attribute::COMPILER_ADD)) {
            strValue = "()";
        }
    } else {
        AddSiPartExprs(doc, litConstExpr, strValue, level);
    }
    doc.members.emplace_back(DocType::STRING, level, strValue);
    if (litConstExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void LitConstExprFormatter::MultiLineInterpolationExprProcessor(Doc& doc, Ptr<Expr> expr, int level)
{
    auto interpolationExpr = As<ASTKind::INTERPOLATION_EXPR>(expr);
    if (interpolationExpr->block->body.size() > 1) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        for (auto& node : interpolationExpr->block->body) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(node, level + 1));
            if (node != interpolationExpr->block->body.back()) {
                doc.members.emplace_back(DocType::LINE, level + 1, "");
            }
        }
        doc.members.emplace_back(DocType::LINE, level, "");
    } else if (interpolationExpr->block->body.size() == 1) {
        for (auto& node : interpolationExpr->block->body) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(node, level));
        }
    }
}

void LitConstExprFormatter::AddSiPartExpr(Doc& doc, Ptr<Expr> expr, std::string& strValue, int level, bool isSingleLine)
{
    if (expr->astKind == ASTKind::LIT_CONST_EXPR) {
        auto siLitConstExpr = As<ASTKind::LIT_CONST_EXPR>(expr);
        strValue += siLitConstExpr->rawString.empty() ? siLitConstExpr->stringValue : siLitConstExpr->rawString;
    } else if (expr->astKind == ASTKind::INTERPOLATION_EXPR) {
        if (isSingleLine) {
            strValue += "${";
            auto interpolationExpr = As<ASTKind::INTERPOLATION_EXPR>(expr);
            for (auto& node : interpolationExpr->block->body) {
                auto nodeDoc = astToFormatSource.ASTToDoc(node, level);
                strValue += CleanString(astToFormatSource.DocToString(nodeDoc));
            }
            strValue += "}";
        } else {
            strValue += "${";
            doc.members.emplace_back(DocType::STRING, level, strValue);
            MultiLineInterpolationExprProcessor(doc, expr, level);
            strValue = "}";
        }
    }
}

void LitConstExprFormatter::AddSiPartExprs(
    Doc& doc, const LitConstExpr& litConstExpr, std::string& strValue, int level, bool isSingleLine)
{
    if (litConstExpr.siExpr) {
        for (auto& expr : litConstExpr.siExpr->strPartExprs) {
            AddSiPartExpr(doc, expr, strValue, level, isSingleLine);
        }
    } else {
        strValue += litConstExpr.rawString.empty() ? litConstExpr.stringValue : litConstExpr.rawString;
    }
}

void LitConstExprFormatter::AddMultiLineRaw(
    Doc& doc, const LitConstExpr& litConstExpr, const std::string& quote, std::string& strValue, int level)
{
    for (unsigned i = 0; i < litConstExpr.delimiterNum; ++i) {
        strValue += "#";
    }
    strValue += quote;
    AddSiPartExprs(doc, litConstExpr, strValue, level);
    strValue += quote;
    for (unsigned i = 0; i < litConstExpr.delimiterNum; ++i) {
        strValue += "#";
    }
}

void LitConstExprFormatter::AddMultiLine(
    Doc& doc, const LitConstExpr& litConstExpr, const std::string& quote, std::string& strValue, int level)
{
    auto tripleQuotes = quote + quote + quote;
    strValue += tripleQuotes + "\n";
    AddSiPartExprs(doc, litConstExpr, strValue, level);
    strValue += tripleQuotes;
}
} // namespace Codira::Format
