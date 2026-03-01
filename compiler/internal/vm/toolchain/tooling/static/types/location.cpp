/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "types/location.h"

#include <cfloat>
#include <cmath>
#include <string>

#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/json_parser.h"

#include "types/numeric_id.h"

using namespace std::literals::string_literals;  // NOLINT(google-build-using-namespace)

namespace ark::tooling::inspector {
Expected<Location, std::string> Location::FromJsonProperty(const JsonObject &object, const char *propertyName)
{
    auto property = object.GetValue<JsonObject::JsonObjPointer>(propertyName);
    if (property == nullptr) {
        return Unexpected("No such property: "s + propertyName);
    }

    auto scriptId = ParseNumericId<ScriptId>(**property, "scriptId");
    if (!scriptId) {
        return Unexpected(scriptId.Error());
    }

    auto optLineNumber = ParseSizeProperty(**property, "lineNumber");
    if (!optLineNumber) {
        return Unexpected(optLineNumber.Error());
    }

    return Location(*scriptId, optLineNumber.Value() + 1);
}

void Location::Serialize(JsonObjectBuilder &builder) const
{
    builder.AddProperty("scriptId", std::to_string(scriptId_));
    builder.AddProperty("lineNumber", lineNumber_ - 1);
    if (columnNumber_) {
        builder.AddProperty("columnNumber", *columnNumber_);
    }
}

}  // namespace ark::tooling::inspector
