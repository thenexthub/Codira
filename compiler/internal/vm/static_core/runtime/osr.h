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
#ifndef PANDA_OSR_H
#define PANDA_OSR_H

#include "libarkbase/macros.h"
#include "runtime/include/cframe.h"
#include "runtime/interpreter/frame.h"
#include "libarkbase/os/mutex.h"
#include "include/method.h"

namespace ark {

extern "C" void *PrepareOsrEntry(const Frame *iframe, uintptr_t bcOffset, const void *osrCode, void *cframePtr,
                                 uintptr_t *regBuffer, uintptr_t *fpRegBuffer);

bool OsrEntry(uintptr_t loopHeadBc, const void *osrCode);

extern "C" void OsrEntryAfterCFrame(Frame *frame, uintptr_t loopHeadBc, const void *osrCode, size_t frameSize);
extern "C" void OsrEntryAfterIFrame(Frame *frame, uintptr_t loopHeadBc, const void *osrCode, size_t frameSize,
                                    size_t stackParams);
extern "C" void OsrEntryTopFrame(Frame *frame, uintptr_t loopHeadBc, const void *osrCode, size_t frameSize,
                                 size_t stackParams);

extern "C" void SetOsrResult(Frame *frame, uint64_t uval, double fval);

class OsrCodeMap {
public:
    void *Get(const Method *method)
    {
        os::memory::ReadLockHolder holder(osrLock_);
        auto iter = codeMap_.find(method);
        if (UNLIKELY(iter == codeMap_.end())) {
            return nullptr;
        }
        return iter->second;
    }

    void Set(const Method *method, void *ptr)
    {
        if (UNLIKELY(ptr == nullptr)) {
            Remove(method);
            return;
        }
        os::memory::WriteLockHolder holder(osrLock_);
        codeMap_.insert({method, ptr});
    }

    void Remove(const Method *method)
    {
        os::memory::WriteLockHolder holder(osrLock_);
        codeMap_.erase(method);
    }

private:
    os::memory::RWLock osrLock_;
    PandaMap<const Method *, void *> codeMap_;
};

}  // namespace ark

#endif  // PANDA_OSR_H
