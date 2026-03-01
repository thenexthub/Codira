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


#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Common/Runtime.h"
#include "Exception/ExceptionCApi.h"
#include "ExceptionManager.inline.h"
#include "Mutator/Mutator.h"
#include "ObjectModel/MObject.h"
#include "UnwindStack/MangleNameHelper.h"
#include "Heap/Collector/CollectorResources.h"
#include "Heap/Collector/GcRequest.h"
namespace MapleRuntime {
std::mutex ExceptionManager::gUncaughtExceptionHandlerMtx;
void ExceptionManager::OutOfMemory()
{
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    if (!eWrapper.IsThrowingOOME()) {
        eWrapper.SetThrowingOOME(true);
        eWrapper.SetFatalException(false);
        eWrapper.SetExceptionType(ExceptionWrapper::ExceptionType::OUT_OF_MEMORY);
        {
            // HeapDump before throwing OOM
            const char* env = std::getenv("codeHeapDumpOnOOM");
            CString s = CString(env).RemoveBlankSpace();
            env = s.Str();
            if (env && !strcmp(env, "on")) { // env variable: codeHeapDumpOnOOM=on/off
                Heap::GetHeap().GetCollectorResources().RequestHeapDump(GCTask::TaskType::TASK_TYPE_DUMP_HEAP_OOM);
            }
        }
        ThrowImplicitException(OOM);
    } else {
        std::vector<uint64_t>& liteFrameInfos = eWrapper.GetLiteFrameInfos();
        liteFrameInfos.clear();
        StackManager::RecordLiteFrameInfos(liteFrameInfos);
        ThrowImplicitException(OOMR);
    }
    eWrapper.SetThrowingOOME(false);
}

MRT_OPTIONAL_DISABLE_TAIL_CALL
void ExceptionManager::StackOverflow(uint32_t adjustedSize __attribute__((unused)), void* ip)
{
    Mutator* mutator = Mutator::GetMutator();
    if (mutator == nullptr) {
        return;
    }
    mutator->StackGuardExpand();
    ExceptionWrapper& eWrapper = mutator->GetExceptionWrapper();
    eWrapper.SetThrowingSOFFramePc(ip);
    eWrapper.SetFatalException(true);
    eWrapper.SetExceptionType(ExceptionWrapper::ExceptionType::STACK_OVERFLOW);
#if defined(__x86_64__)
    if (CodiraRuntime::stackGrowConfig == StackGrowConfig::STACK_GROW_ON) {
        eWrapper.SetAdjustedStackSize(adjustedSize);
    } else {
        eWrapper.SetAdjustedStackSize(COMPENSATE_SIZE);
    }
#elif defined(__aarch64__) || defined(__arm__)
    eWrapper.SetAdjustedStackSize(adjustedSize);
#endif
    ThrowImplicitException(SOF);

    MRT_ThrowPendingException();
}

MRT_OPTIONAL_DISABLE_TAIL_CALL void ExceptionManager::IncompatiblePackageExpection(CString msg)
{
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    eWrapper.SetExceptionType(ExceptionWrapper::ExceptionType::INCOMPATIBILE_PACKAGE);
    eWrapper.SetExceptionMessage(msg.Str(), strlen(msg.Str()));
    ExceptionRaiser func = Runtime::Current().GetExceptionManager().GetExceptionRaiser();
    if (func == nullptr) {
        LOG(RTLOG_FATAL, "Codira initialization failed, implicit exception is nullptr.");
    }
    uintptr_t threadData = MapleRuntime::MRT_GetThreadLocalData();
    ExecuteCodiraStub(reinterpret_cast<void*>(static_cast<intptr_t>(INCOMP)),  reinterpret_cast<void*>(msg.GetStr()),
                       0, reinterpret_cast<void*>(func), reinterpret_cast<void*>(threadData), 0);

    MRT_ThrowPendingException();
}

#if defined(__OHOS__) && (__OHOS__ == 1)
void ExceptionManager::RegisterUncaughtExceptionHandler(const CODEUncaughtExceptionInfo& handler)
{
    std::lock_guard<std::mutex> lock(gUncaughtExceptionHandlerMtx);
    uncaughtExceptionHandler = handler;
}
#endif

#ifdef __APPLE__
void ExceptionManager::DefaultUncaughtTask(const char* sunmary, const CODEErrorObject errorObj)
{
    (void)sunmary;
    (void)errorObj;
    abort();
}
#endif

void ExceptionManager::DumpException()
{
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    std::vector<uint64_t>& liteFrameInfos = eWrapper.GetLiteFrameInfos();
    LOG(RTLOG_ERROR, "An exception has occurred:\n");
    MObject* exceptionObject = eWrapper.GetExceptionRef();
    MangleNameHelper helper(exceptionObject->GetTypeInfo()->GetName());
    CString clsName(helper.GetSimpleClassName());
    std::vector<StackTraceElement> stackTrace;
    StackManager::GetStackTraceByLiteFrameInfos(liteFrameInfos, stackTrace);
    if (stackTrace.empty()) {
        LOG(RTLOG_ERROR, "Stacetrace is empty.");
    }
    // If uncaughtExceptionHandler is registered, then execute the handler.
    // Otherwise, dump the exception information.
    std::lock_guard<std::mutex> lock(gUncaughtExceptionHandlerMtx);
    if (Runtime::Current().GetExceptionManager().GetUncaughtExceptionHandler().uncaughtTask) {
#if defined(__OHOS__) && (__OHOS__ == 1) || (__APPLE__)
        const char* summary = "Uncaught exception was found.";
        CString exceptionMsg(eWrapper.GetExceptionMessage());
#if defined(__APPLE__)
        LOG(RTLOG_ERROR, summary);
        LOG(RTLOG_ERROR, exceptionMsg.Str());
#endif
        CString exceptionStack;
        const int strLen = 10;
        char* str = static_cast<char*>(NativeAllocator::NativeAlloc(strLen * sizeof(char)));
        if (str == nullptr) {
            LOG(RTLOG_FATAL, "Alloca memory failed when Codira dump uncaught exception.");
        }
        for (auto ste : stackTrace) {
            const int strLen = 10;
            char* str = static_cast<char*>(malloc(strLen * sizeof(char)));
            sprintf_s(str, strLen, "%ld", ste.lineNumber);
            exceptionStack += "\t at ";
            exceptionStack += ste.className.Str();
            exceptionStack += (ste.className.Length() > 0 ? "." : "");
            exceptionStack += ste.methodName.Str();
            exceptionStack += "(";
            exceptionStack += ste.fileName.Str();
            exceptionStack += +":";
            exceptionStack += str;
            exceptionStack += ")\n";
#if defined(__APPLE__)
            LOG(RTLOG_ERROR, exceptionStack.Str());
            exceptionStack = "";
#endif
        }
        CODEErrorObject errObj = {clsName.Str(), exceptionMsg.Str(), exceptionStack.Str()};
        eWrapper.ClearInfo();
        Runtime::Current().GetExceptionManager().GetUncaughtExceptionHandler().uncaughtTask(summary, errObj);
#endif
    } else {
#ifdef __APPLE__
        PRINT_ERROR("%s", clsName.Str());
#endif
        LOG(RTLOG_ERROR, clsName.Str());
        if (eWrapper.GetExceptionMessage() != nullptr && eWrapper.GetExceptionMessageLength() != 0) {
            const char* linkStr = ": ";
#ifdef __APPLE__
            PRINT_ERROR("%s", linkStr);
            PRINT_ERROR("%s", eWrapper.GetExceptionMessage());
#endif
            LOG(RTLOG_ERROR, linkStr);
            LOG(RTLOG_ERROR, eWrapper.GetExceptionMessage());
        }
#ifdef __APPLE__
        PRINT_ERROR("\n");
#endif
        LOG(RTLOG_ERROR, "\n");
        constexpr int32_t frameInfoPairLen = 3; // function PC and startpc form one pair in liteFrameInfos
        // When some frames are folded, arraySize is an odd number and the last frame is invalid.
        // In this case, the last frame is discarded.
        SofStackFlag sofFoldedFlag = SofStackFlag::NOT_FOLDED;
        if (liteFrameInfos.size() % frameInfoPairLen == 1) {
            sofFoldedFlag = SofStackFlag(liteFrameInfos.back());
            liteFrameInfos.pop_back();
        }

        if (sofFoldedFlag == SofStackFlag::TOP_FOLDED) {
#ifdef __APPLE__
            PRINT_ERROR("\t ... Some frames are not displayed ...\n");
#endif
            LOG(RTLOG_ERROR, "\t ... Some frames are not displayed ...\n");
        }
        for (auto ste : stackTrace) {
#ifdef __APPLE__
            PRINT_ERROR("\t at %s%s%s(%s:%lld)\n", ste.className.Str(), ste.className.Length() > 0 ? "." : "",
                        ste.methodName.Str(), ste.fileName.Str(), ste.lineNumber);
#endif
            LOG(RTLOG_ERROR, "\t at %s%s%s(%s:%ld)\n", ste.className.Str(), ste.className.Length() > 0 ? "." : "",
                ste.methodName.Str(), ste.fileName.Str(), ste.lineNumber);
        }
        if (sofFoldedFlag == SofStackFlag::BOTTOM_FOLDED) {
#ifdef __APPLE__
            PRINT_ERROR("\t ... Some frames are not displayed ...\n");
#endif
            LOG(RTLOG_ERROR, "\t ... Some frames are not displayed ...\n");
        }
#if defined(__OHOS__) && (__OHOS__ == 1)
        // In OHOS, C calling Codira: uncaught exception allow C-side execution to continue without proactive exit,
        // leading to unexpected behavior. Proactive exit is required.
        LOG(RTLOG_FATAL, "Uncaught exception: %s. Terminate required.", clsName.Str());
#endif
    }
}

void ExceptionManager::ThrowImplicitException(ImplicitExceptionType type)
{
    MRT_SetStackGrow(false);
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    eWrapper.SetExceptionMessage(nullptr, 0);
    ExceptionRaiser func = Runtime::Current().GetExceptionManager().GetExceptionRaiser();
    if (func == nullptr) {
        LOG(RTLOG_FATAL, "Codira initialization failed, implicit exception is nullptr.");
    }
    uintptr_t threadData = MapleRuntime::MRT_GetThreadLocalData();
    ExecuteCodiraStub(reinterpret_cast<void*>(static_cast<intptr_t>(type)), 0, 0, reinterpret_cast<void*>(func),
                       reinterpret_cast<void*>(threadData), 0);
}

#ifndef _WIN64
static bool HandleAbortSignal(int sig, siginfo_t* info, void* context)
{
    LOG(RTLOG_ERROR, "Two pending exceptions are thrown when a C function calls Codira function. Program will abort.");
    return true;
}

static void InstallExceptionAbortHandler()
{
    sigset_t mask;
    CHECK_SIGNAL_CALL(sigemptyset, (&mask), "sigemptyset failed");
    SignalAction sa;
    sa.saSignalAction = HandleAbortSignal;
    sa.scMask = mask;
    sa.scFlags = SA_SIGINFO | SA_ONSTACK;

    AddHandlerToSignalStack(SIGABRT, &sa);
}
#endif

void ExceptionManager::ThrowException(const ExceptionRef& exception)
{
    ScopedEntryTrace trace("CODERT_THROW_EXCEPTION");
    MRT_SetStackGrow(true);
    ExceptionWrapper& mExceptionWrapper = Mutator::GetMutator()->GetExceptionWrapper();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    DLOG(EXCEPTION, "start throw exception");
    DLOG(EXCEPTION, "Struct ExceptionWrapper");
    DLOG(EXCEPTION, "  ExceptionRef\t:\t%p", mExceptionWrapper.GetExceptionRef());
    DLOG(EXCEPTION, "  isCaught\t:\t%d", mExceptionWrapper.IsCaught());
    DLOG(EXCEPTION, "  typeIndex\t:\t%lu", mExceptionWrapper.GetTypeIndex());
    DLOG(EXCEPTION, "  throwingOOME\t:\t%d", mExceptionWrapper.IsThrowingOOME());
    DLOG(EXCEPTION, "  fatalException\t:\t%d", mExceptionWrapper.IsFatalException());
    DLOG(EXCEPTION, "  ExceptionType\t:\t%s", mExceptionWrapper.GetExceptionTypeName());
#endif
    mExceptionWrapper.Reset();

    if (mExceptionWrapper.GetExceptionRef() != nullptr) {
#ifndef _WIN64
        // Because the throwing of exceptions has already modified the rbp and pc of some frames,
        // it is impossible to correctly complete the stack unwind. Here, an empty handler is added to exit directly.
        InstallExceptionAbortHandler();
#endif
        LOG(RTLOG_FATAL, "ThrowException fail, abort.");
    }

    mExceptionWrapper.SetExceptionRef(exception);
    ExceptionHandling eh(mExceptionWrapper, nullptr);
}

void* ExceptionManager::BeginCatch(ExceptionWrapper* mExceptionWrapper __attribute__((unused)))
{
    ScopedEntryTrace trace("CODERT_CATCH_EXCEPTION");
    Mutator* mutator = Mutator::GetMutator();
    ExceptionWrapper& eWrapper = mutator->GetExceptionWrapper();
    if (eWrapper.IsThrowingSOFE()) {
        mutator->StackGuardRecover();
        eWrapper.SetThrowingSOFFramePc(nullptr);
        eWrapper.SetAdjustedStackSize(0);
    }
    eWrapper.SetFatalException(false);
    eWrapper.SetExceptionType(ExceptionWrapper::ExceptionType::UNKNOWN);
    ExceptionRef thrown = eWrapper.GetExceptionRef();
    eWrapper.SetExceptionRef(nullptr);
    return thrown;
}
} // namespace MapleRuntime
