/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_COMMON_H
#define PANDA_TOOLING_INSPECTOR_TEST_COMMON_H

#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/json_parser.h"

namespace ark::tooling::inspector {
using SourceFileFilter = std::function<bool(std::string_view)>;
}  // namespace ark::tooling::inspector

namespace ark::tooling::inspector::test {
template <typename T>
inline JsonObject ToJson(const T &object)
{
    JsonObjectBuilder jsonBuilder;
    object.Serialize(jsonBuilder);
    return JsonObject(std::move(jsonBuilder).Build());
}

inline JsonObject ToObject(std::function<void(JsonObjectBuilder &)> &&params)
{
    JsonObjectBuilder jsonBuilder;
    params(jsonBuilder);
    return JsonObject(std::move(jsonBuilder).Build());
}

}  // namespace ark::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_JSON_OBJECT_MATCHER_H
