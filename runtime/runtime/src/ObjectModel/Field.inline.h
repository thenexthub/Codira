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


#ifndef MRT_FIELD_INLINE_H
#define MRT_FIELD_INLINE_H

#include "Base/LogFile.h"
#include "Common/BaseObject.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
template<typename T, bool isAtomic>
void Field<T, isAtomic>::SetFieldValue(const BaseObject* obj, T v, std::memory_order order)
{
    DLOG(BARRIER, "write obj %p(%zu)+%zu field @%p 0x%zx -> 0x%zx", obj, obj->GetSize(),
         BaseObject::FieldOffset(obj, this), this, value, v);

    if (isAtomic) {
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanAtomicStore(&value, v, order);
#else
        // it is weired that "std::atomic_store_explicit(&value, v, order)" leads to compiling failure
        __atomic_store(&value, &v, order);
#endif
    } else {
        value = v;
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanWriteMemory(&value, sizeof(T));
#endif
    }
}
} // namespace MapleRuntime
#endif // MRT_FIELD_INLINE_H
