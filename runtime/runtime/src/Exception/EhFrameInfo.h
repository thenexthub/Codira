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


#ifndef MRT_EH_FRAMEINFO_H
#define MRT_EH_FRAMEINFO_H

#include "Base/MemUtils.h"
#include "Base/Types.h"
#include "CalleeSavedRegisterContext.h"
#include "Common/TypeDef.h"
#include "EhTable.h"
#include "ObjectModel/MFuncdesc.inline.h"
#include "StackMap/StackMapTable.h"

#if defined(CODIRA_SANITIZER_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

#if defined(_WIN64)
#include "os/Windows/UnwindWin.h"
#endif
namespace MapleRuntime {

class EHFrameInfo : public FrameInfo {
public:
    EHFrameInfo(const FrameInfo& info, const ExceptionWrapper& eWrapper) : FrameInfo(info)
    {
        // Abnormal EHTable layout:
        // lsdaStart:    0x55555555
        if (!EHTable::IsAbnormalEHTable(lsdaStart)) {
            auto pc = mFrame.GetIP();
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
            pc = reinterpret_cast<uint32_t*>(PtrauthAuthWithInstAkey(reinterpret_cast<Uptr>(pc),
                reinterpret_cast<Uptr>(mFrame.GetPtrAuthRAMod())));
#endif
            EHTable ehTable(pc, eWrapper, startProc, lsdaStart, result);
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
            result.landingPad = reinterpret_cast<Uptr>(
                                    PtrauthSignWithInstAkey(reinterpret_cast<Uptr>(result.landingPad),
                                    reinterpret_cast<Uptr>(mFrame.GetPtrAuthRAMod())));
#endif
        }
    }

    EHFrameInfo() = delete;
    ~EHFrameInfo() override = default;

    bool IsCatchException() const { return result.isCaught; }

    uint64_t GetTTypeIndex() const { return result.typeIndex; }

    uint64_t GetLandingPad() const { return result.landingPad; }

    void RestoreToCallerContext(CalleeSavedRegisterContext& context, uint32_t adjustedSize = 0) const
    {
#if defined(__arm__)
        constexpr uint8_t sizeOfAddr = 4;
#else
        constexpr uint8_t sizeOfAddr = 8;
#endif
        constexpr uint8_t sizeOfStackHead = sizeOfAddr * 2; // callee rbp + return addr
#if defined(__x86_64__)
        context.rsp = context.rbp + sizeOfStackHead;
#elif defined(__aarch64__)
        context.sp = context.x29 + sizeOfStackHead;
        uint32_t calleeCount = 0;
#elif defined(__arm__)
        context.sp = context.r11 + sizeOfStackHead;
#endif
        Uptr calleeFrameAddress = reinterpret_cast<Uptr>(mFrame.GetFA());
        if (adjustedSize > 0) {
#if defined(__x86_64__)
            context.rbp = *reinterpret_cast<uint64_t*>(calleeFrameAddress);
#elif defined(__aarch64__)
            context.x29 = *reinterpret_cast<uint64_t*>(calleeFrameAddress);
#elif defined(__arm__)
            context.r11 = *reinterpret_cast<uint32_t*>(calleeFrameAddress);
#endif
            return;
        }
#ifdef __APPLE__
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(mFrame.GetFA());
#else
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(startProc));
#endif
        Uptr* stackMapEntry = funcDesc->GetStackMap();
        uint32_t validPos = 0;

        // Skip the stackmap header.
        (void)ReadVarInt(&stackMapEntry, validPos);
        (void)ReadVarInt(&stackMapEntry, validPos);

        uint32_t calleeSavedBitmap = ReadVarInt(&stackMapEntry, validPos);
        for (uint32_t idx = 0; idx < CALLEE_SAVE_NUMBERS; idx++) {
            if ((calleeSavedBitmap & (static_cast<uint32_t>(1) << idx)) == 0) {
                continue;
            }
            uint32_t offset = ReadVarInt(&stackMapEntry, validPos);
#if defined(_WIN64)
            if (idx < calleeSaveXMMIdxStart) {
                uint64_t* slotAddr = reinterpret_cast<uint64_t*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
                context.SetValueByIdx(idx, *slotAddr);
            } else {
                XMMReg* slotAddr = reinterpret_cast<XMMReg*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
                context.SetXMMValueByIdx(idx - calleeSaveXMMIdxStart, slotAddr);
            }
#elif defined(__aarch64__)
#if defined(__APPLE__)
            if (offset == 0 || offset == 1) {
                uint64_t* slotAddr = reinterpret_cast<uint64_t*>(calleeFrameAddress - SLOT_SIZE_FACTOR * offset);
                context.SetValueByIdx(idx, *slotAddr);
            } else {
                uint64_t* slotAddr = reinterpret_cast<uint64_t*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
                context.SetValueByIdx(idx, *slotAddr);
            }
#else // __APPLE__
            if (offset != 0 && offset != 1) {
                ++calleeCount;
            }
            uint64_t* slotAddr = reinterpret_cast<uint64_t*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
            context.SetValueByIdx(idx, *slotAddr);
#endif // __APPLE__
#elif defined(__arm__)
            uint32_t* slotAddr = reinterpret_cast<uint32_t*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
            context.SetValueByIdx(idx, *slotAddr);
#else // not (_WIN64 || __aarch64__)
            uint64_t* slotAddr = reinterpret_cast<uint64_t*>(calleeFrameAddress + SLOT_SIZE_FACTOR * offset);
            context.SetValueByIdx(idx, *slotAddr);
#endif
        }
#if defined(__x86_64__)
        context.rbp = *reinterpret_cast<uint64_t*>(calleeFrameAddress);
#elif defined(__aarch64__)
        context.x29 = *reinterpret_cast<uint64_t*>(calleeFrameAddress);
        constexpr uint8_t alignSize = 2;
        calleeCount = (calleeCount % alignSize != 0) ? calleeCount + 1 : calleeCount;
        context.sp += calleeCount * sizeOfAddr;
#elif defined(__arm__)
        context.r11 = *reinterpret_cast<uint32_t*>(calleeFrameAddress);
#endif

#if defined(_WIN64)
        Runtime& runtime = Runtime::Current();
        WinModuleManager& winModuleManager = runtime.GetWinModuleManager();
        context.rsp = GetCallerRsp(winModuleManager, mFrame);
#endif

#if defined(CODIRA_TSAN_SUPPORT)
        // update tsan's function trace
        Sanitizer::TsanFuncRestoreContext(reinterpret_cast<const void*>(mFrame.GetIP()));
#endif
    }

    /* Var Int format:
        If the value of reading 4bit is less than 12, the value will be returned directly.
        If the value of reading 4bit is equal to 12, then the next 8bit needs to be read out and returned.
        If the value of reading 4bit is equal to 13, then the next 16bit needs to be read out and returned.
        If the value of reading 4bit is equal to 14, then the next 24bit needs to be read out and returned.
        If the value of reading 4bit is equal to 15, then the next 32bit needs to be read out and returned.
        When validPos is equal to 0, read from the lower four bits, otherwise read from the upper four bits.
    */
    static uint32_t ReadVarInt(Uptr** point, uint32_t& validPos)
    {
        uint32_t res = 0;
        uint32_t bitsMask = 0xf;
        constexpr uint32_t tagLen = 4;
        res = (*reinterpret_cast<uint8_t*>(*point)) >> (tagLen * validPos) & bitsMask;
        *point = (validPos == 1) ? reinterpret_cast<Uptr*>((reinterpret_cast<uint8_t*>(*point)) + 1) : *point;
        validPos = (validPos == 1) ? 0 : 1;
        if (res <= VarInt::TagType::MAX_VALID_VALUE) {
            return res;
        }

        uint32_t bitsLen = 0;
        if (res == VarInt::TagType::VAR_VALUE8) {
            bitsLen = VarInt::BitsLen::FIRST_STEP_VAR_BITS;
        } else if (res == VarInt::TagType::VAR_VALUE16) {
            bitsLen = VarInt::BitsLen::SECOND_STEP_VAR_BITS;
        } else if (res == VarInt::TagType::VAR_VALUE24) {
            bitsLen = VarInt::BitsLen::THIRD_STEP_VAR_BITS;
        } else {
            bitsLen = VarInt::BitsLen::FORTH_STEP_VAR_BITS;
        }
        bitsMask = static_cast<uint32_t>((1UL << bitsLen) - 1);
#ifdef __arm__
        res = *reinterpret_cast<uint32_t*>(*point) >> (tagLen * validPos) & bitsMask;
#else
        res = *reinterpret_cast<uint64_t*>(*point) >> (tagLen * validPos) & bitsMask;
#endif
        uint32_t byteLen = bitsLen >> 3; // 8 bits per byte
        *point = reinterpret_cast<Uptr*>(reinterpret_cast<uint8_t*>(*point) + byteLen);
        return res;
    }

private:
    ScanResult result;
#if defined(__linux__) && defined(__x86_64__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 5;
    static constexpr int64_t SLOT_SIZE_FACTOR = -8;
#elif defined(__APPLE__) && defined(__x86_64__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 5;
    static constexpr int64_t SLOT_SIZE_FACTOR = -8;
#elif defined(__APPLE__) && defined(__aarch64__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 20;
    static constexpr int64_t SLOT_SIZE_FACTOR = -8;
#elif defined(__aarch64__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 20;
    static constexpr int64_t SLOT_SIZE_FACTOR = 8;
#elif defined(__arm__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 17;
    static constexpr int64_t SLOT_SIZE_FACTOR = -4;
#elif defined(__linux__) && defined(__aarch64__)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 20;
    static constexpr int64_t SLOT_SIZE_FACTOR = 8;
#elif defined(_WIN64)
    static constexpr size_t CALLEE_SAVE_NUMBERS = 17;
    static constexpr size_t calleeSaveXMMIdxStart = 7;
    static constexpr int64_t SLOT_SIZE_FACTOR = -8;
#endif
};
} // namespace MapleRuntime
#endif // MRT_EH_FRAMEINFO_H
