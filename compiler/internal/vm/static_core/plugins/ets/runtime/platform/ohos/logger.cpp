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

#include "logger.h"
#include "libarkbase/macros.h"
#include <hilog/log.h>
#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets::ohos {

void DefaultLogger([[maybe_unused]] FILE *stream, int level, const char *component, const char *msg)
{
#ifdef PANDA_OHOS_USE_INNER_HILOG
    constexpr static unsigned int ARK_DOMAIN = 0xD003F00;
    constexpr static auto TAG = "ArkEtsVm";
    constexpr static OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, ARK_DOMAIN, TAG};
    switch (level) {
        case ANI_LOGLEVEL_DEBUG:
            OHOS::HiviewDFX::HiLog::Debug(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ANI_LOGLEVEL_INFO:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ANI_LOGLEVEL_ERROR:
            OHOS::HiviewDFX::HiLog::Error(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ANI_LOGLEVEL_FATAL:
            OHOS::HiviewDFX::HiLog::Fatal(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        case ANI_LOGLEVEL_WARNING:
            OHOS::HiviewDFX::HiLog::Warn(LABEL, "%{public}s: %{public}s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#else
    switch (level) {
        case ANI_LOGLEVEL_DEBUG:
            OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ANI_LOGLEVEL_INFO:
            OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ANI_LOGLEVEL_ERROR:
            OH_LOG_Print(LOG_APP, LOG_ERROR, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ANI_LOGLEVEL_FATAL:
            OH_LOG_Print(LOG_APP, LOG_FATAL, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        case ANI_LOGLEVEL_WARNING:
            OH_LOG_Print(LOG_APP, LOG_WARN, 0xFF00, "ArkEtsVm", "%s: %s", component, msg);
            break;
        default:
            UNREACHABLE();
    }
#endif  // PANDA_OHOS_USE_INNER_HILOG
}

}  // namespace ark::ets::ohos
