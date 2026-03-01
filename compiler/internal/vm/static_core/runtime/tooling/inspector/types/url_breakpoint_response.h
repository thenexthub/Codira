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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_RESPONSE_H
#define PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_RESPONSE_H

#include "tooling/inspector/json_serialization/serializable.h"

#include <vector>

#include "libarkbase/macros.h"

#include "types/location.h"
#include "types/numeric_id.h"

namespace ark {
class JsonObject;
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {

class CustomUrlBreakpointResponse;

class UrlBreakpointResponse final : public JsonSerializable {
public:
    UrlBreakpointResponse(BreakpointId id, std::vector<Location> &&locations)
        : breakpointId_(id), locations_(std::move(locations))
    {
    }

    explicit UrlBreakpointResponse(BreakpointId id) : breakpointId_(id) {}

    DEFAULT_COPY_SEMANTIC(UrlBreakpointResponse);
    DEFAULT_MOVE_SEMANTIC(UrlBreakpointResponse);

    ~UrlBreakpointResponse() override = default;

    BreakpointId GetBreakpointId() const
    {
        return breakpointId_;
    }

    void SetBreakpointId(BreakpointId id)
    {
        breakpointId_ = id;
    }

    const std::vector<Location> &GetLocations() const
    {
        return locations_;
    }

    void AddLocation(Location &&loc)
    {
        locations_.emplace_back(std::move(loc));
    }

    CustomUrlBreakpointResponse ToCustomUrlBreakpointResponse() const;

    void Serialize(JsonObjectBuilder &builder) const override;

private:
    BreakpointId breakpointId_ {0};
    std::vector<Location> locations_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_RESPONSE_H
