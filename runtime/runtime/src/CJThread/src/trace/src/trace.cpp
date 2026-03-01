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


#include "basetime.h"
#include "securec.h"
#include "log.h"
#include "schedule_impl.h"
#include "UnwindStack/GcStackInfo.h"
#include "trace.h"

#ifdef  __cplusplus
extern "C" {
#endif

bool TraceStart(unsigned short traceType)
{
    int error;
    if (g_scheduleManager.trace.openType || g_scheduleManager.trace.shutdown) {
        LOG_ERROR(ERRNO_TRACE_ALREADY_START, "trace is already start");
        return false;
    }
    DulinkInit(&g_scheduleManager.trace.fullBufHead);
    DulinkInit(&g_scheduleManager.trace.freeBufHead);
    if (!g_scheduleManager.trace.mutexInitFlag) {
        error = ScheduleRecursiveLockCreate(&g_scheduleManager.trace.lock);
        if (error) {
            LOG_ERROR(error, "lock init failed");
            return false;
        }
        error = ScheduleRecursiveLockCreate(&g_scheduleManager.trace.bufLock);
        if (error) {
            pthread_mutex_destroy(&g_scheduleManager.trace.lock);
            LOG_ERROR(error, "bufLock init failed");
            return false;
        }
        g_scheduleManager.trace.mutexInitFlag = true;
    }
    /* The system time is recorded only when the trace function is enabled and disabled.
     * The CPU clock cycle is used during the trace process.
     */
    g_scheduleManager.trace.ticksStart = CurrentCPUTicks();
    g_scheduleManager.trace.timeStart = CurrentNanotimeGet();
    g_scheduleManager.trace.openType = traceType;
    g_scheduleManager.trace.headerWritten = false;
    g_scheduleManager.trace.footerWritten = false;
    g_scheduleManager.trace.stringId.store(1);
    g_scheduleManager.trace.stackId.store(SpecialStackId::OTHERS);

    TraceRecordStartEvent();
    return true;
}

/* Put the traceBuf into the fullBufHead queue and wait for the traceReader to write the file. */
void TraceFullQueue(struct TraceBuf **buf)
{
    DulinkPushtail(&g_scheduleManager.trace.fullBufHead, *buf);
    *buf = nullptr;
}

int TraceParkUnlock(void *arg, CODEThreadHandle handle)
{
    (void)arg;
    (void)handle;
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    return 0;
}

bool TraceDeregisterBeforeStop()
{
    struct Dulink *traceBufNode;
    pthread_mutex_lock(&g_scheduleManager.trace.lock);
    pthread_mutex_lock(&g_scheduleManager.trace.bufLock);
    if (g_scheduleManager.trace.buf != nullptr) {
        pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        LOG_ERROR(ERRNO_TRACE_SHUTDOWN_WRONG, "trace->buf not nullptr");
        return false;
    }
    if (!DulinkIsEmpty(&g_scheduleManager.trace.fullBufHead)) {
        pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        LOG_ERROR(ERRNO_TRACE_SHUTDOWN_WRONG, "trace->fullBufHead not empty");
        return false;
    }
    g_scheduleManager.trace.ticksEnd = CurrentCPUTicks();
    g_scheduleManager.trace.timeEnd = CurrentNanotimeGet();
    pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    while (!DulinkIsEmpty(&g_scheduleManager.trace.freeBufHead)) {
        traceBufNode = g_scheduleManager.trace.freeBufHead.next;
        DulinkPopHead(&g_scheduleManager.trace.freeBufHead);
        free(traceBufNode);
    }
    g_scheduleManager.trace.shutdown = false;
    TraceDeregister();
    return true;
}

bool TraceBufExistBeforeStop(struct Dulink *scheduleNode, struct Schedule *schedule,
                             struct ScheduleProcessor *scheduleProcessor)
{
    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        scheduleProcessor = &schedule->schdProcessor;
        for (unsigned int i = 0; i < scheduleProcessor->processorNum; ++i) {
            if (scheduleProcessor->processorGroup[i].traceBuf != nullptr) {
                LOG_ERROR(ERRNO_TRACE_SHUTDOWN_WRONG, "processor-%u traceBuf not nullptr",
                          scheduleProcessor->processorGroup[i].processorId);
                pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
                return true;
            }
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
    return false;
}

bool TraceStop(void)
{
    struct Dulink *scheduleNode = nullptr;
    struct Schedule *schedule = nullptr;
    struct ScheduleProcessor *scheduleProcessor = nullptr;
    struct CODEThread *reader = nullptr;
    if (!g_scheduleManager.trace.openType) {
        LOG_ERROR(ERRNO_TRACE_ALREADY_STOP, "trace is already stop");
        return false;
    }
    /* Record the codethread events of the currently stopped trace. */
    if (CODEThreadGet() != nullptr && CODEThreadGet() != ThreadGet()->codethread0) {
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_RESCHED, TRACE_STACK_1, nullptr, 0);
        ScheduleTraceEvent(TRACE_EV_PROC_STOP, -1, nullptr, 0);
    }
    g_scheduleManager.trace.openType = 0;
    g_scheduleManager.trace.shutdown = true;
    while (atomic_load(&g_scheduleManager.trace.eventCount) != 0) {}
    /* Put the data in traceBuf of all processors into the global traceBufList. */
    pthread_mutex_lock(&g_scheduleManager.trace.lock);
    pthread_mutex_lock(&g_scheduleManager.allScheduleListLock);
    DULINK_FOR_EACH_ITEM(scheduleNode, &g_scheduleManager.allScheduleList) {
        schedule = DULINK_ENTRY(scheduleNode, struct Schedule, allScheduleDulink);
        scheduleProcessor = &schedule->schdProcessor;
        for (unsigned int i = 0; i < scheduleProcessor->processorNum; ++i) {
            if (scheduleProcessor->processorGroup[i].traceBuf != nullptr) {
                TraceFullQueue(&scheduleProcessor->processorGroup[i].traceBuf);
            }
        }
    }
    pthread_mutex_unlock(&g_scheduleManager.allScheduleListLock);
    /* Put the data in global traceBuf into the global traceBufList. */
    pthread_mutex_lock(&g_scheduleManager.trace.bufLock);
    if (g_scheduleManager.trace.buf != nullptr) {
        TraceFullQueue(&g_scheduleManager.trace.buf);
    }
    pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);

    /* Start readerCODETthread and park stopCODEthread until all trace data is written to the file. */
    g_scheduleManager.trace.stopCODEThread = CODEThreadGet();
    if (g_scheduleManager.trace.reader != nullptr) {
        reader = g_scheduleManager.trace.reader;
        g_scheduleManager.trace.reader = nullptr;
        CODEThreadReady(reader);
    }
    CODEThreadPark(TraceParkUnlock, TRACE_EV_CODETHREAD_BLOCK, nullptr);

    /* Check whether traceBuf that is not nullptr exists. */
    if (TraceBufExistBeforeStop(scheduleNode, schedule, scheduleProcessor)) {
        return false;
    }
    /* To prevent the lock in traceDump from not being released. */
    return TraceDeregisterBeforeStop();
}

/* Writes byte data to traceBuf. */
void TraceByte(struct TraceBuf *traceBuf, unsigned char value)
{
    traceBuf->arr[traceBuf->header.pos] = value;
    traceBuf->header.pos++;
}

/* Compresses unsigned long long data and writes it to traceBuf. */
void TraceUint64(struct TraceBuf *traceBuf, unsigned long long value)
{
    unsigned long long tempValue = value;
    for (; tempValue >= TRACE_UINT64_SHIFT_THRESHOLD; tempValue >>= TRACE_UINT64_SHIFTS) {
        traceBuf->arr[traceBuf->header.pos] = static_cast<unsigned char>(TRACE_UINT64_SHIFT_THRESHOLD | tempValue);
        traceBuf->header.pos++;
    }
    traceBuf->arr[traceBuf->header.pos] = static_cast<unsigned char>(tempValue);
    traceBuf->header.pos++;
}

/* Allocate a new memory to traceBuf. If the original address is not nullptr, store it in fullBufHead to be written. */
void TraceFlush(struct TraceBuf **traceBuf, unsigned int processorId, bool batchFlag)
{
    pthread_mutex_lock(&g_scheduleManager.trace.lock);
    if (*traceBuf != nullptr) {
        TraceFullQueue(traceBuf);
    }
    if (!DulinkIsEmpty(&g_scheduleManager.trace.freeBufHead)) {
        *traceBuf = reinterpret_cast<struct TraceBuf *>(g_scheduleManager.trace.freeBufHead.next);
        DulinkPopHead(&g_scheduleManager.trace.freeBufHead);
    } else {
        *traceBuf = (struct TraceBuf *)malloc(sizeof(struct TraceBuf));
        if (*traceBuf == nullptr) {
            LOG_ERROR(ERRNO_TRACE_MALLOC_FAILED, "malloc failed");
            pthread_mutex_unlock(&g_scheduleManager.trace.lock);
            return;
        }
    }
    (*traceBuf)->header.pos = 0;
    (*traceBuf)->header.lastTicks = CurrentCPUTicks();
    /* A batch event needs to be recorded when a traceBuf is created except the header and footer.
     * In this way, the same information does not need to be recorded repeatedly in subsequent events.
     */
    if (batchFlag) {
        TraceByte(*traceBuf, static_cast<unsigned char>(TRACE_EV_BATCH & TRACE_EFFECTIVE_EVENT));
        TraceByte(*traceBuf, static_cast<unsigned char>(1));
        TraceUint64(*traceBuf, static_cast<unsigned long long>(processorId));
        TraceUint64(*traceBuf, (*traceBuf)->header.lastTicks);
    }
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
}

bool TraceCheckEvSizeAndFlush(struct TraceBuf **traceBuf, unsigned int processorId,
                              unsigned int eventMaxSize, bool batchFlag)
{
    if (*traceBuf == nullptr || sizeof((*traceBuf)->arr) - ((*traceBuf)->header.pos) < eventMaxSize) {
        TraceFlush(traceBuf, processorId, batchFlag);
        if (*traceBuf == nullptr && processorId == 0) {
            /* get buflock in TraceBufAquire */
            pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);
            LOG_ERROR(ERRNO_TRACE_BUF_FLUSH_WRONG, "TraceFlush Failed");
            return false;
        } else if (*traceBuf == nullptr && processorId != 0) {
            LOG_ERROR(ERRNO_TRACE_BUF_FLUSH_WRONG, "TraceFlush Failed");
            return false;
        }
    }
    return true;
}

unsigned long long TraceGetTimeDiff(struct TraceBuf **traceBuf)
{
    unsigned long long timeStamp = CurrentCPUTicks();
    unsigned long long timeDiff = timeStamp - (*traceBuf)->header.lastTicks;
    if ((*traceBuf)->header.lastTicks >= timeStamp) {
        timeStamp = (*traceBuf)->header.lastTicks + 1;
        timeDiff = 1;
    }
    (*traceBuf)->header.lastTicks = timeStamp;
    return timeDiff;
}

/* Obtain the current Processor or the global tarceBuf pointer, depending on whether the codethread context. */
struct TraceBuf **TraceBufAquire(unsigned int *processorId)
{
    struct Processor *processor = ProcessorGetWithCheck();
    if (processor != nullptr) {
        *processorId = processor->processorId;
        return &processor->traceBuf;
    }
    pthread_mutex_lock(&g_scheduleManager.trace.bufLock);
    // If processorId is 0, use global traceBuf
    *processorId = 0;
    return &g_scheduleManager.trace.buf;
}

/* Release resources held by traceBuf. */
void TraceBufRelease(void)
{
    if (ProcessorGetWithCheck() == nullptr) {
        pthread_mutex_unlock(&g_scheduleManager.trace.bufLock);
    }
}

bool TraceDoubleCheckShutdown()
{
    if (g_scheduleManager.trace.shutdown == true) {
        LOG_INFO(ERRNO_TRACE_EVENT_WHEN_STOP, "The trace is being closed.");
        return true;
    }
    atomic_fetch_add(&g_scheduleManager.trace.eventCount, 1);
    if (g_scheduleManager.trace.shutdown == true) {
        LOG_INFO(ERRNO_TRACE_EVENT_WHEN_STOP, "The trace is being closed.");
        atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
        return true;
    }
    return false;
}

void TraceRecordEvent(TraceEvent event, int skip, void *mutator, int argNum, va_list args)
{
    unsigned int processorId;
    unsigned long long timeDiff;
    unsigned long long arg;
    unsigned char evArgNum = static_cast<unsigned char>(argNum & TRACE_EFFECTIVE_ARG_NUM);
    struct TraceBuf **traceBuf;
    /* add eventCount here */
    if (TraceDoubleCheckShutdown()) {
        return;
    }
    /* get bufLock here */
    traceBuf = TraceBufAquire(&processorId);
    if (!TraceCheckEvSizeAndFlush(traceBuf, processorId, TRACE_EVENT_MAXSIZE, true)) {
        atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
        return;
    }
    TraceByte(*traceBuf, static_cast<unsigned char>(event & TRACE_EFFECTIVE_EVENT));
    timeDiff = TraceGetTimeDiff(traceBuf);
    if (skip != -1) {
        evArgNum++;
    }
    TraceByte(*traceBuf, evArgNum);
    TraceUint64(*traceBuf, timeDiff);

    for (int i = 0; i < argNum; i++) {
        /* Get an unsigned long parameter from args. */
        arg = va_arg(args, unsigned long long);
        TraceUint64(*traceBuf, arg);
    }
    unsigned long long stackId = 0;
    if (skip > 0 && skip <= TRACE_STACK_MAX_DEPTH) {
        stackId = TraceStackId();
        TraceUint64(*traceBuf, stackId);
    }
    
    TraceBufRelease();
    atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
    if (skip <= 0) {
        return;
    }

    bool success = TraceRecordStackEvent(stackId, skip, mutator);
    if (!success) {
        LOG_ERROR(ERRNO_TRACE_STACK_EVENT, "get CODEThread StackTrace id failed");
    }
}

unsigned char *TraceDumpHeader(int *len)
{
    g_scheduleManager.trace.headerWritten = true;
    TraceFlush(&g_scheduleManager.trace.reading, 0, false);
    if (g_scheduleManager.trace.reading == nullptr) {
        LOG_ERROR(ERRNO_TRACE_BUF_FLUSH_WRONG, "TraceFlush Failed");
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        return nullptr;
    }
    memset_s(g_scheduleManager.trace.reading->arr, TRACE_BUF_LENGTH, 0, TRACE_HEADER_LENGTH);
    memcpy_s(g_scheduleManager.trace.reading->arr, TRACE_BUF_LENGTH, TRACE_HEADER, strlen(TRACE_HEADER));
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    *len = TRACE_HEADER_LENGTH;
    return g_scheduleManager.trace.reading->arr;
}

unsigned char *TraceDumpFooter(int *len)
{
    g_scheduleManager.trace.footerWritten = true;
    double freq = static_cast<double>(g_scheduleManager.trace.ticksEnd - g_scheduleManager.trace.ticksStart) * 1e9 /
            static_cast<double>(g_scheduleManager.trace.timeEnd - g_scheduleManager.trace.timeStart);
    TraceFlush(&g_scheduleManager.trace.reading, 0, false);
    if (g_scheduleManager.trace.reading == nullptr) {
        LOG_ERROR(ERRNO_TRACE_BUF_FLUSH_WRONG, "TraceFlush Failed");
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        return nullptr;
    }
    TraceByte(g_scheduleManager.trace.reading,
              static_cast<unsigned char>(TRACE_EV_FREQUENCY & TRACE_EFFECTIVE_EVENT));
    TraceByte(g_scheduleManager.trace.reading, static_cast<unsigned char>(0));
    TraceUint64(g_scheduleManager.trace.reading, (unsigned long long)freq);
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    *len = g_scheduleManager.trace.reading->header.pos;
    return g_scheduleManager.trace.reading->arr;
}

unsigned char *TraceDump(int *len)
{
    pthread_mutex_lock(&g_scheduleManager.trace.lock);

    if (g_scheduleManager.trace.reader != nullptr) {
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        LOG_ERROR(ERRNO_TRACE_MULTIPLE_READER, "ReadTrace called from multiple goroutines simultaneously");
        return nullptr;
    }
    if (g_scheduleManager.trace.reading != nullptr) {
        DulinkPushtail(&g_scheduleManager.trace.freeBufHead, g_scheduleManager.trace.reading);
        g_scheduleManager.trace.reading = nullptr;
    }
    /* Write header. */
    if (!g_scheduleManager.trace.headerWritten) {
        return TraceDumpHeader(len);
    }
    /* Wait for new data.Park current CODEThread when fullBufHead is empty. */
    if (DulinkIsEmpty(&g_scheduleManager.trace.fullBufHead) && !g_scheduleManager.trace.shutdown) {
        g_scheduleManager.trace.reader = CODEThreadGet();
        CODEThreadPark(TraceParkUnlock, TRACE_EV_CODETHREAD_BLOCK, nullptr);
        pthread_mutex_lock(&g_scheduleManager.trace.lock);
    }
    /* Write a buffer. */
    if (!DulinkIsEmpty(&g_scheduleManager.trace.fullBufHead)) {
        g_scheduleManager.trace.reading = reinterpret_cast<struct TraceBuf *>(g_scheduleManager.trace.fullBufHead.next);
        DulinkPopHead(&g_scheduleManager.trace.fullBufHead);
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        *len = g_scheduleManager.trace.reading->header.pos;
        return g_scheduleManager.trace.reading->arr;
    }
    /* Write footer. */
    if (!g_scheduleManager.trace.footerWritten) {
        return TraceDumpFooter(len);
    }
    /* Wake up the stopCODEThread. */
    if (g_scheduleManager.trace.shutdown) {
        if (g_scheduleManager.trace.stopCODEThread != nullptr) {
            CODEThreadReady(g_scheduleManager.trace.stopCODEThread);
            g_scheduleManager.trace.stopCODEThread = nullptr;
        } else {
            LOG_ERROR(ERRNO_TRACE_STOP_EXCEPTION, "TraceStop exception");
        }
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        return nullptr;
    }
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    LOG_ERROR(ERRNO_TRACE_READER_SPURIOUS_WAKEUP, "runtime: spurious wakeup of trace reader");
    return nullptr;
}

bool TraceReaderAvailable(void)
{
    return g_scheduleManager.trace.reader != nullptr &&
           (!DulinkIsEmpty(&g_scheduleManager.trace.fullBufHead) || g_scheduleManager.trace.shutdown);
}

struct CODEThread *TraceReaderGet(void)
{
    struct CODEThread *reader;
    if (!TraceReaderAvailable()) {
        return nullptr;
    }
    pthread_mutex_lock(&g_scheduleManager.trace.lock);
    if (!TraceReaderAvailable()) {
        pthread_mutex_unlock(&g_scheduleManager.trace.lock);
        return nullptr;
    }
    reader = g_scheduleManager.trace.reader;
    g_scheduleManager.trace.reader = nullptr;
    pthread_mutex_unlock(&g_scheduleManager.trace.lock);
    return reader;
}

bool TraceGetStrIdFromNameArr(char **funcNames, char **fileNames, int stackSize, int limit,
                              unsigned long long *funcNameStringIds,
                              unsigned long long *fileNameStringIds)
{
    unsigned long long funcNameStringId = 0;
    unsigned long long fileNameStringId = 0;
    for (int i = 0; i < stackSize; i++) {
        char* funcName = funcNames[i];
        char* fileName = fileNames[i];
        if (i >= limit) {
            if (funcName != nullptr) {
                free(funcName);
            }
            if (fileName != nullptr) {
                free(fileName);
            }
            continue;
        }

        funcNameStringId = TraceRecordStringEvent(funcName);
        /* 'libcangjie-runtime.so' str is sent to buf when start trace. */
        if (fileName != nullptr && strcmp(fileName, TRACE_RUNTIME_STRING) == 0) {
            fileNameStringId = 1;
        } else {
            fileNameStringId = TraceRecordStringEvent(fileName);
        }

        if (funcName != nullptr) {
            free(funcName);
        }
        if (fileName != nullptr) {
            free(fileName);
        }
        if (funcNameStringId == 0 || fileNameStringId == 0) {
            LOG_ERROR(ERRNO_TRACE_STACK_EVENT, "string id is 0");
            return false;
        }
        funcNameStringIds[i] = funcNameStringId;
        fileNameStringIds[i] = fileNameStringId;
    }
    return true;
}

bool TraceRecordStackEvent(unsigned long long stackId, int skip, void *mutator)
{
    unsigned char evBaseArgNum = 1;
    const unsigned int arrSize = TRACE_STACK_MAX_DEPTH;
    int stackSize = TRACE_STACK_MAX_DEPTH;
    const unsigned int maxStrSize = TRACE_BUF_LENGTH - 1;
    struct TraceBuf **traceBuf;
    unsigned int processorId;
    char *funcNames[arrSize] = {};
    char *fileNames[arrSize] = {};
    unsigned int lineNumbers[arrSize] = {0};
    unsigned long long funcNameStringIds[arrSize];
    unsigned long long fileNameStringIds[arrSize];
    unsigned int framePcs[arrSize];
    stackSize = MapleRuntime::CODE_MCC_InitCODEthreadStackInfo(maxStrSize, mutator, framePcs,
                                                           funcNames, fileNames, lineNumbers);
    if (stackSize <= 0 || stackSize > static_cast<int>(arrSize)) {
        LOG_ERROR(ERRNO_TRACE_STACK_EVENT, "get CODEThread StackTrace failed");
        return false;
    }

    if (!TraceGetStrIdFromNameArr(funcNames, fileNames, stackSize, skip,
                                  funcNameStringIds, fileNameStringIds)) {
        return false;
    }
    atomic_fetch_add(&g_scheduleManager.trace.eventCount, 1);
    traceBuf = TraceBufAquire(&processorId);
    if (!TraceCheckEvSizeAndFlush(traceBuf, processorId, TRACE_STACK_EVENT_MAXSIZE, true)) {
        atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
        return false;
    }

    TraceByte(*traceBuf, static_cast<unsigned char>(TRACE_EV_STACK & TRACE_EFFECTIVE_EVENT));
    TraceByte(*traceBuf, evBaseArgNum + TRACE_STACK_ARG_NUM * stackSize);
    TraceUint64(*traceBuf, stackId);
    TraceByte(*traceBuf, stackSize);
    for (int i = 0; i < stackSize; i++) {
        TraceUint64(*traceBuf, framePcs[i]);
        TraceUint64(*traceBuf, funcNameStringIds[i]);
        TraceUint64(*traceBuf, fileNameStringIds[i]);
        TraceUint64(*traceBuf, lineNumbers[i]);
    }

    TraceBufRelease();
    atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
    return true;
}

bool TraceRecordSpecialStackEvent(unsigned long long stackId, const char* funcStr)
{
    auto fucStringId = TraceRecordStringEvent(funcStr);
    unsigned char evArgNum = 1;
    unsigned int processorId;
    atomic_fetch_add(&g_scheduleManager.trace.eventCount, 1);
    struct TraceBuf **traceBuf = TraceBufAquire(&processorId);
    if (!TraceCheckEvSizeAndFlush(traceBuf, processorId, TRACE_STACK_EVENT_MAXSIZE, true)) {
        atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
        return false;
    }

    TraceByte(*traceBuf, static_cast<unsigned char>(TRACE_EV_STACK & TRACE_EFFECTIVE_EVENT));
    TraceByte(*traceBuf, evArgNum + TRACE_STACK_ARG_NUM * 1);
    TraceUint64(*traceBuf, stackId);
    TraceByte(*traceBuf, 1);

    TraceByte(*traceBuf, 0);
    TraceUint64(*traceBuf, fucStringId);
    TraceUint64(*traceBuf, 1);
    TraceUint64(*traceBuf, 0);

    TraceBufRelease();
    atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
    return true;
}

/* Obtain the id of the call stack. */
unsigned long long TraceStackId()
{
    unsigned long long stackId = atomic_fetch_add(&g_scheduleManager.trace.stackId, 1ULL);
    return stackId;
}

unsigned long long TraceRecordStringEvent(const char* str)
{
    if (str == nullptr) {
        LOG_ERROR(ERRNO_TRACE_STRING_EVENT, "the string is not meeting requirements in string event");
        return 0;
    }
    size_t len = strlen(str);
    struct TraceBuf **traceBuf;
    unsigned int processorId;
    if (TraceDoubleCheckShutdown()) {
        return 0;
    }
    traceBuf = TraceBufAquire(&processorId);
    if (!TraceCheckEvSizeAndFlush(traceBuf, processorId, len, true)) {
        atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
        return 0;
    }
    TraceByte(*traceBuf, static_cast<unsigned char>(TRACE_EV_STRING & TRACE_EFFECTIVE_EVENT));
    auto stringId = atomic_fetch_add(&g_scheduleManager.trace.stringId, 1ULL);
    TraceUint64(*traceBuf, stringId);
    TraceUint64(*traceBuf, len);
    for (size_t i = 0; i < len; i++) {
        TraceByte(*traceBuf, str[i]);
    }
    TraceBufRelease();
    atomic_fetch_sub(&g_scheduleManager.trace.eventCount, 1);
    return stringId;
}

void TraceRecordStartEvent()
{
    /* Record the codethread events of the currently enabled trace. */
    auto currCODEThread = CODEThreadGet();
    auto currThread = ThreadGet();
    if (currCODEThread != nullptr && currCODEThread != currThread->codethread0) {
        ScheduleTraceEvent(TRACE_EV_PROC_WAKE, -1, nullptr, 1,
                           static_cast<unsigned long long>(currThread->tid));
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_CREATE, TRACE_STACK_10, currCODEThread, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(currCODEThread)));
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(currCODEThread)));
    }
    /* Record runtime name for string event. It will be used for stack strace. */
    TraceRecordStringEvent(TRACE_RUNTIME_STRING);
    /* Record codethread special stack event. */
    TraceRecordSpecialStackEvent(SpecialStackId::CODETHREAD_EXIT, TRACE_EXIT_STRING);
    TraceRecordSpecialStackEvent(SpecialStackId::CODETHREAD_RESCHED, TRACE_RESCHED_STRING);
    TraceRecordSpecialStackEvent(SpecialStackId::CODETHREAD_NET_BLOCK, TRACE_NET_BLOCK_STRING);
    TraceRecordSpecialStackEvent(SpecialStackId::CODETHREAD_NET_UNBLOCK, TRACE_NET_UNBLOCK_STRING);
    TraceRecordSpecialStackEvent(SpecialStackId::CODETHREAD_UNBLOCK, TRACE_UNBLOCK_STRING);
    TraceRecordSpecialStackEvent(SpecialStackId::UNKNOWN, TRACE_UNKNOWN_STRING);
}

void TraceRegister()
{
    g_scheduleManager.trace.hooks.traceDeregister = TraceDeregister;
    g_scheduleManager.trace.hooks.traceStart = TraceStart;
    g_scheduleManager.trace.hooks.traceStop = TraceStop;
    g_scheduleManager.trace.hooks.traceRecordEvent = TraceRecordEvent;
    g_scheduleManager.trace.hooks.traceDump = TraceDump;
    g_scheduleManager.trace.hooks.traceReaderGet = TraceReaderGet;
}

void TraceDeregister()
{
    g_scheduleManager.trace.hooks.traceDeregister = nullptr;
    g_scheduleManager.trace.hooks.traceStart = nullptr;
    g_scheduleManager.trace.hooks.traceStop = nullptr;
    g_scheduleManager.trace.hooks.traceRecordEvent = nullptr;
    g_scheduleManager.trace.hooks.traceDump = nullptr;
    g_scheduleManager.trace.hooks.traceReaderGet = nullptr;
}

#ifdef  __cplusplus
}
#endif
