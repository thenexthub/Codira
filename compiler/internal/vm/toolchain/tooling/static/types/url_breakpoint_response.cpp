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

#include "types/url_breakpoint_response.h"

#include "libarkbase/utils/json_builder.h"

#include "types/custom_url_breakpoint_response.h"

namespace ark::tooling::inspector {

void UrlBreakpointResponse::Serialize(JsonObjectBuilder &builder) const
{
    builder.AddProperty("breakpointId", std::to_string(breakpointId_));
    builder.AddProperty("locations", [this](JsonArrayBuilder &locations) {
        for (const auto &loc : locations_) {
            locations.Add(loc);
        }
    });
}

CustomUrlBreakpointResponse UrlBreakpointResponse::ToCustomUrlBreakpointResponse() const
{
    ASSERT(!GetLocations().empty());
    const auto &loc = GetLocations()[0];
    CustomUrlBreakpointResponse converted(loc.GetLineNumber());
    return converted.SetColumnNumber(loc.GetColumnNumber())
        .SetScriptId(loc.GetScriptId())
        .SetBreakpointId(GetBreakpointId());
}

}  // namespace ark::tooling::inspector
