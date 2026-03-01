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

#include <iostream>
#include "mem/vm_handle.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"
#include "libarkbase/utils/utf.h"
#include "intrinsics.h"
#include "utility"
#include "libarkbase/utils/logger.h"

#ifdef PANDA_TARGET_OHOS
#include <hilog/log.h>
#endif  // PANDA_TARGET_OHOS

namespace ark::ets::intrinsics {

enum class ConsoleLevel : int32_t {
    ENUM_BEGIN = 0,
    DEBUG = ENUM_BEGIN,
    INFO = 1,
    LOG = 2,
    WARN = 3,
    ERROR = 4,
    PRINTLN = 5,
    ENUM_END = PRINTLN
};

#ifdef PANDA_TARGET_OHOS
namespace {

void LogPrint(ConsoleLevel level, const char *component, const char *msg)
{
    constexpr static auto TAG = "ArkTSApp";
    constexpr static unsigned int ARK_DOMAIN = 0x3D00;
#ifdef PANDA_OHOS_USE_INNER_HILOG
    constexpr static OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_APP, ARK_DOMAIN, TAG};
    switch (level) {
        case ConsoleLevel::DEBUG:
            OHOS::HiviewDFX::HiLog::Debug(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ConsoleLevel::LOG:
        case ConsoleLevel::INFO:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ConsoleLevel::WARN:
            OHOS::HiviewDFX::HiLog::Warn(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ConsoleLevel::ERROR:
            OHOS::HiviewDFX::HiLog::Error(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#else
    switch (level) {
        case ConsoleLevel::DEBUG:
            OH_LOG_Print(LOG_APP, LOG_DEBUG, ARK_DOMAIN, TAG, "%s: %s", component, msg);
            break;
        case ConsoleLevel::LOG:
        case ConsoleLevel::INFO:
            OH_LOG_Print(LOG_APP, LOG_INFO, ARK_DOMAIN, TAG, "%s: %s", component, msg);
            break;
        case ConsoleLevel::WARN:
            OH_LOG_Print(LOG_APP, LOG_WARN, ARK_DOMAIN, TAG, "%s: %s", component, msg);
            break;
        case ConsoleLevel::ERROR:
            OH_LOG_Print(LOG_APP, LOG_ERROR, ARK_DOMAIN, TAG, "%s: %s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#endif  // PANDA_OHOS_USE_INNER_HILOG
}
}  // namespace
#endif  // PANDA_TARGET_OHOS

extern "C" {
void StdConsolePrintString(ObjectHeader *header [[maybe_unused]], EtsString *data, int32_t level)
{
    auto lvl = static_cast<ConsoleLevel>(level);
    ASSERT(lvl >= ConsoleLevel::ENUM_BEGIN && lvl <= ConsoleLevel::ENUM_END);

    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::String> dH(thread, data->GetCoreType());
    auto res = PandaString(data->GetUtf8());

#ifdef PANDA_TARGET_OHOS
    if (lvl == ConsoleLevel::PRINTLN) {
        std::cout << res << std::flush;
    } else {
        LogPrint(lvl, "arkts.console", res.data());
    }
#else
    (lvl == ConsoleLevel::ERROR ? std::cerr : std::cout) << res << std::flush;
#endif  // PANDA_TARGET_OHOS
}
}

}  // namespace ark::ets::intrinsics
