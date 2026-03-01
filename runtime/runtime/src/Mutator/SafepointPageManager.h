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


#ifndef MRT_SAFEPOINT_PAGE_MANAGER_H
#define MRT_SAFEPOINT_PAGE_MANAGER_H

#include <sys/mman.h>

#include "Base/Globals.h"
#include "Base/SysCall.h"
#include "securec.h"

namespace MapleRuntime {
class SafepointPageManager {
public:
    SafepointPageManager() {}

    void Init()
    {
        readablePage = reinterpret_cast<uint8_t*>(
            mmap(nullptr, MapleRuntime::MRT_PAGE_SIZE, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        unreadablePage = reinterpret_cast<uint8_t*>(
            mmap(nullptr, MapleRuntime::MRT_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        unreadablePageForRawData = reinterpret_cast<uint8_t*>(
            mmap(nullptr, MapleRuntime::MRT_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        CHECK_DETAIL(
            (readablePage != MAP_FAILED && unreadablePage != MAP_FAILED && unreadablePageForRawData != MAP_FAILED),
            "allocate safepoint page failed!");
    }

    ~SafepointPageManager()
    {
        CHECK_E(UNLIKELY(munmap(readablePage, MapleRuntime::MRT_PAGE_SIZE) != EOK),
                "munmap failed in SafepointPageManager readablePage destruction, errno: %d", errno);
        CHECK_E(UNLIKELY(munmap(unreadablePage, MapleRuntime::MRT_PAGE_SIZE) != EOK),
                "munmap failed in SafepointPageManager unreadablePage destruction, errno: %d", errno);
        CHECK_E(UNLIKELY(munmap(unreadablePageForRawData, MapleRuntime::MRT_PAGE_SIZE) != EOK),
                "munmap failed in SafepointPageManager unreadablePageForRawData destruction, errno: %d", errno);
    }

    uint8_t* GetSafepointReadablePage() const { return readablePage; }

    uint8_t* GetSafepointUnreadablePage() const { return unreadablePage; }

    // refactor this code: move to where it is used.
    uint8_t* GetUnreadablePage() const { return unreadablePageForRawData; }

private:
    uint8_t* readablePage = nullptr;
    uint8_t* unreadablePage = nullptr;
    uint8_t* unreadablePageForRawData = nullptr;
};
} // namespace MapleRuntime
#endif // MRT_SAFEPOINT_PAGE_MANAGER_H
