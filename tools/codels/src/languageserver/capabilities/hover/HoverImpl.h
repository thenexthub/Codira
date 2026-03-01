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

#ifndef LSPSERVER_HOVERIMPL_H
#define LSPSERVER_HOVERIMPL_H

#include "../../ArkAST.h"
#include "../../CompilerCodiraProject.h"
#include "../../common/Callbacks.h"
#include "../../common/ItemResolverUtil.h"
#include "../../common/Utils.h"
#include "../../logger/Logger.h"
#include "Codira/AST/Searcher.h"
#include "Codira/Basic/Match.h"

namespace ark {
const std::string PKG_NAME_WHERE_APILEVEL_AT = "ohos.labels";
const std::string APILEVEL_ANNO_NAME = "APILevel";
const std::string LEVEL_IDENTGIFIER = "level";
const std::string SYSCAP_IDENTGIFIER = "syscap";

class HoverImpl {
public:
    static int FindHover(const ArkAST &ast, Hover &result, Codira::Position pos);

    static int GetHoverMessage(Ptr<Codira::AST::Decl>, Hover &, const ArkAST &ast);

private:
    static std::string curFilePath;

    static Decl *GetRealDecl(const std::vector<Ptr<Decl>> &decls);

    static void TrimSpaceAndTab(std::string &s);

    static std::string GetHoverMessageByOuterDecl(const Decl &);

    static void RemoveStar(const std::string &content, std::string &result);

    static void RemoveAboveBlank(const std::string &content, std::vector<std::string> &lines, size_t &minIndent);

    static void RemoveBlankAndStar(const std::string &content, std::string &result);

    static std::string ResolveComment(const std::string &comment, const CommentKind kind);

    static std::string GetDeclApiKey(const Ptr<Decl> &decl);

    static bool IsAnnoAPILevel(Ptr<Annotation> anno, Ptr<ASTContext> ctx);

    static std::string GetDeclApiLevelAnnoInfo(Decl &decl, const ArkAST &ast);
};
} // namespace ark

#endif // LSPSERVER_HOVERIMPL_H
