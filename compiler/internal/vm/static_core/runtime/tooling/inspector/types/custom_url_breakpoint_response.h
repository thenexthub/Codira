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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_CUSTOM_URL_BREAKPOINT_RESPONSE_H
#define PANDA_TOOLING_INSPECTOR_TYPES_CUSTOM_URL_BREAKPOINT_RESPONSE_H

#include "json_serialization/serializable.h"

#include <optional>

#include "libarkbase/macros.h"

#include "types/numeric_id.h"

namespace ark::tooling::inspector {

/// @brief Response for single breakpoint set by custom "Debugger.getPossibleAndSetBreakpointByUrl"
class CustomUrlBreakpointResponse final : public JsonSerializable {
public:
    explicit CustomUrlBreakpointResponse(size_t lineNumber) : lineNumber_(lineNumber) {}

    DEFAULT_COPY_SEMANTIC(CustomUrlBreakpointResponse);
    DEFAULT_MOVE_SEMANTIC(CustomUrlBreakpointResponse);

    ~CustomUrlBreakpointResponse() override = default;

    CustomUrlBreakpointResponse &SetLineNumber(size_t lineNumber)
    {
        lineNumber_ = lineNumber;
        return *this;
    }

    CustomUrlBreakpointResponse &SetColumnNumber(std::optional<size_t> optColumnNumber)
    {
        columnNumber_ = optColumnNumber.has_value() ? *optColumnNumber : DEFAULT_COLUMN_NUMBER;
        return *this;
    }

    CustomUrlBreakpointResponse &SetScriptId(ScriptId scriptId)
    {
        scriptId_ = scriptId;
        return *this;
    }

    CustomUrlBreakpointResponse &SetBreakpointId(BreakpointId id)
    {
        id_ = id;
        return *this;
    }

    void Serialize(JsonObjectBuilder &builder) const override;

private:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t DEFAULT_COLUMN_NUMBER = 0;

private:
    // Default values are selected for compatibility
    size_t lineNumber_ {0};
    size_t columnNumber_ {DEFAULT_COLUMN_NUMBER};
    ScriptId scriptId_ {0};
    std::optional<BreakpointId> id_ {};  // "invalid" default value
};

class CustomUrlBreakpointLocations final : public JsonSerializable {
public:
    void Serialize(JsonObjectBuilder &builder) const override;

    void Add(CustomUrlBreakpointResponse &&loc)
    {
        locations_.emplace_back(std::move(loc));
    }

private:
    std::vector<CustomUrlBreakpointResponse> locations_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_CUSTOM_URL_BREAKPOINT_RESPONSE_H
