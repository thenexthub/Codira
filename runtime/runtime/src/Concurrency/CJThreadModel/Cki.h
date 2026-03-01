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


#ifndef MRT_CKI_H
#define MRT_CKI_H

#include <cstdint>

#include "Base/Log.h"
#include "schedule.h"

namespace MapleRuntime {
namespace Cki { // codethreadKeyItems
const uint8_t SLOT_TABLE_SIZE = 8;
extern CODEThreadKey codethreadKeyItems[SLOT_TABLE_SIZE];

enum CODEThreadKeySlot {
    SLOT_MUTATOR = 0,
    CKI_SIZE = 1, // Number of Used Slots
    RESERVED_2 = 2,
    RESERVED_3 = 3,
    RESERVED_4 = 4,
    RESERVED_5 = 5,
    RESERVED_6 = 6,
    RESERVED_7 = 7
};

int CreateCKI();

inline int SetCKI(const void* value, CODEThreadKeySlot slot)
{
    return CODEThreadSetspecific(codethreadKeyItems[slot], const_cast<void*>(value));
}

inline void* GetCKI(CODEThreadKeySlot slot) { return CODEThreadGetspecific(codethreadKeyItems[slot]); }
} // namespace Cki
} // namespace MapleRuntime

#endif // MRT_CKI_H
