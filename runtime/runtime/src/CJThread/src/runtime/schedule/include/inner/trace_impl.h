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


#ifndef MRT_TRACE_IMPL_H
#define MRT_TRACE_IMPL_H

#include <pthread.h>
#include <atomic>
#include <cstdbool>
#include "list.h"
#ifdef MRT_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* (event type)1byte + (arg num)1byte + (time)1-8byte + (0-9) * (extra arg)1-8byte + (stackId)1-8byte = 90 */
#define TRACE_EVENT_MAXSIZE (90)
/* (event type)1byte + (arg num)1byte + (stackId)1-8byte + (PCs)1byte + frames(240) = 251 */
#define TRACE_STACK_EVENT_MAXSIZE (251)
#define TRACE_HEADER "Codira trace"
#define TRACE_HEADER_LENGTH (32)
#define TRACE_PATH_LENGTH (256)
#define TRACE_EFFECTIVE_EVENT (0x00ff)
#define TRACE_EFFECTIVE_ARG_NUM (0x00ff)
#define TRACE_BUF_LENGTH ((64 << 10) - sizeof(struct TraceBufHeader))
#define TRACE_UINT64_SHIFTS (7)
#define TRACE_UINT64_SHIFT_THRESHOLD (0x80)
#define TRACE_STACK_ARG_NUM (4)
#define TRACE_EXIT_STRING "CODE_CODEThreadExit"
#define TRACE_RESCHED_STRING "CODE_CODEThreadResched"
#define TRACE_NET_BLOCK_STRING "CODE_SchdpollWait"
#define TRACE_NET_UNBLOCK_STRING "CODE_SchdpollReady"
#define TRACE_UNBLOCK_STRING "CODE_CODEThreadReady"
#define TRACE_UNKNOWN_STRING "?"
#define TRACE_RUNTIME_STRING "libcangjie-runtime.so"    /* string id is 1 */

#ifdef MRT_WINDOWS
typedef HMODULE DlHandle ;
#else
typedef void *DlHandle;
#endif

typedef void (*TraceRegisterFunc)(void);

typedef void (*TraceDeregisterFunc)(void);

/**
 * arg: trace type
 * ret: true or false
 */
typedef bool (*TraceStartFunc)(unsigned short traceType);

/**
 * ret: true or false
 */
typedef bool (*TraceStopFunc)(void);

/**
 * arg: event typ, backstack layers, mutator, number of extra arg, extra arg list
 */
typedef void (*TraceEventFunc)(TraceEvent event, int skip, void *mutator, int argNum, va_list args);

/**
 * arg: length of array
 * ret: pointer of array
 */
typedef unsigned char *(*TraceDumpFunc)(int *len);

/**
 * ret: nullptr or CODEThread pointer
 */
typedef struct CODEThread *(*TraceReaderGetFunc)(void);

struct TraceHooks {
    TraceDeregisterFunc traceDeregister;
    TraceStartFunc traceStart;
    TraceStopFunc traceStop;
    TraceEventFunc traceRecordEvent;
    TraceDumpFunc traceDump;
    TraceReaderGetFunc traceReaderGet;
};

struct TraceBufHeader {
    struct Dulink dulink;
    unsigned long long lastTicks;               /* last Event Occurred Event */
    int pos;                                    /* array offset */
};

struct TraceBuf {
    struct TraceBufHeader header;
    unsigned char arr[TRACE_BUF_LENGTH];        /* Path for storing trace events */
};

struct Trace {
    bool mutexInitFlag;                         /* lock creation flag. The lock is created
                                                 * only once and is destroyed when the
                                                 * schedule exits. */
    DlHandle dlHandle;                          /* trace dll handle */
    unsigned short openType;                    /* trace enable flag */
    bool shutdown;                              /* trace disable flag */
    pthread_mutex_t lock;
    /* ------------------------lock protection range-------------------- */
    bool headerWritten;
    bool footerWritten;
    unsigned long long ticksStart;
    unsigned long long ticksEnd;
    unsigned long long timeStart;
    unsigned long long timeEnd;
    struct Dulink fullBufHead;                  /* list of traceBufs to be output */
    struct Dulink freeBufHead;                  /* list of idle traceBufs */
    struct TraceBuf *reading;                   /* traceBuf of trace data being output */
    struct CODEThread *reader;                    /* codethread for outputting trace data */
    struct CODEThread *stopCODEThread;              /* codethread for stop trace */
    /* ------------------------lock protection range-------------------- */
    std::atomic<int> eventCount;                /* event count */
    pthread_mutex_t bufLock;                    /* lock that protects the buf */
    struct TraceBuf *buf;                       /* global traceBuf */
    struct TraceHooks hooks;                    /* trace hooks */
    std::atomic<unsigned long long> stringId;   /* string event id */
    std::atomic<unsigned long long> stackId;    /* stack event id */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_TRACE_IMPL_H */
