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

#ifndef CROSSLANGUANGEDEFINITION_H
#define CROSSLANGUANGEDEFINITION_H

#include "../../../json-rpc/Protocol.h"
#include "../../CompilerCodiraProject.h"
#include "Codira/AST/Symbol.h"

namespace ark {
struct RegisterItem{
    Location definition;
    Location declaration;
    std::string registerName;
    int registerType;
};

struct ExportIDItem{
    std::string exportName;
    std::string containerName;
};

struct RegisterCrossSymbolsResult {
    std::list<RegisterItem> registerItems{};
};

struct CrossSymbolsResult {
    std::set<Location> locations{};
};
class CrossLanguangeDefinition {
public:
    static bool getCrossSymbols(const CrossLanguageJumpParams &params, CrossSymbolsResult &result);
    static bool GetExportSID(IDArray id, ExportIDItem &exportIdItem);
    static bool getRegisterCrossSymbols(const CrossLanguageJumpParams &params, RegisterCrossSymbolsResult &result);
};
}

#endif // CROSSLANGUANGEDEFINITION_H

