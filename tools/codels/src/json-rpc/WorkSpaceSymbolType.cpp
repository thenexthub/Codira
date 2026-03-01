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

#include "WorkSpaceSymbolType.h"

namespace ark {
bool FromJSON(const nlohmann::json &params, WorkspaceSymbolParams &reply)
{
    nlohmann::json query = params["query"];
    if (query.is_null()) {
        return false;
    }
    reply.query = query.get<std::string>();
    return true;
}

bool ToJSON(const SymbolInformation &input, nlohmann::json &item)
{
    item["containerName"] = input.containerName;
    item["name"] = input.name;
    item["kind"] = static_cast<int>(input.kind);
    item["location"]["range"]["start"]["line"] = input.location.range.start.line;
    item["location"]["range"]["start"]["character"] = input.location.range.start.column;
    item["location"]["range"]["end"]["line"] = input.location.range.end.line;
    item["location"]["range"]["end"]["character"] = input.location.range.end.column;
    item["location"]["uri"] = input.location.uri.file;
    return true;
}
}
