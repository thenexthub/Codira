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

#ifndef COMMON_COMPONENTS_MUTATOR_SAFEPOINT_PAGE_MANAGER_H
#define COMMON_COMPONENTS_MUTATOR_SAFEPOINT_PAGE_MANAGER_H

#include <sys/mman.h>

#include "common_components/base/globals.h"
#include "common_components/base/sys_call.h"
#include "securec.h"

namespace common {
class SafepointPageManager {
public:
    SafepointPageManager() {}

    void Init()
    {
        readablePage_ = reinterpret_cast<uint8_t*>(
            mmap(nullptr, COMMON_PAGE_SIZE, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        unreadablePage_ = reinterpret_cast<uint8_t*>(
            mmap(nullptr, COMMON_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        unreadablePageForRawData_ = reinterpret_cast<uint8_t*>(
            mmap(nullptr, COMMON_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        LOGF_CHECK((readablePage_ != MAP_FAILED && unreadablePage_ != MAP_FAILED &&
            unreadablePageForRawData_ != MAP_FAILED)) << "allocate safepoint page failed!";
    }

    ~SafepointPageManager()
    {
        LOGE_IF(UNLIKELY_CC(munmap(readablePage_, COMMON_PAGE_SIZE) != EOK)) <<
            "munmap failed in SafepointPageManager readablePage destruction, errno: " << errno;
        LOGE_IF(UNLIKELY_CC(munmap(unreadablePage_, COMMON_PAGE_SIZE) != EOK)) <<
            "munmap failed in SafepointPageManager unreadablePage destruction, errno: " << errno;
        LOGE_IF(UNLIKELY_CC(munmap(unreadablePageForRawData_, COMMON_PAGE_SIZE) != EOK)) <<
            "munmap failed in SafepointPageManager unreadablePageForRawData destruction, errno: " << errno;
    }

    uint8_t* GetSafepointReadablePage() const { return readablePage_; }

    uint8_t* GetSafepointUnreadablePage() const { return unreadablePage_; }

    // refactor this code: move to where it is used.
    uint8_t* GetUnreadablePage() const { return unreadablePageForRawData_; }

private:
    uint8_t* readablePage_ = nullptr;
    uint8_t* unreadablePage_ = nullptr;
    uint8_t* unreadablePageForRawData_ = nullptr;
};
} // namespace common

#endif // COMMON_COMPONENTS_MUTATOR_SAFEPOINT_PAGE_MANAGER_H
