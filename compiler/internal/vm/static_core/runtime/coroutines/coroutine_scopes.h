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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_SCOPES_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_SCOPES_H

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine.h"

namespace ark {

class ScopedCoroutineNativeCall {
public:
    explicit ScopedCoroutineNativeCall(Coroutine *coro) : saved_(coro)
    {
        saved_->GetManager()->OnNativeCallEnter(saved_);
    }

    ~ScopedCoroutineNativeCall()
    {
        saved_->GetManager()->OnNativeCallExit(saved_);
    }

private:
    NO_COPY_SEMANTIC(ScopedCoroutineNativeCall);
    NO_MOVE_SEMANTIC(ScopedCoroutineNativeCall);

    Coroutine *saved_ = nullptr;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_SCOPES_H */