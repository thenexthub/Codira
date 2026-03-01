/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "json_object_matcher.h"

namespace ark {

std::ostream &operator<<(std::ostream &os, const JsonObject::Value &value)
{
    if (auto *string = value.Get<JsonObject::StringT>()) {
        return os << std::quoted(*string);
    }
    if (auto *number = value.Get<JsonObject::NumT>()) {
        return os << *number;
    }
    if (auto *boolean = value.Get<JsonObject::BoolT>()) {
        return os << std::boolalpha << *boolean;
    }
    if (auto *array = value.Get<JsonObject::ArrayT>()) {
        return os << *array;
    }
    if (auto *object = value.Get<JsonObject::JsonObjPointer>()) {
        if (*object) {
            return os << **object;
        }
        return os << "null";
    }

    UNREACHABLE();
}

}  // namespace ark
