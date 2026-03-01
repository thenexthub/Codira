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

#ifndef LSPSERVER_TYPEHIERARCHYIMPL_H
#define LSPSERVER_TYPEHIERARCHYIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../common/FindDeclUsage.h"
#include "../../logger/Logger.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Lex/Token.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"

namespace ark {
class TypeHierarchyImpl {
public:
    static std::string curFilePath;
    static Position curPos;

    static void FindTypeHierarchyImpl(const ArkAST &ast, TypeHierarchyItem &result, Codira::Position pos);

    static TypeHierarchyItem TypeHierarchyFrom(Ptr<const Decl> decl);

    static void FindSuperTypesImpl(std::vector<TypeHierarchyItem> &results, const TypeHierarchyItem &hierarchyItem);

    static void FindSubTypesImpl(std::vector<TypeHierarchyItem> &results, const TypeHierarchyItem &hierarchyItem);
};
} // namespace ark

#endif // LSPSERVER_TYPEHIERARCHYIMPL_H
