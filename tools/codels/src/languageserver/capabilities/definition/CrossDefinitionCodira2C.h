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

#ifndef LSPSERVER_CROSSDEFINITIONCODIRA2C_H
#define LSPSERVER_CROSSDEFINITIONCODIRA2C_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Codira/AST/Types.h"
#include "Codira/AST/Node.h"

namespace ark {

// extension crossLanguageJump
struct message {
    std::string targetLanguage;
    std::string functionName;
    std::vector<std::string> functionParameters{};
    std::string retType;
};

class CrossDefinitionCodira2C {
public:
    static void Codira2CGetFuncMessage(std::vector<message> &CrossMessage, Ptr<Codira::AST::FuncDecl> funcDecl);

    static std::string GetCType(const Codira::AST::Ty *ty, bool isSimple = false);

    static std::string TypeVarray(const Codira::AST::Ty *ty, bool isSimple);

    static std::string TypeCpointer(const Codira::AST::Ty *ty, bool isSimple);
};
}

#endif // LSPSERVER_CROSSDEFINITIONCODIRA2C_H
