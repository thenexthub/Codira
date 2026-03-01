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

#ifndef CODEFMT_SUBSCRIPTEXPRFORMATTER_H
#define CODEFMT_SUBSCRIPTEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Codira::Format {
class SubscriptExprFormatter : public ExprFormatter {
public:
    explicit SubscriptExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddSubscriptExpr(
        Doc& doc, const Codira::AST::SubscriptExpr& subscriptExpr, int level, FuncOptions& funcOptions);
};
} // namespace Codira::Format
#endif // CODEFMT_SUBSCRIPTEXPRFORMATTER_H
