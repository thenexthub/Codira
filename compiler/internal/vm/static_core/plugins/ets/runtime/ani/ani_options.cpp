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

#include "plugins/ets/runtime/ani/ani_options.h"

#include <sstream>

#ifdef PANDA_TARGET_OHOS
#include "plugins/ets/runtime/platform/ohos/logger.h"
#endif

namespace ark::ets::ani {

LoggerCallback ANIOptions::GetLoggerCallback() const
{
    auto *callback = reinterpret_cast<LoggerCallback>(GetExtra(OptionKey::LOGGER));
#ifdef PANDA_TARGET_OHOS
    return callback == nullptr ? ohos::DefaultLogger : callback;
#else
    return callback;
#endif
}

Expected<bool, std::string> ANIOptions::SetOption(std::string_view key, std::string_view value, void *extra)
{
    auto &map = GetOptionsMap();
    auto it = map.find(key);
    if (it == map.end()) {
        return false;
    }

    auto v = it->second.fn(value, extra);
    if (v.HasValue()) {
        optionValues_[helpers::ToUnderlying(it->second.optionKey)] = std::move(v.Value());
    } else {
        return Unexpected(v.Error());
    }
    return true;
}

// CC-OFFNXT(G.FUD.05) solid logic
const std::map<std::string_view, ANIOptions::OptionHandler> &ANIOptions::GetOptionsMap()
{
    static const std::map<std::string_view, OptionHandler> OPTIONS_MAP = {
        {
            "--logger",
            {
                OptionKey::LOGGER,
                [](std::string_view value, void *extra) -> Expected<std::unique_ptr<OptionValue>, std::string> {
                    if (!value.empty()) {
                        std::stringstream ss;
                        ss << "'--logger' option mustn't have value, value='" << value << "'";
                        return Unexpected(ss.str());
                    }
                    if (extra == nullptr) {
                        return Unexpected(std::string("'--logger' option has 'extra==NULL'"));
                    }
                    return std::make_unique<OptionValue>(OptionValue {std::string(value), extra});
                },
            },
        },
        {
            "--verify:ani",
            {
                OptionKey::VERIFY_ANI,
                [](std::string_view value, void *extra) -> Expected<std::unique_ptr<OptionValue>, std::string> {
                    if (!value.empty()) {
                        std::stringstream ss;
                        ss << "'--verify:ani' option mustn't have value, value='" << value << "'";
                        return Unexpected(ss.str());
                    }
                    if (extra != nullptr) {
                        return Unexpected(std::string("'--verify:ani' option has 'extra != NULL'"));
                    }
                    return std::make_unique<OptionValue>(OptionValue {true, extra});
                },
            },
        },
#ifdef PANDA_ETS_INTEROP_JS
        // clang-format off
        {
            "--ext:interop",
            {
                OptionKey::INTEROP,
                [](std::string_view value, void *extra) -> Expected<std::unique_ptr<OptionValue>, std::string> {
                    if (!value.empty()) {
                        std::stringstream ss;
                        ss << "'--ext:interop' option mustn't have value, value='" << value << "'";
                        return Unexpected(ss.str());
                    }
                    return std::make_unique<OptionValue>(OptionValue {true, extra});
                }
            },
        },
    // clang-format on
#endif  // PANDA_ETS_INTEROP_JS
    };

    return OPTIONS_MAP;
}

}  // namespace ark::ets::ani
