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

#include "types/url_breakpoint_request.h"

#include <cfloat>
#include <cmath>

#include "libarkbase/utils/json_parser.h"

#include "types/numeric_id.h"

namespace ark::tooling::inspector {

Expected<UrlBreakpointRequest, std::string> UrlBreakpointRequest::FromJson(const JsonObject &object)
{
    UrlBreakpointRequest parsed;

    auto optLineNumber = ParseSizeProperty(object, "lineNumber");
    if (!optLineNumber) {
        return Unexpected(optLineNumber.Error());
    }
    parsed.lineNumber_ = optLineNumber.Value() + 1;

    if (auto url = object.GetValue<JsonObject::StringT>("url")) {
        parsed.url_ = *url;
    } else if (auto urlRegex = object.GetValue<JsonObject::StringT>("urlRegex")) {
        parsed.urlRegex_ = *urlRegex;
    } else {
        return Unexpected(std::string("Either 'url' or 'urlRegex' must be specified"));
    }

    if (auto condition = object.template GetValue<JsonObject::StringT>("condition")) {
        parsed.condition_ = *condition;
    }

    return parsed;
}

}  // namespace ark::tooling::inspector
