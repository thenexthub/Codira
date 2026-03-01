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


#ifndef MRT_STACKSIZE_VARINT_H
#define MRT_STACKSIZE_VARINT_H

#include "StackMap/StackMapTable.h"

namespace MapleRuntime {
class StacksizeVarInt {
public:
    StacksizeVarInt(U8* ptr, U32 bitPos) : value(ptr, bitPos) { ResolveVarInt(); }
    explicit StacksizeVarInt(const BitsManager& bitsManager) : value(bitsManager) { ResolveVarInt(); }
    ~StacksizeVarInt() = default;

    BitsManager GetNextTable() const { return nextTable; }

    U32 GetStacksize() const { return stacksize; }

private:
    void ResolveVarInt()
    {
        VarInt sizeBits(value);
        VarPair varPair = sizeBits.GetValue();
        stacksize = varPair.first;
        nextTable = value.GetNext(varPair.second);
    }
    BitsManager value;
    BitsManager nextTable;
    U32 stacksize{ 0 };
};
} // namespace MapleRuntime

#endif // ~MRT_STACKSIZE_VARINT_H
