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


#include "UnwindWin.h"

#include "Base/Log.h"
#include "Common/StackType.h"
#include "Mutator/Mutator.h"

extern uintptr_t CODE_MCC_C2NStub;
extern uintptr_t CODE_MCC_N2CStub;
extern uintptr_t CODE_MCC_ThrowStackOverflowError;

extern uintptr_t unwindPCForN2CStub;
extern uintptr_t unwindPCForC2NStub;
extern uintptr_t unwindPCForTPE;
extern uintptr_t unwindPCForStackGrowStub;

namespace MapleRuntime {
static uint32_t GetSlotsNum(const UnwindCode& unwindCode)
{
    switch (unwindCode.GetUnwindOp()) {
        case PUSH_NON_VOL:
        case ALLOC_SMALL:
        case SET_FP_REG:
        case PUSH_MACH_FRAME:
            return 1;
        case SAVE_NON_VOL:
        case SAVE_XMM128:
            return 2; // 2: unwindCode sizes
        case SAVE_NON_VOL_FAR:
        case SAVE_XMM128_FAR:
            return 3; // 3: unwindCode sizes
        case ALLOC_LARGE:
            return (unwindCode.GetOpInfo() == 0) ? 2 : 3; // 2: 3: unwindCode sizes
        default:
            LOG(RTLOG_FATAL, "UnwindCode is invalid");
            return -1;
    }
}

uint32_t GetStackOffset(UnwindInfo& unwindInfo)
{
    uint32_t pushRegOffset = 0;
    uint32_t allocSize = 0;
    constexpr uint32_t slotSize = 8;

    for (int i = 0; i < unwindInfo.codesCount; i += GetSlotsNum(unwindInfo.unwindCodes[i])) {
        switch (unwindInfo.unwindCodes[i].GetUnwindOp()) {
            case PUSH_NON_VOL:
                pushRegOffset += slotSize;
                break;
            case ALLOC_SMALL:
                allocSize = (unwindInfo.unwindCodes[i].GetOpInfo() + 1) * slotSize;
                break;
            case ALLOC_LARGE:
                if (unwindInfo.unwindCodes[i].GetOpInfo() == 0) {
                    allocSize = unwindInfo.unwindCodes[i + 1].frameOffset * slotSize;
                } else {
                    allocSize = unwindInfo.unwindCodes[i + 1].frameOffset;
                    allocSize +=
                        (static_cast<uint32_t>(unwindInfo.unwindCodes[i + 2].frameOffset) << 16); // 2: idx, 16: high 16
                }
                break;
            default:
                break;
        }
    }
    return allocSize + pushRegOffset;
}

FrameInfo GetCurFrameInfo(WinModuleManager& winModuleManager, Uptr pc, Uptr sp)
{
    WinModule* module = winModuleManager.GetWinModuleByPc(pc);
    if (module == nullptr) {
        winModuleManager.ReadWinModuleAtRunning();
        module = winModuleManager.GetWinModuleByPc(pc);
    }
    CHECK(module != nullptr);
    RuntimeFunction* runtimeFunc = module->GetRuntimeFunction(pc);
    CHECK(runtimeFunc != nullptr);
    UnwindInfo* unwindInfo = reinterpret_cast<UnwindInfo*>(module->GetImageBaseStart() + runtimeFunc->unwindInfoOffset);

    Uptr startProc = module->GetImageBaseStart() + runtimeFunc->startAddress;
    // calculate stackOffset = calleesaved size + allocstack size
    uint32_t stackOffset = GetStackOffset(*unwindInfo);

    FrameInfo frameInfo(reinterpret_cast<uint32_t*>(startProc));
    frameInfo.mFrame.SetIP(reinterpret_cast<uint32_t*>(pc));

    if (frameInfo.mFrame.IsRuntimeFrame()) {
        // 8: the first slot is pushed rbp
        frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(sp + stackOffset - 8));
    } else {
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(startProc));
        Uptr* stackMapEntry = funcDesc->GetStackMap();
        uint32_t validPos = 0;
        stackOffset = EHFrameInfo::ReadVarInt(&stackMapEntry, validPos);
        Uptr* calleeFA = reinterpret_cast<Uptr*>(sp - 0x10);
        Uptr winRbp = reinterpret_cast<Uptr>(*calleeFA);
        frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(winRbp + stackOffset));
    }

    return frameInfo;
}

// Import UnwindContextStatus of Unwinding, it is different from status in the uwContext.
// unContext.status : Current state during operation, to determine the initial state when the stack unwind.
// stackInfo.status(of Unwinding) : status of a frame when unwinding, to determine whether the stack is a runtime stack.
FrameInfo GetCallerFrameInfo(WinModuleManager& winModuleManager, const MachineFrame& curFrame,
                             UnwindContextStatus& status)
{
    static bool isCalleeThrowStackOverFlowError = false;
    Uptr callerPC = reinterpret_cast<Uptr>(curFrame.GetFA()->returnAddress);

    WinModule* module = winModuleManager.GetWinModuleByPc(callerPC);
    if (module == nullptr) {
        winModuleManager.ReadWinModuleAtRunning();
        module = winModuleManager.GetWinModuleByPc(callerPC);
    }
    CHECK(module != nullptr);
    RuntimeFunction* runtimeFunc = module->GetRuntimeFunction(callerPC);
    CHECK(runtimeFunc != nullptr);

    Uptr startProc = module->GetImageBaseStart() + runtimeFunc->startAddress;
    FrameInfo frameInfo(reinterpret_cast<uint32_t*>(startProc));
    frameInfo.mFrame.SetIP(reinterpret_cast<uint32_t*>(callerPC));

    // if startProc is XXXStub, caller fa can load from callee fa address
    // elseif callee is ThrowStackOverFlowError, caller fa is callee fa + 16 (return addr + pushed rbp)
    // otherwise caller fa only can be calculated by callerSP + stackOffset - 8
    if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_C2NStub) ||
        startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_N2CStub) ||
        startProc == reinterpret_cast<uintptr_t>(&ExecuteCodiraStub) ||
        startProc == reinterpret_cast<uintptr_t>(&ApplyCodiraMethodStub)) {
        if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_C2NStub)) {
            status = UnwindContextStatus::RELIABLE;
        }
        if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_N2CStub)) {
            status = UnwindContextStatus::RISKY;
        }
        frameInfo.mFrame.SetFA(curFrame.GetFA()->callerFrameAddress);
    } else if (isCalleeThrowStackOverFlowError) {
        frameInfo.mFrame.SetFA(
            reinterpret_cast<FrameAddress*>(reinterpret_cast<Uptr>(curFrame.GetFA()) + 16)); // 16: return addr + rbp
        isCalleeThrowStackOverFlowError = false;
    } else if (reinterpret_cast<uintptr_t>(curFrame.GetIP()) ==
               reinterpret_cast<uintptr_t>(&unwindPCForStackGrowStub)) {
        Mutator* mutator = Mutator::GetMutator();
        if (mutator == nullptr) {
            LOG(RTLOG_FATAL, "Mutator is invalid");
        }
        frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(
            reinterpret_cast<Uptr>(curFrame.GetFA()) + mutator->GetStackGrowFrameSize()));
    } else {
        if (status != UnwindContextStatus::RISKY && !(frameInfo.mFrame.IsRuntimeFrame())) {
            FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(startProc));
            Uptr* stackMapEntry = funcDesc->GetStackMap();
            uint32_t validPos = 0;
            uint32_t stackOffset = EHFrameInfo::ReadVarInt(&stackMapEntry, validPos);
            Uptr callerRbp = reinterpret_cast<Uptr>(curFrame.GetFA()->callerFrameAddress);
            frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(callerRbp + stackOffset));
        } else {
            UnwindInfo* unwindInfo =
                reinterpret_cast<UnwindInfo*>(module->GetImageBaseStart() + runtimeFunc->unwindInfoOffset);
            // calculate stackOffset = calleesaved size + allocstack size
            uint32_t stackOffset = GetStackOffset(*unwindInfo);

            // |  callee-addr      |-> XXXStub need
            // |  param-stack-size |-> XXXStub need
            // |  return addr      |
            // |  pushed rbp       |
            uint32_t spOffset = 16; // 16: 8 for pushed rbp, 8 for pushed pc
            if (reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForN2CStub) ||
                reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForC2NStub) ||
                reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForTPE)) {
                spOffset += 16; // 16: 8 for param-stack-size, 8 for callee-addrnb
            }
            Uptr callerSP = reinterpret_cast<Uptr>(curFrame.GetFA()) + spOffset;
            frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(callerSP + stackOffset - 8)); // 8: offset of rbp
        }
    }
    frameInfo.mFrame.SetIP(reinterpret_cast<uint32_t*>(callerPC));

    if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_ThrowStackOverflowError)) {
        isCalleeThrowStackOverFlowError = true;
    }
    return frameInfo;
}

uintptr_t GetCallerRsp(WinModuleManager& winModuleManager, const MachineFrame& curFrame)
{
    static bool isCalleeThrowStackOverFlowError = false;
    constexpr uint8_t sizeOfStackHead = 8 * 2; // callee rbp + return addr
    Uptr callerPC = reinterpret_cast<Uptr>(curFrame.GetFA()->returnAddress);

    WinModule* module = winModuleManager.GetWinModuleByPc(callerPC);
    if (module == nullptr) {
        winModuleManager.ReadWinModuleAtRunning();
        module = winModuleManager.GetWinModuleByPc(callerPC);
    }
    CHECK(module != nullptr);
    RuntimeFunction* runtimeFunc = module->GetRuntimeFunction(callerPC);
    CHECK(runtimeFunc != nullptr);

    Uptr startProc = module->GetImageBaseStart() + runtimeFunc->startAddress;
    uintptr_t callerRsp;

    // if startProc is XXXStub, caller fa can load from callee fa address
    // elseif callee is ThrowStackOverFlowError, caller fa is callee fa + 16 (return addr + pushed rbp)
    // otherwise caller fa only can be calculated by callerSP + stackOffset - 8
    if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_C2NStub) ||
        startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_N2CStub) ||
        startProc == reinterpret_cast<uintptr_t>(&ExecuteCodiraStub) ||
        startProc == reinterpret_cast<uintptr_t>(&ApplyCodiraMethodStub) ||
        reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForStackGrowStub)) {
        callerRsp = reinterpret_cast<uintptr_t>(curFrame.GetFA()) + sizeOfStackHead;
    } else if (isCalleeThrowStackOverFlowError) {
        callerRsp = reinterpret_cast<uintptr_t>(curFrame.GetFA()) + sizeOfStackHead;
        isCalleeThrowStackOverFlowError = false;
    } else {
        uint32_t spOffset = sizeOfStackHead;
        if (reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForN2CStub) ||
            reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForC2NStub) ||
            reinterpret_cast<uintptr_t>(curFrame.GetIP()) == reinterpret_cast<uintptr_t>(&unwindPCForTPE)) {
            spOffset += sizeOfStackHead;
        }
        Uptr callerSP = reinterpret_cast<Uptr>(curFrame.GetFA()) + spOffset;
        callerRsp = callerSP;
    }

    if (startProc == reinterpret_cast<uintptr_t>(&CODE_MCC_ThrowStackOverflowError)) {
        isCalleeThrowStackOverFlowError = true;
    }
    return callerRsp;
}
} // namespace MapleRuntime
