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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_SCRIPT_SOURCE_RESPONSE_H
#define PANDA_TOOLING_INSPECTOR_TYPES_SCRIPT_SOURCE_RESPONSE_H

#include "tooling/inspector/json_serialization/serializable.h"

#include "libarkbase/macros.h"
#include "libarkbase/utils/json_builder.h"

namespace ark::tooling::inspector {

class ScriptSourceResponse final : public JsonSerializable {
public:
    explicit ScriptSourceResponse(std::string_view source) : scriptSource_(source) {}
    explicit ScriptSourceResponse(std::string &&source) : scriptSource_(std::move(source)) {}

    DEFAULT_COPY_SEMANTIC(ScriptSourceResponse);
    DEFAULT_MOVE_SEMANTIC(ScriptSourceResponse);

    ~ScriptSourceResponse() override = default;

    void Serialize(JsonObjectBuilder &builder) const override
    {
        builder.AddProperty("scriptSource", scriptSource_);
    }

private:
    std::string scriptSource_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_SCRIPT_SOURCE_RESPONSE_H
