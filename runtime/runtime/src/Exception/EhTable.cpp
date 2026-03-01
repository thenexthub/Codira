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


#include "EhTable.h"

#include "Base/LogFile.h"
#include "Exception.h"
#include "ObjectModel/MObject.inline.h"

namespace MapleRuntime {
void* EHTable::ReadAbsPtr(const uint8_t* p) const
{
#ifdef __arm__
    // For arm, get offset from p and calculate a new address.
    // The new address represents GOT, and then read the content of the address.
    int32_t offset = *reinterpret_cast<const int32_t*>(p);
    const uint8_t* newAddr = p + offset;
    return reinterpret_cast<void*>(*reinterpret_cast<const uint32_t*>(newAddr));
#else
    auto value = *reinterpret_cast<uint64_t*>(const_cast<uint8_t*>(p));
    return reinterpret_cast<void*>(value);
#endif
}

void* EHTable::ReadUData4(const uint8_t* p) const
{
    uint32_t value = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(p)));
    return reinterpret_cast<void*>(value);
}

void* EHTable::ReadIndirPcRelSData4(const uint8_t* p) const
{
    uint32_t offset = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(p)));
    if (offset == 0) {
        return nullptr;
    }
    uint64_t value = *reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(p + offset)));
    return reinterpret_cast<void*>(value);
}

void* EHTable::ReadIndirPcRelSData8(const uint8_t* p) const
{
    uint64_t offset = *reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(p)));
    uint64_t value = *reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(p + offset)));
    return reinterpret_cast<void*>(value);
}

uint64_t EHTable::ReadULEB128(const uint8_t** data)
{
    uint64_t value = 0;
    uint64_t shift = 0;
    uint8_t byte = 0;
    const uint8_t* p = *data;
    do {
        byte = *p++;
        value |= (byte & VALUE_MASK) << shift;
        shift += SHIFT_LENGTH;
    } while ((byte & BYTE_MAX_BIT) != 0);
    *data = p;
    return value;
}

// Utility function to decode a SLEB128 value.
int64_t EHTable::ReadSLEB128(const uint8_t** data)
{
    int64_t value = 0;
    uint64_t shift = 0;
    uint8_t byte = 0;
    const uint8_t* p = *data;
    do {
        byte = *p++;
        value |= (static_cast<uint64_t>(byte) & VALUE_MASK) << shift;
        shift += SHIFT_LENGTH;
    } while ((byte & BYTE_MAX_BIT) != 0);
    // Sign extend negative numbers if needed.
    constexpr uint8_t decodeBitsLen = 64;
    constexpr uint8_t signedFlag = 0x40;
    if ((shift < decodeBitsLen) && ((byte & signedFlag) != 0)) {
#if defined(__APPLE__)
        value |= LLONG_MAX << shift;
#else
        value |= ULLONG_MAX << shift;
#endif
    }
    return value;
}

void EHTable::BuildEHTable(const uint8_t* lsda)
{
    const uint8_t* curPtr = lsda;
    uint8_t lpStartEncoding = *curPtr++;
    ttypeEncoding = curPtr++;
#ifdef _WIN64
    MRT_ASSERT(*ttypeEncoding == static_cast<uint8_t>(TTypeEncoding::ABS_PTR), "Unsupported ttype enconding!");
#else
    MRT_ASSERT(*ttypeEncoding == static_cast<uint8_t>(TTypeEncoding::ABS_PTR) ||
                   *ttypeEncoding == static_cast<uint8_t>(TTypeEncoding::U_DATA_4) ||
                   *ttypeEncoding == static_cast<uint8_t>(TTypeEncoding::INDIR_PC_REL_S_DATA_8) ||
                   *ttypeEncoding == static_cast<uint8_t>(TTypeEncoding::INDIR_PC_REL_S_DATA_4),
               "Unsupported ttype enconding!");
#endif
    uint64_t classInfoOffset = EHTable::ReadULEB128(&curPtr);
    exceptionTypeStart = curPtr + classInfoOffset;

    uint8_t callSiteEncoding = *curPtr++;
    (void)lpStartEncoding;  // eliminate compilation warning
    (void)callSiteEncoding; // eliminate compilation warning
    DLOG(EXCEPTION, "creat eh table lpStartEncoding : %u ttypeEncoding : %u callSiteEncoding : %u",
         static_cast<uint32_t>(lpStartEncoding), static_cast<uint32_t>(*ttypeEncoding),
         static_cast<uint32_t>(callSiteEncoding));
    uint64_t callSiteTableLength = EHTable::ReadULEB128(&curPtr);
    callSiteTableStart = curPtr;
    callSiteTableEnd = callSiteTableStart + callSiteTableLength;
    actionTableStart = callSiteTableEnd;
}

void EHTable::ScanEHTable(const uint32_t* pc, const ExceptionWrapper& eWrapper, const uint32_t* startIp,
                          ScanResult& result) const
{
    ExceptionRef exceptionRef = eWrapper.GetExceptionRef();
    uint64_t ip = reinterpret_cast<uint64_t>(pc) - 1;
    // Get beginning current frame's code (as defined by the emitted dwarf code)
    uint64_t funcstart = reinterpret_cast<uint64_t>(startIp);
    uint64_t ipOffset = ip - funcstart;
    const uint8_t* curPtr = callSiteTableStart;
    while (curPtr < callSiteTableEnd) {
        // There is one entry per call site.
        // The call sites are non-overlapping in [start, start+length)
        // The call sites are ordered in increasing value of start
        uint64_t start = ReadULEB128(&curPtr);
        uint64_t length = ReadULEB128(&curPtr);
        uint64_t landingPad = ReadULEB128(&curPtr);
        uint64_t actionEntry = ReadULEB128(&curPtr);
        if ((start <= ipOffset) && (ipOffset < (start + length))) {
            // Found the call site containing ip.
            if (landingPad == 0) {
                continue;
            }
            landingPad = reinterpret_cast<uintptr_t>(startIp) + landingPad;

            // ActionEntry is 0 means this function cannot catch the exception.
            // The landingPad is a epilog address.
            if (actionEntry == 0) {
                result.landingPad = landingPad;
                return;
            }
            uint8_t exceptionTypeIndex = MatchActionTable(actionEntry, exceptionRef, *ttypeEncoding);
            if (exceptionTypeIndex > 0) {
                result.typeIndex = exceptionTypeIndex;
                result.landingPad = landingPad;
                result.isCaught = true;
                return;
            }
        }
    } // there might be some tricky cases which break out of this loop
}

// Scan action table to match exception handler
// Return value is greater 0 which means matching successfully,
// otherwise return 0 matching failed and need to find next call site.
uint8_t EHTable::MatchActionTable(uint64_t actionEntryIdx, const ExceptionRef& exceptionRef, uint8_t flag) const
{
    // The index of action table start from 1.
    const uint8_t* actionPtr = actionTableStart + (actionEntryIdx - 1);
    // Scan action entries until match a exception handler or reach end of action list.
    // Action list is a sequence of pairs (type index, offset of next action).
    // If offset of next action is 0, means reaching the end of action list.
    for (;;) {
        uint64_t typeIndex = *actionPtr++;
        if (typeIndex > 0) {
            // this is a catch (T) clause
            // Current implementation only supports the absptr/udata4/indirect_pcrel_sdata4/indirect_pcrel_sdata8
            // encoding for ttype.
            void* catchType = nullptr;
            switch (flag) {
                case static_cast<int>(TTypeEncoding::ABS_PTR):
                    catchType = ReadAbsPtr(exceptionTypeStart - typeIndex * sizeof(uintptr_t));
                    break;
                case static_cast<int>(TTypeEncoding::U_DATA_4):
                    catchType = ReadUData4(exceptionTypeStart - typeIndex * sizeof(int));
                    break;
                case static_cast<int>(TTypeEncoding::INDIR_PC_REL_S_DATA_4):
                    catchType = ReadIndirPcRelSData4(exceptionTypeStart - typeIndex * sizeof(uint32_t));
                    break;
                case static_cast<int>(TTypeEncoding::INDIR_PC_REL_S_DATA_8):
                    catchType = ReadIndirPcRelSData8(exceptionTypeStart - typeIndex * sizeof(uint64_t));
                    break;
                default:
                    LOG(RTLOG_FATAL, "Unsupported ttype enconding!");
                    break;
            }
            // Nullptr means matched all exception.
            if (catchType == nullptr || exceptionRef->IsSubType(*(reinterpret_cast<TypeInfo*>(catchType)))) {
                // Found a matching handler and return exception type index
                return static_cast<uint8_t>(typeIndex);
            }
        }

        if (*actionPtr == 0) {
            // Reach the end of action list, no further action handlers or cleanup found,
            // which means the thrown exception can not be caught by this catch clause.
            // We will continue to search next call site table until we reach
            // the last call site entry which is the cleanup entry.
            return 0;
        } else {
            // Currently, the action offset is encoded with SLEB128. Need to decode it first and
            // change the action pointer accordingly.
            int64_t offset = EHTable::ReadSLEB128(&actionPtr);
            actionPtr += offset;
        }
    }
}
} // namespace MapleRuntime
