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


#include <cmath>
#include "MFuncdesc.h"
#include "Utils/SLRUCache.h"

namespace MapleRuntime {
constexpr uint8_t probationalSegmentSize = 8;
constexpr uint8_t protectedSegmentSize = 8;
static SLRU g_stringPoolDictsCache(probationalSegmentSize, protectedSegmentSize);

// StrPoolDictSplitSize is defined through elapsed time test on dict loading
// and string decoding. The maximum size of 5000 can keep the elapsed time below
// 1 millisecond, which is a desired cost. When the string pool dict size
// is between 1000 and 5000, the split size will be correspondingly calculated
// by function GetStrPoolDictSplitSize().
static constexpr U32 StrPoolDictSplitMaxSize = 5000;
static constexpr U32 StrPoolDictSplitMinSize = 1000;

U32 GetStrPoolDictSplitSize(U32 TotalSize)
{
    // 10: supposed that TotalSize = SplitSize * n and SplitSize = 10 * n.
    U32 DesiredSize = static_cast<U32>(std::sqrt(TotalSize / 10) * 10);
    if (DesiredSize <= StrPoolDictSplitMinSize) {
        return StrPoolDictSplitMinSize;
    }
    if (DesiredSize >= StrPoolDictSplitMaxSize) {
        return StrPoolDictSplitMaxSize;
    }
    // 100: round the result to be a multiple of one hundred.
    constexpr U32 alignSize = 100;
    return DesiredSize / alignSize * alignSize;
}

// ULEB: The highest bit of each byte is used to indicate whether the encoding is complete,
// and the remaining 7 bits are used to represent the integer value
uint64_t ULEBDecodeSingleStr(std::vector<uint8_t>& bytes)
{
    uint64_t result = 0;
    uint32_t shift = 0;
    constexpr uint8_t IntValueBits = 7;
    for (const auto& byte : bytes) {
        result |= static_cast<uint64_t>(byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += IntValueBits;
    }
    bytes.erase(bytes.begin(), bytes.begin() + (shift / IntValueBits) + 1);
    return result;
}

CString ULEBDecode(CString& bytes, Uptr offset)
{
    // The offsets of subdictionaries are structured in assembly as follows:
    // .Lstr_pool_dict_offsets:
    //      .long   dictTotalSize
    //      .long   .Lstr_pool_dict.1-.Lstr_pool_dict_offsets
    //      .long   .Lstr_pool_dict.2-.Lstr_pool_dict_offsets
    //      ...
    U32 dictTotalSize = *(reinterpret_cast<const U32*>(offset));
    U32 strPoolDictSplitSize = GetStrPoolDictSplitSize(dictTotalSize);
    CString result;
    int p = 0;
    std::vector<uint8_t> tempCode;
    while (p < bytes.Length()) {
        tempCode.push_back(bytes[p]);
        if ((bytes[p] & 0x80) == 0) {
            uint64_t decode = ULEBDecodeSingleStr(tempCode);
            if (decode > dictTotalSize) {
                LOG(RTLOG_ERROR, "invalid index in stringpool");
                return result;
            }
            U32 subDictIdx = (decode - 1) / strPoolDictSplitSize + 1;
            // 4: offset value size, 4 bytes
            U32 subDictOffset = *(reinterpret_cast<const U32*>(offset + 4 * subDictIdx));
            // use a SLRU cache to save string loading time.
            std::vector<CString> subDictionary = g_stringPoolDictsCache.Get(offset + subDictOffset);
            if (subDictionary.empty()) {
                CString subDictStr = reinterpret_cast<const char*>(offset + subDictOffset);
                subDictionary = CString::Split(subDictStr, ',');
                g_stringPoolDictsCache.Put(offset + subDictOffset, subDictionary);
            }
            CString singleStr = subDictionary[(decode - 1) % strPoolDictSplitSize];
            result += singleStr;
            tempCode.clear();
        }
        p++;
    }
    return result;
}

CString MFuncDesc::GetStringFromDict(U32 offset) const
{
    Uptr base = reinterpret_cast<Uptr>(this);
    CString str = reinterpret_cast<const char*>(offset + base);
    if (str.IsEmpty()) {
        return CString();
    }

    return ULEBDecode(str, dictOffsets + base);
}
}
