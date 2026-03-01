/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_SCOPE_H
#define PANDA_TOOLING_INSPECTOR_TYPES_SCOPE_H

#include "runtime/tooling/inspector/types/remote_object.h"
#include "runtime/tooling/inspector/json_serialization/serializable.h"

#include <optional>
#include <string>

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class Scope final : public JsonSerializable {
public:
    enum class Type { GLOBAL, LOCAL };

    Scope(Type type, RemoteObject object, std::optional<std::string> name = {});

    void Serialize(JsonObjectBuilder &builder) const override;

private:
    Type type_;
    RemoteObject object_;
    std::optional<std::string> name_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_SCOPE_H
