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


#ifndef MRT_SCOPED_LOCK_OBJECT_H
#define MRT_SCOPED_LOCK_OBJECT_H
#include "Common/BaseObject.h"
namespace MapleRuntime {
class ScopedObjectLock {
public:
    ATTR_NO_INLINE explicit ScopedObjectLock(BaseObject& obj)
    {
        do {
            ObjectState::ObjectStateCode state = obj.GetObjectState().GetStateCode();
            if (state == ObjectState::FORWARDING) {
                sched_yield();
                continue;
            } else if (state == ObjectState::LOCKED || state == ObjectState::FORWARDED ||
                       state == ObjectState::NORMAL) {
                fromCopy = &obj;
            } else {
                LOG(RTLOG_FATAL, "this state need to be dealt with when lock object, state: %u\n", state);
                return;
            }
            StateWord curState = fromCopy->GetStateWord();
            if (fromCopy->TryLockObject(curState)) {
                return;
            }
        } while (true);
    }
    ~ScopedObjectLock()
    {
        CHECK_DETAIL(fromCopy != nullptr, "from copy is nullptr when unlock object\n");
        fromCopy->UnlockObject(ObjectState::NORMAL);
    }

private:
    BaseObject* fromCopy = { nullptr };
};
} // namespace MapleRuntime
#endif // ~MRT_SCOPED_LOCK_OBJECT_H
