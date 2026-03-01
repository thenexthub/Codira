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

#include <sys/mman.h>
#include "common_components/log/log.h"
#include "common_components/platform/map.h"

namespace common {

bool PageProtect(void *mem, size_t size, int prot)
{
    int ret = mprotect(mem, size, prot);
    if (ret != 0) {
        LOG_COMMON(ERROR) << "PageProtect mem = " << mem << ", size = " << size << ", change to " << prot
                          << " failed, ret = " << ret << ", error code is " << errno;
        return false;
    }
    return true;
}
}  // namespace common
