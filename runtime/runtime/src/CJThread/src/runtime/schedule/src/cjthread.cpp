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


#include <cstring>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <cstdint>
#include "processor.h"
#include "schedule_impl.h"
#include "thread.h"
#include "log.h"
#include "securec.h"

#include "Mutator/Mutator.inline.h"

#include "codethread.h"
#if defined(CODIRA_SANITIZER_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

#ifndef MRT_WINDOWS
#include <sys/mman.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* attribute is used to optimize performance. When no attribute is added, call get_tls for
 * access. When attributes are added, use the fs register for access. */
#if !defined(TLS_SPEC_OFFSET) && !defined(TLS_COMMON_DYNAMIC)
__thread struct CODEThread *g_codethread __attribute__((tls_model("initial-exec"))) = nullptr;
__thread struct Schedule *g_schedule __attribute__((tls_model("initial-exec"))) = nullptr;
__thread uintptr_t g_preemptFlag __attribute__((tls_model("initial-exec"))) = 0;
__thread uintptr_t g_protectAddr __attribute__((tls_model("initial-exec"))) = 0;
#endif

/* Space reserved for stack overflow check */
uintptr_t g_codethreadStackReservedSize = STACK_DEFAULT_REVERSED;

#if !defined(MRT_WINDOWS)
constexpr size_t HUGE_PAGE = 2UL * 1024 * 1024; // use mmap when stack size is beyond 2mb.
#endif

#ifdef MRT_WINDOWS
void CODEThreadStackMemFree(struct CODEThread *codethread, char *stackTopAddr, size_t stackSize)
{
    char *stackFreeAddr;
    size_t pageSize;
    DWORD oldProt = 0;
    int error;

    if (codethread->schedule->schdCODEThread.stackProtect == false) {
        stackFreeAddr = stackTopAddr;
        free(stackFreeAddr);
    } else {
        pageSize = SchedulePageSize();
        stackFreeAddr = stackTopAddr - pageSize;
        if (!VirtualProtect(stackFreeAddr, pageSize, PAGE_READWRITE, &oldProt)) {
            error = (int)GetLastError();
            LOG_ERROR(error, "VirtualProtect failed, page_size: %u", pageSize);
        }

        if (!VirtualFree(stackFreeAddr, 0, MEM_RELEASE)) {
            error = (int)GetLastError();
            LOG_ERROR(error, "VirtualFree failed, size: %u", stackSize);
        }
    }
}
#else
void StackMemFreeInternel(void* stackAddr, size_t stackSize)
{
    if (stackSize >= HUGE_PAGE) {
        munmap(stackAddr, stackSize);
    } else {
        free(stackAddr);
    }
}
void CODEThreadStackMemFree(struct CODEThread *codethread, char *stackTopAddr, size_t stackSize)
{
    char *stackFreeAddr;
    size_t pageSize;
    int error;

#if defined(CODIRA_TSAN_SUPPORT)
    MapleRuntime::Sanitizer::TsanFree(stackTopAddr, stackSize);
#endif
    if (codethread->schedule->schdCODEThread.stackProtect == false) {
        StackMemFreeInternel(stackTopAddr, stackSize);
    } else {
        pageSize = SchedulePageSize();
        stackFreeAddr = stackTopAddr - pageSize;
        error = mprotect(stackFreeAddr, pageSize, PROT_READ | PROT_WRITE);
        if (error) {
            HILOG_ERROR(errno, "mprotect failed in CODEThreadStackMemFree. size is %u", pageSize);
        }
        error = munmap(stackFreeAddr, stackSize + pageSize);
        if (error) {
            HILOG_ERROR(errno, "munmap failed in CODEThreadStackMemFree. size is %u", stackSize + pageSize);
        }
    }
}
#endif

void CODEThreadMemFree(struct CODEThread *codethread)
{
    CODEThreadStackMemFree(codethread, codethread->stack.stackTopAddr, codethread->stack.stackSize);
    free(codethread);
}

/* Put codethread to free list or free codethread */
void CODEThreadFree(struct CODEThread *codethread, bool reuse)
{
    struct Processor *processor;
    struct ScheduleCODEThread *scheduleCODEThread;
    struct ScheduleGfreeList *gfreelist;
    struct Schedule *targetSchedule = codethread->schedule;
    struct Schedule *schedule = ScheduleGet();

    if (reuse && (schedule == nullptr || targetSchedule == nullptr)) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "schedule not inited");
        return;
    }
#if defined(CODIRA_TSAN_SUPPORT)
    MapleRuntime::Sanitizer::TsanDeleteRaceState(codethread);
#endif
    scheduleCODEThread = &(targetSchedule->schdCODEThread);
    if (reuse && codethread->stack.stackSize == scheduleCODEThread->stackSize) {
        if (targetSchedule != schedule) {
            gfreelist = &targetSchedule->schdCODEThread.gfreelist;
            pthread_mutex_lock(&gfreelist->gfreeLock);
            ScheduleGfreelistPush(gfreelist, codethread);
            pthread_mutex_unlock(&gfreelist->gfreeLock);
        } else {
            processor = ProcessorGet();
            ProcessorFreelistPut(processor, codethread);
        }
    } else {
        ScheduleAllCODEThreadListRemove(codethread);
        CODEThreadMemFree(codethread);
    }
}

/* Obtain the codethread ID in atomic mode. The reentrant lock of the warehouse lock determines
 * whether it is the same codethread based on codethreadId. In multi-scheduler scenarios, the
 * unified global variable is used to obtain codethreadId.
 **/
unsigned long long CODEThreadNewId(void)
{
    return atomic_fetch_add(&g_scheduleManager.codethreadIdGen, 1llu);
}

MRT_STATIC_INLINE int CODEThreadInit(struct CODEThread *newCODEThread, struct ArgAttr *argAttr)
{
    int error;
    char *argBuffer;
    newCODEThread->argSize = argAttr->argSize;
    if (argAttr->argStart != nullptr) {
        argBuffer = (char *)newCODEThread + sizeof(struct CODEThread);
        // Check CODEThreadAttrCheck and ensure that argSize <= COARGS_SIZE_MAX. does not
        // cause memory corruption.
        error = memcpy_s(argBuffer, argAttr->argSize, argAttr->argStart, argAttr->argSize);
        if (error) {
            HILOG_ERROR(error, "memcpy codethread args failed when init codethread");
            return error;
        }
        newCODEThread->argStart = argBuffer;
    } else {
        newCODEThread->argStart = nullptr;
    }

    newCODEThread->boundThread = nullptr;
    DulinkInit(&(newCODEThread->schdDulink));
    atomic_store_explicit(&newCODEThread->state, CODETHREAD_IDLE, std::memory_order_relaxed);
    newCODEThread->name[0] = '\0';

    return 0;
}

/* Init codethread_stack related attributes */
MRT_STATIC_INLINE void CODEThreadStackAttrInit(struct CODEThread *codethread, size_t totalSize,
                                             char *stackAddr, struct StackAttr *stackAttr)
{
    codethread->stack.totalSize = totalSize;
    codethread->stack.stackTopAddr = stackAddr;

    // Reserve a part of the stack size to handle stack overflow.
    codethread->stack.stackGuard = stackAddr + g_codethreadStackReservedSize;
    codethread->stack.stackBaseAddr = stackAddr + stackAttr->stackSizeAlign;
    codethread->stack.stackSize = stackAttr->stackSizeAlign;
    // 16-byte-aligned. Note that the 64 KB lower stack address is not 0x0 - 0x100000000,
    // but 0x0 - 0xffffff, and the 16 bytes are aligned to 0x0 - 0xfffffff0. The stack
    // address must be 16-byte aligned. Otherwise, an error occurs.
    codethread->stack.codethreadStackBaseAddr = (char *)STACK_ADDR_ALIGN_DOWN(
        stackAddr + stackAttr->stackSizeAlign, CODETHREAD_ARG_ALIGN);
#ifdef CODIRA_SANITIZER_SUPPORT
    // reserve space for sanitizer thread-local variables
    codethread->stack.codethreadStackBaseAddr -= CODETHREAD_SANITIZER_CONTEXT_OFFSET;
#endif
    codethread->stack.stackGrowCnt = stackAttr->stackGrow ? 0 : 1;
}

/* low address----------------------------------high address
* -------------------------------------------
* |                  |                      |
* | codethread struct |  copy arg(32Bytes)    |
* |                  |                      |
* -------------------------------------------
*/
struct CODEThread *CODEThreadAndArgsMemAlloc()
{
    int error;
    void *ptr;
    struct CODEThread *codethread;
    size_t corouSize;
    size_t totalSize;

    corouSize = sizeof(struct CODEThread);
    totalSize = corouSize + COARGS_SIZE_MAX;

    ptr = malloc(totalSize);
    if (ptr == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "codethread malloc failed, error %d", errno);
        return nullptr;
    }

    codethread = (struct CODEThread *)ptr;
    error = memset_s(codethread, corouSize, 0, corouSize);
    if (error) {
        free(ptr);
        HILOG_ERROR(error, "codethread memset_s failed");
        return nullptr;
    }
    return codethread;
}

#ifdef MRT_WINDOWS
char *CODEThreadStackMemAlloc(struct Schedule *schedule, struct CODEThread *codethread,
                            size_t stackSizeAlign, size_t *totalSize)
{
    size_t pageSize;
    char *addr;
    char *stackAddr;
    DWORD oldProt = 0;
    int error;

    if (schedule->schdCODEThread.stackProtect == false) {
        /* low address------------high address
        * ----------------------
        * |                    |
        * |   codethread stack   |
        * |                    |
        * ----------------------
        */
        *totalSize = stackSizeAlign;
        stackAddr = static_cast<char *>(malloc(*totalSize));
        if (stackAddr == nullptr) {
            HILOG_FATAL(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "alloc stack failed,  os memory is not enough size is %zu",
                        *totalSize);
            return nullptr;
        }
        codethread->stack.protectAddr = 0;
    } else {
        /* low address--------------------------------high address
        * -----------------------------------------
        * |              |                        |
        * | protect page |      codethread stack    |
        * |              |                        |
        * ------------------------------------------
        */

        pageSize = SchedulePageSize();
        *totalSize = pageSize + stackSizeAlign;
        addr = static_cast<char *>(VirtualAlloc(nullptr, *totalSize,
                                                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (addr == nullptr) {
            error = (int)GetLastError();
            HILOG_FATAL(error, "allocate code stack failed because the os memory is not enough, size is %u", *totalSize);
            return nullptr;
        }
        if (!VirtualProtect(addr, pageSize, PAGE_NOACCESS, &oldProt)) {
            error = (int)GetLastError();
            LOG_ERROR(error, "VirtualProtect failed, page_size: %u", pageSize);
            if (!VirtualFree(addr, 0, MEM_RELEASE)) {
                error = (int)GetLastError();
                LOG_ERROR(error, "VirtualFree failed, size: %u", *totalSize);
            }
            return nullptr;
        }
        codethread->stack.protectAddr = addr;
        stackAddr = addr + pageSize;
    }

    return stackAddr;
}
#else
static char* StackMemAllocInternal(size_t allocSize)
{
    if (allocSize >= HUGE_PAGE) {
        void* stackAddr = mmap(nullptr, allocSize, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (stackAddr == MAP_FAILED) {
            HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED,
                        "allocate codethread stack with huge page failed, error %d", errno);
            return nullptr;
        }
#ifdef CODIRA_HWASAN_SUPPORT
        stackAddr =
            reinterpret_cast<char*>(MapleRuntime::Sanitizer::UntagAddr(reinterpret_cast<uintptr_t>(stackAddr)));
#endif
#if defined (__linux__) || defined(__OHOS__) || defined(__ANDROID__)
        (void)madvise(stackAddr, allocSize, MADV_NOHUGEPAGE);
        MRT_PRCTL(stackAddr, allocSize, "code-stack");
#endif
        return static_cast<char*>(stackAddr);
    } else {
        char* stackAddr = static_cast<char*>(malloc(allocSize));
        if (stackAddr == nullptr) {
            HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "allocate codethread stack failed, error %d", errno);
            return nullptr;
        }
#ifdef CODIRA_HWASAN_SUPPORT
        stackAddr =
            reinterpret_cast<char*>(MapleRuntime::Sanitizer::UntagAddr(reinterpret_cast<uintptr_t>(stackAddr)));
#endif
        return stackAddr;
    }
}
char *CODEThreadStackMemAlloc(struct Schedule *schedule, struct CODEThread *codethread,
                            size_t stackSizeAlign, size_t *totalSize)
{
    size_t pageSize;
    char *addr;
    char *stackAddr;
    int error;

    if (schedule->schdCODEThread.stackProtect == false) {
        /* low address------------high address
        * ----------------------
        * |                    |
        * |   codethread stack   |
        * |                    |
        * ----------------------
        */
       *totalSize = stackSizeAlign;
        stackAddr = StackMemAllocInternal(*totalSize);
        if (stackAddr == nullptr) {
            return nullptr;
        }
        codethread->stack.protectAddr = 0;
    } else {
        /* low address--------------------------------high address
        * -----------------------------------------
        * |              |                        |
        * | protect page |      codethread stack    |
        * |              |                        |
        * ------------------------------------------
        */
        pageSize = SchedulePageSize();
        *totalSize = pageSize + stackSizeAlign;
        addr = (char *)mmap(nullptr, *totalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr == MAP_FAILED) {
            HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED,
                        "codethread stack mmap failed, size %u, error %d", *totalSize, errno);
            return nullptr;
        }
#if defined (__linux__) || defined(__OHOS__) || defined(__ANDROID__)
        MRT_PRCTL(addr, *totalSize, "code-stack");
#endif
#ifdef CODIRA_HWASAN_SUPPORT
        addr = reinterpret_cast<char*>(MapleRuntime::Sanitizer::UntagAddr(reinterpret_cast<uintptr_t>(addr)));
#endif
        // The stack protection area is set to unreadable and non writable
        error = mprotect(addr, pageSize, PROT_NONE);
        if (error) {
            HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED,
                        "codethread stack mprotect failed, page_size %u, error %d", pageSize, errno);
            munmap(addr, *totalSize);
            return nullptr;
        }
        codethread->stack.protectAddr = addr;
        stackAddr = addr + pageSize;
    }

#ifdef CODIRA_HWASAN_SUPPORT
        MapleRuntime::Sanitizer::UntagMemory(reinterpret_cast<void*>(stackAddr), stackSizeAlign);
#endif
    return stackAddr;
}
#endif

/* stackSizeAlign ensures page alignment before passing in */
struct CODEThread *CODEThreadMemAlloc(struct Schedule *schedule, struct StackAttr *stackAttr)
{
    size_t totalSize;
    char *stackAddr;
    struct CODEThread *codethread;

    // Allocate structure memory and args memory of CODEThread.
    codethread = CODEThreadAndArgsMemAlloc();
    if (codethread == nullptr) {
        return nullptr;
    }

    if (schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        // should not create new stack when code thread is created by foreign thread.
        CODEThreadStackAttrInit(codethread, 0, nullptr, stackAttr);
    } else {
        // Allocate stack memeory of CODEThread.
        stackAddr = CODEThreadStackMemAlloc(schedule, codethread, stackAttr->stackSizeAlign, &totalSize);
        if (stackAddr == nullptr) {
            free(codethread);
            return nullptr;
        }
        // Init codethread stack attributes.
        CODEThreadStackAttrInit(codethread, totalSize, stackAddr, stackAttr);
    }

    // Initializes the global management linked list node for the codethread that newly
    // applies for memory.
    DulinkInit(&(codethread->allCODEThreadDulink));
    DulinkInit(&(codethread->codeSingleModeThreadDulink));

    return codethread;
}

/* Alloc CODEThread. */
struct CODEThread *CODEThreadAlloc(struct Schedule *schedule, struct ArgAttr *argAttr,
                               struct StackAttr *stackAttr, CODEThreadBuf coBuf)
{
    int error;
    struct CODEThread *newCODEThread = nullptr;
    struct ScheduleCODEThread *scheduleCODEThread = &(schedule->schdCODEThread);
    bool addToList = false;

    if (stackAttr->stackSizeAlign == schedule->schdCODEThread.stackSize && coBuf == LOCAL_BUF) {
        newCODEThread = ProcessorFreelistGet(ProcessorGet());
    } else if (stackAttr->stackSizeAlign == schedule->schdCODEThread.stackSize && coBuf == GLOBAL_BUF) {
        newCODEThread = ScheduleGfreelistGet(&scheduleCODEThread->gfreelist);
    }
    if (newCODEThread == nullptr) {
        newCODEThread = CODEThreadMemAlloc(schedule, stackAttr);
        addToList = (coBuf != NO_BUF) ? true : false;
    }
    if (newCODEThread == nullptr) {
        HILOG_ERROR(errno, "codethread memory alloc failed");
        return nullptr;
    }

    newCODEThread->schedule = schedule;

    error = CODEThreadInit(newCODEThread, argAttr);
    if (error) {
        HILOG_ERROR(error, "codethread init failed");
        CODEThreadFree(newCODEThread, coBuf);
        return nullptr;
    }

    if (coBuf == NO_BUF) {
        newCODEThread->id = 0;
        newCODEThread->isCODEThread0 = true;
    } else {
        newCODEThread->id = CODETHREAD_INIT_ID;
        newCODEThread->isCODEThread0 = false;
        // co_buf is used to determine whether codethread0 is used. codethread0 is not put
        // in the list.
        if (addToList) {
            if (ScheduleAllCODEThreadListAdd(newCODEThread) != 0) {
                CODEThreadMemFree(newCODEThread);
                return nullptr;
            }
        }
    }
    return newCODEThread;
}

/* CODEThreadMexit */
void *CODEThreadMexit(struct CODEThread *delCODEThread)
{
    unsigned long long codethreadId = delCODEThread->id;
#ifdef __OHOS__
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_EXEC, codethreadId);
    TRACE_START_ASYNC(TRACE_CODETHREAD_EXIT, codethreadId);
#elif defined (__ANDROID__)
    TRACE_FINISH();
    TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXIT, codethreadId));
#endif
    struct Schedule *schedule = delCODEThread->schedule;
    struct ScheduleCODEThread *scheduleCODEThread = &schedule->schdCODEThread;
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(CODEThreadGet());
#endif

    delCODEThread->argStart = nullptr;
    atomic_store_explicit(&delCODEThread->state, CODETHREAD_IDLE, std::memory_order_relaxed);
    CODEThreadKeysClean(delCODEThread);
    // Non-default scheduler waits for CODEThreadNum == 0 on exit to ensure normal execution
    // of CODEThreadFree
    CODEThreadFree(delCODEThread, true);
    if (schedule->scheduleType != SCHEDULE_DEFAULT) {
        atomic_fetch_sub(&scheduleCODEThread->codethreadNum, 1ULL);
    }
    
#ifdef __OHOS__
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_EXIT, codethreadId);
#elif defined (__ANDROID__)
    TRACE_FINISH();
#else
    (void)codethreadId;
#endif
    // The special stackid of the event is set through hardcode.
    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_END)) {
        ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_END, -1, nullptr, 1, SpecialStackId::CODETHREAD_EXIT);
    }
    // This function does not return, get the next codethread run
    ProcessorSchedule();

    return nullptr;
}

MRT_STATIC_INLINE void CODEThreadExit(void)
{
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(CODEThreadGet(), ThreadGet()->codethread0);
#endif
    CODEThreadMcallNosave(reinterpret_cast<void *>(CODEThreadMexit), CODEThreadAddr());
}

void *CODEThreadEntry(struct CODEThread *codethread)
{
    // execute codetrhead task
    codethread->func(codethread->argStart, codethread->argSize);
    // codethread exit. Switch codethread to codethread0, release codethread and switch the next
    CODEThreadExit();

    // Shouldn't be done here
    return nullptr;
}

#ifdef __OHOS__
void DecSingleModelC2NCount()
{
    bool runtimeFinished = MapleRuntime::MRT_CheckRuntimeFinished();
    if (runtimeFinished) {
        return;
    }
    Schedule* schedule = ScheduleGet();
    if (schedule == nullptr || schedule->scheduleType != SCHEDULE_UI_THREAD) {
        return;
    }
    CODEThread* codethread = CODEThreadGet();
    if (codethread == nullptr || codethread->singleModelC2NCount == 0) {
        return;
    }
    codethread->singleModelC2NCount--;
}

void AddSingleModelC2NCount()
{
    bool runtimeFinished = MapleRuntime::MRT_CheckRuntimeFinished();
    if (runtimeFinished) {
        return;
    }
    Schedule* schedule = ScheduleGet();
    if (schedule == nullptr || schedule->scheduleType != SCHEDULE_UI_THREAD) {
        return;
    }
    CODEThread* codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return;
    }
    codethread->singleModelC2NCount++;
}
#endif

/* Creating a codethread directly from C side without going through Codira requires initializing the mutator */
void *CODEThreadEntryInitMutator(struct CODEThread *codethread)
{
    SchdCODEThreadHookFunc hook_func;

    // mutator need to be initialized before running the codethread task
    hook_func = g_scheduleManager.schdCODEThreadHook[SCHD_CREATE_MUTATOR];
    if (hook_func != nullptr) {
        hook_func();
    }

    // execute codetrhead task
    codethread->func(codethread->argStart, codethread->argSize);

    hook_func = g_scheduleManager.schdCODEThreadHook[SCHD_DESTROY_MUTATOR];
    if (hook_func != nullptr) {
        hook_func();
    }

    CODEThreadExit();

    return nullptr;
}

#ifdef MRT_WINDOWS
/*
 * Initializes the GS register. gsStackLow and gsStackDeallocation are initialized to 0,
 * indicating that stack overflow check of Windows is disabled.
 */
static void GsStackContextInit(struct CODEThread *newCODEThread)
{
    newCODEThread->context.gsStackHigh = (unsigned long long)newCODEThread->stack.stackBaseAddr;
    newCODEThread->context.gsStackLow = 0;
    newCODEThread->context.gsStackDeallocation = 0;
}
#endif

/* Generate the context of codethread. */
MRT_STATIC_INLINE void CODEThreadMake(const struct CODEThreadAttrInner *attr,
                                    CODEThreadFunc func, struct CODEThread *newCODEThread)
{
    newCODEThread->func = func;
    (void)memset_s(&newCODEThread->context, sizeof(struct CODEThreadContext), 0, sizeof(struct CODEThreadContext));

    // foreign thread schedule do not create new stack and not need to init codethread context.
    if (newCODEThread->schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return;
    }
    if (attr != nullptr && attr->codeFromC) {
        CODEThreadContextInit(&newCODEThread->context, reinterpret_cast<void *>(CODEThreadEntryInitMutator),
                            newCODEThread->stack.codethreadStackBaseAddr);
    } else {
        // Initialize the SP, PC, and floating-point environments. Note that rbp must be set
        // to 0 in x86 and lr must be set to 0 in ARM64.
        CODEThreadContextInit(&newCODEThread->context, reinterpret_cast<void *>(CODEThreadEntry),
                            newCODEThread->stack.codethreadStackBaseAddr);
    }
#ifdef MRT_WINDOWS
    GsStackContextInit(newCODEThread);
#endif
}

/* The floating-point environment of codethread0 is initialized only once. */
void CODEThread0Make(struct CODEThread *codethread0)
{
    // foreign thread schedule do not create new stack and not need to init codethread context.
    if (codethread0->schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return;
    }
    CODEThreadContextInit(&codethread0->context, nullptr, codethread0->stack.codethreadStackBaseAddr);
#ifdef MRT_WINDOWS
    GsStackContextInit(codethread0);
#endif
}

void CODEThreadAttrInit(struct CODEThreadAttr *attrUser)
{
    struct CODEThreadAttrInner *attr = reinterpret_cast<struct CODEThreadAttrInner *>(attrUser);

    if (attrUser == nullptr) {
        return;
    }

    attr->stackSize = 0;
    attr->named = false;

    attr->codeFromC = false;
    attr->hasSpecificData = false;
}

int CODEThreadAttrNameSet(struct CODEThreadAttr *attrUser, const char *name)
{
    struct CODEThreadAttrInner *attr = reinterpret_cast<struct CODEThreadAttrInner *>(attrUser);
    int ret;

    if (attrUser == nullptr || name == nullptr) {
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }

    ret = strcpy_s(attr->name, CODETHREAD_NAME_SIZE, name);
    if (ret != 0) {
        return ret;
    }
    attr->named = true;
    return 0;
}

void CODEThreadAttrStackSizeSet(struct CODEThreadAttr *attrUser, unsigned int size)
{
    struct CODEThreadAttrInner *attr = reinterpret_cast<struct CODEThreadAttrInner *>(attrUser);
    if (attrUser == nullptr) {
        return;
    }
    attr->stackSize = size;
}

void CODEThreadAttrCodeFromCSet(struct CODEThreadAttr *attrUser, bool flag)
{
    struct CODEThreadAttrInner *attr = reinterpret_cast<struct CODEThreadAttrInner *>(attrUser);
    if (attrUser == nullptr) {
        return;
    }
    attr->codeFromC = flag;
}

/* Set the relevant parameters of the codethread local variable. */
int CODEThreadAttrSpecificSet(struct CODEThreadAttr *attrUser, unsigned int num,
                            struct CODEThreadSpecificDataInner *data)
{
    struct CODEThreadAttrInner *attr = reinterpret_cast<struct CODEThreadAttrInner *>(attrUser);
    if (num > CODETHREAD_KEYS_MAX) {
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    if (num == 0) {
        return 0;
    }
    if (attrUser == nullptr || data == nullptr) {
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    int res = memset_s(attr->specificData, CODETHREAD_KEYS_MAX * sizeof(void *), 0,
                       CODETHREAD_KEYS_MAX * sizeof(void *));
    if (res != 0) {
        return res;
    }
    for (unsigned int i = 0; i < num; ++i) {
        if (data[i].key >= CODETHREAD_KEYS_MAX) {
            return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
        }
        attr->specificData[data[i].key] = data[i].value;
    }
    attr->hasSpecificData = true;
    return 0;
}

MRT_STATIC_INLINE int CODEThreadAttrCheck(const struct CODEThreadAttrInner *attr, CODEThreadFunc func,
                                        const void *argStart, unsigned int argSize)
{
    if (func == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "CODEThreadFunc is nullptr");
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    if ((argStart != nullptr && argSize == 0) || (argStart == nullptr && argSize != 0)) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "argStart is %s, size is %d",
                    argStart == nullptr ? "nullptr" : "not nullptr", argSize);
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }

    if (attr != nullptr) {
        if (attr->stackSize > CODETHREAD_MAX_STACK_SIZE) {
            HILOG_ERROR(ERRNO_SCHD_CODETHREAD_STACK_SIZE_INVALID, "attr stack size is %lu, max is %lu",
                        attr->stackSize, CODETHREAD_MAX_STACK_SIZE);
            return ERRNO_SCHD_CODETHREAD_STACK_SIZE_INVALID;
        }
    }

    if (argSize > COARGS_SIZE_MAX) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "argSize is %u, max is %u",
                    argSize, COARGS_SIZE_MAX);
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    return 0;
}

/* Set local variables in codethread through attr. */
MRT_STATIC_INLINE void CODEThreadNewSetLocalData(struct CODEThread *codethread, const struct CODEThreadAttrInner *attr)
{
    if (attr == nullptr || attr->hasSpecificData == false) {
        return;
    }
    (void)memcpy_s(codethread->localData, CODETHREAD_KEYS_MAX * sizeof(void *),
                   attr->specificData, CODETHREAD_KEYS_MAX * sizeof(void *));
}

/* Set stack info in codethread through attr. */
MRT_INLINE static void CODEThreadNewSetAttr(const struct CODEThreadAttrInner *attr,
                                          struct ScheduleCODEThread *scheduleCODEThread,
                                          struct StackAttr *stackAttr)
{
    stackAttr->stackSizeAlign = scheduleCODEThread->stackSize;
    stackAttr->stackGrow = scheduleCODEThread->stackGrow;
    if (attr != nullptr) {
        if (attr->stackSize != 0) {
            // stacksize page alignment
            stackAttr->stackSizeAlign = STACK_ADDR_ALIGN_UP(attr->stackSize, SchedulePageSize());
        }
    }
}

struct CODEThread* CODEThreadBuild(ScheduleHandle schedule, const struct CODEThreadAttr *attrUser, CODEThreadFunc func,
                               const void *argStart, unsigned int argSize, bool isSignal)
{
    struct StackAttr stackAttr;
    struct CODEThread *newCODEThread;
    struct Schedule *currentSchedule;
    struct Schedule *targetSchedule = (struct Schedule *)schedule;
    struct ArgAttr argAttr;
    struct ScheduleCODEThread *scheduleCODEThread;
    const struct CODEThreadAttrInner *attr = reinterpret_cast<const struct CODEThreadAttrInner *>(attrUser);

    currentSchedule = ScheduleGet();
    argAttr.argStart = argStart;
    argAttr.argSize = argSize;
    if (targetSchedule == nullptr || (targetSchedule->scheduleType != SCHEDULE_DEFAULT &&
                                   targetSchedule->state == SCHEDULE_WAITING)) {
        HILOG_ERROR(ERRNO_SCHD_INVALID, "can't new codethread because schedule state is waiting");
        return nullptr;
    }
    scheduleCODEThread = &targetSchedule->schdCODEThread;
    if (CODEThreadAttrCheck(attr, func, argStart, argSize) != 0) {
        return nullptr;
    }

    // Set stack info in codethread through attr or default value
    CODEThreadNewSetAttr(attr, scheduleCODEThread, &stackAttr);
    // Method of obtaining the codethread control block
    CODEThreadBuf buf =
        (CODEThreadGet() == nullptr || currentSchedule != targetSchedule || isSignal) ? GLOBAL_BUF : LOCAL_BUF;

    // scheduleCODEThread increases by 1. The count is calculated when the codethread upper limit
    // exists. The count is mandatory for a non-default scheduler. The count is used to
    // determine whether a codethread is being executed when the scheduler exits.
    if (targetSchedule->scheduleType != SCHEDULE_DEFAULT) {
        atomic_fetch_add(&scheduleCODEThread->codethreadNum, 1ULL);
    }
    newCODEThread = CODEThreadAlloc(targetSchedule, &argAttr, &stackAttr, buf);
    if (newCODEThread == nullptr) {
        if (targetSchedule->scheduleType != SCHEDULE_DEFAULT) {
            atomic_fetch_sub(&scheduleCODEThread->codethreadNum, 1ULL);
        }
        return nullptr;
    }

    CODEThreadNewSetLocalData(newCODEThread, attr);

#if defined(CODIRA_TSAN_SUPPORT)
    MapleRuntime::Sanitizer::TsanNewRaceState(newCODEThread, CODEThreadGet(), __builtin_return_address(0));
    MapleRuntime::Sanitizer::TsanCleanShadow(newCODEThread->stack.stackTopAddr, newCODEThread->stack.totalSize);
#endif

    CODEThreadMake(attr, func, newCODEThread);

    atomic_store_explicit(&newCODEThread->state, CODETHREAD_READY, std::memory_order_relaxed);

#ifdef __OHOS__
    newCODEThread->stackInfo.stackLimit =
        static_cast<unsigned long long int>(reinterpret_cast<uintptr_t>(newCODEThread->stack.stackTopAddr));
#endif

    return newCODEThread;
}

/* Create a codethread in the codethread context. */
CODEThreadHandle CODEThreadNew(ScheduleHandle schedule, const struct CODEThreadAttr *attrUser, CODEThreadFunc func,
                           const void *argStart, unsigned int argSize, bool isSignal)
{
    unsigned long long codethreadId = CODEThreadNewId();
#ifdef __OHOS__
    TRACE_START_ASYNC(TRACE_CODETHREAD_NEW, codethreadId);
#elif defined(__ANDROID__)
    TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_NEW, codethreadId));
#endif
    int error = 0;
    struct Schedule *currentSchedule;
    struct Schedule *targetSchedule = (struct Schedule *)schedule;
    struct ScheduleCODEThread *scheduleCODEThread = &targetSchedule->schdCODEThread;
    currentSchedule = ScheduleGet();
    struct CODEThread* newCODEThread = CODEThreadBuild(schedule, attrUser, func, argStart, argSize, isSignal);
    if (newCODEThread == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "build codethread failed");
        return nullptr;
    }
    // Set codethread id in CODEThreadNew
    CODEThreadSetId(newCODEThread, codethreadId);
    CODEThreadBuf buf =
        (CODEThreadGet() == nullptr || currentSchedule != targetSchedule || isSignal) ? GLOBAL_BUF : LOCAL_BUF;
#ifdef __OHOS__
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_NEW, codethreadId);
#elif defined(__ANDROID__)
    TRACE_FINISH();
#endif
    // Put new codethread into the running queue of processor
    if (targetSchedule->scheduleType == SCHEDULE_UI_THREAD &&
        g_scheduleManager.postTaskFunc != nullptr) {
        error = AddToCODESingleModeThreadList(newCODEThread);
    } else if (buf == GLOBAL_BUF) {
        error = ScheduleGlobalWrite(&newCODEThread, 1);
    } else {
        error = ProcessorLocalWrite(newCODEThread);
    }
    if (error) {
        atomic_store_explicit(&newCODEThread->state, CODETHREAD_IDLE, std::memory_order_relaxed);
        CODEThreadFree(newCODEThread, true);
        if (targetSchedule->scheduleType != SCHEDULE_DEFAULT) {
            atomic_fetch_sub(&scheduleCODEThread->codethreadNum, 1ULL);
        }
        HILOG_ERROR(error, "codethread add to running queue failed");
        return nullptr;
    }

    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_CREATE)) {
        ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_CREATE, TRACE_STACK_10, nullptr, 1, codethreadId);
    }

    if (targetSchedule->scheduleType == SCHEDULE_UI_THREAD &&
        g_scheduleManager.postTaskFunc != nullptr) {
        return newCODEThread;
    }
    
    // Attempt to start a thread to perform scheduling.
    ProcessorWake(targetSchedule, nullptr);
    return newCODEThread;
}

/* Submit tasks from an external thread to the scheduling framework. */
CODEThreadHandle CODEThreadNewToSchedule(ScheduleHandle schedule, const struct CODEThreadAttr *attr,
                                     CODEThreadFunc func, const void *argStart, unsigned int argSize, bool isSignal)
{
    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "schedule null invalid");
        return nullptr;
    }
    return CODEThreadNew(schedule, attr, func, argStart, argSize, isSignal);
}

CODEThreadHandle CODEThreadNewToDefault(const struct CODEThreadAttr *attr, CODEThreadFunc func,
                                    const void *argStart, unsigned int argSize)
{
    if (g_scheduleManager.defaultSchedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "schedule null invalid");
        return nullptr;
    }
    return CODEThreadNew(g_scheduleManager.defaultSchedule, attr, func, argStart, argSize);
}

/* Hook registered externally with the scheduling framework. */
int CODEThreadSchdHookRegister(SchdCODEThreadHookFunc func, CODEThreadSchdHook type)
{
    struct Schedule *schedule;

    if (type >= SCHD_HOOK_BUTT) {
        return ERRNO_SCHD_HOOK_INVLAID;
    }
    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }
    if (schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_IS_RUNNING;
    }

    g_scheduleManager.schdCODEThreadHook[type] = func;
    return 0;
}

int CODEThreadStateHookRegister(SchdCODEThreadStateHookFunc func, CODEThreadStateHook type)
{
    struct Schedule *schedule;

    if (type >= CODETHREAD_STATE_HOOK_BUTT) {
        return ERRNO_SCHD_HOOK_INVLAID;
    }
    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }
    if (schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_IS_RUNNING;
    }

    g_scheduleManager.schdCODEThreadStateHook[type] = func;
    return 0;
}

void *CODEThreadMpark(struct CODEThread *parkCODEThread)
{
    int error;
    struct CODEThread *codethread0;
    ParkCallbackFunc callbackFunc;
    // Update codethread status to PENDING
    MapleRuntime::Mutator* mutator = parkCODEThread->mutator;
    if (parkCODEThread->schedule->scheduleType == SCHEDULE_UI_THREAD) {
        MapleRuntime::ThreadLocal::SetMutator(nullptr);
    }
    auto& context = parkCODEThread->context;
    mutator->PreparedToPark((void*)context.GetPC(), (void*)context.GetFrameAddress());
    atomic_store_explicit(&parkCODEThread->state, CODETHREAD_PENDING, std::memory_order_relaxed);
    codethread0 = CODEThreadGet();
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(codethread0);
#endif
    callbackFunc = (ParkCallbackFunc)(codethread0->func);
    if (callbackFunc != nullptr) {
        // Note: park_codethread cannot be accessed when the callback function succeeds. After
        // the callback is successful, park_codethread may be taken by other threads and then
        // freed. In this case, park_codethread is a wild pointer.
        error = callbackFunc(codethread0->argStart, (CODEThreadHandle)parkCODEThread);
        if (error != 0) {
            // If the callback function fails, roll back to park_codethread.
            atomic_store_explicit(&parkCODEThread->state, CODETHREAD_RUNNING, std::memory_order_relaxed);
            parkCODEThread->result = error;
            MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
            tlData->mutator = mutator;
            mutator->PreparedToRun(tlData);
            if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_UNBLOCK)) {
                ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_UNBLOCK, TRACE_STACK_1, nullptr, 1,
                                         CODEThreadGetId(static_cast<CODEThreadHandle>(parkCODEThread)));
                ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                                         CODEThreadGetId(static_cast<CODEThreadHandle>(parkCODEThread)));
            }
#ifdef CODIRA_ASAN_SUPPORT
            // target to next code thread, just switch
            MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(codethread0, parkCODEThread);
            MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(parkCODEThread);
#endif
            TRACE_FINISH_ASYNC(TRACE_CODETHREAD_PARK, parkCODEThread->id);
#ifdef __OHOS__
            TRACE_START_ASYNC(TRACE_CODETHREAD_EXEC, parkCODEThread->id);
#elif defined(__ANDROID__)
            TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXEC, parkCODEThread->id));
#endif
            CODEThreadExecute(parkCODEThread, (void**)&tlData->codethread);
        }
    }

    // Search a schedulable codethread
    ProcessorSchedule();
    return nullptr;
}

int CODEThreadParkInForeignThread(CODEThread* codethread, ParkCallbackFunc func, void *arg)
{
    // codethread park in foreign thread do not call CODEThreadMCall,
    // so we should update the codethread context by call CODEThreadContextGet
    CODEThreadContextGet(&codethread->context);
    // Update codethread status to PENDING
    MapleRuntime::Mutator* mutator = codethread->mutator;
    auto& context = codethread->context;
    mutator->PreparedToPark((void*)context.GetPC(), (void*)context.GetFrameAddress());
    Thread* thread = codethread->thread;
    thread->state = THREAD_PRE_SLEEP;
    atomic_store_explicit(&codethread->state, CODETHREAD_PENDING, std::memory_order_relaxed);
    int error = func(arg, (CODEThreadHandle)codethread);
    if (error != 0) {
        return codethread->result;
    }

    // Wait for the semaphor, do not schedule in foreign thread.
    thread->state = THREAD_SLEEP;
    int ret = SemaphoreWait(&(thread->sem));
    if (ret != 0) {
        LOG_ERROR(ret, "sem_wait failed");
    }
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_PARK, codethread->id);
#ifdef __OHOS__
    TRACE_START_ASYNC(TRACE_CODETHREAD_EXEC, codethread->id);
#elif defined(__ANDROID__)
    TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXEC, codethread->id));
#endif

    // Check the timer.
    ProcessorCheckFunc checkFunc;
    unsigned long long now = 0;
    checkFunc = g_scheduleManager.check[PROCESSOR_TIMER_HOOK];
    if (checkFunc != nullptr) {
        checkFunc(ProcessorGet(), &now, nullptr);
    }

    atomic_store_explicit(&codethread->state, CODETHREAD_RUNNING, std::memory_order_relaxed);
    MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
    tlData->mutator = mutator;
    mutator->PreparedToRun(tlData);
    thread->state = THREAD_RUNNING;
    return codethread->result;
}

/* Park the current codethread and schedule the next codethread. */
int CODEThreadPark(ParkCallbackFunc func, TraceEvent waitReason, void *arg)
{
    struct CODEThread *codethread0;
    struct CODEThread *codethread;

    codethread = CODEThreadGet();
    if (UNLIKELY(codethread == nullptr)) {
        MapleRuntime::ThreadType threadType = MapleRuntime::ThreadLocal::GetThreadType();
        const char* threadTypeInfo = threadType == MapleRuntime::ThreadType::FP_THREAD ?
                                                  "finalizer thread" :
                                                  "normal thread";
        HILOG_FATAL(ERRNO_SCHD_CODETHREAD_PARK_FAILED,
                    "codethread park failed because of null codethread and current thread is %s", threadTypeInfo);
    }
    codethread->result = 0;

#ifdef __OHOS__
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_EXEC, codethread->id);
#elif defined(__ANDROID__)
    TRACE_FINISH();
#endif
    TRACE_START_ASYNC(TRACE_CODETHREAD_PARK, codethread->id);
#ifdef __OHOS__
    // codethread is with foreign context if singleModelC2NCount size greater than 1.
    // codethread is on UI thread if schedule type is SCHEDULE_UI_THREAD.
    if (codethread->schedule->scheduleType == SCHEDULE_UI_THREAD && codethread->singleModelC2NCount > 0) {
        HILOG_WARN(ERRNO_SCHD_UITHREAD_ERROR,
                   "parking codethread with foreign function context on UI thread is not permitted.");
        return CODEThreadParkInForeignThread(codethread, func, arg);
    }
#endif

    // If codethread is in foregin thread schedule, do not schedule, just park.
    if (codethread->schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return CODEThreadParkInForeignThread(codethread, func, arg);
    }
    codethread0 = static_cast<struct CODEThread *>(ThreadGet()->codethread0);
    // The func field of codethread0 is used to store callback functions.
    codethread0->func = (CODEThreadFunc)func;
    codethread0->argStart = arg;
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(codethread, codethread0);
#endif
    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & waitReason)) {
        if (waitReason == TRACE_EV_CODETHREAD_BLOCK_NET) {
            ScheduleTraceEventOrigin(waitReason, -1, nullptr, 1, SpecialStackId::CODETHREAD_NET_BLOCK);
        } else {
            ScheduleTraceEventOrigin(waitReason, TRACE_STACK_1, nullptr, 0);
        }
    }
    CODEThreadMcall(reinterpret_cast<void *>(CODEThreadMpark), CODEThreadAddr());
    return codethread->result;
}

void CODEThreadWait()
{
    CODEThreadPark(NULL, TRACE_EV_CODETHREAD_BLOCK, NULL);
}

/**
 * CODEThreadMRAW Parks the given thread, and executes the next thread specified in
 * codethread0->argStart.
 * It does this without going through the scheduler, but deterministically choosing the next
 * codethread to execute.
 *
 * Note: the implementation is based on CODEThreadMPark and ProcessorSchedule
 */
void *CODEThreadMRAW(struct CODEThread *parkCODEThread)
{
    /* COPIED AND MODIFIED FROM CODEThreadMpark */
    {
        // Park parkCODEThread
        MapleRuntime::Mutator* mutator = parkCODEThread->mutator;
        auto& context = parkCODEThread->context;
        mutator->PreparedToPark((void*)context.GetPC(), (void*)context.GetFrameAddress());
        atomic_store_explicit(&parkCODEThread->state, CODETHREAD_PENDING, std::memory_order_relaxed);
    }
    /* END COPY */

    struct CODEThread *codethread0 = CODEThreadGet();
    // Recover the next thread to run from `argStart`
    // See the invocation from CODEThreadResumeAndWait
    struct CODEThread *nextCODEThread = static_cast<struct CODEThread *>(codethread0->argStart);

    /* COPIED AND MODIFIED FROM ProcessorSchedule() */
    {
        atomic_store_explicit(&nextCODEThread->state, CODETHREAD_RUNNING, std::memory_order_relaxed);
        ProtectAddrSet((uintptr_t)nextCODEThread->stack.stackGuard);
        if (nextCODEThread->boundThread != nullptr) {
            LOG_ERROR(-1, "BOUND THREADS NOT SUPPORTED WITH EFFECTS");
        } else {
            // Execute the thread
            MapleRuntime::Mutator* mutator = nextCODEThread->mutator;
            MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
            tlData->mutator = mutator;
            mutator->PreparedToRun(tlData);
#ifdef CODIRA_ASAN_SUPPORT
            // target to next code thread, just switch
            MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(CODEThreadGet(), nextCODEThread);
            MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(nextCODEThread);
#endif
#ifdef __OHOS__
            TRACE_START_ASYNC(TRACE_CODETHREAD_EXEC, nextCODEThread->id);
#elif defined (__ANDROID__)
            TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXEC, nextCODEThread->id));
#endif
#ifdef __OHOS__
            if (nextCODEThread->schedule->scheduleType == SCHEDULE_UI_THREAD) {
                SingleCODEThreadStoreSP();
            }
#endif

            CODEThreadExecute(nextCODEThread, (void**)&tlData->codethread);
        }
    }
    /* END COPY */

    return nullptr;
}

void CODEThreadResumeAndWait(CODEThreadHandle readyThread)
{
    struct CODEThread *codethread0 = static_cast<struct CODEThread *>(ThreadGet()->codethread0);
    /* This is a trick to pass an extra argument to CODEThreadMRAW */
    codethread0->argStart = readyThread;
    CODEThreadMcall(reinterpret_cast<void *>(CODEThreadMRAW), CODEThreadAddr());
}

void *CODEThreadMresched(struct CODEThread *reCODEThread)
{
    struct Schedule *schedule = reCODEThread->schedule;
    MapleRuntime::Mutator* mutator = reCODEThread->mutator;
    auto& context = reCODEThread->context;
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(CODEThreadGet());
#endif
    mutator->PreparedToPark((void*)context.GetPC(), (void*)context.GetFrameAddress());
    struct CODEThread *lastCODEThread;
    atomic_store_explicit(&reCODEThread->state, CODETHREAD_READY, std::memory_order_relaxed);
    // If the global queue is not empty, the codethread is added into the global queue.
    // Otherwise, the codethread is recorded in lastCODEThread of the scheduler and the last
    // reschedule codethread is added to the local queue. Minimize access to global queues
    // while ensuring fairness. The scheduler searches for codethreads in the following
    // sequence: local -> lastCODEThread -> global
    if (schedule->schdCODEThread.num != 0) {
        ScheduleGlobalWrite(&reCODEThread, 1);
    } else {
        lastCODEThread = reinterpret_cast<struct CODEThread *>(atomic_exchange(&schedule->lastCODEThread,
                                                                           reCODEThread));
        if (lastCODEThread) {
            ProcessorLocalWrite(lastCODEThread, true);
        }
    }
    ProcessorSchedule();
    return nullptr;
}

/* Change the codethread from running to ready and put it back at the running queue.
 * Note: This function always returns 0. The return value is meaningless. However, the function
 * definition must return int and cannot be defined as void. This is mainly to prevent compiler
 * optimizations. If void is returned, the compiler exits the stack before calling the second
 * hook function. The instruction for calling the hook is jmp instead of call, which causes
 * the stack frame to change. If the return value is int, the compiler does not perform this
 * optimization, the instruction that calls the hook is still call, and the stack is not
 * dropped first.
 * Disables inlining of this function. The same problem also occurs if the function is called
 * by another function and is inline. For example, CODEThreadPreempt.
 **/
__attribute__((noinline)) int CODEThreadResched(void)
{
    // If codethread is in foreign thread schedule, do not reschedule.
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread->schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return 0;
    }
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(CODEThreadGet(), ThreadGet()->codethread0);
#endif
#ifdef __OHOS__
    TRACE_FINISH_ASYNC(TRACE_CODETHREAD_EXEC, codethread->id);
#elif defined(__ANDROID__)
    TRACE_FINISH();
#endif
    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_RESCHED)) {
        ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_RESCHED, -1, nullptr,
                                 1, SpecialStackId::CODETHREAD_RESCHED);
    }
    CODEThreadMcall(reinterpret_cast<void *>(CODEThreadMresched), CODEThreadAddr());
    return 0;
}

int CODEThreadTryResched(void)
{
    struct Processor* processor = ProcessorGet();
    unsigned long overload = processor->schedCnt - processor->obRecord.lastSchedCnt;
    if (overload > PROCESSOR_SCHED_COUNT_THRESHOLD && overload % PROCESSOR_SCHED_COUNT_THRESHOLD != 0) {
        return 1;
    }
    return CODEThreadResched();
}

/* Collaborative preemption. The current codethread abandons the execution. */
void CODEThreadPreemptResched(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (PreemptFlagGet() != PREEMPT_DO_FLAG) {
        return;
    }
    PreemptFlagSet(0);
    if (codethread->preemptOffCnt != 0) {
        return;
    }
    ScheduleGet()->schdCODEThread.preemptCnt++;
    CODEThreadResched();
}

bool ShouldWakeDirectly(Schedule* schedule, CODEThread* codethread)
{
    // If code thread is in foreign thread schedule, just wake this schedule.
    if (schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return true;
    }
#ifdef __OHOS__
    // If code thread is in UI Thread schedule and the post task fun is registered,
    // and C2N count is greater than 0(there are native func in stack),
    // just wake this schedule.
    if (schedule->scheduleType == SCHEDULE_UI_THREAD &&
        g_scheduleManager.postTaskFunc != nullptr &&
        codethread->singleModelC2NCount > 0) {
        return true;
    }
#else
    (void)codethread;
#endif
    return false;
}

/* CODEThread ready, adds the specified codethread to the running queue. */
void CODEThreadReady(CODEThreadHandle readyCODEThread)
{
    struct CODEThread *codethread = (struct CODEThread *)readyCODEThread;
    CODEThreadState expected = CODETHREAD_PENDING;
    struct Schedule *schedule;
    if (readyCODEThread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "ready_codethread is null");
        return;
    }

    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_UNBLOCK)) {
        struct CODEThread *currCODEThread = CODEThreadGet();
        if (currCODEThread != nullptr && currCODEThread->isCODEThread0) {
            ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_UNBLOCK, -1, nullptr, TRACE_ARGS_2,
                                     CODEThreadGetId(readyCODEThread), SpecialStackId::CODETHREAD_UNBLOCK);
        } else {
            ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_UNBLOCK, TRACE_STACK_10, nullptr,
                                     TRACE_ARGS_1, CODEThreadGetId(readyCODEThread));
        }
    }
    schedule = codethread->schedule;
    // Use CAS to prevent CODEThreadReady concurrency. CAS may fail, which is normal.
    if (atomic_compare_exchange_strong(&codethread->state, &expected, CODETHREAD_READY)) {
        // If code thread is in foreign thread schedule, just wake this schedule,
        // do not push codethread to global or local list.
        if (ShouldWakeDirectly(schedule, codethread)) {
            ProcessorWake(schedule, nullptr);
            return;
        }
        TRACE_FINISH_ASYNC(TRACE_CODETHREAD_PARK, codethread->id);
        if (schedule->scheduleType == SCHEDULE_UI_THREAD &&
            g_scheduleManager.postTaskFunc != nullptr) {
            AddToCODESingleModeThreadList(codethread);
            return;
        }
        if (ScheduleGet() != schedule || CODEThreadGet() == nullptr) {
            ScheduleGlobalWrite(&codethread, 1);
        } else {
            ProcessorLocalWrite(codethread);
        }
        ProcessorWake(schedule, nullptr);
    }
}

/* Add codethreads to the running queue in batches. Codethreads have been set to the ready
 * state before calling this function. */
int CODEThreadAddBatch(CODEThreadHandle *list, unsigned int num)
{
    int error;
    struct CODEThread *codethread;
    struct Schedule *schedule;
    unsigned int proNum;
    unsigned int i;

    if (num == 0) {
        return 0;
    }
    schedule = ((struct CODEThread **)list)[0]->schedule;
    codethread = CODEThreadGet();
    if (codethread == nullptr) {
        // The current context is not a codethread. All are put into the global queue.
        error = ScheduleGlobalWrite((struct CODEThread **) list, num);
        if (error) {
            LOG_ERROR(error, "ScheduleGlobalWrite failed");
            return error;
        }
        for (i = 0; i < num; i++) {
            ProcessorWake(schedule, nullptr);
        }
    } else {
        proNum = schedule->schdProcessor.freeNum;
        if (proNum >= num) {
            // If the number of free processors is greater than the number of codethreads, all
            // the codethreads are put into the global queue.
            error = ScheduleGlobalWrite((struct CODEThread **) list, num);
            if (error) {
                LOG_ERROR(error, "ScheduleGlobalWrite failed");
                return error;
            }
            for (i = 0; i < num; i++) {
                ProcessorWake(schedule, nullptr);
            }
        } else {
            // If the number of free processors is less than the number of codethreads, n codethreads
            // are selected to enter the global queue (n indicates the number of idle processors
            // currently), and others enter the local queue.
            error = ScheduleGlobalWrite((struct CODEThread **) list, proNum);
            if (error) {
                LOG_ERROR(error, "ScheduleGlobalWrite failed");
                return error;
            }
            error = ProcessorLocalWriteBatch((struct CODEThread **)(list + proNum), num - proNum);
            if (error) {
                LOG_ERROR(error, "ProcessorLocalWriteBatch failed");
                return error;
            }
            for (i = 0; i < proNum; i++) {
                ProcessorWake(schedule, nullptr);
            }
        }
    }
    return 0;
}

unsigned long long int CODEThreadId(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return 0;
    }
    if (codethread->id == CODETHREAD_INIT_ID) {
        codethread->id = CODEThreadNewId();
    }

    return codethread->id;
}

char* CODEThreadGetName(void* codethreadPtr)
{
    struct CODEThread *codethread = reinterpret_cast<struct CODEThread *>(codethreadPtr);
    if (codethread == nullptr) {
        return nullptr;
    }
    return codethread->name;
}

int CODEThreadGetState(void* codethreadPtr)
{
    struct CODEThread *codethread = reinterpret_cast<struct CODEThread *>(codethreadPtr);
    if (codethread == nullptr) {
        return -1;
    }
    return codethread->state.load();
}

unsigned long long int CODEThreadGetId(CODEThreadHandle handle)
{
    struct CODEThread *codethread = (struct CODEThread*)(handle);
    if (codethread == nullptr) {
        return 0;
    }
    if (codethread->id == CODETHREAD_INIT_ID) {
        codethread->id = CODEThreadNewId();
    }
    return codethread->id;
}

/* ensure codethread is not nullptr */
void CODEThreadSetId(struct CODEThread *codethread, unsigned long long id)
{
    if (codethread->id == CODETHREAD_INIT_ID) {
        codethread->id = id;
    }
}

CODEThreadHandle CODEThreadGetHandle()
{
    return CODEThreadGet();
}

void CODEThreadSetName(CODEThreadHandle handle, const char *name, size_t len)
{
    int error = 0;
    struct CODEThread *codethread = (struct CODEThread*)(handle);
    if (codethread == nullptr || name == nullptr) {
        return;
    }
    if (len > CODETHREAD_NAME_SIZE - 1) {
        len = CODETHREAD_NAME_SIZE - 1;
    }

    error = memcpy_s(codethread->name, CODETHREAD_NAME_SIZE, name, len);
    if (error) {
        LOG_ERROR(error, "codethread name set failed");
        return;
    } else {
        codethread->name[len] = '\0';
    }
#ifdef __OHOS__
    MapleRuntime::ScopedEntryAsyncTrace(TRACE_CODETHREAD_SETNAME, codethread->id, codethread->name);
#elif defined(__ANDROID__)
    TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_SETNAME, codethread->id, 1, codethread->name));
    TRACE_FINISH();
#endif
}

void CODEThreadGetInfo(struct CODEThread *codethread, struct CODEThreadInfo *codethreadInfo)
{
    struct Processor *processor = nullptr;
    unsigned int processorId = static_cast<unsigned int>(-1);
    struct Thread *thread = static_cast<struct Thread *>(codethread->thread);

    if (codethread->id == CODETHREAD_INIT_ID) {
        codethread->id = CODEThreadNewId();
    }
    codethreadInfo->state = codethread->state;
    codethreadInfo->id = codethread->id;

    CHECK_DETAIL(strcpy_s(codethreadInfo->name, CODETHREAD_NAME_SIZE, codethread->name) == EOK,
                 "strcpy_s failed when copy func or file names.\n");

    if (thread != nullptr) {
        processor = static_cast<struct Processor *>(thread->processor);
#ifdef MRT_MACOS
        codethreadInfo->pthreadId = 0;
#else
        codethreadInfo->pthreadId = static_cast<unsigned long long>(thread->osThread);
#endif
        codethreadInfo->tid = static_cast<unsigned int>(thread->tid);
        if (processor != nullptr) {
            processorId = processor->processorId;
        }
    } else {
        codethreadInfo->pthreadId = 0;
        codethreadInfo->tid = 0;
    }
    codethreadInfo->processorId = processorId;
    codethreadInfo->argSize = codethread->argSize;
    codethreadInfo->argStart = codethread->argStart;
    codethreadInfo->context = codethread->context;
}

int CODEThreadStackReversedSet(uintptr_t size)
{
    struct Schedule *schedule = ScheduleGet();
    // Not allowed to modify at runtime
    if (schedule != nullptr) {
        return ERRNO_SCHD_IS_RUNNING;
    }

    // The value cannot be smaller than the default value because space needs to be reserved
    // for stack overflow processing.
    if (size < STACK_DEFAULT_REVERSED) {
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    g_codethreadStackReservedSize = size;
    return 0;
}

uintptr_t CODEThreadStackReversedGet(void)
{
    return g_codethreadStackReservedSize;
}

/* This function is used for the exception try catch mechanism of cangjie. After the stack
 * overflow exception is caught, the stack boundary needs to be temporarily extended. */
void CODEThreadStackGuardExpand(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    codethread->stack.stackGuard -= g_codethreadStackReservedSize;
    ProtectAddrSet(reinterpret_cast<uintptr_t>(codethread->stack.stackGuard));
}

/* This function is used for the exception try catch mechanism of cangjie. After the stack
 * overflow exception is caught, the stack boundary needs to be restored. */
void CODEThreadStackGuardRecover(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    codethread->stack.stackGuard += g_codethreadStackReservedSize;
    ProtectAddrSet(reinterpret_cast<uintptr_t>(codethread->stack.stackGuard));
}

int CODEBindOSThread(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is null");
        return -1;
    }
    struct Thread *thread = ThreadGet();
    if (codethread->boundThread != nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_HAS_BEEN_BOUND_TO_THREAD, "codethread has been bound to thread");
        return -1;
    }
    codethread->boundThread = thread;
    thread->boundCODEThread = codethread;
    return 0;
}

int CODEUnbindOSThread(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is null");
        return -1;
    }
    struct Thread *thread = ThreadGet();
    if (codethread->boundThread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NOT_BOUND_TO_THREAD, "codethread not bound to thread");
        return -1;
    }
    codethread->boundThread = nullptr;
    thread->boundCODEThread = nullptr;
    return 0;
}

void *CODEThreadStackGuardGet(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return nullptr;
    }
#ifdef __arm__
    struct Schedule *schedule = ScheduleGet();
    if (schedule == nullptr) {
        return nullptr;
    }
    if (schedule->scheduleType == SCHEDULE_FOREIGN_THREAD) {
        return nullptr;
    }
#endif
    return codethread->stack.stackGuard;
}

void *CODEThreadStackAddrGet(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return nullptr;
    }
    return codethread->stack.stackTopAddr;
}

void *CODEThreadStackBaseAddrGet(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return nullptr;
    }
    return codethread->stack.codethreadStackBaseAddr;
}

void *CODEThreadStackAddrGetByCODEThrd(struct CODEThread *codethread)
{
    if (codethread == nullptr) {
        codethread = CODEThreadGet();
    }
    if (codethread == nullptr) {
        return nullptr;
    }
    return codethread->stack.stackTopAddr;
}

void *CODEThreadStackBaseAddrGetByCODEThrd(struct CODEThread *codethread)
{
    if (codethread == nullptr) {
        codethread = CODEThreadGet();
    }
    if (codethread == nullptr) {
        return nullptr;
    }
    return codethread->stack.codethreadStackBaseAddr;
}

int CODEThreadPreemptOffCntAdd(void)
{
    struct CODEThread *codethread = CODEThreadGet();

    if (codethread == nullptr) {
        return ERRNO_SCHD_PREEMPT_CNT_INVALID;
    }
    if (codethread->preemptOffCnt == UINT_MAX) {
        LOG_ERROR(ERRNO_SCHD_PREEMPT_CNT_INVALID, "preempt_off_cnt overflow");
        return ERRNO_SCHD_PREEMPT_CNT_INVALID;
    }
    codethread->preemptOffCnt++;
    return 0;
}

int CODEThreadPreemptOffCntSub(void)
{
    struct CODEThread *codethread = CODEThreadGet();

    if (codethread == nullptr) {
        return ERRNO_SCHD_PREEMPT_CNT_INVALID;
    }
    if (codethread->preemptOffCnt == 0) {
        LOG_ERROR(ERRNO_SCHD_PREEMPT_CNT_INVALID, "preempt_off_cnt flip");
        return ERRNO_SCHD_PREEMPT_CNT_INVALID;
    }
    codethread->preemptOffCnt--;
    return 0;
}


int CODEThreadDestructorHookRegister(SchdDestructorHookFunc func)
{
    struct Schedule *schedule;

    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }
    if (schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_IS_RUNNING;
    }

    g_scheduleManager.destructorFunc = func;
    return 0;
}

int CODEThreadGetMutatorStatusHookRegister(SchdMutatorStatusHookFunc func)
{
    struct Schedule *schedule;

    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }
    if (schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_IS_RUNNING;
    }

    g_scheduleManager.mutatorStatusFunc = func;
    return 0;
}

int CODEThreadSetMutator(void *mutator)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return ERRNO_SCHD_CODETHREAD_NULL;
    }
    codethread->mutator = reinterpret_cast<MapleRuntime::Mutator*>(mutator);
    codethread->mutator->SetCodethreadPtr(codethread);
    return 0;
}

void *CODEThreadGetMutator(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return nullptr;
    }
    return codethread->mutator;
}

void* GetCODEThreadScheduler()
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return nullptr;
    }
    return codethread->schedule;
}

void RebindCODEThread(void* codethread)
{
    CODEThread* codeRebindThread = reinterpret_cast<CODEThread*>(codethread);
    if (codeRebindThread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return;
    }
    codeRebindThread->thread = codeRebindThread->schedule->thread0;
}

/* lua codethreads run wrapper functions for user tasks */
void *LuaFuncWrap(void *arg, unsigned int size)
{
    (void)size;
    struct CODEThread *codethread = CODEThreadGet();
    struct LuaCODEThread *luaCODEThread = *(struct LuaCODEThread **)arg;

    codethread->isLuaCODEThread = true;
    luaCODEThread->result = luaCODEThread->func(luaCODEThread->arg);
    codethread->isLuaCODEThread = false;
    luaCODEThread->state = LUA_CODETHREAD_DONE;
    // Wake up the caller after the lua codethread ends
    SemaphorePost(&luaCODEThread->sem);
    return nullptr;
}

CODEThreadHandle CODEThreadCreate(const struct CODEThreadAttr *attrUser, LuaCODEThreadFunc func, void *arg)
{
    struct LuaCODEThread *luaCODEThread;

    if (func == nullptr || attrUser == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "arg invalid");
        return nullptr;
    }

    if (ScheduleGet() == nullptr) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "schedule not inited");
        return nullptr;
    }

    luaCODEThread = (struct LuaCODEThread*)malloc(sizeof(struct LuaCODEThread));
    if (luaCODEThread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "malloc failed");
        return nullptr;
    }

    (void)memset_s(luaCODEThread, sizeof(struct LuaCODEThread), 0, sizeof(struct LuaCODEThread));
    luaCODEThread->func = func;
    luaCODEThread->arg = arg;
    luaCODEThread->state = LUA_CODETHREAD_INIT;
    (void)memcpy_s(&luaCODEThread->attrUser, sizeof(struct CODEThreadAttr),
                   attrUser, sizeof(struct CODEThreadAttr));

    // The C side thread invokes this interface and uses semaphore for synchronization control.
    if (SemaphoreInit(&luaCODEThread->sem, 0, 0) != 0) {
        LOG_ERROR(errno, "sem_init failed");
        free(luaCODEThread);
        return nullptr;
    }

    return luaCODEThread;
}

int CODEThreadYieldCallback(void *arg, CODEThreadHandle handle)
{
    (void)handle;
    struct LuaCODEThread *luaCODEThread = (struct LuaCODEThread *)arg;
    luaCODEThread->state = LUA_CODETHREAD_SUSPENDING;
    SemaphorePost(&luaCODEThread->sem);
    return 0;
}

int CODEThreadYield(void)
{
    struct CODEThread *codethread;
    struct LuaCODEThread *luaCODEThread;
    int state;

    codethread = CODEThreadGet();
    if (codethread == nullptr || codethread->argStart == nullptr || codethread->isLuaCODEThread == false) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "not in lua codethread");
        return ERRNO_SCHD_CODETHREAD_NULL;
    }
    luaCODEThread = *static_cast<struct LuaCODEThread **>(codethread->argStart);

    state = luaCODEThread->state;
    if (state == LUA_CODETHREAD_RUNNING) {
        CODEThreadPark(CODEThreadYieldCallback, TRACE_EV_CODETHREAD_BLOCK, (void *)luaCODEThread);
        return 0;
    } else {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_STATE_INVALID, "codethread state wrong: %d", state);
        return ERRNO_SCHD_CODETHREAD_STATE_INVALID;
    }
}

int CODEThreadResume(CODEThreadHandle codethread)
{
    struct LuaCODEThread *luaCODEThread;
    int state;

    luaCODEThread = (struct LuaCODEThread *)codethread;
    if (luaCODEThread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return ERRNO_SCHD_CODETHREAD_NULL;
    }
    state = luaCODEThread->state;
    switch (state) {
        case LUA_CODETHREAD_INIT: {
            luaCODEThread->state = LUA_CODETHREAD_RUNNING;
            luaCODEThread->codethread = CODEThreadNewToDefault((const struct CODEThreadAttr *)(&luaCODEThread->attrUser),
                                                         LuaFuncWrap, (void *)&luaCODEThread, sizeof(void *));
            if (luaCODEThread->codethread == nullptr) {
                luaCODEThread->state = LUA_CODETHREAD_INIT;
                LOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "CODEThreadNew failed");
                return ERRNO_SCHD_CODETHREAD_ALLOC_FAILED;
            }
            SemaphoreWait(&luaCODEThread->sem);
            return 0;
        }
        case LUA_CODETHREAD_SUSPENDING: {
            luaCODEThread->state = LUA_CODETHREAD_RUNNING;
            CODEThreadReady(luaCODEThread->codethread);
            SemaphoreWait(&luaCODEThread->sem);
            return 0;
        }
        default: {
            LOG_ERROR(ERRNO_SCHD_CODETHREAD_STATE_INVALID, "codethread state wrong: %d", state);
            return ERRNO_SCHD_CODETHREAD_STATE_INVALID;
        }
    }
}

int CODEThreadStateGet(CODEThreadHandle handle)
{
    struct LuaCODEThread *codethread = (struct LuaCODEThread *)handle;
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return ERRNO_SCHD_CODETHREAD_NULL;
    }
    return codethread->state;
}

int CODEThreadDestroy(CODEThreadHandle handle)
{
    struct LuaCODEThread *codethread = (struct LuaCODEThread *)handle;
    // Resources can be released only in the initial and end states to avoid resource leakage.
    if (codethread == nullptr || codethread->state == LUA_CODETHREAD_SUSPENDING ||
        codethread->state == LUA_CODETHREAD_RUNNING) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "codethread is nullptr, or not finished");
        return ERRNO_SCHD_CODETHREAD_ARG_INVALID;
    }
    SemaphoreDestroy(&codethread->sem);
    free(codethread);
    return 0;
}

void *CODEThreadResultGet(CODEThreadHandle handle)
{
    struct LuaCODEThread *codethread = (struct LuaCODEThread *)handle;
    if (codethread == nullptr || codethread->state != LUA_CODETHREAD_DONE) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_ARG_INVALID, "codethread is nullptr, or not finished");
        return nullptr;
    }
    return codethread->result;
}

void *CODEThreadGetArg(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread != nullptr) {
        return codethread->argStart;
    }
    return nullptr;
}

int CODEThreadSetStackGrow(bool enableStackGrow)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return ERRNO_SCHD_CODETHREAD_NULL;
    }

    // The stack scaling function is disabled globally. This interface is directly returned.
    if (!codethread->schedule->schdCODEThread.stackGrow) {
        return 0;
    }

    if (enableStackGrow) {
        if (codethread->stack.stackGrowCnt == 0) {
            return 0;
        }
        codethread->stack.stackGrowCnt--;
    } else {
        if (codethread->stack.stackGrowCnt == UINT_MAX) {
            LOG_ERROR(ERRNO_SCHD_STACKGROW_CNT_INVALID, "stackGrow_cnt overflow");
            return ERRNO_SCHD_STACKGROW_CNT_INVALID;
        }
        codethread->stack.stackGrowCnt++;
    }
    
    return 0;
}

size_t CODEThreadStackSizeGet()
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return static_cast<size_t>(-1);
    }

    return codethread->stack.stackSize;
}

size_t CODEThreadStackSizeGetByCODEThrd(struct CODEThread *codethread)
{
    if (codethread == nullptr) {
        codethread = CODEThreadGet();
    }
    if (codethread == nullptr) {
        return static_cast<size_t>(-1);
    }

    return codethread->stack.stackSize;
}

void CODEThreadOldStackFree(void *stackAddr, size_t stackSize)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return;
    }

    CODEThreadStackMemFree(codethread, (char *)stackAddr, stackSize);
}

intptr_t CODEThreadStackAdjust(struct CODEThread *codethread, size_t newStackSizeAlign)
{
    struct Schedule *schedule = codethread->schedule;
    struct StackAttr newStackAttr;
    char *newStackAddr;
    char *spAddress = nullptr;
    char *oldStackBaseAddr;
    char *newStackBaseAddr;
    size_t totalSize;
    size_t stackUsed;

#ifdef CODIRA_SANITIZER_SUPPORT
    // make sure sanitizer context is copied to new stack as well
    oldStackBaseAddr = codethread->stack.codethreadStackBaseAddr + CODETHREAD_SANITIZER_CONTEXT_OFFSET;
#else
    oldStackBaseAddr = codethread->stack.codethreadStackBaseAddr;
#endif
    // Allocate new codethread stack memory
    newStackAttr.stackGrow = true;
    newStackAttr.stackSizeAlign = newStackSizeAlign;
    newStackAddr = CODEThreadStackMemAlloc(schedule, codethread, newStackSizeAlign, &totalSize);
    if (newStackAddr == nullptr) {
        return -1;
    }
    // Update stack attr
    CODEThreadStackAttrInit(codethread, totalSize, newStackAddr, &newStackAttr);
#if defined(CODIRA_TSAN_SUPPORT)
    MapleRuntime::Sanitizer::TsanCleanShadow(codethread->stack.stackTopAddr, codethread->stack.totalSize);
#endif
    // Update the thread local variable protected by the stack.
    ProtectAddrSet(reinterpret_cast<uintptr_t>(codethread->stack.stackGuard));
#ifdef CODIRA_SANITIZER_SUPPORT
    // make sure sanitizer context is copied to new stack as well
    newStackBaseAddr = codethread->stack.codethreadStackBaseAddr + CODETHREAD_SANITIZER_CONTEXT_OFFSET;
#else
    newStackBaseAddr = codethread->stack.codethreadStackBaseAddr;
#endif

#if (VOS_WORDSIZE == 64) && (MRT_HARDWARE_PLATFORM == MRT_ARM)
    asm volatile (
    "mov %0, sp \n"
    :"=r"(spAddress)
    );
#endif

#if (VOS_WORDSIZE == 32) && (MRT_HARDWARE_PLATFORM == MRT_ARM)
    asm volatile (
    "mov %0, sp \n"
    :"=r"(spAddress)
    );
#endif

#if ((MRT_HARDWARE_PLATFORM == MRT_X86) || (MRT_HARDWARE_PLATFORM == MRT_WINDOWS_X86)) && (VOS_WORDSIZE == 64)
    asm volatile (
    "mov %%rsp, %0 \n"
    :"=r"(spAddress)
    );
#endif

    // Calculate the total used size of the old stack.(CodethreadEntry to the stack size used
    // by the current function)
    stackUsed = oldStackBaseAddr - spAddress;
    if (memmove_s(newStackBaseAddr - stackUsed, stackUsed, oldStackBaseAddr - stackUsed, stackUsed) != EOK) {
        LOG_ERROR(-1, "memmove_s failed");
        return -1;
    }

    return newStackBaseAddr - oldStackBaseAddr;
}

intptr_t CODEThreadStackGrow(size_t stackSize)
{
    size_t newStackSize;
    size_t oldStackSize;
    size_t newStackSizeAlign;
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr");
        return -1;
    }
    // Check whether the stack scaling function is disabled.
    if (codethread->stack.stackGrowCnt != 0) {
        return 0;
    }

    oldStackSize = codethread->stack.stackSize;
    newStackSize = (stackSize == 0) ? oldStackSize << 1 : stackSize;
    if (newStackSize <= oldStackSize) {
        return 0;
    }

    // codethread stack size requires page alignment.
    newStackSizeAlign = STACK_ADDR_ALIGN_UP(newStackSize, SchedulePageSize());
    if (newStackSizeAlign > CODETHREAD_MAX_STACK_SIZE || newStackSizeAlign <= oldStackSize) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_STACK_EXPAND_FAILED,
                  "codethread stack overflow, newStackSizeAlign = %u", newStackSizeAlign);
        return -1;
    }

    return CODEThreadStackAdjust(codethread, newStackSizeAlign);
}

#ifdef CODIRA_SANITIZER_SUPPORT
void* CODEThreadGetSanitizerContext(void* codethread)
{
    CODEThread* thread = static_cast<CODEThread*>(codethread);
    return *reinterpret_cast<void**>(thread->stack.codethreadStackBaseAddr);
}

void CODEThreadSetSanitizerContext(void *codethread, void *state)
{
    CODEThread *thread = static_cast<CODEThread *>(codethread);
    *reinterpret_cast<void **>(thread->stack.codethreadStackBaseAddr) = state;
}
#endif
#ifdef __cplusplus
}
#endif
