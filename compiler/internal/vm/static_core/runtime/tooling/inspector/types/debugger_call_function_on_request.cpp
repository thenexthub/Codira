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

#include "types/debugger_call_function_on_request.h"
#include "libarkbase/utils/json_parser.h"

#include "types/numeric_id.h"

namespace ark::tooling::inspector {

Expected<DebuggerCallFunctionOnRequest, std::string> DebuggerCallFunctionOnRequest::FromJson(const JsonObject &object)
{
    DebuggerCallFunctionOnRequest parsed;

    auto optFrameId = ParseNumericId<FrameId>(object, "callFrameId");
    if (!optFrameId) {
        return Unexpected(optFrameId.Error());
    }
    parsed.callFrameId_ = *optFrameId;

    const auto *optFunctionDeclaration = object.GetValue<JsonObject::StringT>("functionDeclaration");
    if (optFunctionDeclaration == nullptr) {
        return Unexpected(std::string("no 'functionDeclaration' field"));
    }
    parsed.functionDeclaration_ = optFunctionDeclaration;

    auto optSilent = object.GetValue<JsonObject::BoolT>("silent");
    if (optSilent != nullptr) {
        parsed.silent_ = *optSilent;
    }

    auto optReturnByValue = object.GetValue<JsonObject::BoolT>("returnByValue");
    if (optReturnByValue != nullptr) {
        parsed.returnByValue_ = *optReturnByValue;
    }

    auto optGeneratePreview = object.GetValue<JsonObject::BoolT>("generatePreview");
    if (optGeneratePreview != nullptr) {
        parsed.generatePreview_ = *optGeneratePreview;
    }

    return parsed;
}

}  // namespace ark::tooling::inspector
