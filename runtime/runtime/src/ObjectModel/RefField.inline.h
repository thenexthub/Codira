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


#ifndef MRT_REF_FIELD_INLINE_H
#define MRT_REF_FIELD_INLINE_H

#include "Base/LogFile.h"
#include "Common/BaseObject.h"
#include "ObjectModel/RefField.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
template<bool isAtomic>
void RefField<isAtomic>::SetTargetObject(const BaseObject* obj, std::memory_order order)
{
    RefField<> newField(obj);
    uintptr_t newVal = newField.GetFieldValue();
    RefFieldValue oldVal = fieldVal;
    (void)oldVal;

    if (isAtomic) {
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanAtomicStore(&fieldVal, static_cast<RefFieldValue>(newVal), order);
#else
        __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
#endif
    } else {
        fieldVal = static_cast<RefFieldValue>(newVal);
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanWriteMemory(&fieldVal, GetSize());
#endif
    }

    DLOG(BARRIER, "write field @%p 0x%zx -> %p", this, oldVal, obj);
}

template<bool isAtomic>
void RefField<isAtomic>::SetFieldValue(MAddress newVal, std::memory_order order)
{
    RefFieldValue oldVal = fieldVal;
    (void)oldVal;

    if (isAtomic) {
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanAtomicStore(&fieldVal, static_cast<RefFieldValue>(newVal), order);
#else
        __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
#endif
    } else {
        fieldVal = static_cast<RefFieldValue>(newVal);
#if defined(CODIRA_TSAN_SUPPORT)
        Sanitizer::TsanWriteMemory(&fieldVal, GetSize());
#endif
    }
    DLOG(BARRIER, "write field @%p 0x%zx -> 0x%zx", this, oldVal, newVal);
}
} // namespace MapleRuntime
#endif // MRT_REF_FIELD_INLINE_H
