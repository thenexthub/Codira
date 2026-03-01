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

#ifndef LSPSERVER_WORKSPACESYMBOLTYPE_H
#define LSPSERVER_WORKSPACESYMBOLTYPE_H
#include "Common.h"
#include "nlohmann/json.hpp"

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {
enum class SymbolKind {
    FILE = 1,
    PACKAGE = 4,
    CLASS = 5,
    PROPERTY = 7,
    CONSTRUCTOR = 9,
    ENUM = 10,
    INTERFACE_DECL = 11,
    FUNCTION = 12,
    VARIABLE = 13,
    BOOLEAN = 17,
    OBJECT = 19,
    NULL_KIND = 21,
    ENUMMEMBER = 22,
    STRUCT = 23,
    OPERATOR = 25,
};

struct WorkspaceSymbolParams {
    std::string query {};
};

struct SymbolInformation {
    std::string name {};
    SymbolKind kind {SymbolKind::NULL_KIND};
    Location location {};
    std::string containerName {};

    bool operator<(const ark::SymbolInformation &right) const
    {
        return std::tie(this->location, this->kind, this->name, this->containerName) <
               std::tie(right.location, right.kind, right.name, right.containerName);
    }

    bool operator==(const ark::SymbolInformation &right) const
    {
        return std::tie(this->location, this->kind, this->name, this->containerName) ==
               std::tie(right.location, right.kind, right.name, right.containerName);
    }

    bool operator!=(const ark::SymbolInformation &right) const
    {
        return !(*this == right);
    }
};

bool FromJSON(const nlohmann::json &params, WorkspaceSymbolParams &reply);
bool ToJSON(const SymbolInformation &params, nlohmann::json &item);
}

#endif // LSPSERVER_WORKSPACESYMBOLTYPE_H
