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

#if !defined(NDEBUG) || defined(FASTVERIFY)
#define VERBOSE_THREAD_STATE_LOG true
#endif

#include "runtime/include/thread-inl.h"
#include "libarkbase/os/stacktrace.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/locks.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/safepoint_timer.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/object_helpers.h"
#include "tooling/pt_thread_info.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include <sys/time.h>

namespace ark {
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;

mem::TLAB *ManagedThread::zeroTlab_ = nullptr;
static const int MIN_PRIORITY = os::thread::LOWEST_PRIORITY;
static constexpr int MICROSEC_CONVERSION = 1000000;

static mem::InternalAllocatorPtr GetInternalAllocator(Thread *thread)
{
    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have HeapManager, so we get internal allocator from
    // runtime
    mem::HeapManager *heapManager = thread->GetVM()->GetHeapManager();
    if (heapManager != nullptr) {
        return heapManager->GetInternalAllocator();
    }
    return Runtime::GetCurrent()->GetInternalAllocator();
}

ManagedThread::ThreadId ManagedThread::GetInternalId()
{
    ASSERT(internalId_ != 0);
    return internalId_;
}

Thread::~Thread()
{
    FreeAllocatedMemory();
}

void Thread::FreeInternalMemory()
{
    FreeAllocatedMemory();
}

void Thread::FreeAllocatedMemory()
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    ASSERT(allocator != nullptr);
    allocator->Delete(preBuff_);
    preBuff_ = nullptr;

#ifdef PANDA_USE_CUSTOM_SIGNAL_STACK
    allocator->Free(signalStack_.ss_sp);
#endif
}

Thread::Thread(PandaVM *vm, ThreadType threadType) : ThreadProxy(vm->GetMutatorLock()), vm_(vm), threadType_(threadType)
{
    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have GC, so we skip setting barriers for this case
    mem::GC *gc = vm->GetGC();
    if (gc != nullptr) {
        barrierSet_ = vm->GetGC()->GetBarrierSet();
        InitCardTableData(barrierSet_);
    }
    InitializeThreadFlag();
    InitThreadRandomState();
#ifdef PANDA_USE_CUSTOM_SIGNAL_STACK
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    signalStack_.ss_sp = allocator->Alloc(SIGSTKSZ * 8U);
    signalStack_.ss_size = SIGSTKSZ * 8U;
    signalStack_.ss_flags = 0;
    sigaltstack(&signalStack_, nullptr);
#endif

    entrypointsTable_ = Runtime::GetCurrent()->GetEntrypointsTable();
}

void Thread::InitThreadRandomState()
{
    struct timeval tv = {0, 0};
    gettimeofday(&tv, nullptr);
    threadRandomState_ = static_cast<uint64_t>(tv.tv_sec * MICROSEC_CONVERSION + tv.tv_usec);
}

void Thread::InitCardTableData(mem::GCBarrierSet *barrier)
{
    auto postBarrierType = barrier->GetPostType();
    switch (postBarrierType) {
        case ark::mem::BarrierType::POST_INTERREGION_BARRIER:
            cardTableAddr_ = std::get<uint8_t *>(barrier->GetPostBarrierOperand("CARD_TABLE_ADDR").GetValue());
            cardTableMinAddr_ = std::get<void *>(barrier->GetPostBarrierOperand("MIN_ADDR").GetValue());
            postWrbOneObject_ = reinterpret_cast<void *>(PostInterRegionBarrierMarkSingleFast);
            postWrbTwoObjects_ = reinterpret_cast<void *>(PostInterRegionBarrierMarkPairFast);
            break;
        case ark::mem::BarrierType::POST_WRB_NONE:
            postWrbOneObject_ = reinterpret_cast<void *>(EmptyPostWriteBarrier);
            postWrbTwoObjects_ = reinterpret_cast<void *>(EmptyPostWriteBarrier);
            break;
        case mem::POST_RB_NONE:
        case ark::mem::BarrierType::POST_CMC_WRITE_BARRIER:
            break;
        case mem::PRE_WRB_NONE:
        case mem::PRE_RB_NONE:
        case mem::PRE_SATB_BARRIER:
        case ark::mem::BarrierType::PRE_CMC_READ_BARRIER:
            LOG(FATAL, RUNTIME) << "Post barrier expected";
            break;
    }
}

void Thread::InitPreBuff()
{
    auto allocator = GetInternalAllocator(this);
    mem::GC *gc = GetVM()->GetGC();
    auto barrier = gc->GetBarrierSet();
    if (barrier->GetPreType() != ark::mem::BarrierType::PRE_WRB_NONE) {
        preBuff_ = allocator->New<PandaVector<ObjectHeader *>>();
    }
}

/* static */
void ManagedThread::Initialize()
{
    ASSERT(!Thread::GetCurrent());
    ASSERT(!zeroTlab_);
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    zeroTlab_ = allocator->New<mem::TLAB>(nullptr, 0U);
    InitializeInitThreadFlag();
}

/* static */
void ManagedThread::Shutdown()
{
    ASSERT(zeroTlab_);
    ManagedThread::SetCurrent(nullptr);
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(zeroTlab_);
    zeroTlab_ = nullptr;
    /* @sync 1
     * @description: Runtime is terminated at this point and we cannot create new threads
     * */
}

/* static */
void MTManagedThread::Yield()
{
    LOG(DEBUG, RUNTIME) << "Reschedule the execution of a current thread";
    os::thread::Yield();
}

/* static - creation of the initial Managed thread */
ManagedThread *ManagedThread::Create(Runtime *runtime, PandaVM *vm, ark::panda_file::SourceLang threadLang)
{
    trace::ScopedTrace scopedTrace("ManagedThread::Create");
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    // Create thread structure using new, we rely on this structure to be accessible in child threads after
    // runtime is destroyed
    return new ManagedThread(os::thread::GetCurrentThreadId(), allocator, vm, Thread::ThreadType::THREAD_TYPE_MANAGED,
                             threadLang);
}

/* static - creation of the initial MT Managed thread */
MTManagedThread *MTManagedThread::Create(Runtime *runtime, PandaVM *vm, ark::panda_file::SourceLang threadLang)
{
    trace::ScopedTrace scopedTrace("MTManagedThread::Create");
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    // Create thread structure using new, we rely on this structure to be accessible in child threads after
    // runtime is destroyed
    auto thread = new MTManagedThread(os::thread::GetCurrentThreadId(), allocator, vm, threadLang);
    thread->ProcessCreatedThread();

    runtime->GetNotificationManager()->ThreadStartEvent(thread);

    return thread;
}

ManagedThread::ManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *pandaVm,
                             Thread::ThreadType threadType, ark::panda_file::SourceLang threadLang)
    : Thread(pandaVm, threadType),
      id_(id),
      threadLang_(threadLang),
      ptThreadInfo_(allocator->New<tooling::PtThreadInfo>()),
      threadFrameStates_(allocator->Adapter())
{
    ASSERT(zeroTlab_ != nullptr);
    tlab_ = zeroTlab_;

    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have GC, so we skip setting barriers for this case
    mem::GC *gc = pandaVm->GetGC();
    if (gc != nullptr) {
        preBarrierType_ = gc->GetBarrierSet()->GetPreType();
        postBarrierType_ = gc->GetBarrierSet()->GetPostType();
        auto barrierSet = gc->GetBarrierSet();
        if (barrierSet->GetPreType() != ark::mem::BarrierType::PRE_WRB_NONE) {
            preBuff_ = allocator->New<PandaVector<ObjectHeader *>>();
            // need to initialize in constructor because we have barriers between constructor and InitBuffers in
            // InitializedClasses
            g1PostBarrierRingBuffer_ = allocator->New<mem::GCG1BarrierSet::G1PostBarrierRingBufferType>();
        }
    }

    stackFrameAllocator_ =
        allocator->New<mem::StackFrameAllocator>(Runtime::GetOptions().UseMallocForInternalAllocations());
    internalLocalAllocator_ =
        mem::InternalAllocator<>::SetUpLocalInternalAllocator(static_cast<mem::Allocator *>(allocator));
    taggedHandleStorage_ = allocator->New<HandleStorage<TaggedType>>(allocator);
    taggedGlobalHandleStorage_ = allocator->New<GlobalHandleStorage<TaggedType>>(allocator);
    objectHeaderHandleStorage_ = allocator->New<HandleStorage<ObjectHeader *>>(allocator);
    if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
        constexpr size_t MAX_GROW_RATIO = 2;
        constexpr float WEIGHT = 0.5;
        constexpr float DESIRED_FILL_FRACTION = 0.9;
        size_t initTlabSize = Runtime::GetOptions().GetInitTlabSize();
        size_t maxTlabSize = Runtime::GetOptions().GetMaxTlabSize();
        if (initTlabSize < 4_KB) {
            LOG(FATAL, RUNTIME) << "Initial TLAB size must be greater than 4Kb";
        }
        if (initTlabSize > maxTlabSize) {
            LOG(FATAL, RUNTIME) << "Initial TLAB size must be less or equal to max TLAB size";
        }
        weightedAdaptiveTlabAverage_ = allocator->New<WeightedAdaptiveTlabAverage>(
            initTlabSize, maxTlabSize, MAX_GROW_RATIO, WEIGHT, DESIRED_FILL_FRACTION);
    }
}

ManagedThread::~ManagedThread()
{
    // ManagedThread::ShutDown() may not be called when exiting js_thread, so need set current_thread = nullptr
    // NB! ThreadManager is expected to store finished threads in separate list and GC destroys them,
    // current_thread should be nullified in Destroy()
    // We should register TLAB size for MemStats during thread destroy.
    // (zero_tlab == nullptr means that we destroyed Runtime and do not need to register TLAB)
    if (zeroTlab_ != nullptr) {
        ASSERT(tlab_ == zeroTlab_);
    }

    mem::InternalAllocatorPtr allocator = GetInternalAllocator(this);
    allocator->Delete(objectHeaderHandleStorage_);
    allocator->Delete(taggedGlobalHandleStorage_);
    allocator->Delete(taggedHandleStorage_);
    allocator->Delete(weightedAdaptiveTlabAverage_);
    mem::InternalAllocator<>::FinalizeLocalInternalAllocator(internalLocalAllocator_,
                                                             static_cast<mem::Allocator *>(allocator));
    internalLocalAllocator_ = nullptr;
    allocator->Delete(stackFrameAllocator_);
    allocator->Delete(ptThreadInfo_.release());

#if defined(VERBOSE_THREAD_STATE_LOG)
    ASSERT(threadFrameStates_.empty() && "stack should be empty");
#endif
}

void ManagedThread::InitBuffers()
{
    auto allocator = GetInternalAllocator(this);
    mem::GC *gc = GetVM()->GetGC();
    auto barrier = gc->GetBarrierSet();
    if (barrier->GetPreType() != ark::mem::BarrierType::PRE_WRB_NONE) {
        // we need to recreate buffers if it was detach (we removed all structures) and attach again
        // skip initializing in first attach after constructor
        if (preBuff_ == nullptr) {
            ASSERT(preBuff_ == nullptr);
            preBuff_ = allocator->New<PandaVector<ObjectHeader *>>();
            ASSERT(g1PostBarrierRingBuffer_ == nullptr);
            g1PostBarrierRingBuffer_ = allocator->New<mem::GCG1BarrierSet::G1PostBarrierRingBufferType>();
        }
    }
}

NO_INLINE static uintptr_t GetStackTop()
{
    return ToUintPtr(__builtin_frame_address(0));
}

NO_INLINE static void LoadStackPages(uintptr_t endAddr)
{
    // ISO C++ forbids variable length array and alloca is unsafe,
    // so we have to extend stack step by step via recursive call
    constexpr size_t MARGIN = 512;
    constexpr size_t STACK_PAGE_SIZE = 4_KB;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    volatile uint8_t stackBuffer[STACK_PAGE_SIZE - MARGIN];
    if (ToUintPtr(&(stackBuffer[0])) >= endAddr + STACK_PAGE_SIZE) {
        LoadStackPages(endAddr);
    }
    stackBuffer[0] = 0;
}

bool ManagedThread::RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize)
{
    int error = os::thread::ThreadGetStackInfo(os::thread::GetNativeHandle(), &stackAddr, &stackSize, &guardSize);
    if (error != 0) {
        LOG(ERROR, RUNTIME) << "RetrieveStackInfo: fail to get stack info, error = " << strerror(errno);
    }
    return error == 0;
}

void ManagedThread::InitForStackOverflowCheck(size_t nativeStackReservedSize, size_t nativeStackProtectedSize)
{
    void *stackBase = nullptr;
    size_t guardSize;
    size_t stackSize;
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON) || !defined(NDEBUG)
    static constexpr size_t RESERVED_SIZE = 64_KB;
#else
    static constexpr size_t RESERVED_SIZE = 12_KB;
#endif
    static_assert(STACK_OVERFLOW_RESERVED_SIZE == RESERVED_SIZE);  // compiler depends on this to test load!!!
    if (!RetrieveStackInfo(stackBase, stackSize, guardSize)) {
        return;
    }
    if (guardSize < ark::os::mem::GetPageSize()) {
        guardSize = ark::os::mem::GetPageSize();
    }
    if (stackSize <= nativeStackReservedSize + nativeStackProtectedSize + guardSize) {
        LOG(ERROR, RUNTIME) << "InitForStackOverflowCheck: stack size not enough, stack_base = " << stackBase
                            << ", stack_size = " << stackSize << ", guard_size = " << guardSize;
        return;
    }
    LOG(DEBUG, RUNTIME) << "InitForStackOverflowCheck: stack_base = " << stackBase << ", stack_size = " << stackSize
                        << ", guard_size = " << guardSize;
    nativeStackBegin_ = ToUintPtr(stackBase) + guardSize;
    nativeStackEnd_ = nativeStackBegin_ + nativeStackProtectedSize + nativeStackReservedSize;
    nativeStackReservedSize_ = nativeStackReservedSize;
    nativeStackProtectedSize_ = nativeStackProtectedSize;
    nativeStackGuardSize_ = guardSize;
    nativeStackSize_ = stackSize;
    // init frame stack size same with native stack size (*4 - is just an heuristic to pass some tests)
    // But frame stack size cannot be larger than max memory size in frame allocator
    auto iframeStackSize = stackSize * 4;
    auto allocatorMaxSize = stackFrameAllocator_->GetFullMemorySize();
    iframeStackSize_ = iframeStackSize <= allocatorMaxSize ? iframeStackSize : allocatorMaxSize;
    ProtectNativeStack();
    stackFrameAllocator_->SetReservedMemorySize(iframeStackSize_);
    stackFrameAllocator_->ReserveMemory();
}

void ManagedThread::ProtectNativeStack()
{
    if (nativeStackProtectedSize_ == 0) {
        return;
    }

    // Try to mprotect directly
    if (!ark::os::mem::MakeMemProtected(ToVoidPtr(nativeStackBegin_), nativeStackProtectedSize_)) {
        return;
    }

    // If fail to mprotect, try to load stack page and then retry to mprotect
    uintptr_t nativeStackTop = AlignDown(GetStackTop(), ark::os::mem::GetPageSize());
    LOG(DEBUG, RUNTIME) << "ProtectNativeStack: try to load pages, mprotect error = " << strerror(errno)
                        << ", stack_begin = " << nativeStackBegin_ << ", stack_top = " << nativeStackTop
                        << ", stack_size = " << nativeStackSize_ << ", guard_size = " << nativeStackGuardSize_;
    if (nativeStackSize_ > STACK_MAX_SIZE_OVERFLOW_CHECK || nativeStackEnd_ >= nativeStackTop ||
        nativeStackTop > nativeStackEnd_ + STACK_MAX_SIZE_OVERFLOW_CHECK) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: too large stack, mprotect error = " << strerror(errno)
                            << ", max_stack_size = " << STACK_MAX_SIZE_OVERFLOW_CHECK
                            << ", stack_begin = " << nativeStackBegin_ << ", stack_top = " << nativeStackTop
                            << ", stack_size = " << nativeStackSize_ << ", guard_size = " << nativeStackGuardSize_;
        return;
    }
    LoadStackPages(nativeStackBegin_);
    if (ark::os::mem::MakeMemProtected(ToVoidPtr(nativeStackBegin_), nativeStackProtectedSize_)) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: fail to protect pages, error = " << strerror(errno)
                            << ", stack_begin = " << nativeStackBegin_ << ", stack_top = " << nativeStackTop
                            << ", stack_size = " << nativeStackSize_ << ", guard_size = " << nativeStackGuardSize_;
    }
    size_t releaseSize = nativeStackTop - nativeStackBegin_ - ark::os::mem::GetPageSize();
    if (ark::os::mem::ReleasePages(nativeStackBegin_, nativeStackBegin_ + releaseSize) != 0) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: fail to release pages, error = " << strerror(errno)
                            << ", stack_begin = " << nativeStackBegin_ << ", stack_top = " << nativeStackTop
                            << ", stack_size = " << nativeStackSize_ << ", guard_size = " << nativeStackGuardSize_
                            << ", release_size = " << releaseSize;
    }
}

void ManagedThread::DisableStackOverflowCheck()
{
    nativeStackEnd_ = nativeStackBegin_;
    iframeStackSize_ = std::numeric_limits<size_t>::max();
    if (nativeStackProtectedSize_ > 0) {
        ark::os::mem::MakeMemReadWrite(ToVoidPtr(nativeStackBegin_), nativeStackProtectedSize_);
    }
}

void ManagedThread::EnableStackOverflowCheck()
{
    nativeStackEnd_ = nativeStackBegin_ + nativeStackProtectedSize_ + nativeStackReservedSize_;
    iframeStackSize_ = nativeStackSize_ * 4U;
    if (nativeStackProtectedSize_ > 0) {
        ark::os::mem::MakeMemProtected(ToVoidPtr(nativeStackBegin_), nativeStackProtectedSize_);
    }
}

void ManagedThread::NativeCodeBegin()
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(!(threadFrameStates_.empty() || threadFrameStates_.top() != NATIVE_CODE), FATAL, RUNTIME)
        << LogThreadStack(NATIVE_CODE) << " or stack should be empty";
    threadFrameStates_.push(NATIVE_CODE);
#endif
    SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(GetInternalId(), true));
    UpdateStatus(ThreadStatus::NATIVE);
    isManagedScope_ = false;
}

void ManagedThread::NativeCodeEnd()
{
    // thread_frame_states_ should not be accessed without MutatorLock (as runtime could have been destroyed)
    // If this was last frame, it should have been called from Destroy() and it should UpdateStatus to FINISHED
    // after this method
    UpdateStatus(ThreadStatus::RUNNING);
    SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(GetInternalId(), false));
    isManagedScope_ = true;
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(threadFrameStates_.empty(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(threadFrameStates_.top() != NATIVE_CODE, FATAL, RUNTIME) << LogThreadStack(NATIVE_CODE);
    threadFrameStates_.pop();
#endif
}

bool ManagedThread::IsInNativeCode() const
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    return threadFrameStates_.top() == NATIVE_CODE;
#endif
    return GetStatus() == ThreadStatus::NATIVE;
}

void ManagedThread::ManagedCodeBegin()
{
    // thread_frame_states_ should not be accessed without MutatorLock (as runtime could have been destroyed)
    UpdateStatus(ThreadStatus::RUNNING);
    SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(GetInternalId(), false));
    isManagedScope_ = true;
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(threadFrameStates_.top() != NATIVE_CODE, FATAL, RUNTIME) << LogThreadStack(MANAGED_CODE);
    threadFrameStates_.push(MANAGED_CODE);
#endif
}

void ManagedThread::ManagedCodeEnd()
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(threadFrameStates_.top() != MANAGED_CODE, FATAL, RUNTIME) << LogThreadStack(MANAGED_CODE);
    threadFrameStates_.pop();
#endif
    SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(GetInternalId(), true));
    // Should be NATIVE_CODE
    UpdateStatus(ThreadStatus::NATIVE);
    isManagedScope_ = false;
}

bool ManagedThread::IsManagedCode() const
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    return threadFrameStates_.top() == MANAGED_CODE;
#endif
    return GetStatus() == ThreadStatus::RUNNING;
}

// Since we don't allow two consecutive NativeCode frames, there is no managed code on stack if
// its size is 1 and last frame is Native
bool ManagedThread::HasManagedCodeOnStack() const
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    if (HasClearStack()) {
        return false;
    }
    if (threadFrameStates_.size() == 1 && IsInNativeCode()) {
        return false;
    }
    return true;
#else
    LOG(FATAL, RUNTIME) << "This method only supported in debug mode";
    return !IsInNativeCode();
#endif
}

bool ManagedThread::HasClearStack() const
{
    return threadFrameStates_.empty();
}

PandaString ManagedThread::ThreadStatusAsString(enum ThreadStatus status)
{
    switch (status) {
        case ThreadStatus::CREATED:
            return "New";
        case ThreadStatus::RUNNING:
            return "Runnable";
        case ThreadStatus::IS_BLOCKED:
            return "Blocked";
        case ThreadStatus::IS_WAITING:
            return "Waiting";
        case ThreadStatus::IS_TIMED_WAITING:
            return "Timed_waiting";
        case ThreadStatus::IS_SUSPENDED:
            return "Suspended";
        case ThreadStatus::IS_COMPILER_WAITING:
            return "Compiler_waiting";
        case ThreadStatus::IS_WAITING_INFLATION:
            return "Waiting_inflation";
        case ThreadStatus::IS_SLEEPING:
            return "Sleeping";
        case ThreadStatus::IS_TERMINATED_LOOP:
            return "Terminated_loop";
        case ThreadStatus::TERMINATING:
            return "Terminating";
        case ThreadStatus::NATIVE:
            return "Native";
        case ThreadStatus::FINISHED:
            return "Terminated";
        default:
            return "unknown";
    }
}

PandaString ManagedThread::LogThreadStack(ThreadState newState) const
{
    PandaStringStream debugMessage;
    static std::unordered_map<ThreadState, std::string> threadStateToStringMap = {
        {ThreadState::NATIVE_CODE, "NATIVE_CODE"}, {ThreadState::MANAGED_CODE, "MANAGED_CODE"}};
    auto newStateIt = threadStateToStringMap.find(newState);
    auto topFrameIt = threadStateToStringMap.find(threadFrameStates_.top());
    ASSERT(newStateIt != threadStateToStringMap.end());
    ASSERT(topFrameIt != threadStateToStringMap.end());

    debugMessage << "threadId: " << GetId() << " "
                 << "tried go to " << newStateIt->second << " state, but last frame is: " << topFrameIt->second << ", "
                 << threadFrameStates_.size() << " frames in stack (from up to bottom): [";

    PandaStack<ThreadState> copyStack(threadFrameStates_);
    while (!copyStack.empty()) {
        auto it = threadStateToStringMap.find(copyStack.top());
        ASSERT(it != threadStateToStringMap.end());
        debugMessage << it->second;
        if (copyStack.size() > 1) {
            debugMessage << "|";
        }
        copyStack.pop();
    }
    debugMessage << "]";
    return debugMessage.str();
}

MTManagedThread::MTManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *pandaVm,
                                 ark::panda_file::SourceLang threadLang)
    : ManagedThread(id, allocator, pandaVm, Thread::ThreadType::THREAD_TYPE_MT_MANAGED, threadLang),
      enteringMonitor_(nullptr)
{
    ASSERT(pandaVm != nullptr);
    auto threadManager = reinterpret_cast<MTThreadManager *>(GetVM()->GetThreadManager());
    internalId_ = threadManager->GetInternalThreadId();

    auto ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(GetThreadLang());
    if (ext != nullptr) {
        stringClassPtr_ = ext->GetClassRoot(ClassRoot::LINE_STRING);
    }

    auto *rs = allocator->New<mem::ReferenceStorage>(pandaVm->GetGlobalObjectStorage(), allocator, false);
    LOG_IF((rs == nullptr || !rs->Init()), FATAL, RUNTIME) << "Cannot create pt reference storage";
    ptReferenceStorage_ = PandaUniquePtr<mem::ReferenceStorage>(rs);
}

MTManagedThread::~MTManagedThread()
{
    ASSERT(internalId_ != 0);
    auto threadManager = reinterpret_cast<MTThreadManager *>(GetVM()->GetThreadManager());
    threadManager->RemoveInternalThreadId(internalId_);
}

void ManagedThread::PushLocalObject(ObjectHeader **objectHeader)
{
    ASSERT(TestLockState());
    localObjects_.push_back(objectHeader);
    LOG(DEBUG, GC) << "PushLocalObject for thread " << std::hex << this << ", obj = " << *objectHeader;
}

void ManagedThread::PopLocalObject()
{
    ASSERT(TestLockState());
    ASSERT(!localObjects_.empty());
    LOG(DEBUG, GC) << "PopLocalObject from thread " << std::hex << this << ", obj = " << *localObjects_.back();
    localObjects_.pop_back();
}

bool ManagedThread::TestLockState() const
{
#ifndef NDEBUG
    // Object handles can be created during class initialization, so check lock state only after GC is started.
    return !ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           (GetMutatorLock()->GetState() != MutatorLock::MutatorLockState::UNLOCKED);
#else
    return true;
#endif
}

void MTManagedThread::PushLocalObjectLocked(ObjectHeader *obj)
{
    localObjectsLocked_.EmplaceBack(obj, GetFrame());
}

void MTManagedThread::PopLocalObjectLocked([[maybe_unused]] ObjectHeader *out)
{
    if (LIKELY(!localObjectsLocked_.Empty())) {
#ifndef NDEBUG
        ObjectHeader *obj = localObjectsLocked_.Back().GetObject();
        if (obj != out) {
            LOG(WARNING, RUNTIME) << "Locked object is not paired";
        }
#endif  // !NDEBUG
        localObjectsLocked_.PopBack();
    } else {
        LOG(WARNING, RUNTIME) << "PopLocalObjectLocked failed, current thread locked object is empty";
    }
}

Span<LockedObjectInfo> MTManagedThread::GetLockedObjectInfos()
{
    return localObjectsLocked_.Data();
}

void ManagedThread::UpdateTLAB(mem::TLAB *tlab)
{
    ASSERT(tlab_ != nullptr);
    ASSERT(tlab != nullptr);
    tlab_ = tlab;
}

void ManagedThread::ClearTLAB()
{
    ASSERT(zeroTlab_ != nullptr);
    tlab_ = zeroTlab_;
}

/* Common actions for creation of the thread. */
void MTManagedThread::ProcessCreatedThread()
{
    ManagedThread::SetCurrent(this);
    // Runtime takes ownership of the thread
    trace::ScopedTrace scopedTrace2("ThreadManager::RegisterThread");
    auto threadManager = reinterpret_cast<MTThreadManager *>(GetVM()->GetThreadManager());
    threadManager->RegisterThread(this);
    NativeCodeBegin();
}

void ManagedThread::UpdateAndSweep([[maybe_unused]] const ReferenceUpdater &updater) {}

/* return true if sleep is interrupted */
bool MTManagedThread::Sleep(uint64_t ms)
{
    auto thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    bool isInterrupted = thread->IsInterrupted();
    if (!isInterrupted) {
        thread->TimedWait(ThreadStatus::IS_SLEEPING, ms, 0);
        isInterrupted = thread->IsInterrupted();
    }
    return isInterrupted;
}

void ManagedThread::SetThreadPriority(int32_t prio)
{
    ThreadId tid = GetId();
    int res = os::thread::SetPriority(tid, prio);
    if (!os::thread::IsSetPriorityError(res)) {
        LOG(DEBUG, RUNTIME) << "Successfully changed priority for thread " << tid << " to " << prio;
    } else {
        LOG(DEBUG, RUNTIME) << "Cannot change priority for thread " << tid << " to " << prio;
    }
}

uint32_t ManagedThread::GetThreadPriority()
{
    ThreadId tid = GetId();
    return os::thread::GetPriority(tid);
}

void MTManagedThread::UpdateAndSweep(const ReferenceUpdater &updater)
{
    ManagedThread::UpdateAndSweep(updater);
    for (auto &it : localObjectsLocked_.Data()) {
        it.UpdateObject(updater);
    }
}

void MTManagedThread::VisitGCRoots(const GCRootVisitor &cb)
{
    ManagedThread::VisitGCRoots(cb);

    // Visit enter_monitor_object_
    if (enterMonitorObject_ != nullptr) {
        cb({mem::RootType::ROOT_THREAD, &enterMonitorObject_});
    }

    ptReferenceStorage_->VisitObjects(cb, mem::RootType::ROOT_PT_LOCAL);
}
void MTManagedThread::SetDaemon()
{
    isDaemon_ = true;
    auto threadManager = reinterpret_cast<MTThreadManager *>(GetVM()->GetThreadManager());
    threadManager->AddDaemonThread();
    SetThreadPriority(MIN_PRIORITY);
}

void MTManagedThread::Interrupt(MTManagedThread *thread)
{
    os::memory::LockHolder lock(thread->condLock_);
    LOG(DEBUG, RUNTIME) << "Interrupt a thread " << thread->GetId();
    thread->SetInterruptedWithLockHeld(true);
    thread->SignalWithLockHeld();
    thread->InterruptPostImpl();
}

bool MTManagedThread::Interrupted()
{
    os::memory::LockHolder lock(condLock_);
    bool res = IsInterruptedWithLockHeld();
    SetInterruptedWithLockHeld(false);
    return res;
}

void MTManagedThread::StopDaemonThread()
{
    SetRuntimeTerminated();
    MTManagedThread::Interrupt(this);
}

void ManagedThread::VisitGCRoots(const GCRootVisitor &cb)
{
    if (exception_ != nullptr) {
        cb({mem::RootType::ROOT_THREAD, &exception_});
    }
    for (auto obj : localObjects_) {
        cb({mem::RootType::ROOT_THREAD, obj});
    }

    if (!taggedHandleScopes_.empty()) {
        taggedHandleStorage_->VisitGCRoots(cb);
        taggedGlobalHandleStorage_->VisitGCRoots(cb);
    }
    if (!objectHeaderHandleScopes_.empty()) {
        objectHeaderHandleStorage_->VisitGCRoots(cb);
    }

    if (flattenedStringCache_ != nullptr) {
        cb({mem::RootType::ROOT_THREAD, &flattenedStringCache_});
    }
}

void MTManagedThread::Destroy()
{
    ASSERT(this == ManagedThread::GetCurrent());
    ASSERT(GetStatus() != ThreadStatus::FINISHED);

    UpdateStatus(ThreadStatus::TERMINATING);  // Set this status to prevent runtime for destroying itself while this
                                              // NATTIVE thread
    // is trying to acquire runtime.
    ReleaseMonitors();
    if (!IsDaemon()) {
        Runtime *runtime = Runtime::GetCurrent();
        runtime->GetNotificationManager()->ThreadEndEvent(this);
    }

    auto threadManager = reinterpret_cast<MTThreadManager *>(GetVM()->GetThreadManager());
    if (threadManager->UnregisterExitedThread(this)) {
        // Clear current_thread only if unregistration was successfull
        ManagedThread::SetCurrent(nullptr);
    }
}

CustomTLSData *ManagedThread::GetCustomTLSData(const char *key)
{
    os::memory::LockHolder lock(*Locks::customTlsLock_);
    auto it = customTlsCache_.find(key);
    if (it == customTlsCache_.end()) {
        return nullptr;
    }
    return it->second.get();
}

void ManagedThread::SetCustomTLSData(const char *key, CustomTLSData *data)
{
    os::memory::LockHolder lock(*Locks::customTlsLock_);
    PandaUniquePtr<CustomTLSData> tlsData(data);
    auto it = customTlsCache_.find(key);
    if (it == customTlsCache_.end()) {
        customTlsCache_[key] = {PandaUniquePtr<CustomTLSData>()};
    }
    customTlsCache_[key].swap(tlsData);
}

bool ManagedThread::EraseCustomTLSData(const char *key)
{
    os::memory::LockHolder lock(*Locks::customTlsLock_);
    return customTlsCache_.erase(key) != 0;
}

void ManagedThread::SetFlattenedStringCache(ObjectHeader *cacheInstance)
{
    flattenedStringCache_ = cacheInstance;
}

ObjectHeader *ManagedThread::GetFlattenedStringCache() const
{
    return flattenedStringCache_;
}

LanguageContext ManagedThread::GetLanguageContext()
{
    return Runtime::GetCurrent()->GetLanguageContext(threadLang_);
}

void MTManagedThread::FreeInternalMemory()
{
    localObjectsLocked_.~LockedObjectList<>();
    ptReferenceStorage_.reset();

    ManagedThread::FreeInternalMemory();
}

void ManagedThread::CollectTLABMetrics()
{
    if (zeroTlab_ != nullptr) {
        GetVM()->GetHeapManager()->RegisterTLAB(GetTLAB());
    }
}

void ManagedThread::DestroyInternalResources()
{
    GetVM()->GetGC()->OnThreadTerminate(this, mem::BuffersKeepingFlag::DELETE);
    ASSERT(preBuff_ == nullptr);
    ASSERT(g1PostBarrierRingBuffer_ == nullptr);
    ptThreadInfo_->Destroy();
}

void ManagedThread::CleanupInternalResources()
{
    GetVM()->GetGC()->OnThreadTerminate(this, mem::BuffersKeepingFlag::KEEP);
}

void ManagedThread::FreeInternalMemory()
{
#if defined(VERBOSE_THREAD_STATE_LOG)
    threadFrameStates_.~PandaStack<ThreadState>();
#endif
    DestroyInternalResources();

    localObjects_.~PandaVector<ObjectHeader **>();
    {
        os::memory::LockHolder lock(*Locks::customTlsLock_);
        customTlsCache_.~PandaMap<const char *, PandaUniquePtr<CustomTLSData>>();
    }

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(stackFrameAllocator_);
    allocator->Delete(internalLocalAllocator_);

    allocator->Delete(ptThreadInfo_.release());
    allocator->Delete(weightedAdaptiveTlabAverage_);

    taggedHandleScopes_.~PandaVector<HandleScope<coretypes::TaggedType> *>();
    allocator->Delete(taggedHandleStorage_);
    allocator->Delete(taggedGlobalHandleStorage_);

    allocator->Delete(objectHeaderHandleStorage_);
    objectHeaderHandleScopes_.~PandaVector<HandleScope<ObjectHeader *> *>();

    Thread::FreeInternalMemory();
}

void ManagedThread::PrintSuspensionStackIfNeeded()
{
    /* @sync 1
     * @description Before getting runtime options
     */
    if (!Runtime::GetOptions().IsSafepointBacktrace()) {
        /* @sync 2
         * @description After getting runtime options
         */
        return;
    }
    /* @sync 3
     * @description After getting runtime options
     */
    PandaStringStream out;
    out << "Thread " << GetId() << " is suspended at\n";
    PrintStack(out);
    LOG(INFO, RUNTIME) << out.str();
}

void ManagedThread::CleanUp()
{
    // Cleanup Exception, TLAB, cache interpreter, HandleStorage
    ClearException();
    ClearTLAB();

#if defined(VERBOSE_THREAD_STATE_LOG)
    while (!threadFrameStates_.empty()) {
        threadFrameStates_.pop();
    }
#endif
    localObjects_.clear();
    {
        os::memory::LockHolder lock(*Locks::customTlsLock_);
        customTlsCache_.clear();
    }
    interpreterCache_.Clear();

    taggedHandleScopes_.clear();
    taggedHandleStorage_->FreeHandles(0);
    taggedGlobalHandleStorage_->FreeHandles();

    objectHeaderHandleStorage_->FreeHandles(0);
    objectHeaderHandleScopes_.clear();

    CleanUpThreadStatus();
    // NOTE(molotkovnikhail, 13159) Add cleanup of signal_stack for windows target
}

}  // namespace ark
