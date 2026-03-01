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


#include "CompilerCalls.h"

#include "Base/CString.h"
#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Common/BaseObject.h"

// module interfaces
#include "ObjectManager.inline.h"
#if defined(CODIRA_SANITIZER_SUPPORT) || defined(CODIRA_GWPASAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
#include "SignalManager.h"
#endif
#include "Common/ScopedObjectAccess.h"
#include "ExceptionManager.inline.h"
#include "Heap/Barrier/Barrier.h"
#include "Heap/Collector/CollectorResources.h"
#include "Heap/Heap.h"
#include "HeapManager.inline.h"
#include "LoaderManager.h"
#include "TypeInfoManager.h"
#include "ObjectModel/Field.inline.h"
#include "ObjectModel/RefField.inline.h"
#ifdef _WIN64
#include "Mutator/MutatorManager.h"
#include "Mutator/ThreadLocal.h"
#endif
#include "Sync/Sync.h"
#include "UnwindStack/GcStackInfo.h"
#include "CpuProfiler/CpuProfiler.h"
#ifdef __OHOS__
#include "schedule.h"
#include "Base/SpinLock.h"
#include <dlfcn.h>
#endif

namespace MapleRuntime {
// runtime interfaces provided to compiler for code generation.
// The compiler should only call MCC_* to access runtime functions.
// MCC_* calls follows C standard calling convention.

extern "C" ObjRef MCC_NewObject(const TypeInfo* klass, MSize size)
{
    DCHECK(size == (AlignUp<size_t>(klass->GetInstanceSize(), 8) + TYPEINFO_PTR_SIZE)); // 8-byte alignment
    ObjRef obj = ObjectManager::NewObject(klass, size);
    if (obj == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewObject return nullptr");
    }
    return obj;
}

extern "C" ObjRef MCC_NewWeakRefObject(const TypeInfo* klass, MSize size)
{
    DCHECK(size == (AlignUp<size_t>(klass->GetInstanceSize(), 8) + TYPEINFO_PTR_SIZE)); // 8-byte alignment
    ObjRef obj = ObjectManager::NewWeakRefObject(klass, size);
    if (obj == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewWeakRefObject return nullptr");
    }
    return obj;
}

extern "C" ObjRef MCC_NewPinnedObject(const TypeInfo* klass, MSize size, bool isFinalizer)
{
    DCHECK(size == (AlignUp<size_t>(klass->GetInstanceSize(), 8) + TYPEINFO_PTR_SIZE)); // 8-byte alignment
    ObjRef obj = ObjectManager::NewPinnedObject(klass, size, isFinalizer);
    if (obj == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewPinnedObject return nullptr");
    }
    return obj;
}

extern "C" ObjRef MCC_NewFinalizer(const TypeInfo* klass, MSize size)
{
    DCHECK(size == (AlignUp<size_t>(klass->GetInstanceSize(), 8) + TYPEINFO_PTR_SIZE)); // 8-byte alignment
    ObjRef obj = ObjectManager::NewFinalizer(klass, size);
    if (obj == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewFinalizer return nullptr");
    }
    return obj;
}

extern "C" ObjRef MCC_OnFinalizerCreated(ObjRef ref)
{
    ref->OnFinalizerCreated();
    return ref;
}
extern "C" ArrayRef MCC_NewArray(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewArray(static_cast<MIndex>(nElems), arrayInfo);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewArray return nullptr");
    }
    return array;
}

extern "C" ArrayRef MCC_NewObjArray(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewObjArray(nElems, arrayInfo);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewObjArray return nullptr");
    }
    return array;
}

extern "C" ArrayRef MCC_NewArray8(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_8B);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray return nullptr");
    }
    return array;
}

extern "C" ArrayRef MCC_NewArray16(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_16B);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray(16B) return nullptr");
    }
    return array;
}

extern "C" ArrayRef MCC_NewArray32(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_32B);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray(32B) return nullptr");
    }
    return array;
}

extern "C" ArrayRef MCC_NewArray64(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_64B);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray(64B) return nullptr");
    }
    return array;
}

extern "C" void MCC_WriteRefField(const ObjectPtr ref, const ObjectPtr obj, RefField<false>* field)
{
    if (!Heap::IsHeapAddress(obj)) {
        field->SetTargetObject(ref);
        return;
    }
    Heap::GetBarrier().WriteReference(obj, *field, ref);
}

extern "C" void MCC_WriteStructField(ObjectPtr obj, MAddress dst, size_t dstLen, MAddress src, size_t srcLen)
{
    CHECK_DETAIL((dst != 0u && src != 0u), "MCC_WriteStructField wrong parameter, dst: %p src: %p", dst, src);
    if (UNLIKELY(!Heap::IsHeapAddress(obj))) {
        CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(dst), dstLen, reinterpret_cast<void*>(src), srcLen) == EOK,
                     "memcpy_s failed");
        return;
    }
    Heap::GetBarrier().WriteStruct(obj, dst, dstLen, src, srcLen);
}

extern "C" void MCC_WriteStaticRef(const ObjectPtr ref, RefField<false>* field)
{
    Heap::GetBarrier().WriteStaticRef(*field, ref);
}

extern "C" void MCC_WriteStaticStruct(MAddress dst, size_t dstLen, MAddress src, size_t srcLen, const GCTib gcTib)
{
    CHECK_DETAIL((dst != 0u && src != 0u), "MCC_WriteStaticStruct wrong parameter, dst: %p src: %p", dst, src);
    Heap::GetBarrier().WriteStaticStruct(dst, dstLen, src, srcLen, gcTib);
}

extern "C" TypeInfo* MCC_GetObjClass(const ObjectPtr obj)
{
    // it may be better by installing load barrier.
    return obj->GetTypeInfo();
}

extern "C" TypeInfo* MCC_GetTypeForAny(const ObjectPtr obj) { return obj->GetTypeInfo(); }

extern "C" bool MCC_IsWrapperClassForAutoEnv(const TypeInfo* ti)
{
    const char* tiName = ti->GetName();
    const char* targetPattern = ":$Cw$";
    const char *p = strchr(tiName, ':');
    if (p == nullptr) {
        return false;
    } else if (strncmp(p, targetPattern, strlen(targetPattern)) == 0) {
        return true;
    } else {
        return false;
    }
}

extern "C" void CODE_MCC_ArrayCopyRef(const ObjectPtr dstObj, MAddress dstField, size_t dstSize, const ObjectPtr srcObj,
                                    MAddress srcField, size_t srcSize)
{
    if (dstSize == 0) {
        return;
    }
    MRT_ASSERT(dstSize <= SECUREC_MEM_MAX_LEN, "size too big in CODE_MCC_ArrayCopy");
    Heap::GetBarrier().CopyRefArray(dstObj, dstField, dstSize, srcObj, srcField, srcSize);
}

extern "C" void CODE_MCC_ArrayCopyStruct(const ObjectPtr dstObj, MAddress dstField, size_t dstSize,
                                       const ObjectPtr srcObj, MAddress srcField, size_t srcSize)
{
    if (dstSize == 0) {
        return;
    }
    MRT_ASSERT(dstSize <= SECUREC_MEM_MAX_LEN, "size too big in CODE_MCC_ArrayCopy");
    Heap::GetBarrier().CopyStructArray(dstObj, dstField, dstSize, srcObj, srcField, srcSize);
}
extern "C" void MCC_AtomicWriteReference(const ObjectPtr ref, const ObjectPtr obj, RefField<true>* field,
                                         MemoryOrder order)
{
    Heap::GetBarrier().AtomicWriteReference(obj, *field, ref, order);
}

extern "C" ObjectPtr MCC_AtomicReadReference(const ObjectPtr obj, RefField<true>* field, MemoryOrder order)
{
    return Heap::GetBarrier().AtomicReadReference(obj, *field, order);
}

extern "C" ObjectPtr MCC_AtomicSwapReference(const ObjectPtr ref, const ObjectPtr obj, RefField<true>* field,
                                             MemoryOrder order)
{
    return Heap::GetBarrier().AtomicSwapReference(obj, *field, ref, order);
}

extern "C" bool MCC_AtomicCompareSwapReference(const ObjectPtr oldRef, const ObjectPtr newRef, const ObjectPtr obj,
                                               RefField<true>* field, MemoryOrder succOrder, MemoryOrder failOrder)
{
    return Heap::GetBarrier().CompareAndSwapReference(obj, *field, oldRef, newRef, succOrder, failOrder);
}

extern "C" void MCC_InvokeGCImpl(bool sync) { HeapManager::RequestGC(GC_REASON_USER, !sync); }

extern "C" ssize_t MCC_GetRealHeapSize() { return Heap::GetHeap().GetHeapPhysicalMemorySize(); }

extern "C" size_t MCC_GetAllocatedHeapSize() { return Heap::GetHeap().GetAllocatedSize(); }

extern "C" size_t MCC_GetMaxHeapSize() { return Heap::GetHeap().GetMaxCapacity(); }

extern "C" bool MCC_DumpCODEHeapData(int fd)
{
    ScopedEnterSaferegion enterSaferegion(false);
    CodeHeapData* codeHeapData = new CodeHeapData();
    CHECK_DETAIL(codeHeapData != nullptr, "new codeHeapData fail");
    bool result = codeHeapData->DumpHeap(fd);
    delete codeHeapData;
    return result;
}

extern "C" size_t MCC_GetCODEThreadNumber() { return ScheduleCODEThreadCountPublic(CODETHREAD_PSTATE_ALL); }

extern "C" size_t MCC_GetBlockingCODEThreadNumber() { return ScheduleCODEThreadCountPublic(CODETHREAD_PSTATE_BLOCKING); }

extern "C" size_t MCC_GetNativeThreadNumber() { return ScheduleRunningOSThreadCount(); }

extern "C" size_t MCC_GetGCCount() { return g_gcCount; }

extern "C" uint64_t MCC_GetGCTimeUs() { return g_gcTotalTimeUs; }

extern "C" size_t MCC_GetGCFreedSize() { return g_gcCollectedTotalBytes; }

extern "C" bool MCC_StartCpuProfiling()
{
    return CpuProfiler::GetInstance().StartCpuProfilerForFile();
}

extern "C" bool MCC_StopCpuProfiling(int fd)
{
    return CpuProfiler::GetInstance().StopCpuProfilerForFile(fd);
}

extern "C" void MCC_SetGCThreshold(uint64_t GCThreshold) { Runtime::Current().SetGCThreshold(GCThreshold); }

extern "C" void* MCC_PostThrowException(ExceptionWrapper* mExceptionWrapper)
{
    return ExceptionManager::BeginCatch(mExceptionWrapper);
}

extern "C" void MCC_ThrowException(ExceptionRef exception) { ExceptionManager::ThrowException(exception); }

extern "C" void MCC_RegisterImplicitExceptionRaisers(void* raiser)
{
    ExceptionManager& exceptionManager = Runtime::Current().GetExceptionManager();
    exceptionManager.RegisterExceptionRaiser(raiser);
}

extern "C" void* MCC_GetExceptionWrapper() { return ExceptionManager::GetExceptionWrapper(); }

extern "C" uint32_t MCC_GetExceptionTypeID() { return ExceptionManager::GetExceptionTypeID(); }

extern "C" void MCC_EndCatch(){};

extern "C" MRT_OPTIONAL_DISABLE_TAIL_CALL void MCC_ThrowStackOverflowError(uint32_t size = 0)
{
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    // get the caller return address since there is an asm stub when cfi is enabled.
    void* fp = __builtin_frame_address(0);
    void *ip = *(*reinterpret_cast<void***>(fp) + 1);
#else
    void* ip = __builtin_return_address(0);
#endif
    ExceptionManager::StackOverflow(size, ip);
}

#ifndef _WIN64
extern "C" sighandler_t CODE_MCC_SignalRegister(int sig, sighandler_t handler)
{
    return signal(sig, handler);
}
extern "C" int CODE_MCC_SignalProcMask(int how, const sigset_t* newSet, sigset_t* oldSet)
{
    return sigprocmask(how, newSet, oldSet);
}
extern "C" void CODE_MCC_SignalKill(pid_t pid, int sig)
{
    kill(pid, sig);
}
extern "C" void CODE_MCC_SignalRaise(int sig)
{
    std::raise(sig);
}
extern "C" void CODE_MCC_AddSignalHandler(int signal, struct SignalAction* sa)
{
    if (signal == SIGABRT || signal == SIGILL) {
        SignalStack::GetStacks()[signal].setUserSigHandler(true);
    }
    // By default, block the SIGPIPE signal, and take no action on the SIGPIPE signal;
    // when the user registers a handler, unblock the SIGPIPE signal,
    // allowing the user to take action on the SIGPIPE signal.
    if (signal == SIGPIPE) {
        sigset_t set;
        CHECK_SIGNAL_CALL(sigemptyset, (&set), "sigemptyset failed in AddHandlerToSignalStack");
        CHECK_SIGNAL_CALL(sigaddset, (&set, SIGPIPE), "sigaddset failed in AddHandlerToSignalStack");
        CHECK_SIGNAL_CALL(pthread_sigmask, (SIG_UNBLOCK, &set, nullptr),
                          "pthread_sigmask failed in AddHandlerToSignalStack");
    }

    AddHandlerToSignalStack(signal, sa);
}
extern "C" void CODE_MCC_RemoveSignalHandler(int signal, bool (*fn)(int, siginfo_t*, void*))
{
    RemoveHandlerFromSignalStack(signal, fn);
}
#endif // _WIN64

extern "C" ArrayRef MCC_FillInStackTraceImpl(const TypeInfo* arrayInfo, const ArrayRef excepMsg)
{
    ExceptionWrapper& eWrapper = Mutator::GetMutator()->GetExceptionWrapper();
    MIndex msgLength = excepMsg->GetLength();
    if (msgLength != 0) {
        eWrapper.SetExceptionMessage(reinterpret_cast<const char*>(excepMsg->ConvertToCArray()), msgLength);
    }
    std::vector<uint64_t>& liteFrameInfos = eWrapper.GetLiteFrameInfos();
    liteFrameInfos.clear();
    StackManager::RecordLiteFrameInfos(liteFrameInfos);
    constexpr int frameInfoPairLen = 3; // function PC and startpc form one pair in liteFrameInfos
    if (eWrapper.IsThrowingSOFE()) {
        constexpr int defaultSize = 32;
        // Frames from std::core and to be filtered out, 2 in total
        //     std$core::StackOverflowError::<init>() ()
        //     rt$ThrowStackOverflowError$real ()
        constexpr int coreOrFiltFuncSize = 2 * frameInfoPairLen;
        // 2 frames need to be filtered at top
        //     user[main]
        //     code_entry$
        constexpr int topFiltFrameSize = 2 * frameInfoPairLen;
        auto env = std::getenv("CODE_SOF_SIZE");
        CString s(env);
        int64_t sofSize = CString::ParseNumFromEnv(s.Str());
        if (s.Str() == nullptr || (sofSize == 0 && s != "0")) {
            sofSize = defaultSize;
        }
        sofSize *= frameInfoPairLen;
        // When a stack is folded, an additional element is added behind liteFrameInfos to record the folding.
        // At this time, the size of liteFrameInfos is changed to 3n+1.
        if (sofSize > 0 && liteFrameInfos.size() > sofSize + coreOrFiltFuncSize + topFiltFrameSize) {
            liteFrameInfos.erase(liteFrameInfos.begin() + sofSize + coreOrFiltFuncSize, liteFrameInfos.end());
            liteFrameInfos.push_back(static_cast<uint64_t>(SofStackFlag::BOTTOM_FOLDED));
        } else if (sofSize < 0 && liteFrameInfos.size() > coreOrFiltFuncSize + topFiltFrameSize - sofSize) {
            sofSize -= topFiltFrameSize;
            liteFrameInfos.erase(liteFrameInfos.begin(), liteFrameInfos.end() + sofSize);
            liteFrameInfos.push_back(static_cast<uint64_t>(SofStackFlag::TOP_FOLDED));
        }
    }
    MIndex size = liteFrameInfos.size();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (UNLIKELY(ENABLE_LOG(EXCEPTION))) {
        MIndex arraySize = size;
        if (arraySize % frameInfoPairLen == 1) {
            --arraySize;
        }
        DLOG(EXCEPTION, "fill in stack info");
        DLOG(EXCEPTION, "layer\tframePC\t\tframeFuncStart\tsize : %zu", (arraySize / frameInfoPairLen));
        for (int i = 0; i < arraySize; ++i) {
            uint64_t framePC = liteFrameInfos[i];
            uint64_t frameFuncStart = liteFrameInfos[++i];
            DLOG(EXCEPTION, "#%x\t0x%x\t0x%x", ((i - 1) / frameInfoPairLen), framePC, frameFuncStart);
        }
    }
#endif

    // framePCs contains the following five Codira method pcs in sequence.
    // #0 core::Throwable::init()
    // #1 core::Exception::init()
    // #2 default::Func()
    // #3 user.main()
    // #4 code_entry()
    // In the call stack visible to the user, #0 #1 #4 should be removed.
    ArrayRef array =
        ObjectManager::NewKnownWidthArray(size, arrayInfo, ObjectManager::ObjectManager::ArrayElemBits::ELEM_64B);
    if (array == nullptr) {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
        DLOG(EXCEPTION, "BuildEHFrameInfo");
#endif
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray return nullptr");
        return nullptr;
    }
    for (MIndex i = 0; i < size; ++i) {
        array->SetPrimitiveElement(i, static_cast<int64_t>(liteFrameInfos[i]));
    }
    return array;
}

extern "C" StackTraceData MCC_DecodeStackTraceImpl(const uint64_t ip, const uint64_t pc, const uint64_t funcDesc,
                                                   const TypeInfo* charArray)
{
    StackTraceElement stackTrace;
    StackManager::GetStackTraceByLiteFrameInfo(ip, pc, funcDesc, stackTrace);
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    DLOG(EXCEPTION, "get stack frame info");
    DLOG(EXCEPTION, "   framePc:0x%lx\t frameFuncStart:0x%lx", ip, pc);
    DLOG(EXCEPTION, "get stack frame metainfo");
    DLOG(EXCEPTION, "   ClassName  \t : \t%s", stackTrace.className.Str());
    DLOG(EXCEPTION, "   MethodName  \t : \t%s", stackTrace.methodName.Str());
    DLOG(EXCEPTION, "   FileName  \t : \t%s", stackTrace.fileName.Str());
    DLOG(EXCEPTION, "   LineNumber  \t : \t%lu", stackTrace.lineNumber);
#endif
    StackTraceData std;
    std.className = ObjectManager::NewKnownWidthArray(
        stackTrace.className.Length(), charArray, ObjectManager::ArrayElemBits::ELEM_8B, AllocType::RAW_POINTER_OBJECT);
    if (std.className != nullptr) {
        for (MIndex i = 0; i < stackTrace.className.Length(); ++i) {
            std.className->SetPrimitiveElement(i, static_cast<int8_t>(stackTrace.className[i]));
        }
    } else {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray return nullptr");
    }

    std.fileName = ObjectManager::NewKnownWidthArray(
        stackTrace.fileName.Length(), charArray, ObjectManager::ArrayElemBits::ELEM_8B, AllocType::RAW_POINTER_OBJECT);
    if (std.fileName != nullptr) {
        for (MIndex i = 0; i < stackTrace.fileName.Length(); ++i) {
            std.fileName->SetPrimitiveElement(i, static_cast<int8_t>(stackTrace.fileName[i]));
        }
    } else {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray return nullptr");
    }

    std.methodName =
        ObjectManager::NewKnownWidthArray(stackTrace.methodName.Length(), charArray,
                                          ObjectManager::ArrayElemBits::ELEM_8B, AllocType::RAW_POINTER_OBJECT);
    if (std.methodName != nullptr) {
        for (MIndex i = 0; i < stackTrace.methodName.Length(); ++i) {
            std.methodName->SetPrimitiveElement(i, static_cast<int8_t>(stackTrace.methodName[i]));
        }
    } else {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewKnownWidthArray return nullptr");
    }

    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    std.lineNumber = stackTrace.lineNumber;
    return std;
}

// Warning: Caller MUST call `CommitRawPointerRegions` to make sure that
// the new object will be correctly tracked by gc.
static ArrayRef CreateCharArrayFromCString(const TypeInfo* charArray, CString name)
{
    ArrayRef array = ObjectManager::NewKnownWidthArray(
        name.Length(), charArray, ObjectManager::ArrayElemBits::ELEM_8B, AllocType::RAW_POINTER_OBJECT);
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("CreateCharArrayFromCString: nullptr array");
    }
    for (MIndex i = 0; i < name.Length(); ++i) {
        array->SetPrimitiveElement(i, static_cast<int8_t>(name[i]));
    }
    return array;
}

// Warning: Caller MUST call `CommitRawPointerRegions` to make sure that
// the new object will be correctly tracked by gc.
static ArrayRef CreateStackTrace(const TypeInfo* arrayStackTrace, const TypeInfo* charArray,
                                 const std::vector<FrameInfo*> &srcSracks)
{
    // fix size after discard native frame
    MSize size = 0;
    for (auto frame : srcSracks) {
        // skip native frame
        if (frame->GetFrameType() == FrameType::NATIVE) {
            continue;
        }
        size++;
    }

    ArrayRef trace = ObjectManager::NewArray(size, arrayStackTrace, AllocType::RAW_POINTER_OBJECT);
    if (trace == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("CreateStackTrace: nullptr array");
    }
    int stackIndex = 0;
    for (auto frame : srcSracks) {
        // skip native frame
        if (frame->GetFrameType() == FrameType::NATIVE) {
            continue;
        }

        CString className = frame->GetPackClassName();
        CString fileName = frame->GetFileNameForTrace();
        CString methodName = frame->GetMethodName();
        uint32_t lineNumber = frame->GetLineNum();

        StackTraceData frameData;
        // fill frame
        frameData.className = CreateCharArrayFromCString(charArray, className);
        frameData.fileName = CreateCharArrayFromCString(charArray, fileName);
        frameData.methodName = CreateCharArrayFromCString(charArray, methodName);
        frameData.lineNumber = lineNumber;

        // push frame to stack array
        MSize elementSize = trace->GetElementSize();
        MAddress dstAddr = reinterpret_cast<Uptr>(trace) + MArray::GetContentOffset() + elementSize * stackIndex;
        Heap::GetBarrier().WriteStruct(trace, dstAddr, elementSize,
                                       reinterpret_cast<MAddress>(&frameData), elementSize);
        stackIndex++;
    }
    return trace;
}

static ArrayRef GetAllThreadSnapshot(const TypeInfo* arraySnapshot, const TypeInfo* arrayStackTrace,
                                     const TypeInfo* charArray)
{
    std::vector<std::unique_ptr<RecordStackInfo>> records;
    MutatorManager::Instance().VisitAllMutators([&records](Mutator &mutator) {
        if (!mutator.IsVaildCODEThread()) {
            return;
        }
        int state = mutator.GetCODEThreadState();
        if (state == 0) {  // 0: IDLE
            return;
        }

        // 4: syscall which we treat as 2:running
        state = state == 4 ? 2 : state;
        uint32_t threadId = static_cast<uint32_t>(mutator.GetCODEThreadId());
        CString threadName;
        if (mutator.GetCODEThreadName() != nullptr) {
            threadName = CString(mutator.GetCODEThreadName());
        }

        auto record = std::make_unique<RecordStackInfo>(
            RecordStackInfo(&(mutator.GetUnwindContext()), threadId, threadName, state));
        record->FillInStackTrace();
        records.emplace_back(std::move(record));
    });

    ArrayRef allRecords = ObjectManager::NewArray(records.size(), arraySnapshot, AllocType::RAW_POINTER_OBJECT);
    if (allRecords == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("GetAllThreadSnapshot: nullptr array");
    }
    int recordIndex = 0;
    for (const auto &record : records) {
        ThreadSnapshot snapshot;
        // fill thread snapshot
        snapshot.name = CreateCharArrayFromCString(charArray, record->GetThreadName());
        snapshot.id = record->GetStackTid();
        snapshot.stackTrace = CreateStackTrace(arrayStackTrace, charArray, record->stacks);
        snapshot.state = record->GetThreadState();

        // push snapshot to record array
        MSize elementSize = allRecords->GetElementSize();
        MAddress dstAddr =
            reinterpret_cast<Uptr>(allRecords) + MArray::GetContentOffset() + elementSize * recordIndex;
        Heap::GetBarrier().WriteStruct(allRecords, dstAddr, elementSize,
                                       reinterpret_cast<MAddress>(&snapshot), elementSize);
        recordIndex++;
    }

    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    return allRecords;
}

extern "C" ArrayRef MCC_GetAllThreadSnapshotImpl(const TypeInfo* arraySnapshot, const TypeInfo* arrayStackTrace,
                                                 const TypeInfo* charArray)
{
    ScopedEnterSaferegion enterSaferegion(false);
    ArrayRef allRecords = nullptr;
    if (MutatorManager::Instance().WorldStopped()) {
        allRecords = GetAllThreadSnapshot(arraySnapshot, arrayStackTrace, charArray);
    } else {
        ScopedStopTheWorld stw("dump all thread");
        allRecords = GetAllThreadSnapshot(arraySnapshot, arrayStackTrace, charArray);
    }
    return allRecords;
}

extern "C" ThreadSnapshot MCC_GetCurrentThreadSnapshotImpl(const TypeInfo* arrayStackTrace, const TypeInfo* charArray)
{
    Mutator* mutator = Mutator::GetMutator();
    CHECK_DETAIL(mutator != nullptr, "Can not get mutator");
    CHECK_DETAIL(mutator->IsVaildCODEThread(), "Get invalid mutator");
    mutator->EnterSaferegion(true);

    uint32_t threadId = static_cast<uint32_t>(mutator->GetCODEThreadId());
    CString threadName;
    if (mutator->GetCODEThreadName() != nullptr) {
        threadName = CString(mutator->GetCODEThreadName());
    }
    int state = mutator->GetCODEThreadState();
    CHECK_DETAIL(state != 0, "Get invalid thread state idle");
    // 4: syscall which we treat as 2:running
    state = state == 4 ? 2 : state;

    RecordStackInfo record(&(mutator->GetUnwindContext()), threadId, threadName, state);
    record.FillInStackTrace();

    ThreadSnapshot snapshot;
    // fill thread snapshot
    snapshot.name = CreateCharArrayFromCString(charArray, record.GetThreadName());
    snapshot.id = record.GetStackTid();
    snapshot.stackTrace = CreateStackTrace(arrayStackTrace, charArray, record.stacks);
    snapshot.state = record.GetThreadState();

    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    mutator->LeaveSaferegion();
    return snapshot;
}

static ArrayRef PinArray(const ArrayRef array)
{
    Mutator* mutator = Mutator::GetMutator();
    CHECK_DETAIL(mutator != nullptr, "Mutator has not initialized or has been fini: %p", mutator);
    CHECK_DETAIL(!mutator->InSaferegion(), "Mutator to be fini should not be in saferegion");
    // forbid gc thread to move this region.
    Heap::GetHeap().GetCollector().AddRawPointerObject(array);
    return static_cast<ArrayRef>(array);
}

// Return the raw pointer of input array object, isCopy records whether memory copy occurs.
// If GC is running, try to copy the payload of array and return the copy data pointer, isCopy set true
// If copy failed, just return the content pointer of real array, isCopy set false,
// but can't return until GC finish current work.
extern "C" void* MCC_AcquireRawData(const ArrayRef array, bool* isCopy)
{
    if (!Heap::IsHeapAddress(array)) {
        return array->ConvertToCArray();
    }
#ifdef _WIN64
    static void* unreadablePage = reinterpret_cast<void*>(0x1234);
#else
    static void* unreadablePage = MutatorManager::Instance().GetSafepointPageManager()->GetUnreadablePage();
#endif
    MRT_ASSERT(unreadablePage != nullptr, "runtime is not initialized\n");
    if (UNLIKELY(array == nullptr)) {
        return nullptr;
    }
    if (UNLIKELY(array->GetContentSize() == 0)) {
        return unreadablePage;
    }
    MRT_ASSERT(array->IsPrimitiveArray(), "Expect primitive array in MCC_AcquireRawData");
    if (isCopy != nullptr) {
        *isCopy = false;
    }
    (void)CODEThreadPreemptOffCntAdd();
    ArrayRef pArray = PinArray(array);
#if defined(GENERAL_ASAN_SUPPORT_INTERFACE) || defined(CODIRA_GWPASAN_SUPPORT)
    return Sanitizer::ArrayAcquireMemoryRegion(pArray, pArray->ConvertToCArray(), pArray->GetContentSize());
#else
    return pArray->ConvertToCArray();
#endif
}

// Release the raw pointer
extern "C" void MCC_ReleaseRawData(ArrayRef array, void* rawPtr)
{
    if (!Heap::IsHeapAddress(array)) {
        return;
    }
    MRT_ASSERT(array->IsPrimitiveArray(), "Expect primitive array in MCC_ReleaseRawData");
#ifdef _WIN64
    static void* unreadablePage = reinterpret_cast<void*>(0x1234);
#else
    static void* unreadablePage = MutatorManager::Instance().GetSafepointPageManager()->GetUnreadablePage();
#endif
    MRT_ASSERT(unreadablePage != nullptr, "runtime is not initialized\n");
    if (UNLIKELY(array == nullptr || rawPtr == nullptr)) {
        return;
    }
    if (rawPtr == unreadablePage) {
        return;
    }
#if defined(GENERAL_ASAN_SUPPORT_INTERFACE) || defined(CODIRA_GWPASAN_SUPPORT)
    // sanitizer will convert alias/colorized pointer to real pointer for runtime
    rawPtr = Sanitizer::ArrayReleaseMemoryRegion(array, rawPtr, array->GetContentSize());
#endif
    auto regionInfo = RegionInfo::GetRegionInfoAt(reinterpret_cast<uintptr_t>(rawPtr));
    (void)regionInfo->DecRawPointerObjectCount();
    (void)CODEThreadPreemptOffCntSub();
}

enum LoadPackageStatus {
    LOAD_FAIL = 0,
    HAS_MULTI_PACKAGE = 1,
    // file with the same package name is loaded multiple times.
    LOAD_PACKAGE_REPEATED = 2,
    // file with the same name is loaded multiple times.
    LOAD_FILENAME_REPEATED = 3
};

extern "C" void* MCC_LoadPackage(const char* path)
{
    if (path == nullptr || *path == '\0') {
        return reinterpret_cast<void*>(LOAD_FAIL);
    }
    if (LoaderManager::GetInstance()->FileHasLoaded(path)) {
        return reinterpret_cast<void*>(LOAD_FILENAME_REPEATED);
    }
    if (LoadCODELibrary(path) != E_OK) {
        return reinterpret_cast<void*>(LOAD_FAIL);
    }
    if (LoaderManager::GetInstance()->GetPackageInfoByPath(path) == nullptr) {
        LoaderManager::GetInstance()->RemovePackageInfo(path);
        return reinterpret_cast<void*>(LOAD_PACKAGE_REPEATED);
    }
    if (LoaderManager::GetInstance()->FileHasMultiPackage(path)) {
        LoaderManager::GetInstance()->RemovePackageInfo(path);
        return reinterpret_cast<void*>(HAS_MULTI_PACKAGE);
    }
    if (InitCODELibrary(path) != E_OK) {
        return reinterpret_cast<void*>(LOAD_FAIL);
    }
    return LoaderManager::GetInstance()->GetPackageInfoByPath(path);
}

extern "C" PackageInfo* MCC_GetPackageByQualifiedName(const char* packageName)
{
    if (packageName == nullptr || *packageName == '\0') {
        return nullptr;
    }
    return LoaderManager::GetInstance()->GetPackageInfoByName(packageName);
}

extern "C" const char* MCC_GetPackageVersion(PackageInfo* packageInfo)
{
    return packageInfo->GetVersion();
}

extern "C" ObjectPtr MCC_GetSubPackages(PackageInfo* packageInfo, TypeInfo* arrayTi)
{
    std::vector<PackageInfo*> subPackages = {};
    LoaderManager::GetInstance()->GetSubPackages(packageInfo, subPackages);
    size_t subPkgCnt = subPackages.size();
    // Array<CPointer<Unit>> layout likes { Rarray<CPointer<Unit>>, Int64, Int64 }
    TypeInfo* rawArrayTi = arrayTi->GetFieldTypeInfo(0); // 0: first field type RawArray<CPointer<Unit>>.ti
    ArrayRef rawArrayObj = ObjectManager::NewKnownWidthArray(subPkgCnt, rawArrayTi,
        ObjectManager::ArrayElemBits::ELEM_64B, AllocType::RAW_POINTER_OBJECT);
    for (size_t idx = 0; idx < subPkgCnt; ++idx) {
        rawArrayObj->SetPrimitiveElement(idx, reinterpret_cast<int64_t>(subPackages[idx]));
    }
    U32 size = arrayTi->GetInstanceSize();
    MSize objSize = MRT_ALIGN(size + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
    MObject* obj = ObjectManager::NewObject(arrayTi, objSize, AllocType::RAW_POINTER_OBJECT);
    // set rawArray
    Heap::GetBarrier().WriteReference(obj, obj->GetRefField(TYPEINFO_PTR_SIZE), static_cast<BaseObject*>(rawArrayObj));
    CODEArray* codeArray = reinterpret_cast<CODEArray*>(reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE);
    codeArray->start = 0;
    codeArray->length = subPkgCnt;
    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    return obj;
}

// for package
extern "C" const char* MCC_GetPackageName(PackageInfo* packageInfo) { return packageInfo->GetPackageName(); }

extern "C" PackageInfo* MCC_GetRelatedPackageInfo(PackageInfo* packageInfo)
{
    return packageInfo->GetRelatedPackageInfo(); // todo
}

extern "C" U32 MCC_GetNumOfTypeInfos(PackageInfo* packageInfo) { return packageInfo->GetNumOfTypeInfos(); }

extern "C" TypeInfo* MCC_GetPackageTypeInfo(PackageInfo* packageInfo, U32 index)
{
    return packageInfo->GetTypeInfo(index);
}

extern "C" U32 MCC_GetPackageNumOfGlobalMethodInfos(PackageInfo* packageInfo)
{
    return packageInfo->GetNumOfGlobalMethodInfos();
}

extern "C" MethodInfo* MCC_GetPackageGlobalMethodInfo(PackageInfo* packageInfo, U32 index)
{
    return packageInfo->GetGlobalMethodInfo(index);
}

extern "C" U32 MCC_GetPackageNumOfGlobalFieldInfos(PackageInfo* packageInfo)
{
    return packageInfo->GetNumOfGlobalFieldInfos();
}

extern "C" StaticFieldInfo* MCC_GetPackageGlobalFieldInfo(PackageInfo* packageInfo, U32 index)
{
    return packageInfo->GetGlobalFieldInfo(index);
}

// for reflection, method
extern "C" const char* MCC_GetMethodName(MethodInfo* methodInfo) { return methodInfo->GetMethodName(); }

extern "C" TypeInfo* MCC_GetMethodReturnType(MethodInfo* methodInfo) { return methodInfo->GetReturnType(); }

extern "C" U32 MCC_GetMethodModifier(MethodInfo* methodInfo) { return methodInfo->GetModifier(); }

extern "C" bool MCC_MethodEntryPointIsNull(MethodInfo* methodInfo) { return methodInfo->MethodEntryPointIsNull(); }

extern "C" void* MCC_ApplyCODEInstanceMethod(MethodInfo* methodInfo, ObjRef obj, void* args)
{
    return methodInfo->ApplyCODEMethod(obj, nullptr, args, nullptr);
}

extern "C" void* MCC_ApplyCODEStaticMethod(MethodInfo* methodInfo, void* args, TypeInfo* thisTi)
{
    return methodInfo->ApplyCODEMethod(nullptr, nullptr, args, thisTi);
}

extern "C" void* MCC_ApplyCODEGenericInstanceMethod(MethodInfo* methodInfo, ObjRef obj, void* genericArgs, void* args)
{
    return methodInfo->ApplyCODEMethod(obj, genericArgs, args, nullptr);
}

extern "C" void* MCC_ApplyCODEGenericStaticMethod(MethodInfo* methodInfo, void* genericArgs,
                                                void* args, TypeInfo* thisTi)
{
    return methodInfo->ApplyCODEMethod(nullptr, genericArgs, args, thisTi);
}

extern "C" U32 MCC_GetNumOfActualParameters(MethodInfo* methodInfo)
{
    return methodInfo->GetNumOfActualParameterInfos();
}
extern "C" U32 MCC_GetNumOfGenericParameters(MethodInfo* methodInfo)
{
    return methodInfo->GetNumOfGenericParameterInfos();
}

extern "C" ParameterInfo* MCC_GetActualParameterInfo(MethodInfo* methodInfo, U32 index)
{
    return methodInfo->GetActualParameterInfo(index);
}

extern "C" GenericTypeInfo* MCC_GetGenericParameterInfo(MethodInfo* methodInfo, U32 index)
{
    return methodInfo->GetGenericParameterInfo(index);
}

extern "C" bool MCC_CheckMethodActualArgs(MethodInfo* methodInfo, void* genericArgs, void* args)
{
    return methodInfo->CheckMethodActualArgs(genericArgs, args);
}

extern "C" void* MCC_GetMethodAnnotations(MethodInfo* methodInfo, TypeInfo* arrayTi)
{
    return methodInfo->GetAnnotations(arrayTi);
}

static void RepalceLastDot(char* str)
{
    char *ptr1 = str;
    char *ptr2 = str;

    while (*ptr2 != '\0') {
        if (*ptr2 == '.') {
            ptr1 = ptr2;
        }
        if ((*ptr2 == '<' || *ptr2 == ',' || *ptr2 == '>' || *(ptr2 + 1) == '\0') && *ptr1 == '.') {
            *ptr1 = ':';
            ptr1 = ptr2; // reset ptr1 to ptr2's position
        }
        ptr2++;
    }
}

extern "C" TypeInfo* MCC_GetTypeByQualifiedName(char* qualifiedName)
{
    if (qualifiedName == nullptr || *qualifiedName == '\0') {
        return nullptr;
    }
    RepalceLastDot(qualifiedName);
    return LoaderManager::GetInstance()->FindTypeInfoFromLoadedFiles(qualifiedName);
}

// class/struct/interface
extern "C" TypeInfo* MCC_GetTypeByMangledName(const char* mangledName)
{
    if (mangledName == nullptr || *mangledName == '\0') {
        return nullptr;
    }
    return LoaderManager::GetInstance()->FindTypeInfoFromLoadedFiles(mangledName);
}

extern "C" TypeTemplate* MCC_GetTypeTemplate(char* name)
{
    if (name == nullptr || *name == '\0') {
        return nullptr;
    }
    RepalceLastDot(name);
    return LoaderManager::GetInstance()->FindTypeTemplateFromLoadedFiles(name);
}

static TypeInfo* GetActualTypeFromGenericType(GenericTypeInfo* genericTi, void* args, GenericTypeInfo* genericArgs)
{
    CODEArray* codeArray = static_cast<CODEArray*>(args);
    TypeInfo** actualTypeInfos = reinterpret_cast<TypeInfo**>(&(codeArray->rawPtr->data));
    U64 len = genericTi->GetGenericArgsNum();
    if (len == 0) {
        return nullptr;
    }
    void* mem = calloc(len, TYPEINFO_PTR_SIZE);
    CHECK_DETAIL(mem != nullptr, "GetActualTypeFromGenericType calloc failed");
    TypeInfo** typeInfos = static_cast<TypeInfo**>(mem);
    for (U64 idx = 0; idx < len; ++idx) {
        void* genericArg = genericTi->GetGenericArg(idx);
        for (U64 argIdx = 0; argIdx < codeArray->rawPtr->len; ++argIdx) {
            if (genericArg == genericArgs->GetGenericArg(argIdx)) {
                typeInfos[idx] = actualTypeInfos[argIdx];
            }
        }
    }
    TypeInfo* ti = TypeInfoManager::GetTypeInfoManager().GetOrCreateTypeInfo(
        genericTi->GetSourceGeneric(), len, typeInfos);
    free(mem);
    mem = nullptr;
    return ti;
}

static bool CheckGenericConstraint(GenericTypeInfo* genericTi, TypeInfo* ti, void* args, GenericTypeInfo* genericArgs)
{
    for (U64 constraintIdx = 0; constraintIdx < genericTi->GetGenericConstraintNum(); ++constraintIdx) {
        TypeInfo* constraintTi = genericTi->GetGenericConstraint(constraintIdx);
        if (constraintTi->IsGeneric()) {
            constraintTi =
                GetActualTypeFromGenericType(reinterpret_cast<GenericTypeInfo*>(constraintTi), args, genericArgs);
        }
        if (constraintTi == nullptr || !ti->IsSubType(constraintTi)) {
            return false;
        }
    }
    return true;
}

extern "C" TypeInfo* MCC_GetOrCreateTypeInfoForReflect(TypeTemplate* tt, void* args)
{
    if (tt == nullptr || args == nullptr) {
        return nullptr;
    }
    CODEArray* codeArray = static_cast<CODEArray*>(args);
    U64 len = codeArray->rawPtr->len;
    if (len == 0) {
        return nullptr;
    }

    if (tt->IsRawArray() || tt->IsVArray() || tt->IsCPointer()) {
        if (len != 1) {
            return nullptr;
        }
    } else {
        if (tt->GetTypeArgNum() != len) {
            return nullptr;
        }
    }

    Uptr base = reinterpret_cast<Uptr>(&(codeArray->rawPtr->data));
    void* mem = calloc(len, TYPEINFO_PTR_SIZE);
    TypeInfo** typeInfos = static_cast<TypeInfo**>(mem);
    CHECK_DETAIL(mem != nullptr, "MCC_GetOrCreateTypeInfoForReflect calloc failed");
    GenericTypeInfo* genericArgs = reinterpret_cast<GenericTypeInfo*>(
        tt->GetReflectInfo()->GetDeclaringGenericTypeInfo());
    for (U64 idx = 0; idx < len; ++idx) {
        typeInfos[idx] = *reinterpret_cast<TypeInfo**>(base + idx * TYPEINFO_PTR_SIZE);
        GenericTypeInfo* genericArg = static_cast<GenericTypeInfo*>(genericArgs->GetGenericArg(idx));
        if (!CheckGenericConstraint(genericArg, typeInfos[idx], args, genericArgs)) {
            free(mem);
            mem = nullptr;
            return nullptr;
        }
    }
    TypeInfo* ti = TypeInfoManager::GetTypeInfoManager().GetOrCreateTypeInfo(tt, len, typeInfos);
    free(mem);
    mem = nullptr;
    return ti;
}

extern "C" const char* MCC_GetTypeName(TypeInfo* ti) { return ti->GetName(); }

extern "C" bool MCC_IsInterface(TypeInfo* ti) { return ti->IsInterface(); }

extern "C" bool MCC_IsClass(TypeInfo* ti) { return ti->IsClass(); }

extern "C" bool MCC_IsStruct(TypeInfo* ti) { return ti->IsStruct(); }

extern "C" bool MCC_IsPrimitive(TypeInfo* ti) { return ti->IsPrimitiveType(); }

extern "C" bool MCC_IsGeneric(TypeInfo* ti) { return ti->IsGeneric(); }

extern "C" bool MCC_IsReflectUnsupportedType(TypeInfo* ti) { return ti->IsReflectUnsupportedType(); }

// @deprecated
extern "C" U32 MCC_GetQualifiedNameLength(TypeInfo* ti) { return 0; }
// @deprecated
extern "C" void MCC_GetQualifiedName(TypeInfo* ti, char* qualifiedName) {}

// member functions
extern "C" U32 MCC_GetNumOfInstanceMethodInfos(TypeInfo* ti)
{
    return ti->GetNumOfInstanceMethodInfos();
}

extern "C" MethodInfo* MCC_GetInstanceMethodInfo(TypeInfo* ti, U32 index)
{
    return ti->GetInstanceMethodInfo(index);
}

// static member functions
extern "C" U32 MCC_GetNumOfStaticMethodInfos(TypeInfo* ti)
{
    return ti->GetNumOfStaticMethodInfos();
}

extern "C" MethodInfo* MCC_GetStaticMethodInfo(TypeInfo* ti, U32 index)
{
    return ti->GetStaticMethodInfo(index);
}

// member variables
extern "C" U32 MCC_GetNumOfInstanceFieldInfos(TypeInfo* ti)
{
    return ti->GetNumOfInstanceFieldInfos();
}

extern "C" InstanceFieldInfo* MCC_GetInstanceFieldInfo(TypeInfo* ti, U32 index)
{
    return ti->GetInstanceFieldInfo(index);
}

// static member functions
extern "C" U32 MCC_GetNumOfStaticFieldInfos(TypeInfo* ti)
{
    return ti->GetNumOfStaticFieldInfos();
}

extern "C" StaticFieldInfo* MCC_GetStaticFieldInfo(TypeInfo* ti, U32 index)
{
    return ti->GetStaticFieldInfo(index);
}

extern "C" U32 MCC_GetTypeInfoModifier(TypeInfo* ti) { return ti->GetModifier(); }


extern "C" TypeInfo* MCC_GetSuperTypeInfo(TypeInfo* ti) { return ti->GetSuperTypeInfo(); }

extern "C" U32 MCC_GetNumOfInterface(TypeInfo* ti)
{
    return LoaderManager::GetInstance()->GetNumOfInterface(ti);
}

extern "C" TypeInfo* MCC_GetInterface(TypeInfo* ti, U32 idx)
{
    return LoaderManager::GetInstance()->GetInterface(ti, idx);
}

// field
extern "C" const char* MCC_GetInstanceFieldName(InstanceFieldInfo* fieldInfo, TypeInfo* declaringTypeInfo)
{
    return fieldInfo->GetName(declaringTypeInfo);
}

extern "C" const char* MCC_GetStaticFieldName(StaticFieldInfo* fieldInfo) { return fieldInfo->GetName(); }

extern "C" TypeInfo* MCC_GetInstanceFieldType(InstanceFieldInfo* fieldInfo, TypeInfo* declaringTypeInfo)
{
    return fieldInfo->GetFieldType(declaringTypeInfo);
}

extern "C" TypeInfo* MCC_GetStaticFieldType(StaticFieldInfo* fieldInfo) { return fieldInfo->GetFieldType(); }

extern "C" U32 MCC_GetInstanceFieldModifier(InstanceFieldInfo* fieldInfo) { return fieldInfo->GetModifier(); }

extern "C" U32 MCC_GetStaticFieldModifier(StaticFieldInfo* fieldInfo) { return fieldInfo->GetModifier(); }

extern "C" void* MCC_GetInstanceFieldValue(InstanceFieldInfo* fieldInfo, TypeInfo* declaringTypeInfo, ObjRef obj)
{
    return fieldInfo->GetValue(declaringTypeInfo, obj);
}

extern "C" void MCC_SetInstanceFieldValue(InstanceFieldInfo* fieldInfo, TypeInfo* declaringTypeInfo,
                                          ObjRef obj, ObjRef newValue)
{
    fieldInfo->SetValue(declaringTypeInfo, obj, newValue);
}

extern "C" void* MCC_GetStaticFieldValue(StaticFieldInfo* fieldInfo) { return fieldInfo->GetValue(); }

extern "C" void MCC_SetStaticFieldValue(StaticFieldInfo* fieldInfo, ObjRef newValue)
{
    fieldInfo->SetValue(newValue);
}

extern "C" void* MCC_GetInstanceFieldAnnotations(InstanceFieldInfo* fieldInfo, TypeInfo* arrayTi)
{
    return fieldInfo->GetAnnotations(arrayTi);
}

extern "C" void* MCC_GetStaticFieldAnnotations(StaticFieldInfo* fieldInfo, TypeInfo* arrayTi)
{
    return fieldInfo->GetAnnotations(arrayTi);
}

// for reflect, parameter
extern "C" const char* MCC_GetParameterName(ParameterInfo* parameterInfo) { return parameterInfo->GetName(); }

extern "C" U32 MCC_GetParameterIndex(ParameterInfo* parameterInfo) { return parameterInfo->GetIndex(); }

extern "C" TypeInfo* MCC_GetParameterType(ParameterInfo* parameterInfo) { return parameterInfo->GetType(); }

extern "C" void* MCC_GetParameterAnnotations(ParameterInfo* parameterInfo, TypeInfo* arrayTi)
{
    return parameterInfo->GetAnnotations(arrayTi);
}

extern "C" ObjectPtr CODE_MCC_ReadRefField(const ObjectPtr obj, RefField<false>* field)
{
    return Heap::GetHeap().GetBarrier().ReadReference(obj, *field);
}

extern "C" ObjectPtr CODE_MCC_ReadWeakRef(const ObjectPtr obj, RefField<false>* field)
{
    return Heap::GetHeap().GetBarrier().ReadWeakRef(obj, *field);
}
extern "C" void CODE_MCC_ReadStructField(MAddress dstPtr, ObjectPtr obj, MAddress srcField, size_t size)
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    if (Heap::IsHeapAddress((void*)dstPtr)) {
        LOG(RTLOG_FATAL, "dstPtr is in heap");
    }
#endif
    if (size == 0) {
        return;
    }
    Heap::GetHeap().GetBarrier().ReadStruct(dstPtr, obj, srcField, size);
}
extern "C" ObjectPtr CODE_MCC_ReadStaticRef(RefField<false>* field)
{
    return Heap::GetHeap().GetBarrier().ReadStaticRef(*field);
}
extern "C" void CODE_MCC_ReadStaticStruct(MAddress dstPtr, size_t dstSize, MAddress srcPtr, size_t srcSize, GCTib gctib)
{
    Heap::GetHeap().GetBarrier().ReadStaticStruct(dstPtr, srcPtr, dstSize, gctib);
}
extern "C" void* MCC_GetTypeInfoAnnotations(TypeInfo* cls, TypeInfo* arrayTi) { return cls->GetAnnotations(arrayTi); }

// for generic
extern "C" TypeInfo* CODE_MCC_GetOrCreateTypeInfo(TypeTemplate* typeTemplate, U32 argSize, TypeInfo* typeArgs[])
{
    return TypeInfoManager::GetTypeInfoManager().GetOrCreateTypeInfo(typeTemplate, argSize, typeArgs);
}

extern "C" bool CODE_MCC_IsSubType(TypeInfo* typeInfo, TypeInfo* superTypeInfo)
{
    if (typeInfo == nullptr || superTypeInfo == nullptr) {
        return false;
    }
    if (typeInfo == superTypeInfo) {
        return true;
    }
    
    bool isSub = typeInfo->IsSubType(superTypeInfo);
    return isSub;
}

static bool IsTupleTypeOf(ObjectPtr obj, TypeInfo* typeInfo, TypeInfo* targetTypeInfo)
{
    // targetTypeInfo may not be of the tuple type. In this case, false is returned.
    if (!targetTypeInfo->IsTuple()) {
        return false;
    }
    TypeInfo* ti = typeInfo;
    U32 base = 0;
    if (typeInfo == nullptr) {
        ti = obj->GetTypeInfo();
        // 8: typeInfo offset
        base = 8;
    }
    if (ti == nullptr) {
        LOG(RTLOG_FATAL, "IsTupleTypeOf: get typeInfo failed");
    }
    if (ti->GetFieldNum() != targetTypeInfo->GetFieldNum()) {
        return false;
    }
    for (U16 idx = 0; idx < ti->GetFieldNum(); ++idx) {
        TypeInfo* fieldTypeInfo = ti->GetFieldType(idx);
        TypeInfo* fieldTargetTI = targetTypeInfo->GetFieldType(idx);
        U32 offset = ti->GetFieldOffsets(idx) + base;
        ObjectPtr curObj = nullptr;
        if (fieldTargetTI->IsRef()) {
            if (!fieldTypeInfo->IsClass() && !fieldTypeInfo->IsInterface()) {
                return false;
            }
            if (Heap::IsHeapAddress(obj)) {
                curObj = Heap::GetBarrier().ReadReference(obj, obj->GetRefField(offset));
            } else {
                curObj = obj->GetRefField(offset).GetTargetObject();
            }
            TypeInfo* curti = curObj->GetTypeInfo();
            if (!curti->IsSubType(fieldTargetTI)) {
                return false;
            }
        } else if (fieldTargetTI->IsTuple()) {
            curObj = reinterpret_cast<ObjectPtr>(reinterpret_cast<Uptr>(obj) + offset);
            if (!IsTupleTypeOf(curObj, fieldTypeInfo, fieldTargetTI)) {
                return false;
            }
        } else if (!fieldTypeInfo->IsSubType(fieldTargetTI)) {
            return false;
        }
    }
    return true;
}

extern "C" bool CODE_MCC_IsTupleTypeOf(ObjectPtr obj, TypeInfo* typeInfo, TypeInfo* targetTypeInfo)
{
    if (obj == nullptr || targetTypeInfo == nullptr) {
        return false;
    }
    return IsTupleTypeOf(obj, typeInfo, targetTypeInfo);
}

extern "C" void CODE_MCC_WriteGeneric(const ObjectPtr obj, void* fieldPtr, const ObjectPtr src, size_t size)
{
    if (src == nullptr || size == 0) {
        return;
    }
    Heap::GetBarrier().WriteGeneric(obj, fieldPtr, src, size);
}

extern "C" void CODE_MCC_AssignGeneric(ObjectPtr dst, ObjectPtr src, TypeInfo* typeInfo)
{
    size_t instanceSize = typeInfo->GetInstanceSize();
    if (instanceSize == 0) {
        return;
    }
    if (!typeInfo->HasRefField()) {
        CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(dst) + TYPEINFO_PTR_SIZE),
                              instanceSize,
                              reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(src) + TYPEINFO_PTR_SIZE),
                              instanceSize) == EOK,
                     "MCC_AssignGeneric memcpy_s failed");
    } else {
        MAddress dstAddr = reinterpret_cast<MAddress>(dst) + TYPEINFO_PTR_SIZE;
        Heap::GetBarrier().WriteGeneric(dst, reinterpret_cast<void*>(dstAddr), src, instanceSize);
    }
}

extern "C" void CODE_MCC_WriteGenericPayload(ObjectPtr dst, MAddress srcField, size_t srcSize)
{
    TypeInfo* typeInfo = dst->GetTypeInfo();
    if (srcSize == 0) {
        return;
    }

    if (!typeInfo->HasRefField()) {
        CHECK_DETAIL(memcpy_s(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(dst) + TYPEINFO_PTR_SIZE),
                              GENERIC_PAYLOAD_SIZE,
                              reinterpret_cast<void*>(srcField),
                              srcSize) == EOK,
                     "MCC_WriteGenericPayload memcpy_s failed");
    } else {
        MAddress dstAddr = reinterpret_cast<MAddress>(dst) + TYPEINFO_PTR_SIZE;
        Heap::GetBarrier().WriteStruct(dst, dstAddr, srcSize, srcField, srcSize);
    }
}

extern "C" void CODE_MCC_ReadGeneric(const ObjectPtr dstPtr, ObjectPtr obj, void* fieldPtr, size_t size)
{
    if (size == 0) {
        return;
    }
    Heap::GetBarrier().ReadGeneric(dstPtr, obj, fieldPtr, size);
}

extern "C" FuncPtr* CODE_MCC_GetMTable(TypeInfo* ti, TypeInfo* itf)
{
    return ti->GetMTable(itf);
}

extern "C" TypeInfo* CODE_MCC_GetMethodOuterTI(TypeInfo* ti, TypeInfo* itf, U64 index)
{
    return ti->GetMethodOuterTI(itf, index);
}

extern "C" void CODE_MCC_UpdateVMT(TypeInfo* ti, TypeInfo* itf, ExtensionData* extensionData)
{
    if (UNLIKELY(!extensionData->IsFuncTableUpdated())) {
        return ti->TryUpdateExtensionData(itf, extensionData);
    }
}

extern "C" ObjRef MCC_NewGenericObject(const TypeInfo* klass, MSize size)
{
    ObjRef obj = ObjectManager::NewObject(klass, size);
    if (obj == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("ObjectManager::NewObject return nullptr");
    }
    return obj;
}

extern "C" ArrayRef MCC_NewArrayGeneric(const TypeInfo* arrayInfo, MIndex nElems)
{
    ArrayRef array = nullptr;
    if (!arrayInfo->IsArrayType()) {
        return array;
    }
    TypeInfo* componentTypeInfo = arrayInfo->GetComponentTypeInfo();
    I8 type = componentTypeInfo->GetType();
    switch (type) {
        case TypeKind::TYPE_KIND_CLASS:
        case TypeKind::TYPE_KIND_EXPORTED_REF:
        case TypeKind::TYPE_KIND_FOREIGN_PROXY:
        case TypeKind::TYPE_KIND_WEAKREF_CLASS:
        case TypeKind::TYPE_KIND_INTERFACE:
        case TypeKind::TYPE_KIND_TEMP_ENUM:
        case TypeKind::TYPE_KIND_RAWARRAY:
        case TypeKind::TYPE_KIND_FUNC: {
            array = ObjectManager::NewObjArray(nElems, arrayInfo);
            break;
        }
        case TypeKind::TYPE_KIND_UNIT:
        case TypeKind::TYPE_KIND_NOTHING:
        case TypeKind::TYPE_KIND_VARRAY:
        case TypeKind::TYPE_KIND_TUPLE:
        case TypeKind::TYPE_KIND_STRUCT:
        case TypeKind::TYPE_KIND_ENUM: {
            array = ObjectManager::NewArray(nElems, arrayInfo);
            break;
        }
        case TypeKind::TYPE_KIND_BOOL:
        case TypeKind::TYPE_KIND_UINT8:
        case TypeKind::TYPE_KIND_INT8: {
            array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_8B);
            break;
        }
        case TypeKind::TYPE_KIND_UINT16:
        case TypeKind::TYPE_KIND_INT16:
        case TypeKind::TYPE_KIND_FLOAT16: {
            array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_16B);
            break;
        }
        case TypeKind::TYPE_KIND_UINT32:
        case TypeKind::TYPE_KIND_INT32:
        case TypeKind::TYPE_KIND_RUNE:
        case TypeKind::TYPE_KIND_FLOAT32: {
            array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_32B);
            break;
        }
        case TypeKind::TYPE_KIND_INT_NATIVE:
        case TypeKind::TYPE_KIND_UINT_NATIVE:
        case TypeKind::TYPE_KIND_UINT64:
        case TypeKind::TYPE_KIND_INT64:
        case TypeKind::TYPE_KIND_FLOAT64:
        case TypeKind::TYPE_KIND_CSTRING:
        case TypeKind::TYPE_KIND_CPOINTER:
        case TypeKind::TYPE_KIND_CFUNC: {
            array = ObjectManager::NewKnownWidthArray(nElems, arrayInfo, ObjectManager::ArrayElemBits::ELEM_64B);
            break;
        }
        default:
            break;
    }
    if (array == nullptr) {
        ExceptionManager::CheckAndThrowPendingException("NewArrayGeneric return nullptr");
    }
    return array;
}

extern "C" void CODE_MCC_ArrayCopyGeneric(const ObjectPtr dstObj, MAddress dstField, size_t dstSize,
                                        const ObjectPtr srcObj, MAddress srcField, size_t srcSize)
{
    if (dstSize == 0) {
        return;
    }
    MRT_ASSERT(dstSize <= SECUREC_MEM_MAX_LEN, "size too big in MCC_ArrayCopyGeneric");
    TypeInfo* arrayInfo = srcObj->GetTypeInfo();

    TypeInfo* componentTypeInfo = arrayInfo->GetComponentTypeInfo();
    I8 type = componentTypeInfo->GetType();
    if (componentTypeInfo->IsStructType() && !componentTypeInfo->HasRefField()) {
        CHECK_DETAIL(memmove_s(reinterpret_cast<void*>(dstField), dstSize,
                               reinterpret_cast<void*>(srcField), srcSize) == EOK,
                     "MCC_ArrayCopyGeneric memmove_s failed");
        return;
    }
    switch (type) {
        case TypeKind::TYPE_KIND_CLASS:
        case TypeKind::TYPE_KIND_EXPORTED_REF:
        case TypeKind::TYPE_KIND_FOREIGN_PROXY:
        case TypeKind::TYPE_KIND_WEAKREF_CLASS:
        case TypeKind::TYPE_KIND_INTERFACE:
        case TypeKind::TYPE_KIND_TEMP_ENUM:
        case TypeKind::TYPE_KIND_RAWARRAY:
        case TypeKind::TYPE_KIND_FUNC: {
            Heap::GetBarrier().CopyRefArray(dstObj, dstField, dstSize, srcObj, srcField, srcSize);
            break;
        }
        case TypeKind::TYPE_KIND_UNIT:
        case TypeKind::TYPE_KIND_BOOL:
        case TypeKind::TYPE_KIND_INT8:
        case TypeKind::TYPE_KIND_UINT8:
        case TypeKind::TYPE_KIND_NOTHING:
        case TypeKind::TYPE_KIND_UINT16:
        case TypeKind::TYPE_KIND_INT16:
        case TypeKind::TYPE_KIND_FLOAT16:
        case TypeKind::TYPE_KIND_UINT32:
        case TypeKind::TYPE_KIND_INT32:
        case TypeKind::TYPE_KIND_RUNE:
        case TypeKind::TYPE_KIND_FLOAT32:
        case TypeKind::TYPE_KIND_UINT64:
        case TypeKind::TYPE_KIND_INT64:
        case TypeKind::TYPE_KIND_FLOAT64:
        case TypeKind::TYPE_KIND_INT_NATIVE:
        case TypeKind::TYPE_KIND_UINT_NATIVE:
        case TypeKind::TYPE_KIND_CSTRING:
        case TypeKind::TYPE_KIND_CPOINTER:
        case TypeKind::TYPE_KIND_CFUNC:
        case TypeKind::TYPE_KIND_VARRAY: {
            CHECK_DETAIL(memmove_s(reinterpret_cast<void*>(dstField), dstSize,
                                   reinterpret_cast<void*>(srcField), srcSize) == EOK,
                         "MCC_ArrayCopyGeneric memmove_s failed");
            break;
        }
        case TypeKind::TYPE_KIND_TUPLE:
        case TypeKind::TYPE_KIND_STRUCT:
        case TypeKind::TYPE_KIND_ENUM: {
            Heap::GetBarrier().CopyStructArray(dstObj, dstField, dstSize, srcObj, srcField, srcSize);
            break;
        }
        default:
            break;
    }
}

extern "C" void CODE_MCC_CopyStructField(BaseObject* dstBase, void* dstField, size_t dstLen,
                                       BaseObject* srcBase, void* srcField, size_t srcLen) {}

extern "C" int32_t __ccc_personality_v0() { return 0; }
// @deprecated
extern "C" void CODE_MCC_IVCallInstrumentation(TypeInfo* cls, const char* callBaseKey) {}

void CODE_MCC_CrossAccessBarrier(U64 codeExport)
{
    Heap::GetHeap().CrossAccessBarrier(codeExport);
}

U64 CODE_MCC_CreateExportHandle(BaseObject *obj)
{
    U64 id = Heap::GetHeap().RegisterExportRoot(obj);
    return id;
}

BaseObject* CODE_MCC_GetExportedRef(U64 id)
{
    return Heap::GetHeap().GetExportObject(id);
}
void CODE_MCC_RemoveExportedRef(U64 id)
{
    Heap::GetHeap().RemoveExportObject(id);
}


#if defined(__OHOS__)
void* ARKTS_CreateEngine = nullptr;
void* ARKTS_UpdateStackInfo = nullptr;
void* ARKTS_GetContext = nullptr;
bool g_hasFindArkTSFunctions = false;
bool FindArkTSFunctions()
{
    static SpinLock spinLock;
    ScopedEnterSpinLock scopedEnterSpinLock(spinLock);
    if (g_hasFindArkTSFunctions) {
        return true;
    }
    void* handler = dlopen("libark_interop.z.so", RTLD_NOW | RTLD_GLOBAL);
    if (handler == nullptr) {
        return false;
    }
    ARKTS_CreateEngine = dlsym(handler, "ARKTS_CreateEngine");
    if (ARKTS_CreateEngine == nullptr) {
        return false;
    }
    ARKTS_UpdateStackInfo = dlsym(handler, "ARKTS_UpdateStackInfo");
    if (ARKTS_UpdateStackInfo == nullptr) {
        return false;
    }
    ARKTS_GetContext = dlsym(handler, "ARKTS_GetContext");
    if (ARKTS_GetContext == nullptr) {
        return false;
    }
    g_hasFindArkTSFunctions = true;
    return true;
}

extern "C" void* CODE_MRT_ARKTS_CreateEngine()
{
    if (!FindArkTSFunctions()) {
        LOG(RTLOG_ERROR, "Failed to find arkts functions.");
        return nullptr;
    }
    auto res = ((void* (*)())ARKTS_CreateEngine)();
    if (res == nullptr) {
        LOG(RTLOG_ERROR, "Failed to create arkts engine.");
        return res;
    }
    auto arkVm = ((unsigned long long (*)(void*))ARKTS_GetContext)(res);
    if (arkVm == 0) {
        LOG(RTLOG_ERROR, "Failed to get arkvm.");
        return res;
    }
    RegisterArkVMInRuntime(arkVm);
    RegisterStackInfoCallbacks(((UpdateStackInfoFunc)ARKTS_UpdateStackInfo));
    if (!IsForeignThread()) {
        UpdateArkVMStackInfo(arkVm);
    }
    return res;
}
#endif
} // namespace MapleRuntime
