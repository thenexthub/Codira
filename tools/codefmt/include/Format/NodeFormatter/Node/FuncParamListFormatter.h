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

#ifndef CODEFMT_FUNCPARAMLISTFORMATTER_H
#define CODEFMT_FUNCPARAMLISTFORMATTER_H
#include "Format/NodeFormatter/NodeFormatter.h"

namespace Codira::Format {
class FuncParamListFormatter : public NodeFormatter {
public:
    explicit FuncParamListFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddFuncParamList(Doc& doc, const AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions);
    bool IsMultipleLineMacroExpandParam(const AST::FuncParamList& funcParamList);
    bool IsMultipleLine(const int& rightParentPosLine, const std::vector<OwnedPtr<AST::FuncParam>>& params) const;
    void AddEmptyParam(Doc& doc, int level);
};
} // namespace Codira::Format
#endif // CODEFMT_FUNCPARAMLISTFORMATTER_H
