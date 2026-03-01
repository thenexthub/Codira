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

#ifndef COMMON_COMPONENTS_COMMON_SCOPED_LOCK_OBJECT_H
#define COMMON_COMPONENTS_COMMON_SCOPED_LOCK_OBJECT_H

#include "common_components/common/base_object.h"
#include "common_components/log/log.h"
namespace common {
class ScopedObjectLock {
public:
    NO_INLINE_CC explicit ScopedObjectLock(BaseObject& obj)
    {
        do {
            BaseStateWord::ForwardState state = obj.GetBaseStateWord().GetForwardState();
            if (state == BaseStateWord::ForwardState::FORWARDING ||
                state == BaseStateWord::ForwardState::FORWARDED ||
                state == BaseStateWord::ForwardState::NORMAL) {
                lockedObj_ = &obj;
            } else {
                LOG_COMMON(FATAL) << "this state need to be dealt with when lock object, state: " <<
                    static_cast<uint8_t>(state);
                return;
            }
            BaseStateWord curState = lockedObj_->GetBaseStateWord();
            if (lockedObj_->TryLockExclusive(curState)) {
                return;
            }
        } while (true);
    }
    ~ScopedObjectLock()
    {
        LOGF_CHECK(lockedObj_ != nullptr) << "from copy is nullptr when unlock object\n";
        lockedObj_->UnlockExclusive(BaseStateWord::ForwardState::NORMAL);
    }

private:
    BaseObject* lockedObj_ = { nullptr };
};
} // namespace common
#endif // ~COMMON_COMPONENTS_COMMON_SCOPED_LOCK_OBJECT_H
