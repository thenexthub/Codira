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

#ifndef CODEFMT_LITCONSTEXPRFORMATTER_H
#define CODEFMT_LITCONSTEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Codira::Format {
class LitConstExprFormatter : public ExprFormatter {
public:
    explicit LitConstExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&) override;

private:
    std::string CleanString(const std::string& input);
    void AddLitConstExpr(Doc& doc, const Codira::AST::LitConstExpr& litConstExpr, int level);
    void MultiLineInterpolationExprProcessor(Doc& doc, Ptr<Codira::AST::Expr> expr, int level);
    void AddSiPartExpr(Doc& doc, Ptr<Codira::AST::Expr> expr, std::string& strValue, int level, bool isSingleLine);
    void AddSiPartExprs(Doc& doc, const Codira::AST::LitConstExpr& litConstExpr, std::string& strValue, int level,
        bool isSingleLine = false);
    void AddMultiLineRaw(Doc& doc, const Codira::AST::LitConstExpr& litConstExpr,
        const std::string& quote, std::string& strValue, int level);
    void AddMultiLine(Doc& doc, const Codira::AST::LitConstExpr& litConstExpr,
        const std::string& quote, std::string& strValue, int level);
};
} // namespace Codira::Format
#endif // CODEFMT_LITCONSTEXPRFORMATTER_H
