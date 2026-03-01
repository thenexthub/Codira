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


#ifndef MRT_BARRIER_INLINE_H
#define MRT_BARRIER_INLINE_H

#include "Barrier.h"
#include "securec.h"

namespace MapleRuntime {
template<>
inline void Barrier::WriteField<int8_t>(BaseObject* obj, Field<int8_t>& field, int8_t val) const
{
    WriteI8(obj, field, val);
}

template<>
inline void Barrier::WriteField<int16_t>(BaseObject* obj, Field<int16_t>& field, int16_t val) const
{
    WriteI16(obj, field, val);
}

template<>
inline void Barrier::WriteField<int32_t>(BaseObject* obj, Field<int32_t>& field, int32_t val) const
{
    WriteI32(obj, field, val);
}

template<>
inline void Barrier::WriteField<int64_t>(BaseObject* obj, Field<int64_t>& field, int64_t val) const
{
    WriteI64(obj, field, val);
}

template<>
inline void Barrier::WriteField<float>(BaseObject* obj, Field<float>& field, float val) const
{
    WriteF32(obj, field, val);
}

template<>
inline void Barrier::WriteField<double>(BaseObject* obj, Field<double>& field, double val) const
{
    WriteF64(obj, field, val);
}
} // namespace MapleRuntime
#endif // ~MRT_BARRIER_INLINE_H
