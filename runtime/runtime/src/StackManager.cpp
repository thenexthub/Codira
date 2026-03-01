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


#include "StackManager.h"
#include <cstdint>

#include "Base/SysCall.h"
#include "Common/StackType.h"
#include "ExceptionManager.inline.h"
#include "Mutator/Mutator.h"
#include "ObjectManager.inline.h"
#include "UnwindStack/EhStackInfo.h"
#include "UnwindStack/GcStackInfo.h"
#include "UnwindStack/PrintSignalStackInfo.h"
#include "UnwindStack/PrintStackInfo.h"
#include "UnwindStack/StackGrowStackInfo.h"
#ifdef _WIN64
#include "UnwindWin.h"
#endif
#ifdef __APPLE__
#include <dlfcn.h>
#ifndef __IOS__
#include <libproc.h>
#endif
#endif
#include "Inspector/CodeHeapData.h"
#include "Heap/Allocator/AllocBuffer.h"
#ifdef CODIRA_SANITIZER_SUPPORT
#include "Sanitizer/SanitizerInterface.h"
#include "StackMap/StackMap.h"
#endif
#include "CpuProfiler/CpuProfiler.h"

#define LIBCODIRA_RUNTIME "libcangjie-runtime"
#define LIBCODIRA_STD_CORE "libcangjie-std-core"
#define LIBCODIRA_CODETHREAD_TRACE "libcangjie-trace"

namespace MapleRuntime {
#ifdef __arm__
Uptr STACK_ADDR_MAX = UINT32_MAX;
#else
Uptr STACK_ADDR_MAX = ULLONG_MAX;
#endif
Uptr StackManager::rtStartAddr = STACK_ADDR_MAX;
Uptr StackManager::rtEndAddr = 0;
Uptr StackManager::codecSoStartAddr = STACK_ADDR_MAX;
Uptr StackManager::codecSoEndAddr = 0;
Uptr StackManager::traceSoStartAddr = STACK_ADDR_MAX;
Uptr StackManager::traceSoEndAddr = 0;

#if defined (COMPILE_DYNAMIC)
#if !defined (__APPLE__)
extern "C" Uptr* g_runtimeDynamicStart;
extern "C" Uptr* g_runtimeDynamicEnd;
#endif
#else
extern "C" Uptr* g_runtimeStaticStart;
extern "C" Uptr* g_runtimeStaticEnd;
#endif

StackManager::StackManager() {}

void StackManager::Init()
{
    InitAddressScope();
    InitStackGrowConfig();
}

void StackManager::Fini() const {}

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
void StackManager::PrintStackTrace(UnwindContext* uwContext)
{
    PrintStackInfo printStackInfo(uwContext);
    printStackInfo.FillInStackTrace();
    printStackInfo.PrintStackTrace();
}
#endif

void StackManager::PrintSignalStackTrace(UnwindContext* uwContext, uintptr_t pc, uintptr_t fa)
{
    PrintSignalStackInfo printSignalStackInfo(uwContext);
    if (uwContext->GetUnwindContextStatus() == UnwindContextStatus::RISKY) {
        printSignalStackInfo.GetSignalStack()[printSignalStackInfo.GetStackIndex()] = SigHandlerFrameinfo(
            MachineFrame(reinterpret_cast<FrameAddress*>(fa), reinterpret_cast<uint32_t*>(pc)), FrameType::NATIVE);
        printSignalStackInfo.SetStackIndex(printSignalStackInfo.GetStackIndex() + 1);
    }
    printSignalStackInfo.FillInStackTrace();
    printSignalStackInfo.PrintStackTrace();
}

void StackManager::PrintStackTraceForCpuProfile(UnwindContext* unContext, unsigned long long int codeThreadId)
{
    PrintStackInfo printStackInfo(unContext);
    printStackInfo.FillInStackTrace();
    auto stacks = printStackInfo.GetStack();
    std::vector<uint64_t> funcDescRefs;
    std::vector<FrameType> frameTypes;
    std::vector<uint32_t> lineNumbers;
    for (auto& frame : stacks) {
#ifdef __APPLE__
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(frame.mFrame.GetFA());
#else
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(frame.GetFuncStartPC()));
#endif
        StackMapBuilder stackMapBuild(reinterpret_cast<uintptr_t>(frame.GetFuncStartPC()),
            reinterpret_cast<uintptr_t>(frame.mFrame.GetIP()), 0, reinterpret_cast<uint64_t*>(funcDesc));
        MethodMap methodMap = stackMapBuild.Build<MethodMap>();
        uint32_t lineNum = methodMap.IsValid() ? methodMap.GetLineNum() : 0;
        funcDescRefs.emplace_back(reinterpret_cast<uint64_t>(funcDesc));
        frameTypes.emplace_back(frame.GetFrameType());
        lineNumbers.emplace_back(lineNum);
    }
    CpuProfiler::GetInstance().GetGenerator().Post(codeThreadId, funcDescRefs, frameTypes, lineNumbers);
}

void StackManager::RecordLiteFrameInfos(std::vector<uint64_t>& liteFrameInfos, size_t steps)
{
    PrintStackInfo printStackInfo;
    printStackInfo.FillInStackTrace();
    printStackInfo.ExtractLiteFrameInfoFromStack(liteFrameInfos, steps);
}

void StackManager::GetStackTraceByLiteFrameInfos(const std::vector<uint64_t>& liteFrameInfos,
                                                 std::vector<StackTraceElement>& stackTrace)
{
    StackInfo::GetStackTraceByLiteFrameInfos(liteFrameInfos, stackTrace);
}

void StackManager::GetStackTraceByLiteFrameInfo(const uint64_t ip, const uint64_t pc, const uint64_t funcDesc,
                                                StackTraceElement& ste)
{
    StackInfo::GetStackTraceByLiteFrameInfo(ip, pc, funcDesc, ste);
}

void StackManager::VisitStackRoots(const UnwindContext& topFrame, const RootVisitor& func, Mutator& mutator)
{
    GCStackInfo gcStackInfo(&topFrame);
    gcStackInfo.FillInStackTrace();
    gcStackInfo.VisitStackRoots(func, mutator);
}

void StackManager::VisitHeapReferencesOnStack(const UnwindContext& topFrame, const RootVisitor& rootVisitor,
                                              const DerivedPtrVisitor& derivedPtrVisitor, Mutator& mutator)
{
    GCStackInfo gcStackInfo(&topFrame);
    gcStackInfo.FillInStackTrace();
    gcStackInfo.VisitHeapReferencesOnStack(rootVisitor, derivedPtrVisitor, mutator);
}

void StackManager::VisitStackPtrMap(const UnwindContext& topFrame, const StackPtrVisitor& traceAndFixPtrVisitor,
                                    const StackPtrVisitor& fixPtrVisitor, const DerivedPtrVisitor& derivedPtrVisitor,
                                    Mutator& mutator)
{
    // Reuse gcStackInfo for stack unwind.
    StackGrowStackInfo stackInfo(&topFrame);
    stackInfo.FillInStackTrace();
    stackInfo.RecordStackPtrs(traceAndFixPtrVisitor, fixPtrVisitor, derivedPtrVisitor, mutator);
}

void StackManager::InitStackGrowConfig()
{
    auto codeStackGrow = std::getenv("codeStackGrow");
    if (codeStackGrow == nullptr) {
        return;
    }
    if (strlen(codeStackGrow) != 1) {
        LOG(RTLOG_ERROR, "unsupported codeStackGrow, codeStackGrow should be 0 or 1.\n");
        return;
    }

    switch (codeStackGrow[0]) {
        case '0':
            CodiraRuntime::stackGrowConfig = StackGrowConfig::STACK_GROW_OFF;
            return;
        case '1':
            CodiraRuntime::stackGrowConfig = StackGrowConfig::STACK_GROW_ON;
            return;
        default:
            LOG(RTLOG_ERROR, "unsupported codeStackGrow, codeStackGrow should be 0 or 1.\n");
    }
    return;
}

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
std::vector<FrameInfo> GetCurrentStack(StackMode mode)
{
    switch (mode) {
        case StackMode::EH: {
            EHStackInfo ehStackInfo;
            ehStackInfo.FillInStackTrace();
            return ehStackInfo.GetStack();
        }
        case StackMode::GC: {
            GCStackInfo gcStackInfo;
            gcStackInfo.FillInStackTrace();
            return gcStackInfo.GetStack();
        }
        case StackMode::PRINT: {
            PrintStackInfo printStackInfo;
            printStackInfo.FillInStackTrace();
            return printStackInfo.GetStack();
        }
        default:
            LOG(RTLOG_FATAL, "StackMode is invalid");
    }
}
#endif

#if defined(__linux__) || defined(hongmeng) || defined(__arm__)
static void GetSoAddrScope(const CString& str, Uptr& startAddr, Uptr& endAddr)
{
    int pos1 = str.Find('-');
    int pos2 = str.Find(' ');
    if (pos1 < 0 || pos2 < pos1) {
        return;
    }
    constexpr int8_t baseValue = 16;
    Uptr start = std::strtoull(str.SubStr(0, static_cast<uint64_t>(pos1)).Str(), nullptr, baseValue);
    startAddr = start < startAddr ? start : startAddr;
    Uptr end = std::strtoull(str.SubStr(pos1 + 1, static_cast<uint64_t>(pos2 - pos1)).Str(), nullptr, baseValue);
    endAddr = end > endAddr ? end : endAddr;
}


static void GetEachSoAddrScope(std::vector<CString>& soNameVec)
{
    FILE* file = fopen("/proc/self/maps", "r");
    if (file == nullptr) {
        LOG(RTLOG_ERROR, "StackManager::InitAddressScope(): fail to open the file");
        return;
    }
    const int bufSize = 1024;
    char buf[bufSize] = { '\0' };
    while (fgets(buf, bufSize, file) != nullptr) {
        CString lineStr(buf);
        int protPos = lineStr.Find(' ');
        if (protPos < 0) {
            continue;
        }
        constexpr uint8_t protLen = 4;
        if (lineStr.SubStr(protPos + 1, protLen).Find('x') < 0) {
            continue;
        }
        char* baseName = CString::BaseName(lineStr);
        auto it = std::find(soNameVec.begin(), soNameVec.end(), baseName);
        if (it != soNameVec.end()) {
            if (strcmp(baseName, LIBCODIRA_RUNTIME ".so\n") == 0) {
                GetSoAddrScope(lineStr, StackManager::rtStartAddr, StackManager::rtEndAddr);
            } else if  (strcmp(baseName, "codec\n") == 0) {
                GetSoAddrScope(lineStr, StackManager::codecSoStartAddr, StackManager::codecSoEndAddr);
            }
        }
    }
    std::fclose(file);
}
#endif

#if defined(__APPLE__) and !(defined(__IOS__) && !defined(COMPILE_DYNAMIC))
static bool EndWith(const char* str, const char* suffix)
{
    if (str == nullptr || suffix == nullptr) {
        return false;
    }
    size_t strLen = strlen(str);
    size_t suffixLen = strlen(suffix);
    if (suffixLen > strLen) {
        return false;
    }
    return strncmp(str + strLen - suffixLen, suffix, suffixLen) == 0;
}
#endif

#if defined(__APPLE__) and !defined(__IOS__)
static void InitAddressInfoOnDarwin(const char* dylib, Uptr& start, Uptr& end)
{
    int pid = GetPid();
    struct proc_regionwithpathinfo info;
    Uptr curAddr = 0;
    while (proc_pidinfo(pid, PROC_PIDREGIONPATHINFO, curAddr, &info, sizeof(info)) > 0) {
        const char* path = info.prp_vip.vip_path;
        Uptr priAddr = info.prp_prinfo.pri_address;
        Uptr priSize = info.prp_prinfo.pri_size;
        if (EndWith(path, dylib) && start == STACK_ADDR_MAX && end == 0) {
            start = priAddr;
            end = priAddr + priSize;
            break;
        }
        curAddr = priAddr + priSize;
    }
}
#endif

#if defined(_WIN64)
static void InitAddressInfoOnWindows(const char* lib, Uptr& start, Uptr& end)
{
    Runtime& runtime = Runtime::Current();
    WinModuleManager& winModuleManager = runtime.GetWinModuleManager();
    const WinModule* module = winModuleManager.GetWinModuleByName(lib);
    if (module != nullptr) {
        start = module->GetImageBaseStart();
        end = module->GetImageBaseEnd();
    }
}
#endif

void StackManager::InitAddressScope()
{
// Init cangjie-runtime address info.
#if defined(COMPILE_DYNAMIC)
#if defined(_WIN64) // Windows
    InitAddressInfoOnWindows(LIBCODIRA_RUNTIME ".dll", StackManager::rtStartAddr, StackManager::rtEndAddr);
#elif defined(__APPLE__)
#if !defined(__IOS__) // MacOS
    InitAddressInfoOnDarwin("/" LIBCODIRA_RUNTIME ".dylib", StackManager::rtStartAddr, StackManager::rtEndAddr);
#endif
// For iOS, use `dladdr` to obtain the dylib name and perform string comparison.
#elif defined(__OHOS__) || defined(__ANDROID__) // OHOS, ANDROID
    std::vector<CString> rtSoNameVec = { LIBCODIRA_RUNTIME ".so\n" };
    GetEachSoAddrScope(rtSoNameVec);
#else                                                            // Linux
    StackManager::rtStartAddr = reinterpret_cast<Uptr>(&g_runtimeDynamicStart);
    StackManager::rtEndAddr = reinterpret_cast<Uptr>(&g_runtimeDynamicEnd);
#endif
#else                  // !COMPILE_DYNAMIC
#if defined(__APPLE__) // MacOS, iOS
    StackManager::rtStartAddr = reinterpret_cast<Uptr>(g_runtimeStaticStart);
    StackManager::rtEndAddr = reinterpret_cast<Uptr>(g_runtimeStaticEnd);
#else
    StackManager::rtStartAddr = reinterpret_cast<Uptr>(&g_runtimeStaticStart);
    StackManager::rtEndAddr = reinterpret_cast<Uptr>(&g_runtimeStaticEnd);
#endif
#endif

// Init codec address info.
#if defined(__linux__) // Linux, ANDROID, OHOS
    std::vector<CString> codecSoNameVec = { "codec\n" };
    GetEachSoAddrScope(codecSoNameVec);
#elif defined(_WIN64)                         // Windows
    InitAddressInfoOnWindows("codec.exe", StackManager::codecSoStartAddr, StackManager::codecSoEndAddr);
#elif defined(__APPLE__) && !defined(__IOS__) // MacOS
    InitAddressInfoOnDarwin("/codec", StackManager::codecSoStartAddr, StackManager::codecSoEndAddr);
#endif
}

void InitAddressScopeForCODEthreadTrace()
{
#ifdef _WIN64
    Runtime& runtime = Runtime::Current();
    WinModuleManager& winModuleManager = runtime.GetWinModuleManager();
    const WinModule* traceModule = winModuleManager.GetWinModuleByName(LIBCODIRA_CODETHREAD_TRACE ".dll");
    if (traceModule != nullptr) {
        StackManager::traceSoStartAddr = traceModule->GetImageBaseStart();
        StackManager::traceSoEndAddr = traceModule->GetImageBaseEnd();
    }
#elif defined(__APPLE__)
#ifndef __IOS__
    InitAddressInfoOnDarwin("/" LIBCODIRA_CODETHREAD_TRACE ".dylib", StackManager::traceSoStartAddr,
                            StackManager::traceSoEndAddr);
#endif
#else
    CString procFileName("/proc/self/maps");
    FILE* file = fopen(procFileName.Str(), "r");
    if (file == nullptr) {
        LOG(RTLOG_ERROR, "StackManager::InitAddressScope(): fail to open the file");
        return;
    }

    const int bufSize = 1024;
    char buf[bufSize] = { '\0' };
    while (fgets(buf, bufSize, file) != nullptr) {
        CString lineStr(buf);
        int protPos = lineStr.Find(' ');
        if (protPos < 0) {
            continue;
        }
        constexpr uint8_t protLen = 4;
        if (lineStr.SubStr(protPos + 1, protLen).Find('x') < 0) {
            continue;
        }
        char* baseName = CString::BaseName(lineStr);
        if (strcmp(baseName, LIBCODIRA_CODETHREAD_TRACE ".so\n") == 0) {
            GetSoAddrScope(lineStr, StackManager::traceSoStartAddr, StackManager::traceSoEndAddr);
        }
    }
    
    std::fclose(file);
    if (StackManager::traceSoStartAddr == STACK_ADDR_MAX && StackManager::traceSoEndAddr == 0) {
        LOG(RTLOG_FATAL, "can not find Runtime trace so");
    }
#endif
}

bool StackManager::IsRuntimeFrame(Uptr pc)
{
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    pc = PtrauthStripInstPointer(pc);
#endif
#ifdef CODIRA_HWASAN_SUPPORT
    pc = Sanitizer::UntagAddr(pc);
#endif
#if defined(__IOS__) && defined(COMPILE_DYNAMIC) // iOS dynamic cangjie-runtime.
    Dl_info info;
    const void* addr = reinterpret_cast<const void*>(pc);
    if (dladdr(addr, &info) &&
        (EndWith(info.dli_fname, "/" LIBCODIRA_RUNTIME ".dylib") ||
         EndWith(info.dli_fname, "/" LIBCODIRA_CODETHREAD_TRACE ".dylib"))) {
        return true;
    }
    return false;
#else // Otherwise.
    // For platforms that do not support macro expansion, such as iOS and HOS, the second condition will always be true
    // because there is no display setting for the address info of codec. The same logic applies to codethread-trace.
    return (pc > rtStartAddr && pc < rtEndAddr) || (pc > codecSoStartAddr && pc < codecSoEndAddr) ||
        (pc > traceSoStartAddr && pc < traceSoEndAddr);
#endif
}
} // namespace MapleRuntime
