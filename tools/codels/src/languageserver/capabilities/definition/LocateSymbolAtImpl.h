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

#ifndef LSPSERVER_LOCATESYMBOLATIMPL_H
#define LSPSERVER_LOCATESYMBOLATIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "Codira/Lex/Token.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/Basic/Match.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Symbol.h"
#include "Codira/Modules/ImportManager.h"
#include "../../logger/Logger.h"
#include "../../common/Callbacks.h"
#include "../../CompilerCodiraProject.h"
#include "../../common/Utils.h"
#include "CrossDefinitionCodira2C.h"

namespace ark {
struct LocatedSymbol {
    // The name of the symbol.
    std::string Name = "";
    // Where the symbol is defined.
    Location Definition;

    std::vector<message> CrossMessage;
};

class LocateSymbolAtImpl {
public:
    static std::string curFilePath;

    static bool LocateSymbolAt(const ArkAST &ast, LocatedSymbol &result, Codira::Position pos);

    static void CrossDefinition(std::vector<message> &CrossMessage, Ptr<Codira::AST::FuncDecl> funcDecl);
};
} // namespace ark

#endif // LSPSERVER_LOCATESYMBOLATIMPL_H
