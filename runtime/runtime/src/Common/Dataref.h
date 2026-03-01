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


#ifndef MRT_DATAREF_H
#define MRT_DATAREF_H

#include <cstdlib>

#include "Base/Log.h"
#include "Base/Types.h"
namespace MapleRuntime {
// DataRefOffset aims to represent a reference to data in maple file, which is already an offset.
// DataRefOffset is meant to have pointer size.
// All Xx32 data types defined in this file aim to use 32 bits to save 64-bit address, and thus are
// specific for 64-bit platforms.
template<typename T>
struct DataRefOffset32 {
    I32 refOffset;

    inline T* GetDataRef() const
    {
        T* dataRef = nullptr;
        if (refOffset != 0) {
            Sptr ref = static_cast<Sptr>(refOffset);
            ref += reinterpret_cast<Sptr>(this);
            dataRef = reinterpret_cast<T*>(static_cast<Uptr>(ref));
        }
        return dataRef;
    }
};

template<typename T>
struct DataRefOffset64 {
    I64 refOffset;

    inline T* GetDataRef() const
    {
        T* dataRef = nullptr;
        if (refOffset != 0) {
            Sptr ref = static_cast<Sptr>(refOffset);
            ref += reinterpret_cast<Sptr>(this);
            dataRef = reinterpret_cast<T*>(static_cast<Uptr>(ref));
        }
        return dataRef;
    }
};
} // namespace MapleRuntime
#endif // MRT_DATAREF_H
