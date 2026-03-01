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
#include <unistd.h>
#include <cstdlib>
#include "schedule_impl.h"
#include "log.h"
#include "schmon.h"
#include "securec.h"
#include "basetime.h"
#include "Base/Log.h"
#if defined(CODIRA_ASAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif
#include "StackManager.h"
#include "Common/NativeAllocator.h"
#if defined (MRT_LINUX) || defined (MRT_MACOS)
#include "schdpoll.h"
#endif
#ifdef __IOS__
#include "Mutator/MutatorManager.h"
#include "UnwindStack/PrintStackInfo.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

const int SCHEDULE_PROCESSOR_EXIT_WAIT_TIME = 10;   /* wait 10us */
/* When the non-default scheduler exits, if there are unfinished codethreads, wait10 us */
const int SCHEDULE_CODETHREAD_EXIT_WAIT_TIME = 10;
const unsigned long long NANO_TO_MILLISECONDS = 1000 * 1000; /* Used to convert nanoseconds to milliseconds */
const unsigned long long MAX_RUN_CODESINGLETHREAD_TIME = 10000000; /* Max run code single thread time is 10ms */
const size_t MAX_RUN_CODESINGLETHREAD_COUNT = 5; /* Max run code single thread count is 5 times */
const unsigned int MAX_RETRY_CODESINGLETHREAD_COUNT = 3; /* Max retry code single thread count is 3 times */

#if defined(TLS_COMMON_DYNAMIC)
GetTlsHookFunc g_getTlsFunc = nullptr;

int ScheduleGetTlsHookRegister(GetTlsHookFunc func)
{
    if (g_getTlsFunc != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_getTlsFunc = func;
    return 0;
}
#endif

struct ScheduleAttrInner g_schdAttr = {
    .thstackSize = THSTACK_SIZE_DEFAULT,
    .costackSize = COSTACK_SIZE_DEFAULT,
    .stackProtect = false,
    .stackGrow = true,
    .processorNum = PROCESSOR_NUM_DEFAULT,
};

struct ScheduleManager g_scheduleManager;

TimerControlFunc g_timerHookFunc[TIMER_HOOK_BUTT];  // timer hooks

size_t g_pageSize = 0;
bool g_tryExit;

/* Initialize the default parameters of the scheduler. */
struct ScheduleAttrInner *ScheduleAttributeGet(void)
{
    struct ScheduleAttrInner *attr;
    long res;
    attr = &g_schdAttr;
    res = GetSystemProcessorsNums();
    if (res == -1) {
        LOG_ERROR(errno, "sysconf failed");
        return nullptr;
    }
    attr->processorNum = static_cast<unsigned int>(res);
    return attr;
}

void SetSchedulerState(int state)
{
    Schedule* scheduler = ScheduleGet();
    scheduler->state = static_cast<ScheduleState>(state);
}

int ScheduleAttrInit(struct ScheduleAttr *usrAttr)
{
    int res;
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->thstackSize = THSTACK_SIZE_DEFAULT;
    attr->costackSize = COSTACK_SIZE_DEFAULT;
    attr->stackProtect = false;
    attr->stackGrow = true;
    res = static_cast<int>(GetSystemProcessorsNums());
    if (res == -1) {
        res = errno;
        LOG_ERROR(errno, "sysconf failed");
        return res;
    }
    attr->processorNum = static_cast<unsigned int>(res);
    return 0;
}

int ScheduleAttrCostackSizeSet(struct ScheduleAttr *usrAttr, unsigned int size)
{
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->costackSize = size;

    return 0;
}

int ScheduleAttrThstackSizeSet(struct ScheduleAttr *usrAttr, unsigned int size)
{
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->thstackSize = size;

    return 0;
}

int ScheduleAttrProcessorNumSet(struct ScheduleAttr *usrAttr, unsigned int num)
{
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->processorNum = num;

    return 0;
}

int ScheduleAttrStackProtectSet(struct ScheduleAttr *usrAttr, bool open)
{
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->stackProtect = open;

    return 0;
}

int ScheduleAttrStackGrowSet(struct ScheduleAttr *usrAttr, bool open)
{
    struct ScheduleAttrInner *attr = reinterpret_cast<struct ScheduleAttrInner *>(usrAttr);

    if (attr == nullptr) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    attr->stackGrow = open;

    return 0;
}

int ScheduleRecursiveLockCreate(pthread_mutex_t *mutex)
{
    int error;
    pthread_mutexattr_t mutexAttr;

    error = pthread_mutexattr_init(&mutexAttr);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    error = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    error = pthread_mutex_init(mutex, &mutexAttr);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    return 0;
}

/**
 * @ingroup schedule
 * @brief Initialize the processor control block.
 * @par Initializes the processor control block bound to the specified scheduler.
 * @attention
 * @param schedule     [IN] Scheduler of the processor to be initialized
 * @param processorNum [IN] Number of initialized processors
 * @see
 */
int ScheduleProcessorInit(struct Schedule *schedule, unsigned int processorNum)
{
    int error;
    size_t mallocSize;
    unsigned int id;
    struct ScheduleProcessor *schdProcessor;
    struct Processor *processorGroup;
    struct Thread *thread;

    if (schedule->scheduleType != SCHEDULE_DEFAULT) {
        processorNum = PROCESSOR_NUM_SINGLE_THREAD;
    }
    // Allocating processor control blocks
    schdProcessor = &(schedule->schdProcessor);
    mallocSize = processorNum * sizeof(struct Processor);
    processorGroup = static_cast<struct Processor *>(MapleRuntime::NativeAllocator::NativeAlloc(mallocSize));
    if (processorGroup == nullptr) {
        return ERRNO_SCHD_MALLOC_FAILED;
    }
    schdProcessor->processorNum = processorNum;
    schdProcessor->processorGroup = processorGroup;

    // Init processor
    for (id = 0; id < processorNum; id++) {
        error = ProcessorInit(schedule, &(processorGroup[id]), ProcessorNewId());
        if (error) {
            LOG_ERROR(error, "processor id:%d init failed", id);
            MapleRuntime::NativeAllocator::NativeFree(processorGroup, mallocSize);
            return error;
        }
    }

    // bind processor0 to thread0.
    thread = schedule->thread0;
    thread->processor = &processorGroup[0];
    processorGroup[0].thread = thread;
    processorGroup[0].state = PROCESSOR_RUNNING;
    // free num is processor_num - 1, because processor0 is running.
    schdProcessor->freeNum = processorNum - 1;

    return 0;
}

/**
 * @ingroup schedule
 * @brief Initialize the thread control block.
 * @par Initializes the processor control block bound to the specified scheduler.
 * @attention
 * @param schdThread [IN] Structure used to initialize the thread attributes of the scheduler.
 * @param attr       [IN] Structure used to initialize the attributes of the scheduler.
 * @see
 */
int ScheduleThreadInit(struct ScheduleThread *schdThread, const struct ScheduleAttrInner *attr)
{
    int error;
    schdThread->freeNum = 0;
    schdThread->threadExit = false;
    DulinkInit(&(schdThread->threadHead));
    DulinkInit(&schdThread->allThreadList);
    schdThread->stackSize = attr->thstackSize;

    // init lock
    error = ScheduleRecursiveLockCreate(&(schdThread->mutex));
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    error = ScheduleRecursiveLockCreate(&(schdThread->allthreadMutex));
    if (error) {
        pthread_mutex_destroy(&(schdThread->mutex));
        LOG_ERROR(error, "mutex init failed");
        return error;
    }

    // Add thread0 to all thread list
    ScheduleAllThreadListAdd(ThreadGet(), ScheduleGet());

    return 0;
}

int ScheduleGfreelistInit(struct ScheduleCODEThread *schdCODEThread)
{
    int error;
    struct ScheduleGfreeList *schGfreelist = &schdCODEThread->gfreelist;

    error = ScheduleRecursiveLockCreate(&schGfreelist->gfreeLock);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    DulinkInit(&schGfreelist->gfreeList);
    schGfreelist->freeCODEThreadNum = 0;

    return 0;
}

/**
 * @ingroup schedule
 * @brief Initialize the codethread control block.
 * @par Initializes the codethread control block of the scheduler.
 * @attention
 * @param schdCODEThread [IN] codethread attributes of the scheduler.
 * @param attr         [IN] attributes of the scheduler.
 */
int ScheduleCODEThreadInit(struct ScheduleCODEThread *schdCODEThread, const struct ScheduleAttrInner *attr)
{
    int error;

    schdCODEThread->codethreadNum = 0;
    schdCODEThread->stackProtect = attr->stackProtect;
    schdCODEThread->stackGrow = attr->stackGrow;
    schdCODEThread->stackSize = STACK_ADDR_ALIGN_UP(attr->costackSize, SchedulePageSize());

    error = ScheduleRecursiveLockCreate(&schdCODEThread->mutex);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }

    schdCODEThread->num = 0;
    DulinkInit(&schdCODEThread->runq);

    error = ScheduleGfreelistInit(schdCODEThread);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        pthread_mutex_destroy(&(schdCODEThread->mutex));
        return error;
    }
    return 0;
}

int ScheduleThread0Init(struct Schedule *schedule, const struct ScheduleAttrInner *schedAttr)
{
    struct Thread *thread0;
    struct CODEThread *codethread0;
    struct ArgAttr argAttr;
    struct StackAttr stackAttr;

    argAttr.argStart = nullptr;
    argAttr.argSize = 0;
    stackAttr.stackSizeAlign = STACK_ADDR_ALIGN_UP(schedAttr->costackSize, SchedulePageSize());
    stackAttr.stackGrow = false;
    // init codethread0
    codethread0 = CODEThreadAlloc(schedule, &argAttr, &stackAttr, NO_BUF);
    if (codethread0 == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "codethread0 init failed");
        return ERRNO_SCHD_INIT_FAILED;
    }
    codethread0->schedule = schedule;
    CODEThread0Make(codethread0);
    // init thread0
    schedule->thread0 = (struct Thread *)malloc(sizeof(struct Thread));
    if (schedule->thread0 == nullptr) {
        LOG_ERROR(ERRNO_SCHD_MALLOC_FAILED, "thread0 malloc failed");
        CODEThreadMemFree(codethread0);
        return ERRNO_SCHD_MALLOC_FAILED;
    }
    thread0 = schedule->thread0;
    (void)memset_s(thread0, sizeof(struct Thread), 0, sizeof(struct Thread));
    DulinkInit(&(thread0->link2schd));
    thread0->state = THREAD_INIT;
    thread0->codethread0 = codethread0;
    thread0->boundCODEThread = nullptr;
    thread0->nextProcessor = nullptr;
    int error = SemaphoreInit(&(thread0->sem), 0, 0);
    if (error) {
        LOG_ERROR(errno, "semaphore init failed");
        CODEThreadMemFree(codethread0);
        free(schedule->thread0);
        schedule->thread0 = nullptr;
        return errno;
    }
    // thread0 is the main thread to call ScheduleNew()
    thread0->osThread = pthread_self();
    thread0->tid = GetSystemThreadId();
    thread0->codethread = codethread0;
    codethread0->thread = thread0;
    // set codethread0 to tls to use CODEThreadGet() to get the codethread context
    CODEThreadSet(codethread0);
    ThreadPreemptFlagInit();

    return 0;
}

const struct ScheduleAttrInner *ScheduleAttrCheck(const struct ScheduleAttr *userAttr)
{
    const struct ScheduleAttrInner *attr = reinterpret_cast<const struct ScheduleAttrInner *>(userAttr);
    if (attr == nullptr) {
        return ScheduleAttributeGet();
    }

    if (attr->thstackSize == 0 ||
        attr->costackSize == 0 ||
        attr->costackSize > CODETHREAD_MAX_STACK_SIZE ||
        attr->processorNum == 0) {
        return nullptr;
    }
    return attr;
}

struct Schedule *ScheduleAlloc(ScheduleType scheduleType)
{
    struct Schedule *schedule;

    schedule = (struct Schedule *)MapleRuntime::NativeAllocator::NativeAlloc(sizeof(struct Schedule));
    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_MALLOC_FAILED, "schedule malloc failed");
        return nullptr;
    }
    (void)memset_s(schedule, sizeof(struct Schedule), 0, sizeof(struct Schedule));
    schedule->scheduleType = scheduleType;
    ScheduleSet(schedule);
    if (scheduleType == SCHEDULE_DEFAULT && g_scheduleManager.defaultSchedule == nullptr) {
        g_scheduleManager.defaultSchedule = schedule;
    } else if (scheduleType == SCHEDULE_DEFAULT && g_scheduleManager.defaultSchedule != nullptr) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "can't create second default schedule");
        MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
        return nullptr;
    }
    return schedule;
}

void ScheduleThread0Fini(struct Schedule *schedule)
{
    // release thread0
    if (schedule->thread0 != nullptr) {
        SemaphoreDestroy(&(schedule->thread0->sem));
        if (schedule->thread0->codethread0 != nullptr) {
            CODEThreadMemFree(static_cast<struct CODEThread*>(schedule->thread0->codethread0));
            schedule->thread0->codethread0 = nullptr;
        }
        free(schedule->thread0);
        schedule->thread0 = nullptr;
    }
    CODEThreadSet(nullptr);
}

void ScheduleCODEThreadFini(struct ScheduleCODEThread *schdCODEThread)
{
    pthread_mutex_destroy(&schdCODEThread->mutex);
    pthread_mutex_destroy(&schdCODEThread->gfreelist.gfreeLock);
}

void ScheduleThreadFini(struct ScheduleThread *schdThread)
{
    pthread_mutex_destroy(&schdThread->mutex);
    pthread_mutex_destroy(&schdThread->allthreadMutex);
}

void ScheduleFree(struct Schedule *schedule)
{
    if (schedule != nullptr) {
        if (schedule->scheduleType == SCHEDULE_DEFAULT) {
            g_scheduleManager.defaultSchedule = nullptr;
            ScheduleManagerDestroy();
        }
        MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
    }
    ScheduleSet(nullptr);
}

void ScheduleFini(struct Schedule *schedule, ScheduleFinishPhase phase)
{
    switch (phase) {
        case FINI_PROCESSOR:    // fall through
            ScheduleThreadFini(&(schedule->schdThread));
            [[fallthrough]];
        case FINI_THREAD:       // fall through
            ScheduleThread0Fini(schedule);
            [[fallthrough]];
        case FINI_THREAD0:       // fall through
            ScheduleCODEThreadFini(&(schedule->schdCODEThread));
            [[fallthrough]];
        default:
            ScheduleFree(schedule);
    }
}

int ScheduleManagerInit(void)
{
    int error;

    error = ScheduleRecursiveLockCreate(&g_scheduleManager.allScheduleListLock);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    error = ScheduleRecursiveLockCreate(&g_scheduleManager.allCODEThreadListLock);
    if (error) {
        pthread_mutex_destroy(&g_scheduleManager.allScheduleListLock);
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    // Init scheduler management list
    DulinkInit(&g_scheduleManager.allScheduleList);
    // Init global codethread management list for default mode
    DulinkInit(&g_scheduleManager.allCODEThreadList);
    // Init global codethread management list for UI mode
    DulinkInit(&g_scheduleManager.codeSingleModeThreadList);
    g_scheduleManager.codeSingleModeThreadRetryTime.store(0);
    g_scheduleManager.codethreadIdGen = 1;
    g_scheduleManager.processorIdGen = 1;
    g_scheduleManager.initFlag = true;
    memset_s(&g_scheduleManager.trace, sizeof(struct Trace), 0, sizeof(struct Trace));
    // Init SchdfdManager
    struct SchdfdManager *schdfdManager = SchdfdManagerInit();
    if (schdfdManager == nullptr) {
        LOG_ERROR(-1, "schdfdManager init failed");
        return -1;
    }
    g_scheduleManager.schdfdManager = schdfdManager;
    return 0;
}

void ScheduleManagerDestroy(void)
{
    if (g_scheduleManager.initFlag) {
        pthread_mutex_destroy(&g_scheduleManager.allCODEThreadListLock);
        pthread_mutex_destroy(&g_scheduleManager.allScheduleListLock);
        FreeSchdfdManager(g_scheduleManager.schdfdManager);
        if (g_scheduleManager.trace.mutexInitFlag) {
            pthread_mutex_destroy(&g_scheduleManager.trace.lock);
            pthread_mutex_destroy(&g_scheduleManager.trace.bufLock);
        }
        (void)memset_s(&g_scheduleManager, sizeof(struct ScheduleManager), 0, sizeof(struct ScheduleManager));
    }
}

ScheduleHandle ScheduleNew(ScheduleType scheduleType, const struct ScheduleAttr *userAttr)
{
    int error;
    struct Schedule *schedule;
    const struct ScheduleAttrInner *schedAttr;

    // When the scheduler is initialized, the created thread is bound to thread0. It is
    // convenient to use CODEThreadGet() to obtain the codethread. Therefore, only one scheduler
    // can be created for a thread.
    if (scheduleType != SCHEDULE_FOREIGN_THREAD && CODEThreadGet() != nullptr) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "schedule has been inited");
        return nullptr;
    }

    schedAttr = ScheduleAttrCheck(userAttr);
    if (schedAttr == nullptr) {
        LOG_ERROR(ERRNO_SCHD_ATTR_INVALID, "schedule attr invalid");
        return nullptr;
    }

    // Init schedule structure
    schedule = ScheduleAlloc(scheduleType);
    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "schedule malloc failed");
        return nullptr;
    }

    if (scheduleType == SCHEDULE_DEFAULT && !g_scheduleManager.initFlag) {
        error = ScheduleManagerInit();
        if (error) {
            MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
            return nullptr;
        }
    } else if (scheduleType != SCHEDULE_DEFAULT && !g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "default schedule hasn't been inited");
        MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
        return nullptr;
    } else if (scheduleType == SCHEDULE_DEFAULT && g_scheduleManager.initFlag) {
        // Before the default scheduler is created, g_scheduleManager should not be
        // initialized and should not go to this branch. Therefore, g_scheduleManager cannot
        // be created.
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "g_shceduleManager shouldn't have been initialized");
        MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
        return nullptr;
    }

    // Init codethread structure
    error = ScheduleCODEThreadInit(&(schedule->schdCODEThread), schedAttr);
    if (error) {
        LOG_ERROR(error, "schedule codethread control block init failed");
        ScheduleFini(schedule, FINI_CODETHREAD);
        return nullptr;
    }
    
    // Init thread0 and codethread0
    error = ScheduleThread0Init(schedule, schedAttr);
    if (error) {
        LOG_ERROR(error, "thread0 init failed");
        ScheduleFini(schedule, FINI_THREAD0);
        return nullptr;
    }

    // Init thread structure
    error = ScheduleThreadInit(&(schedule->schdThread), schedAttr);
    if (error) {
        LOG_ERROR(error, "schedule thread control block init failed");
        ScheduleFini(schedule, FINI_THREAD);
        return nullptr;
    }

    // Init processor group
    error = ScheduleProcessorInit(schedule, schedAttr->processorNum);
    if (error) {
        LOG_ERROR(error, "schedule processor control block init failed");
        ScheduleFini(schedule, FINI_PROCESSOR);
        return nullptr;
    }
    ScheduleListAdd(schedule);
    RandSeedInit();
    return (ScheduleHandle)schedule;
}

void RegisterEventHandlerCallbacks(PostTaskFunc pFunc, HasHigherPriorityTaskFunc hFunc)
{
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "schedule manager is not init");
        return;
    }
    g_scheduleManager.postTaskFunc = pFunc;
    g_scheduleManager.hasHigherPriorityTaskFunc = hFunc;
}

void CODERegisterStackInfoCallbacks(UpdateStackInfoFunc uFunc)
{
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "schedule manager is not init");
        return;
    }
    g_scheduleManager.updateStackInfoFunc = uFunc;
}
 
void CODERegisterArkVMInRuntime(unsigned long long vm)
{
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_INIT_FAILED, "schedule manager is not init");
        return;
    }
    g_scheduleManager.arkVM = vm;
}

int ScheduleNetpollInit(void)
{
    struct Schedule *schedule = ScheduleGet();
    int error;
    // Reentry lock. The lock may reentrant in rpc.
    error = ScheduleRecursiveLockCreate(&schedule->netpoll.pollMutex);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }
    error = PthreadSpinInit(&schedule->netpoll.closingLock);
    if (error) {
        pthread_mutex_destroy(&schedule->netpoll.pollMutex);
        LOG_ERROR(error, "pthread spin init failed");
        return error;
    }
    return 0;
}

void ScheduleNetpollDestroy(void)
{
    struct Schedule *schedule = ScheduleGet();
    pthread_mutex_destroy(&schedule->netpoll.pollMutex);
    PthreadSpinDestroy(&schedule->netpoll.closingLock);
}

#ifdef __OHOS__
void StoreNativeSPForUIThread(void* sp)
{
    g_scheduleManager.nativeSPForUIThread = sp;
}

void UpdateArkVMStackInfo(unsigned long long arkvm)
{
    // update stack info for arkts, thus use __OHOS__ macro.
    UpdateStackInfoFunc UpdateStackInfo = g_scheduleManager.updateStackInfoFunc;
    if (UpdateStackInfo == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_UITHREAD_ERROR, "UpdateStackInfoFunc is not registered");
        return;
    }
    CODEThread* codethread = CODEThreadGet();
    if (codethread == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_UITHREAD_ERROR, "codethread is nullptr when UpdateArkVMStackInfo");
        return;
    }
    if (g_scheduleManager.arkVM == 0) {
        g_scheduleManager.arkVM = arkvm;
    }
    UpdateStackInfo(g_scheduleManager.arkVM, &(codethread->stackInfo), SWITCH_TO_SUB_STACKINFO);
}

void* GetNativeSPForUIThread()
{
    return g_scheduleManager.nativeSPForUIThread;
}

bool IsForeignThread()
{
    struct CODEThread *codethread = CODEThreadGet();
    ScheduleType type = codethread->schedule->scheduleType;
    switch (type) {
        case SCHEDULE_DEFAULT:
            HILOG_FATAL(ERRNO_SCHD_WRONG_TYPE, "Forbidden to use JSRuntime() in a spawn!");
        case SCHEDULE_UI_THREAD:
            return false;
        case SCHEDULE_FOREIGN_THREAD:
            return true;
    }
}
#endif

/* Scheduler startup process
 * 1. Call ScheduleNew() to create a scheduler
 * 2. Call CODEThreadNew() to create main
 * 3. Call ScheduleStart() to execute main
 **/
int ScheduleStart(void)
{
    struct Schedule *schedule;
    int ret;

    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }
    if (schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_IS_RUNNING;
    }
    ScheduleNetpollInit();
    if (schedule->scheduleType == SCHEDULE_DEFAULT) {
        ret = SchmonStart(schedule);
        if (ret != 0) {
            ScheduleNetpollDestroy();
            return ret;
        }
    }

    schedule->state = SCHEDULE_RUNNING;

    g_tryExit = false;
    CODEThreadContextGet(&ThreadGet()->context);
    // The judgment is used when the schedule_try_exit interface is invoked.
    if (g_tryExit || (schedule->scheduleType != SCHEDULE_DEFAULT &&
                      (schedule->state == SCHEDULE_EXITING || schedule->state == SCHEDULE_EXITED))) {
        return 0;
    }
    // consider delete it because trace start is later than schedule start.
    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_PROC_WAKE)) {
        ScheduleTraceEventOrigin(TRACE_EV_PROC_WAKE, -1, nullptr, 1,
                                 static_cast<unsigned long long>(ThreadGet()->tid));
    }
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEnterCODEThread(ThreadGet());
#endif
    ProcessorSchedule();

    return 0;
}

bool ScheduleExistTask(void)
{
    ProcessorCheckReadyFunc checkFunc;
    struct Schedule *schedule = ScheduleGet();
    if (ScheduleAnyCODEThread(schedule)) {
        return true;
    }
    checkFunc = g_scheduleManager.checkReady;
    if (checkFunc != nullptr) {
        return checkFunc(ProcessorGet());
    }

    return false;
}

int ScheduleStartNoWait(unsigned long long timeout)
{
    struct Schedule *schedule;

    schedule = ScheduleGet();
    if (schedule == nullptr || schedule->scheduleType == SCHEDULE_DEFAULT || schedule->state != SCHEDULE_INIT) {
        return ERRNO_SCHD_ATTR_INVALID;
    }
    if (!schedule->noWaitAttr.netpollInit) {
        ScheduleNetpollInit();
        schedule->noWaitAttr.netpollInit = true;
    }
    if (!ScheduleExistTask()) {
        return 0;
    }
    schedule->state = SCHEDULE_RUNNING;

    CODEThreadContextGet(&ThreadGet()->context);
    if (schedule->noWaitAttr.nowait) {
        schedule->state = SCHEDULE_INIT;
        schedule->noWaitAttr.nowait = false;
        return 0;
    }
    schedule->noWaitAttr.nowait = true;
    schedule->noWaitAttr.timeout = timeout * NANO_TO_MILLISECONDS;
    schedule->noWaitAttr.startTime = CurrentNanotimeGet();
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEnterCODEThread(ThreadGet());
#endif
    ProcessorSchedule();

    return 0;
}

/* Non-default scheduler thread exits */
void ScheduleNonDefaultThreadExit(struct Schedule *schedule, bool wait)
{
    while (atomic_load(&schedule->thread0->state) == THREAD_PRE_SLEEP) {}
    if (atomic_load(&schedule->thread0->state) == THREAD_SLEEP) {
        int error = SemaphorePost(&(schedule->thread0->sem));
        if (error != 0) {
            LOG_ERROR(error, "sem post failed");
            return;
        }
    }
    if (wait) {
        pthread_join(schedule->thread0->osThread, nullptr);
    }
}

/* Releases thread resources in the thread pool. Note: This interface needs to be invoked
 * only in schedule_try_exit, not in schedule_stop.
 */
void ScheduleThreadsFree(struct Schedule *schedule)
{
    struct ScheduleThread *schdThread;
    struct Thread *thread;

    schdThread = &schedule->schdThread;
    while (!DulinkIsEmpty(&schdThread->threadHead)) {
        thread = DULINK_ENTRY(schdThread->threadHead.next, struct Thread, link2schd);
        DulinkRemove(&thread->link2schd);
        // Wakes up the thread. The thread checks the exit flag and exits.
        SemaphorePost(&thread->sem);
        pthread_join(thread->osThread, nullptr);
        CODEThreadMemFree(static_cast<struct CODEThread*>(thread->codethread0));
        free(thread);
    }
}

void ScheduleProcessorFree(struct Schedule *schedule)
{
    struct Processor *processor;
    struct CODEThread *codethread;
    ProcessorExitFunc func;
    unsigned int i;
    unsigned int j;

    for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
        processor = &schedule->schdProcessor.processorGroup[i];
        while (1) {
            codethread = ProcessorFreelistGet(processor);
            if (codethread == nullptr) {
                break;
            }
            CODEThreadFree(codethread, false);
        }
        free(processor->runq.buf);

        // Release the timer of processors.
        for (j = 0; j < PROCESSOR_PARRAY_NUM; ++j) {
            func = g_scheduleManager.exit[j];
            if (func != nullptr) {
                func(processor);
            }
        }
    }

    // Release processor.
    MapleRuntime::NativeAllocator::NativeFree(schedule->schdProcessor.processorGroup,
        schedule->schdProcessor.processorNum * sizeof(struct Processor));
}

void ScheduleSchmonExit(void)
{
    if (g_scheduleManager.schmon.schmonId != 0) {
        pthread_join(g_scheduleManager.schmon.schmonId, nullptr);
        g_scheduleManager.schmon.schmonId = 0;
    }
}

/* Releasing global and local codethreads */
void ScheduleCODEThreadFree(struct Schedule *schedule)
{
    struct CODEThread *codethread;
    struct Processor *processor;
    unsigned int i;

    // Release codethreads in the global queue
    while (1) {
        codethread = ProcessorGlobalRead(schedule, false);
        if (codethread == nullptr) {
            break;
        }
        CODEThreadFree(codethread, false);
    }

    // Release codethreads in the local queues
    for (i = 0; i < schedule->schdProcessor.processorNum; i++) {
        processor = &schedule->schdProcessor.processorGroup[i];

        codethread = ProcessorCODEhreadNextRead(processor);
        if (codethread != nullptr) {
            CODEThreadFree(codethread, false);
        }

        while (1) {
            codethread = ProcessorLocalRead(processor);
            if (codethread == nullptr) {
                break;
            }
            CODEThreadFree(codethread, false);
        }
    }
}

void ScheduleNetpollExit(struct Schedule *schedule)
{
    if (schedule->netpoll.npfd != nullptr) {
        NetpollExit(schedule->netpoll.npfd);
    }
#if defined (MRT_LINUX) || defined (MRT_MACOS)
    SchdpollFreePd();
#endif
}

/* Checks for unfinished codethread tasks on a non-default scheduler */
bool ScheduleAnyCODEThreadRunning(struct Schedule *schedule)
{
    struct ScheduleCODEThread *schdCODEThread;
    struct CODEThread *codethread;
    struct Processor *processor;
    ProcessorCheckExistenceFunc func;
    struct ScheduleGfreeList *gfreelist;

    schdCODEThread = &schedule->schdCODEThread;
    gfreelist = &schedule->schdCODEThread.gfreelist;
    while (1) {
        codethread = ScheduleGfreelistGet(gfreelist);
        if (codethread == nullptr) {
            break;
        }
        CODEThreadFree(codethread, false);
    }
    processor = &schedule->schdProcessor.processorGroup[0];
    while (1) {
        codethread = ProcessorFreelistGet(processor);
        if (codethread == nullptr) {
            break;
        }
        CODEThreadFree(codethread, false);
    }
    if (atomic_load(&schdCODEThread->codethreadNum) != 0) {
        return true;
    }
    // Check whether a timer exists.
    func = g_scheduleManager.checkExistence;
    if (func != nullptr) {
        return func(processor);
    }
    return false;
}

void ScheduleNonDefaultFree(struct Schedule *schedule)
{
    ScheduleListRemove(schedule);
    // The network module exits after the processor stops. Otherwise, the processor is still
    // accessing the network, and problems occur.
    ScheduleNetpollExit(schedule);
    // Release global and local codethreads.
    ScheduleCODEThreadFree(schedule);
    // Release processor resources and auxiliary resources, including timers.
    ScheduleProcessorFree(schedule);

    CODEThreadMemFree(static_cast<struct CODEThread*>(schedule->thread0->codethread0));
    free(schedule->thread0);
    schedule->thread0 = nullptr;
    MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
}

/* Check whether the current processor is in the FFI state. The processor is in ScheduleStop
 * and will not be scheduled again.
 */
bool ScheduleProcessorSkipFFI(struct Processor *processor)
{
    SchdMutatorStatusHookFunc hookFunc;
    struct Thread *thread;
    struct CODEThread *codethread;

    thread = processor->thread;

    if (thread == nullptr) {
        return false;
    }
    codethread = static_cast<struct CODEThread*>(thread->codethread);

    if (codethread == nullptr) {
        return false;
    }

    hookFunc = g_scheduleManager.mutatorStatusFunc;
    if (hookFunc == nullptr || codethread->mutator == nullptr) {
        return false;
    }
    return hookFunc(codethread->mutator);
}

void ScheduleAllNonDefaultExit(void)
{
    struct Schedule *schedule;
    struct Schedule *curschdeule = ScheduleGet();
    struct Dulink *scheduleNode;

    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    // Set the exit status bit for all non-default schedulers without confirming the end of
    // thread execution.
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        if (schedule == curschdeule) {
            continue;
        }
        atomic_store(&schedule->state, SCHEDULE_EXITING);
        ScheduleNonDefaultThreadExit(schedule, false);
        // Preemption is set only for threads of the non-default scheduler. Do not change the
        // processor state in the syscall state to exiting. Otherwise, the thread cannot exit
        // after executing ThreadStop.
        SchmonPreemptRunning(&(schedule->schdProcessor.processorGroup[0]));
    }
    // Confirm that all default scheduler threads are executed.
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        if (schedule == curschdeule) {
            continue;
        }
        if (pthread_self() != schedule->thread0->osThread &&
            !ScheduleProcessorSkipFFI(&(schedule->schdProcessor.processorGroup[0]))) {
            pthread_join(schedule->thread0->osThread, nullptr);
            atomic_store(&schedule->schdProcessor.processorGroup[0].state, PROCESSOR_EXITING);
        }
        atomic_store(&schedule->state, SCHEDULE_EXITED);
        // Because the current node is released when resources are released, the next element
        // traversed after the current node is released should be the next node of the
        // previous node of the current node.
        scheduleNode = scheduleNode->prev;
        // Releasing Non-Default Scheduler Resources
        ScheduleNonDefaultFree(schedule);
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
}

/* Preempt and stop all worker threads */
void ScheduleProcessorExit(struct Schedule *schedule)
{
    unsigned int i;
    struct Processor *processor;
    struct Processor *curProcessor;
    const int waitTime = SCHEDULE_PROCESSOR_EXIT_WAIT_TIME; // wait 10us
    ProcessorState pstate = PROCESSOR_IDLE;

    // If processor is NULL, the call is made outside the scheduling framework.
    if (CODEThreadGet() == nullptr) {
        curProcessor = nullptr;
    } else {
        curProcessor = ProcessorGet();
    }
    for (i = 0; i < schedule->schdProcessor.processorNum; i++) {
        processor = &schedule->schdProcessor.processorGroup[i];
        if (processor == curProcessor) {
            continue;
        }
        // Set the idle state to the exit state, and notify the preemption of the running state.
        // Change the status of the processor in the syscall state to exit. When the thread
        // finishes executing from the syscall, it searches for an idle processor. If it cannot
        // find an idle processor, it sleeps itself.
        pstate = processor->state;
        if (pstate == PROCESSOR_IDLE || pstate == PROCESSOR_SYSCALL) {
            atomic_compare_exchange_strong(&processor->state, &pstate, PROCESSOR_EXITING);
        } else if (pstate == PROCESSOR_RUNNING) {
            SchmonPreemptRunning(processor);
        }
    }
    // Traversal check. The setting and check are divided into two cycles because preemption
    // usually takes a period of time after the setting. If the setting is combined into one
    // cycle, the preemption needs to be repeated for n times.
    for (i = 0; i < schedule->schdProcessor.processorNum; i++) {
        processor = &schedule->schdProcessor.processorGroup[i];
        if (processor == curProcessor) {
            continue;
        }

        while (atomic_load(&processor->state) != PROCESSOR_EXITING || processor->thread != nullptr) {
            pstate = processor->state;
            if (pstate == PROCESSOR_IDLE || pstate == PROCESSOR_SYSCALL) {
                atomic_compare_exchange_strong(&processor->state, &pstate, PROCESSOR_EXITING);
            } else if (pstate == PROCESSOR_RUNNING) {
                SchmonPreemptRunning(processor);
            }

            // Detects that the current processor is running FFI and exits directly. If the
            // FFI ends and is ready to leave the security zone, the stw mechanism is triggered
            // and the thread is blocked.
            if (ScheduleProcessorSkipFFI(processor)) {
                break;
            }
            usleep(waitTime);
        }
    }
    ScheduleAllNonDefaultExit();

    return;
}

void ScheduleExitMode(struct Schedule *schedule, bool threadExit)
{
    SchdCODEThreadHookFunc hookFunc;

    // Set state to exit state
    schedule->schdThread.threadExit = threadExit;

    // Ensure that all processors are stopped. When the system exits, the processor is not
    // woken up. The non-default scheduler has only one processor and does not sleep.
    if (schedule->scheduleType == SCHEDULE_DEFAULT) {
        schedule->state = SCHEDULE_EXITING;
        // Ensure that the monitoring thread is stopped.
        ScheduleSchmonExit();
        hookFunc = g_scheduleManager.schdCODEThreadHook[SCHD_STOP];
        if (hookFunc != nullptr) {
            hookFunc();
        }
        ScheduleProcessorExit(schedule);
        // All processors enter the PROCESSOR_EXITING state in ScheduleProcessorExit. The
        // processors in ffi are blocked in the stub of the warehouse program. At this time,
        // the scheduling framework does not run the codethread and can exit safely.
        schedule->state = SCHEDULE_EXITED;
    } else {
        // The non-default scheduling framework exits actively. The framework has gone through
        // the Schedule_WAITING phase. At this time, no codethread is running and no codethread
        // can be created. You can switch the Schedule_EXITED state. You can safely exit.
        schedule->state = SCHEDULE_EXITED;
        ScheduleNonDefaultThreadExit(schedule, true);
    }

    ScheduleListRemove(schedule);

    // The network module exits after the processor stops. Otherwise, the processor is still
    // accessing the network, and problems occur.
    ScheduleNetpollExit(schedule);

    // Release global and local codethreads
    ScheduleCODEThreadFree(schedule);

    /*  */
    if (threadExit) {
        ScheduleThreadsFree(schedule);
    }

    // No other worker thread is running except the current thread. If thread_exit == false,
    // the function will not be executed. Releases processor resources and auxiliary resources,\
    // including timers.
    ScheduleProcessorFree(schedule);
}

/* Stops the current scheduling framework. Note: This interface suspends the thread without
 * exiting, so that the thread exits with the process. This interface cannot be used outside
 * the scheduling framework. */
void ScheduleStop(ScheduleHandle scheduleHandle)
{
    struct CODEThread *codethread = CODEThreadGet();
    struct Schedule *schedule = (struct Schedule *)scheduleHandle;
    // The default scheduler exits in the codethread context, not the default scheduler exits
    // specific context requirements, but cannot exit in its own codethread task.
    if (schedule == nullptr || (schedule->scheduleType == SCHEDULE_DEFAULT && codethread == nullptr)) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "codethread or schedule not exist!\n");
        return;
    }

    // The non-default scheduler needs to wait for all codethreads run to finish when shutting down.
    if (schedule->scheduleType != SCHEDULE_DEFAULT) {
        schedule->state = SCHEDULE_WAITING;
        while (ScheduleAnyCODEThreadRunning(schedule)) {
            usleep(SCHEDULE_CODETHREAD_EXIT_WAIT_TIME);
        }
    }
    ScheduleExitMode(schedule, false);
    if (schedule->scheduleType == SCHEDULE_DEFAULT) {
        ScheduleAllCODEThreadListRemove(codethread);
        g_scheduleManager.defaultSchedule = nullptr;
        ScheduleManagerDestroy();
    } else {
        CODEThreadMemFree(static_cast<struct CODEThread*>(schedule->thread0->codethread0));
        free(schedule->thread0);
        MapleRuntime::NativeAllocator::NativeFree(schedule, sizeof(struct Schedule));
    }
}

/* Stops the scheduling framework. Note: This interface suspends the thread without exiting,
 * so that the thread exits with the process. This interface can be used outside the scheduling framework. */
int ScheduleStopOutside(ScheduleHandle scheduleHandle)
{
    struct Schedule *oldSchedule;
    struct Schedule *schedule = reinterpret_cast<Schedule *>(scheduleHandle);

    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "no schedule");
        return ERRNO_SCHD_INVALID;
    }

    ScheduleType scheduleType = schedule->scheduleType;
    oldSchedule = ScheduleGet();
    ScheduleSet(schedule);
    ScheduleExitMode(schedule, false);
    // This parameter is added to solve the memory leakage problem when the dlclose exits in
    // the macro expansion scenario.
    if (scheduleType == SCHEDULE_DEFAULT && g_scheduleManager.initFlag) {
        FreeSchdfdManager(g_scheduleManager.schdfdManager);
    }
    ScheduleSet(oldSchedule);
    return 0;
}

/* It is invoked in a codethread to exit the scheduling framework and each worker thread. */
int ScheduleTryExit(void)
{
    struct Schedule *schedule;
    struct ScheduleCODEThread *schdCODEThread;
    struct CODEThread *codethread;
    unsigned long long codethreadNum = 0;
    struct Dulink *scheduleCODEThreadNode = nullptr;

    schedule = ScheduleGet();
    if (schedule == nullptr) {
        return ERRNO_SCHD_UNINITED;
    }

    schdCODEThread = &schedule->schdCODEThread;
    if (ThreadGet() != schedule->thread0) {
        return ERRNO_SCHD_EXIT_FAILED;
    }

    // Need to ensure that only the current codethread is running and there are no other codethreads left.
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
        codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
        if (codethread->state != CODETHREAD_IDLE) {
            ++codethreadNum;
            if (codethreadNum > 1) {
                pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);
                LOG_INFO(ERRNO_SCHD_EXIT_FAILED, "now codethreadNum is %lld", atomic_load(&schdCODEThread->codethreadNum));
                return ERRNO_SCHD_EXIT_FAILED;
            }
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);

    ScheduleExitMode(schedule, true);

    // Switch back to the context in which the main thread invokes schedule start.
    g_tryExit = true;
    auto *thread = ThreadGet();
#ifdef CODIRA_ASAN_SUPPORT
    // return to ThreadEntry, switch to original thread
    MapleRuntime::Sanitizer::AsanExitCODEThread(thread);
#endif
    CODEThreadContextSet(&thread->context);

    return 0;
}

/* After the schedule_try_exit exits, the last codethread exists and thread0 is not cleared.
 * This interface is invoked to clear the codethread. */
void ScheduleClean(void)
{
    struct CODEThread *codethread;
    struct Schedule *schedule;
    struct Thread *thread;

    codethread = CODEThreadGet();
    if (codethread != nullptr) {
        thread = codethread->thread;
        CODEThreadFree(codethread, false);
        CODEThreadSet(nullptr);
        if (thread != nullptr) {
            CODEThreadMemFree(static_cast<struct CODEThread*>(thread->codethread0));
            free(thread);
        }
    }

    schedule = ScheduleGet();
    ScheduleFree(schedule);
}

int ScheduleGlobalWrite(struct CODEThread *codethreadList[], unsigned int num)
{
    struct Dulink *globalQueue;
    struct Schedule *schedule;
    unsigned long i;

    if (num == 0) {
        return 0;
    }

    schedule = codethreadList[0]->schedule;
    globalQueue = &schedule->schdCODEThread.runq;

    pthread_mutex_lock(&schedule->schdCODEThread.mutex);
    for (i = 0; i < num; i++) {
        DulinkPushtail(globalQueue, codethreadList[i]);
    }
    schedule->schdCODEThread.num += num;
    pthread_mutex_unlock(&schedule->schdCODEThread.mutex);

    return 0;
}

/* Check whether there are any codethreads waiting to run in the schedule. */
bool ScheduleAnyCODEThread(ScheduleHandle scheduleHandle)
{
    unsigned int i;
    struct Schedule *schedule = (struct Schedule *)scheduleHandle;
    struct Processor *processor;

    for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
        processor = &schedule->schdProcessor.processorGroup[i];
        if (QueueLength(&processor->runq) != 0) {
            return true;
        }
    }

    if (atomic_load(&schedule->lastCODEThread) != static_cast<struct CODEThread *>(nullptr)) {
        return true;
    }
    return schedule->schdCODEThread.num != 0;
}

unsigned long long ScheduleCODEThreadCount(void)
{
    struct CODEThread *codethread = nullptr;
    unsigned long long codethreadNum = 0;
    struct Dulink *scheduleCODEThreadNode = nullptr;
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "allCODEThreadList haven't init");
        return static_cast<unsigned long long>(-1);
    }
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
        codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
        if (codethread->state != CODETHREAD_IDLE) {
            ++codethreadNum;
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);
    TRACE_COUNT("CODERT_codethreadNum", codethreadNum);
    return codethreadNum;
}

int SchdProcessorHookRegister(ProcessorCheckFunc func, unsigned int key)
{
    if (g_scheduleManager.check[key] != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_scheduleManager.check[key] = func;
    return 0;
}

int SchdSchmonHookRegister(SchmonCheckFunc func, unsigned int key)
{
    if (g_scheduleManager.schmon.checkFunc[key] != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_scheduleManager.schmon.checkFunc[key] = func;
    return 0;
}

int SchdExitHookRegister(ProcessorExitFunc func, unsigned int key)
{
    if (g_scheduleManager.exit[key] != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_scheduleManager.exit[key] = func;
    return 0;
}

int SchdCheckExistenceHookRegister(ProcessorCheckExistenceFunc func)
{
    if (g_scheduleManager.checkExistence != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_scheduleManager.checkExistence = func;
    return 0;
}

int SchdCheckReadyHookRegister(ProcessorCheckReadyFunc func)
{
    if (g_scheduleManager.checkReady != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_scheduleManager.checkReady = func;
    return 0;
}

int ScheduleTimerHookRegister(TimerControlFunc func, unsigned int key)
{
    if (g_timerHookFunc[key] != nullptr) {
        return ERRNO_SCHD_HOOK_REGISTED;
    }
    g_timerHookFunc[key] = func;
    return 0;
}

void ScheduleGfreelistPush(struct ScheduleGfreeList *gfreelist, struct CODEThread *codethread)
{
    DulinkAdd(&codethread->schdDulink, &(gfreelist->gfreeList));
    gfreelist->freeCODEThreadNum++;
}

struct CODEThread *ScheduleGfreelistPop(struct ScheduleGfreeList *gfreelist)
{
    struct CODEThread *codethread;
    if (gfreelist->freeCODEThreadNum != 0) {
        codethread = DULINK_ENTRY(gfreelist->gfreeList.next,
                                 struct CODEThread, schdDulink);
        DulinkRemove(&(codethread->schdDulink));
        gfreelist->freeCODEThreadNum--;

        return codethread;
    }
    return nullptr;
}

struct CODEThread *ScheduleGfreelistGet(struct ScheduleGfreeList *gfreelist)
{
    struct CODEThread *codethread;
    pthread_mutex_lock(&gfreelist->gfreeLock);
    codethread = ScheduleGfreelistPop(gfreelist);
    pthread_mutex_unlock(&gfreelist->gfreeLock);
    return codethread;
}

void ScheduleListAdd(struct Schedule *schedule)
{
    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DulinkAdd(&(schedule->allScheduleDulink), &(g_scheduleManager.allScheduleList));
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
}

void ScheduleListRemove(struct Schedule *schedule)
{
    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DulinkRemove(&(schedule->allScheduleDulink));
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
}

void ScheduleAllThreadListAdd(struct Thread *thread, struct Schedule *schedule)
{
    pthread_mutex_lock(&(schedule->schdThread.allthreadMutex));
    DulinkAdd(&(thread->allThreadDulink), &(schedule->schdThread.allThreadList));
    pthread_mutex_unlock(&(schedule->schdThread.allthreadMutex));
}

int ScheduleAllCODEThreadListAdd(struct CODEThread *codethread)
{
    ScheduleState scheduleState = codethread->schedule->state.load();
    if (scheduleState != SCHEDULE_RUNNING &&
        scheduleState != SCHEDULE_INIT &&
        scheduleState != SCHEDULE_EXITING) {
        HILOG_ERROR(ERRNO_SCHD_INVALID,
                    "can't add codethread to the target scheduler, schedule type %d, schedule state %d",
                    codethread->schedule->scheduleType, scheduleState);
        return -1;
    }
    codethread->mutator = MapleRuntime::Mutator::NewMutator();
    codethread->mutator->MapleRuntime::Mutator::SetCodethreadPtr(static_cast<void*>(codethread));
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DulinkAdd(&(codethread->allCODEThreadDulink), &(g_scheduleManager.allCODEThreadList));
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);

    return 0;
}

int AddToCODESingleModeThreadList(struct CODEThread *codethread)
{
    if (codethread == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_CODETHREAD_NULL, "codethread is nullptr when add to codeSingleModeThreadList.");
        return -1;
    }
    PostTaskFunc PostTask = g_scheduleManager.postTaskFunc;
    if (PostTask == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_EVENT_HANDLER_FUNC_NULL,
                    "The event handler function is nullptr when add to codeSingleModeThreadList.");
        return -1;
    }

    pthread_mutex_lock(&g_scheduleManager.codeSingleModeThreadListLock);
    int isEmpty = DulinkIsEmpty(&(g_scheduleManager.codeSingleModeThreadList));
    DulinkAdd(&(codethread->codeSingleModeThreadDulink), &(g_scheduleManager.codeSingleModeThreadList));
    pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
    if (isEmpty) {
        while (!PostTask(reinterpret_cast<void*>(TryRunCODESingleModeThread))) {}
    }
    return 0;
}

void RunCODESingleModeThread()
{
    pthread_mutex_lock(&g_scheduleManager.codeSingleModeThreadListLock);
    struct Dulink* scheduleCODEUIThreadNode = (&(g_scheduleManager.codeSingleModeThreadList))->prev;
    struct CODEThread* codethread = DULINK_ENTRY(scheduleCODEUIThreadNode, struct CODEThread, codeSingleModeThreadDulink);
    if (codethread == nullptr) {
        pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
        HILOG_ERROR(ERRNO_SCHD_UITHREAD_ERROR, "code single mode thread is nullptr");
        return;
    }
    DulinkRemove(&(codethread->codeSingleModeThreadDulink));
    pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);

    Schedule* schedule = codethread->schedule;
    CODEThreadSet(reinterpret_cast<CODEThread*>(schedule->thread0->codethread0));
    ScheduleSet(schedule);
    ScheduleGlobalWrite(&codethread, 1);

#ifdef __OHOS__
    // update stack info for arkts, thus use __OHOS__ macro.
    UpdateStackInfoFunc UpdateStackInfo = g_scheduleManager.updateStackInfoFunc;
    if (UpdateStackInfo != nullptr) {
        UpdateStackInfo(g_scheduleManager.arkVM, &(codethread->stackInfo), SWITCH_TO_SUB_STACKINFO);
    }
#endif

    ProcessorWake(schedule, nullptr);
    ScheduleStartNoWait(0);

#ifdef __OHOS__
    // update stack info for arkts, thus use __OHOS__ macro.
    if (UpdateStackInfo != nullptr) {
        UpdateStackInfo(g_scheduleManager.arkVM, &(codethread->stackInfo), SWITCH_TO_MAIN_STACKINFO);
    }
#endif
    return;
}

extern "C" void CODE_MRT_RolveCycleRef();

void RunResolveCycle(void* funcPtr)
{
    PostTaskFunc PostTask = g_scheduleManager.postTaskFunc;
    if (PostTask == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_UITHREAD_ERROR,
                    "The event handler function is nullptr when try run codeSingleModeThread.");
        return;
    }
    while (!PostTask(funcPtr)) {}
}

void TryRunCODESingleModeThread()
{
    PostTaskFunc PostTask = g_scheduleManager.postTaskFunc;
    HasHigherPriorityTaskFunc HasHigherPriorityTask = g_scheduleManager.hasHigherPriorityTaskFunc;
    if (PostTask == nullptr) {
        HILOG_ERROR(ERRNO_SCHD_UITHREAD_ERROR,
                    "The event handler function is nullptr when try run codeSingleModeThread.");
        return;
    }
    unsigned long long startTime = CurrentNanotimeGet();
    size_t runCount = 0;
    pthread_mutex_lock(&g_scheduleManager.codeSingleModeThreadListLock);
    if (DulinkIsEmpty(&(g_scheduleManager.codeSingleModeThreadList))) {
        pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
        return;
    }
    pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
    while (true) {
        // if have reruned 3 times, just run it directly.
        // if TryRunCODESingleModeThread run for 10ms or 5 times or there are more higher priority tasks,
        // then stop it and trigger event handler func.
        if (g_scheduleManager.codeSingleModeThreadRetryTime.load() < MAX_RETRY_CODESINGLETHREAD_COUNT &&
            (CurrentNanotimeGet() - startTime > MAX_RUN_CODESINGLETHREAD_TIME ||
            runCount >= MAX_RUN_CODESINGLETHREAD_COUNT ||
            HasHigherPriorityTask == nullptr ? false : HasHigherPriorityTask())) {
            atomic_fetch_add(&g_scheduleManager.codeSingleModeThreadRetryTime, 1U);
            while (!PostTask(reinterpret_cast<void*>(TryRunCODESingleModeThread))) {}
            break;
        }
        
        g_scheduleManager.codeSingleModeThreadRetryTime.store(0);
        RunCODESingleModeThread();
        runCount++;
        pthread_mutex_lock(&g_scheduleManager.codeSingleModeThreadListLock);
        if (DulinkIsEmpty(&(g_scheduleManager.codeSingleModeThreadList))) {
            pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
            break;
        }
        pthread_mutex_unlock(&g_scheduleManager.codeSingleModeThreadListLock);
    }
}

void ScheduleAllCODEThreadListRemove(struct CODEThread *codethread)
{
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DulinkRemove(&(codethread->allCODEThreadDulink));
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);

    // Cooperate with Codira GC to release the mutator.
    SchdDestructorHookFunc hook_func = g_scheduleManager.destructorFunc;
    if (hook_func != nullptr && codethread->mutator) {
        hook_func(codethread->mutator);
    }
}

/* When type is set to 1, the visited object is arg_start. When type is set to 0, the visited
 * object is mutator.
 */
void ScheduleAllCODEThreadVisitImpl(AllCODEThreadListProcFunc visitor, void *handle, int type)
{
    struct CODEThread *codethread;
    struct Dulink *scheduleCODEThreadNode = nullptr;
    // This function is not invoked when the scheduler exits. Do not need to add lock.
    if (visitor == nullptr) {
        return;
    }
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    if (type == 1) {
        DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
            codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
            void *arg = codethread->argStart;
            if (arg != nullptr) {
                visitor(arg, handle);
            }
        }
    } else {
        DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
            codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
            void *mutator = codethread->mutator;
            if (codethread->state != CODETHREAD_IDLE && mutator != nullptr) {
                visitor(mutator, handle);
            }
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);
}

unsigned long long ScheduleCODEThreadCountPublic(CODEthreadStatePublic state)
{
    struct CODEThread *codethread;
    struct Dulink *scheduleCODEThreadNode = nullptr;
    unsigned long long codethreadNum = 0;
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "allCODEThreadList haven't init");
        return static_cast<unsigned long long>(-1);
    }

    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
        codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
        if (state == CODETHREAD_PSTATE_ALL && codethread->state != CODETHREAD_IDLE) {
            codethreadNum++;
        } else if (state == CODETHREAD_PSTATE_RUNNING &&
                   (codethread->state == CODETHREAD_READY ||
                    codethread->state == CODETHREAD_RUNNING || codethread->state == CODETHREAD_SYSCALL)) {
            codethreadNum++;
        } else if (state == CODETHREAD_PSTATE_BLOCKING && codethread->state == CODETHREAD_PENDING) {
            codethreadNum++;
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);

    return codethreadNum;
}

/* Collects statistics on the number of running processors. */
unsigned int ScheduleRunningOSThreadCount(void)
{
    struct Schedule *schedule = ScheduleGet();
    struct Dulink *scheduleNode;
    unsigned int processorNum = 0;
    struct Processor *processor;
    unsigned int i;

    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "schedule is null");
        return -1;
    }

    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
            processor = &schedule->schdProcessor.processorGroup[i];
            processorNum = (processor->state == PROCESSOR_RUNNING) ? processorNum + 1 : processorNum;
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
    return processorNum;
}

void ScheduleAllCODEThreadVisit(AllCODEThreadListProcFunc visitor, void *handle)
{
    ScheduleAllCODEThreadVisitImpl(visitor, handle, 1);
}

void ScheduleAllCODEThreadVisitMutator(AllCODEThreadListProcFunc visitor, void *handle)
{
    ScheduleAllCODEThreadVisitImpl(visitor, handle, 0);
}

bool ScheduleAnyCODEThreadOrTimer(void)
{
    struct Schedule *schedule;
    if (g_scheduleManager.defaultSchedule == nullptr) {
        return false;
    }
    struct CODEThread *codethread;
    struct Processor *processor;
    struct ScheduleGfreeList *gfreelist;
    bool flag = false;
    unsigned int i;
    struct Dulink *scheduleNode = nullptr;

    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        gfreelist = &schedule->schdCODEThread.gfreelist;
        // Clear the global resource pool and local resource pool. This operation is performed
        // only when the scheduler is in Schedule_SUSPEND state.
        if (schedule->state != SCHEDULE_SUSPENDING) {
            continue;
        }
        while (1) {
            codethread = ScheduleGfreelistGet(gfreelist);
            if (codethread == nullptr) {
                break;
            }
            CODEThreadFree(codethread, false);
        }
        for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
            processor = &schedule->schdProcessor.processorGroup[i];
            codethread = ProcessorFreelistGet(processor);
            while (codethread != nullptr) {
                CODEThreadFree(codethread, false);
                codethread = ProcessorFreelistGet(processor);
            }
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
    // After the local and global resource pools are cleared, codethread_free will not be
    // reclaimed to the resource pool in the suspended state. If codethread_num is 0, the
    // resource pool and existing codethreads have ended and cannot be added.
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    if (!DulinkIsEmpty(&g_scheduleManager.allCODEThreadList)) {
        flag = true;
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);
    if (g_timerHookFunc[ANY_TIMER_HOOK]() != 0) {
        flag = true;
    }
    return flag;
}

void ScheduleLockAll(void)
{
    struct Schedule *schedule;
    struct Processor *processor;
    unsigned int i;
    struct Dulink *scheduleNode;

    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    pthread_mutex_lock(&g_scheduleManager.allCODEThreadListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
            processor = &schedule->schdProcessor.processorGroup[i];
            PthreadSpinLock(&processor->lock);
        }
        pthread_mutex_lock(&schedule->schdCODEThread.gfreelist.gfreeLock);
        pthread_mutex_lock(&schedule->netpoll.pollMutex);
    }
}

void ScheduleUnlockAll(void)
{
    struct Schedule *schedule;
    struct Processor *processor;
    unsigned int i;
    struct Dulink *scheduleNode;

    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        pthread_mutex_unlock(&schedule->schdCODEThread.gfreelist.gfreeLock);
        for (i = 0; i < schedule->schdProcessor.processorNum; ++i) {
            processor = &schedule->schdProcessor.processorGroup[i];
            PthreadSpinUnlock(&processor->lock);
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allCODEThreadListLock);
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
}

int ScheduleSuspend(void)
{
    struct Schedule *schedule;
    if (g_scheduleManager.defaultSchedule == nullptr) {
        LOG_ERROR(ERRNO_SCHED_SUSPEND_WHEN_NOT_RUNNING, "schedule is null");
        return -1;
    }
    ScheduleLockAll();

    ScheduleState value = SCHEDULE_RUNNING;
    struct Dulink *scheduleNode;

    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        /* After the scheduler status is marked as SCHEDULE_SUSPENDING, codethreads will no
         * longer be reclaimed and will be released uniformly, and codethreads cannot be added.
         * At this point, the concurrent codethread_alloc can obtain free codethreads, but after
         * clearing, it can no longer be obtained. The local free list has temporarily added
         * muetx to avoid concurrency with any_codethread recycling, which needs to be optimized. */
        if (!atomic_compare_exchange_strong(&schedule->state, &value, SCHEDULE_SUSPENDING)) {
            value = SCHEDULE_RUNNING;
            scheduleNode = schedule->allScheduleDulink.prev;
            while (scheduleNode != &g_scheduleManager.allScheduleList) {
                schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
                atomic_store(&schedule->state, value);
                scheduleNode = schedule->allScheduleDulink.prev;
            }
            ScheduleUnlockAll();
            LOG_ERROR(ERRNO_SCHED_SUSPEND_WHEN_NOT_RUNNING, "suspend when schedule is not running");
            return -1;
        }
    }
    ScheduleUnlockAll();

    // clear codethread free list.
    (void)ScheduleAnyCODEThreadOrTimer();
    return 0;
}

int ScheduleResume(void)
{
    struct Schedule *schedule;
    if (g_scheduleManager.defaultSchedule == nullptr) {
        LOG_ERROR(ERRNO_SCHED_RESUME_WHEN_NOT_SUSPENDING, "schedule is null");
        return -1;
    }
    ScheduleLockAll();
    ScheduleState value = SCHEDULE_SUSPENDING;
    struct Dulink *scheduleNode;

    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        if (!atomic_compare_exchange_strong(&schedule->state, &value, SCHEDULE_RUNNING)) {
            value = SCHEDULE_SUSPENDING;
            scheduleNode = schedule->allScheduleDulink.prev;
            while (scheduleNode != &g_scheduleManager.allScheduleList) {
                schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
                atomic_store(&schedule->state, value);
                scheduleNode = schedule->allScheduleDulink.prev;
            }
            ScheduleUnlockAll();
            LOG_ERROR(ERRNO_SCHED_RESUME_WHEN_NOT_SUSPENDING, "resume when schedule is not suspending");
            return -1;
        }
    }
    ScheduleUnlockAll();

    return 0;
}

void SchedulePreemptCheck(void)
{
    struct CODEThread *curCODEThread;
    uintptr_t spAddress = -1;
    uintptr_t preemptFlag;

    curCODEThread = CODEThreadGet();
    if (curCODEThread == nullptr) {
        return;
    }
#ifdef MRT_HARDWARE_PLATFORM

#if (MRT_HARDWARE_PLATFORM == MRT_ARM) && (VOS_WORDSIZE == 64)
    asm volatile (
    "mov %0, sp \n"
    :"=r"(spAddress)
    );
#endif

#if (MRT_HARDWARE_PLATFORM == MRT_ARM) && (VOS_WORDSIZE == 32)
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

#endif

    preemptFlag = PreemptFlagGet();
    if (spAddress <= preemptFlag) {
        CODEThreadPreemptResched();
    }
}

bool ScheduleIsRunning(ScheduleHandle scheduleHandle)
{
    struct Schedule *schedule = (struct Schedule *)scheduleHandle;
    if (schedule == nullptr) {
        return false;
    }
    if (schedule->state == SCHEDULE_RUNNING) {
        return true;
    }

    return false;
}

void ScheduleSetToCurrentThread(ScheduleHandle schedule)
{
    ScheduleSet((struct Schedule *)schedule);
}

unsigned int ScheduleGetProcessorNum(void)
{
    unsigned int processorNum = 0;
    struct Schedule *schedule = g_scheduleManager.defaultSchedule;
    struct Dulink *scheduleNode;
    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "default schedule is null");
        return 0;
    }

    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        processorNum += schedule->schdProcessor.processorNum;
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
    return processorNum;
}

/* codethread trace is open for linux or win. */
#if defined (MRT_LINUX) || defined (MRT_WINDOWS)
int ScheduleTraceDlclose(DlHandle dlHandle)
{
#ifdef MRT_WINDOWS
    return FreeLibrary(dlHandle);
#elif defined (MRT_LINUX)
    return dlclose(dlHandle);
#endif
}

#if defined (MRT_WINDOWS)
FARPROC ScheduleLoadTraceForWin(char* dlPath, DlHandle *dlHandlePtr)
{
    FARPROC traceRegister;
    *dlHandlePtr = LoadLibrary(dlPath);
    if (*dlHandlePtr == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "trace dlopen failed");
        return nullptr;
    }
    traceRegister= GetProcAddress(*dlHandlePtr, "CODE_TraceRegister");
    if (traceRegister == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "trace dlopen failed");
        ScheduleTraceDlclose(*dlHandlePtr);
        return nullptr;
    }
    return traceRegister;
}
#endif

#if defined (MRT_LINUX)
TraceRegisterFunc ScheduleLoadTrace(char* dlPath, DlHandle *dlHandlePtr)
{
    TraceRegisterFunc traceRegister;
    *dlHandlePtr = dlopen(dlPath, RTLD_LAZY);
    if (*dlHandlePtr == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, dlerror());
        return nullptr;
    }
    MapleRuntime::InitAddressScopeForCODEthreadTrace();
    traceRegister = reinterpret_cast<TraceRegisterFunc>(dlsym(*dlHandlePtr, "CODE_TraceRegister"));
    if (traceRegister == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, dlerror());
        ScheduleTraceDlclose(*dlHandlePtr);
        return nullptr;
    }
    return traceRegister;
}
#endif

bool ScheduleStartTrace(unsigned short traceType)
{
    DlHandle dlHandle = nullptr;
    char dlPath[TRACE_PATH_LENGTH];
#ifdef MRT_WINDOWS
    char tracePathFormat[] = "%s\\runtime\\lib\\%s_codenative\\libcangjie-trace.dll";
#elif defined (MRT_LINUX)
    char tracePathFormat[] = "%s/runtime/lib/%s_codenative/libcangjie-trace.so";
#endif
    if (g_scheduleManager.trace.openType) {
        LOG_ERROR(ERRNO_SCHD_TRACE_ALREADY_START, "trace is already start");
        return false;
    }
    // Static memory is used and does not need to be manually released
    char *envCodiraHome = std::getenv("CODIRA_HOME");
    if (envCodiraHome == nullptr) {
        LOG_ERROR(ERRNO_SCHD_GET_ENV_FAILED, "CODIRA_HOME is not set");
        return false;
    }
    int ret = sprintf_s(dlPath, TRACE_PATH_LENGTH, tracePathFormat, envCodiraHome, TRACE_FOLDER_PREFIX);
    if (ret == -1) {
        LOG_ERROR(errno, "sprintf_s failed");
        return false;
    }
#ifdef MRT_WINDOWS
    FARPROC traceRegister = ScheduleLoadTraceForWin(dlPath, &dlHandle);
#elif defined (MRT_LINUX)
    TraceRegisterFunc traceRegister = ScheduleLoadTrace(dlPath, &dlHandle);
#endif
    if (traceRegister == nullptr) {
        return false;
    }
    traceRegister();
    if (g_scheduleManager.trace.hooks.traceStart == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "trace traceRegister failed");
        ScheduleTraceDlclose(dlHandle);
        return false;
    }
    bool result = g_scheduleManager.trace.hooks.traceStart(traceType);
    if (!result) {
        g_scheduleManager.trace.hooks.traceDeregister();
        ScheduleTraceDlclose(dlHandle);
        return false;
    }
    g_scheduleManager.trace.dlHandle = dlHandle;
    return true;
}

bool ScheduleStopTrace()
{
    bool result;
    if (g_scheduleManager.trace.hooks.traceStop == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "func not register");
        return false;
    }
    result = g_scheduleManager.trace.hooks.traceStop();
    if (!result) {
        return false;
    }
    ScheduleTraceDlclose(g_scheduleManager.trace.dlHandle);
    g_scheduleManager.trace.dlHandle = nullptr;
    return result;
}

unsigned char *ScheduleDumpTrace(int *len)
{
    if (g_scheduleManager.trace.hooks.traceDump == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "func not register");
        return nullptr;
    }
    return g_scheduleManager.trace.hooks.traceDump(len);
}

#else
bool ScheduleStartTrace(unsigned short traceType)
{
    (void)traceType;
    return false;
}

bool ScheduleStopTrace()
{
    return false;
}

unsigned char *ScheduleDumpTrace(int *len)
{
    (void)len;
    return nullptr;
}
#endif

struct CODEThread *ScheduleGetTraceReader()
{
    if (g_scheduleManager.trace.hooks.traceReaderGet == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "func not register");
        return nullptr;
    }
    return g_scheduleManager.trace.hooks.traceReaderGet();
}

/* Entry function for recording trace events in schedule logic. */
void ScheduleTraceEventOrigin(TraceEvent event, int skip, struct CODEThread *codethread, int argNum, ...)
{
    va_list args;
    if (g_scheduleManager.trace.hooks.traceRecordEvent == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "func not register");
        return;
    }
    auto mutator = (codethread == nullptr) ? nullptr : static_cast<void *>(codethread->mutator);
    va_start(args, argNum);
    g_scheduleManager.trace.hooks.traceRecordEvent(event, skip, mutator, argNum, args);
    va_end(args);
}

void ScheduleTraceEvent(TraceEvent event, int skip, struct CODEThread *codethread, int argNum, ...)
{
    va_list args;
    if (!g_scheduleManager.trace.openType || !(g_scheduleManager.trace.openType & event)) {
        return;
    }
    if (g_scheduleManager.trace.hooks.traceRecordEvent == nullptr) {
        LOG_ERROR(ERRNO_SCHD_TRACE_DL_FAILED, "func not register");
        return;
    }
    auto mutator = (codethread == nullptr) ? nullptr : static_cast<void *>(codethread->mutator);
    va_start(args, argNum);
    g_scheduleManager.trace.hooks.traceRecordEvent(event, skip, mutator, argNum, args);
    va_end(args);
}

#ifdef __cplusplus
}
#endif
// The method is presented in c++ to implement codethead functions of codedb in debug mode.
unsigned long long CODE_MRT_GetCODEThreadNumberUnsafe(void)
{
    unsigned long long codethreadNum = 0;
    struct CODEThread *codethread;
    struct Dulink *scheduleCODEThreadNode = nullptr;
    if (!g_scheduleManager.initFlag) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "allCODEThreadList haven't init");
        return static_cast<unsigned long long>(-1);
    }

    DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
        codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
        if (codethread->state != CODETHREAD_IDLE) {
            ++codethreadNum;
        }
    }

    return codethreadNum;
}

// The method is presented in c++ to implement codethead functions of codedb in debug mode.
// codethreadBufPtr must be void*, cannot be uintptr or CODEThreadInfo*.
int CODE_MRT_GetAllCODEThreadInfo(void *codethreadBufPtr, unsigned int num)
{
    unsigned int index = 0;
    struct Schedule *schedule;
    struct Dulink *scheduleCODEThreadNode = nullptr;
    struct CODEThread *codethread;
    struct CODEThreadInfo *codethreadBuf = reinterpret_cast<struct CODEThreadInfo *>(codethreadBufPtr);

    schedule = ScheduleGet();
    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_UNINITED, "schedule is null");
        return 0;
    }

    if (CODEThreadGet() == nullptr || codethreadBuf == nullptr) {
        return 0;
    }
    DULINK_FOR_EACH_ITEM(scheduleCODEThreadNode, &g_scheduleManager.allCODEThreadList) {
        if (index == num) {
            break;
        }
        codethread = DULINK_ENTRY(scheduleCODEThreadNode, struct CODEThread, allCODEThreadDulink);
        if (codethread->state == CODETHREAD_IDLE) {
            continue;
        }
        CODEThreadGetInfo(codethread, &(codethreadBuf[index]));
        index++;
    }
    return index;
}

void CODEForeignThreadExit(CODEThreadHandle foreignThread)
{
    auto* foreignCODEThread = reinterpret_cast<CODEThread*>(foreignThread);
    Schedule* schedule = foreignCODEThread->schedule;
    if (schedule->scheduleType != SCHEDULE_FOREIGN_THREAD) {
        LOG_FATAL(ERRNO_SCHD_WRONG_TYPE, "foreign code thread has wrong scheduler");
    }
    MapleRuntime::Mutator* mutator = foreignCODEThread->mutator;
    if (mutator != nullptr && mutator->IsForeignThread()) {
        mutator->SetForeignCODEThreadExit();
    }
}

#ifdef __IOS__
MapleRuntime::CString GetThreadStateString(void *codethreadPtr)
{
    if (codethreadPtr == nullptr) {
        return MapleRuntime::CString();
    }
    struct CODEThread *codethread = static_cast<struct CODEThread*>(codethreadPtr);
    if (codethread->state == CODETHREAD_IDLE) {
        return MapleRuntime::CString("idle");
    } else if (codethread->state == CODETHREAD_READY) {
        return MapleRuntime::CString("ready");
    } else if (codethread->state == CODETHREAD_RUNNING) {
        return MapleRuntime::CString("running");
    } else if (codethread->state == CODETHREAD_PENDING) {
        return MapleRuntime::CString("pending");
    } else if (codethread->state == CODETHREAD_SYSCALL) {
        return MapleRuntime::CString("syscall");
    } else {
        LOG_FATAL(ERRNO_SCHD_CODETHREAD_STATE_INVALID, "codethread has wrong state");
    }
}

bool IsPendingThread(void *codethreadPtr)
{
    if (codethreadPtr == nullptr) {
        return false;
    }
    struct CODEThread *codethread = static_cast<struct CODEThread*>(codethreadPtr);
    return (codethread->state == CODETHREAD_READY || codethread->state == CODETHREAD_PENDING);
}

int CODE_MRT_GetAllCODEThreadStackTrace(void *codeStackTraceBufPtr, unsigned int num)
{
    unsigned int recordCnt = 0;
    char *codeStackTraceBuffer = reinterpret_cast<char*>(codeStackTraceBufPtr);
    MapleRuntime::MutatorManager::Instance().VisitAllMutators(
        [&recordCnt, num, codeStackTraceBuffer](MapleRuntime::Mutator &mutator) {
            // skip finalizer
            if (!mutator.IsVaildCODEThread() || recordCnt == num ||
                !IsPendingThread(mutator.GetCodethreadPtr())) {
                return;
            }
            uint32_t threadId = static_cast<uint32_t>(mutator.GetCODEThreadId());
            MapleRuntime::CString threadState = GetThreadStateString(mutator.GetCodethreadPtr());
            MapleRuntime::CString threadName;
            if (mutator.GetCODEThreadName() != nullptr) {
                threadName = MapleRuntime::CString(mutator.GetCODEThreadName());
            }
            MapleRuntime::CString get;
            get.Append(MapleRuntime::CString::FormatString("codethread #%d state: %s name: %s\n",
                                                           threadId, threadState.Str(), threadName.Str()));
            MapleRuntime::PrintStackInfo printStackInfo(&(mutator.GetUnwindContext()));
            get.Append(printStackInfo.GetStackTraceString());
            snprintf_s(codeStackTraceBuffer + recordCnt * CODETHREAD_STACK_STRING_SIZE,
                       CODETHREAD_STACK_STRING_SIZE, CODETHREAD_STACK_STRING_SIZE - 1, "%s", get.Str());
            recordCnt++;
        }
    );
    return recordCnt;
}
#endif
