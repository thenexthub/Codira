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

#ifndef LSPSERVER_SEMANTICHIGHLIGHT_H
#define LSPSERVER_SEMANTICHIGHLIGHT_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "Codira/Lex/Token.h"
#include "Codira/Parse/Parser.h"
#include "Codira/AST/Match.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Basic/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Modules/ImportManager.h"
#include "../../logger/Logger.h"

namespace ark {
bool operator==(const SemanticHighlightToken &, const SemanticHighlightToken &);

class SemanticHighlightImpl {
public:
    static void FindHighlightsTokens(const ArkAST &ast, std::vector<SemanticHighlightToken> &result,
                                     unsigned int fileID);
    static bool NodeValid(const Ptr<Node> node, unsigned int fileID, const std::string &name);
    static bool NeedHightlight(const ArkAST &ast, const Ptr<Node> &node);
};
} // namespace ark

#endif // LSPSERVER_SEMANTICHIGHLIGHT_H
