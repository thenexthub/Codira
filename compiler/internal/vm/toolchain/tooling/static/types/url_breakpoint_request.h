/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_REQUEST_H
#define PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_REQUEST_H

#include <optional>

#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"

namespace ark {
class JsonObject;
}  // namespace ark

namespace ark::tooling::inspector {

/// @brief Request parameters of `Debugger.setBreakpointByUrl` method
class UrlBreakpointRequest final {
public:
    static Expected<UrlBreakpointRequest, std::string> FromJson(const JsonObject &object);

    DEFAULT_COPY_SEMANTIC(UrlBreakpointRequest);
    DEFAULT_MOVE_SEMANTIC(UrlBreakpointRequest);

    ~UrlBreakpointRequest() = default;

    int32_t GetLineNumber() const
    {
        return lineNumber_;
    }

    const std::optional<std::string> &GetUrl() const
    {
        return url_;
    }

    const std::optional<std::string> &GetUrlRegex() const
    {
        return urlRegex_;
    }

    const std::optional<std::string> &GetCondition() const
    {
        return condition_;
    }

private:
    UrlBreakpointRequest() = default;

private:
    int32_t lineNumber_ {0};
    std::optional<std::string> url_;
    std::optional<std::string> urlRegex_;
    std::optional<std::string> condition_;
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_URL_BREAKPOINT_REQUEST_H
