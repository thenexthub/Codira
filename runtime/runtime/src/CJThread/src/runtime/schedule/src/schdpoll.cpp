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


#include "schdpoll.h"
#include "schedule_impl.h"
#include "log.h"

#ifdef MRT_MACOS
#include <sys/event.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void SchdpollInit(void)
{
    struct Netpoll *netpoll = &ScheduleGet()->netpoll;
    pthread_mutex_lock(&netpoll->pollMutex);
    if (netpoll->npfd == nullptr) {
        netpoll->npfd = NetpollCreate();
    }
    pthread_mutex_unlock(&netpoll->pollMutex);
}

struct SchdpollDesc *SchdpollAdd(FdHandle fd)
{
    struct SchdpollDesc *pd;
    struct Schedule *schedule = ScheduleGet();

    if (schedule->netpoll.npfd == nullptr) {
        SchdpollInit();
    }

    pd = (struct SchdpollDesc *)malloc(sizeof(struct SchdpollDesc));
    if (pd == nullptr) {
        LOG_ERROR(ERRNO_SCHD_MALLOC_FAILED, "malloc failed, size: %u", sizeof(struct SchdpollDesc));
        return nullptr;
    }
    (void)memset_s(pd, sizeof(struct SchdpollDesc), 0, sizeof(struct SchdpollDesc));
    pd->codethread.readWaiter = PD_NOWAIT;
    pd->codethread.writeWaiter = PD_NOWAIT;
#ifdef MRT_LINUX
    pd->type = SCHDPOLL_CODETHREAD;
    if (NetpollAdd(schedule->netpoll.npfd, fd, pd, EPOLLIN | EPOLLOUT | EPOLLRDHUP) != 0) {
        free(pd);
        return nullptr;
    }
#elif defined (MRT_WINDOWS)
    if (NetpollAdd(schedule->netpoll.npfd, fd, nullptr, 0) != 0) {
        free(pd);
        return nullptr;
    }
#elif defined (MRT_MACOS)
    NetpollFd npfd = schedule->netpoll.npfd;
    if (NetpollAdd(npfd, fd, pd, EVFILT_READ) != 0 ||
        NetpollAdd(npfd, fd, pd, EVFILT_WRITE) != 0) {
        free(pd);
        return nullptr;
    }
#endif

    return pd;
}

#ifdef MRT_WINDOWS

int SchdpollDel(struct Schedule *schedule, FdHandle fd, struct SchdpollDesc *pd, int netpollState)
{
    int ret;
    // The lock is added to ensure that the schmon thread does not access the pd after the pd
    // is unbound from the Netpoll.
    if (netpollState == NETPOLL_ADDED) {
        pthread_mutex_lock(&schedule->netpoll.pollMutex);
        ret = NetpollDel(schedule->netpoll.npfd, fd);
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        if (ret != 0) {
            return ret;
        }
    }

    if (pd != nullptr) {
        free(pd);
    }
    return 0;
}

#elif defined (MRT_LINUX)

int SchdpollDel(struct Schedule *schedule, FdHandle fd, struct SchdpollDesc *pd, int netpollState)
{
    int ret;
    // The value of netpollState indicates whether netpoll_add has been executed for the fd.
    // If netpoll_ADD has been executed for the fd, netpoll_del must be executed for the fd.
    if (netpollState == NETPOLL_ADDED) {
        ret = NetpollDel(schedule->netpoll.npfd, fd);
        if (ret != 0) {
            return ret;
        }
    }

    // To avoid the pd wild pointer problem caused by concurrency with SchdpollAcquire, the pd
    // is added to the linked list and released at the end of each SchdpollAcquire. At this
    // time, the pd is removed from the netpoll listening queue and will not be accessed after
    // being released.
    if (pd != nullptr) {
        PthreadSpinLock(&schedule->netpoll.closingLock);
        pd->next = schedule->netpoll.closingPd;
        schedule->netpoll.closingPd = pd;
        PthreadSpinUnlock(&schedule->netpoll.closingLock);
    }

    return 0;
}

#elif defined (MRT_MACOS)

int SchdpollDel(struct Schedule *schedule, FdHandle fd, struct SchdpollDesc *pd, int netpollState)
{
    int ret = 0;
    if (netpollState == NETPOLL_ADDED) {
        ret = NetpollDel(schedule->netpoll.npfd, fd, EVFILT_READ);
        if (ret != 0) {
            return ret;
        }
        ret = NetpollDel(schedule->netpoll.npfd, fd, EVFILT_WRITE);
        if (ret != 0) {
            return ret;
        }
    }

    if (pd != nullptr) {
        PthreadSpinLock(&schedule->netpoll.closingLock);
        pd->next = schedule->netpoll.closingPd;
        schedule->netpoll.closingPd = pd;
        PthreadSpinUnlock(&schedule->netpoll.closingLock);
    }

    return 0;
}

#endif // MRT_WINDOWS

/* Callback function transferred to codethreadPark */
int SchdpollWaitPark(void *arg, CODEThreadHandle codethread)
{
    std::atomic<uintptr_t> *waiter = (std::atomic<uintptr_t> *)arg;
    uintptr_t old;
    uintptr_t expected = PD_NOWAIT;

    while (1) {
        old = *waiter;
        // If the status changes to ready or closing after entering the park state, do not park.
        if (old == PD_READY || old == PD_CLOSING) {
            return -1;
        }
        // Convert from nowait to codethread so that the codethread is subsequently woken up.
        if (atomic_compare_exchange_weak(waiter, &expected, (uintptr_t)codethread)) {
            return 0;
        }
    }

    return 0;
}

/* Monitor the corresponding pd and call copark to let out the codethread.
 * Note: The caller must ensure that this interface is not invoked concurrently for the same pd.
 * In this case, the status can only be PD_NEWAIT, PD_READY, or PD_CLOSING before the pd enters
 * the park state.
 */
bool SchdpollWait(struct SchdpollDesc *pd, SchdpollEventType type)
{
    std::atomic<uintptr_t> *waiter;
    uintptr_t state;

    if (type == SHCDPOLL_READ) {
        waiter = &pd->codethread.readWaiter;
    } else {
        waiter = &pd->codethread.writeWaiter;
    }

    // Check whether the event is ready or closed before entering the park.
    state = *waiter;
    if (state == PD_READY) {
        atomic_store(waiter, (uintptr_t)PD_NOWAIT);
        return true;
    } else if (state == PD_CLOSING) {
        return false;
    }

    // The park may fail, which is normal.
    CODEThreadPark(SchdpollWaitPark, TRACE_EV_CODETHREAD_BLOCK_NET, waiter);

    // The status can be ready or closing, or the status can be changed from ready to closing.
    state = PD_READY;
    if (atomic_compare_exchange_strong(waiter, &state, (uintptr_t)PD_NOWAIT)) {
        return true;
    } else {
        return false;
    }
}

/* Obtain the codethread to be woken up on the pd when the corresponding event arrives. */
struct CODEThread *SchdpollReady(struct SchdpollDesc *pd, SchdpollEventType type, bool ioready)
{
    std::atomic<uintptr_t> *waiter;
    uintptr_t old;
    uintptr_t newState = ioready ? PD_READY : PD_CLOSING;
    struct CODEThread *codethread;
    CODEThreadState pending = CODETHREAD_PENDING;

    if (type == SHCDPOLL_READ) {
        waiter = &pd->codethread.readWaiter;
    } else {
        waiter = &pd->codethread.writeWaiter;
    }

    while (1) {
        old = *waiter;
        // If the process is closed, only the PD_CLOSING state is returned. If the event is
        // ready, both PD_CLOSING and PD_READY are returned.
        if (old == PD_CLOSING || old == newState) {
            return nullptr;
        }

        // ready event: The status of PD_NEWAIT and codethread changes to PD_READY.
        // close process: PD_NOWAIT, PD_READY, and codethread states change to PD_CLOSING.
        if (atomic_compare_exchange_weak(waiter, &old, newState)) {
            // No need to wake up when no codethread waits on pd
            if (old == PD_NOWAIT || old == PD_READY) {
                return nullptr;
            }

            // If old is not equal to nowait, it indicates that a codethread is blocked in poll
            // wait and needs to be woken up.
            codethread = (struct CODEThread *)old;
            if (atomic_compare_exchange_strong(&codethread->state, &pending, CODETHREAD_READY)) {
                TRACE_FINISH_ASYNC(TRACE_CODETHREAD_PARK, codethread->id);
                ScheduleTraceEvent(TRACE_EV_CODETHREAD_UNBLOCK, -1, CODEThreadGet(), TraceArgNum::TRACE_ARGS_2,
                                   CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)), CODETHREAD_NET_UNBLOCK);
                return codethread;
            }
            return nullptr;
        }
    }
}

#if defined (MRT_LINUX) || defined (MRT_MACOS)

void SchdpollFreePd(void)
{
    struct SchdpollDesc *pd;
    struct SchdpollDesc *closingPd;
    struct Schedule *schedule = ScheduleGet();

    PthreadSpinLock(&schedule->netpoll.closingLock);
    closingPd = schedule->netpoll.closingPd;
    while (closingPd != nullptr) {
        pd = closingPd;
        closingPd = closingPd->next;
        free(pd);
    }
    schedule->netpoll.closingPd = nullptr;
    PthreadSpinUnlock(&schedule->netpoll.closingLock);
}

#endif

int InitEventsNum(unsigned int bufLen)
{
    bufLen = (bufLen > SCHDPOLL_ACQUIRE_MAX_NUM) ? SCHDPOLL_ACQUIRE_MAX_NUM : bufLen;
    int num = static_cast<int>(bufLen >> 1);
    return num;
}

#ifdef MRT_WINDOWS
/* Call netpoll to obtain the ready codethread queue. */
int SchdpollAcquire(struct Schedule *schedule, void *buf[], unsigned int bufLen, int timeout)
{
    int entriesNum;
    int entriesIdx;
    int bufIdx = 0;
    int error;
    int ret;
    OVERLAPPED *overlapped;
    OVERLAPPED_ENTRY *pollEntry;
    OVERLAPPED_ENTRY entries[SCHDPOLL_EVENT_NUM];
    DWORD transfer;
    DWORD flag;
    struct IocpOperation *ioperation;
    struct SchdpollDesc *pd;
    struct CODEThread *wakeCODEThread;

    // Only one thread needs to perform netpoll_wait at a time. Because one thread can obtain
    // all events, multiple threads do not need to be concurrent.
    if (pthread_mutex_trylock(&schedule->netpoll.pollMutex) != 0) {
        return 0;
    }
    entriesNum = InitEventsNum(bufLen);
    entriesNum = NetpollWait(schedule->netpoll.npfd, entries, entriesNum, timeout);
    if (entriesNum <= 0) {
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        return entriesNum;
    }

    // events_num <= buf_len / 2, so buf_idx is not out of bounds.
    for (entriesIdx = 0; entriesIdx < entriesNum; ++entriesIdx) {
        pollEntry = &(entries[entriesIdx]);
        overlapped = pollEntry->lpOverlapped;
        if (overlapped == nullptr) {
            LOG_ERROR(ERRNO_NETPOLL_ARG_INVAILD, "NetpollWait success, but overlapped is nullptr");
            continue;
        }

        error = 0;
        transfer = 0;
        ioperation = (struct IocpOperation *)overlapped;
        ret = WSAGetOverlappedResult(ioperation->rawFd, &ioperation->overlapped, &transfer, FALSE, &flag);
        if (ret == FALSE) {
            error = WSAGetLastError();
        }
        ioperation->transBytes = transfer;
        ioperation->error = error;
        atomic_store(&ioperation->iocpComplete, true);
        pd = ioperation->pd;

        wakeCODEThread = SchdpollReady(pd, ioperation->type, true);
        if (wakeCODEThread != nullptr) {
            buf[bufIdx] = wakeCODEThread;
            bufIdx++;
        }
    }

    pthread_mutex_unlock(&schedule->netpoll.pollMutex);
    return bufIdx;
}
#elif defined (MRT_LINUX)
/* Processing callback functions in schdpoll */
void SchdpollAcquireCallback(struct SchdpollDesc *pd, struct epoll_event *pollEvent)
{
    if (pd->type == SCHDPOLL_CALLBACK_EVENT) {
        SchdpollEventProcFunc func = (SchdpollEventProcFunc)pd->callback.func;
        func(pollEvent, pd->callback.arg);
    } else if (pd->type == SCHDPOLL_CALLBACK_FD_OUTSIDE) {
        SchdpollFdEventProcFunc func = (SchdpollFdEventProcFunc)pd->callback.func;
        func(pd->callback.fd, pollEvent->events, pd->callback.arg);
        free(pd);
    } else if (pd->type == SCHDPOLL_CALLBACK) {
        SchdpollFdEventProcFunc func = (SchdpollFdEventProcFunc)pd->callback.func;
        func(pd->callback.fd, pollEvent->events, pd->callback.arg);
    }
}

/* Call netpoll to obtain the ready codethread queue. */
int SchdpollAcquire(struct Schedule *schedule, void *buf[], unsigned int bufLen, int timeout)
{
    int eventsNum;
    int eventsIdx;
    int bufIdx = 0;
    struct epoll_event *pollEvent;
    struct SchdpollDesc *pd;
    struct CODEThread *wakeCODEThread;
    struct epoll_event events[SCHDPOLL_EVENT_NUM];

    // Only one thread needs to perform netpoll_wait at a time. Because one thread can obtain
    // all events, multiple threads do not need to be concurrent.
    if (pthread_mutex_trylock(&schedule->netpoll.pollMutex) != 0) {
        return 0;
    }
    if (schedule->state == SCHEDULE_SUSPENDING) {
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        return 0;
    }
    
    eventsNum = InitEventsNum(bufLen);
    // Wait events
    eventsNum = NetpollWait(schedule->netpoll.npfd, events, eventsNum, timeout);
    if (eventsNum <= 0) {
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        return eventsNum;
    }

    // events_num <= buf_len / 2, so buf_idx is not out of bounds.
    for (eventsIdx = 0; eventsIdx < eventsNum; ++eventsIdx) {
        pollEvent = &(events[eventsIdx]);
        pd = static_cast<struct SchdpollDesc *>(pollEvent->data.ptr);
        if (pd->callback.func != nullptr) {
            SchdpollAcquireCallback(pd, pollEvent);
            continue;
        }
        // codethread asynchronous IO
        if ((pollEvent->events & NETPOLL_READ_EVENT) != 0) {
            wakeCODEThread = SchdpollReady(pd, SHCDPOLL_READ, true);
            if (wakeCODEThread != nullptr) {
                buf[bufIdx] = wakeCODEThread;
                bufIdx++;
            }
        }
        if ((pollEvent->events & NETPOLL_WRITE_EVENT) != 0) {
            wakeCODEThread = SchdpollReady(pd, SHCDPOLL_WRITE, true);
            if (wakeCODEThread != nullptr) {
                buf[bufIdx] = wakeCODEThread;
                bufIdx++;
            }
        }
    }

    if (schedule->netpoll.closingPd != nullptr) {
        SchdpollFreePd();
    }

    pthread_mutex_unlock(&schedule->netpoll.pollMutex);
    return bufIdx;
}

int SchdpollInnerFd(void)
{
    struct Schedule *schedule = ScheduleGet();
    if (schedule->netpoll.npfd == nullptr) {
        SchdpollInit();
    }
    return NetpollInnerFd(ScheduleGet()->netpoll.npfd);
}

void *SchdpollCallbackCODEThread(void *arg, unsigned int argSize)
{
    (void)argSize;
    struct SchdpollNotifyUsrInfo *info = (struct SchdpollNotifyUsrInfo *)arg;
    SchdpollFdEventProcFunc func = (SchdpollFdEventProcFunc)info->func;

    if (func == nullptr) {
        LOG_ERROR(ERRNO_SCHD_HOOK_INVLAID, "func should not be null");
        return nullptr;
    }

    func(info->fd, info->events, info->arg);

    return nullptr;
}

/* This function is called in schmon to generate a new codethread to run the user function. */
unsigned int SchdpollNotifyCallback(int fd, int events, void *arg)
{
    (void)fd;
    (void)events;
    CODEThreadHandle codethread;
    struct CODEThreadAttr attr;
    struct SchdpollNotifyUsrInfo *info = (struct SchdpollNotifyUsrInfo *)arg;

    info->events = events;
    CODEThreadAttrInit(&attr);
    CODEThreadAttrCodeFromCSet(&attr, true);
    codethread = CODEThreadNew(ScheduleGet(), &attr, SchdpollCallbackCODEThread,
                           info, sizeof(struct SchdpollNotifyUsrInfo));
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_CODETHREAD_ALLOC_FAILED, "CODEThreadNew failed");
    }

    return 0;
}

/* Add the callback function of schdpoll. */
struct SchdpollDesc *SchdpollCallbackAdd(int fd, int events, void *func, void *arg, SchdpollDescType type)
{
    int ret;
    struct SchdpollDesc *pd;
    struct Schedule *schedule = g_scheduleManager.defaultSchedule;

    if (schedule->netpoll.npfd == nullptr) {
        SchdpollInit();
    }

    pd = (struct SchdpollDesc *)malloc(sizeof(struct SchdpollDesc));
    if (pd == nullptr) {
        LOG_ERROR(ERRNO_SCHD_MALLOC_FAILED, "malloc failed, size: %u", sizeof(struct SchdpollDesc));
        return nullptr;
    }
    pd->codethread.writeWaiter = 0;
    pd->codethread.readWaiter = 0;
    pd->callback.func = func;
    pd->callback.arg = arg;
    pd->type = type;
    ret = NetpollAdd(schedule->netpoll.npfd, fd, pd, events);
    if (ret != 0) {
        LOG_ERROR(ret, "NetpollAdd failed");
        free(pd);
        return nullptr;
    }
    return pd;
}

void *SchdpollNotifyAdd(int fd, int events, SchdpollNotifyFunc func, void *arg)
{
    struct Schedule *schedule = g_scheduleManager.defaultSchedule;
    struct SchdpollNotifyUsrInfo *info;

    if (func == nullptr) {
        LOG_ERROR(ERRNO_SCHD_HOOK_INVLAID, "func should not be null");
        return nullptr;
    }

    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "schedule should not be null");
        return nullptr;
    }

    // info is released at schdpoll_notify_del.
    info = (struct SchdpollNotifyUsrInfo *)malloc(sizeof(struct SchdpollNotifyUsrInfo));
    if (info == nullptr) {
        LOG_ERROR(ERRNO_SCHD_MALLOC_FAILED, "malloc failed, size: %u", sizeof(struct SchdpollNotifyUsrInfo));
        return nullptr;
    }
    info->fd = fd;
    info->func = (void *)func;
    info->arg = arg;
    return SchdpollCallbackAdd(fd, events, (void *)SchdpollNotifyCallback, info, SCHDPOLL_CALLBACK);
}

int SchdpollNotifyDel(int fd, void *pd)
{
    struct Schedule *schedule = g_scheduleManager.defaultSchedule;
    int ret;

    if (pd == nullptr) {
        LOG_ERROR(ERRNO_SCHD_ARG_INVALID, "pd should not be null");
        return ERRNO_SCHD_ARG_INVALID;
    }

    if (schedule == nullptr) {
        LOG_ERROR(ERRNO_SCHD_INVALID, "schedule should not be null");
        return ERRNO_SCHD_INVALID;
    }

    // The netpoll_add operation has been performed on all FDs. Therefore, the value of
    // netpoll_flag is true.
    ret = SchdpollDel(schedule, fd, (struct SchdpollDesc *)pd, NETPOLL_ADDED);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
#elif defined (MRT_MACOS)

int SchdpollAcquire(struct Schedule *schedule, void *buf[], unsigned int bufLen, int timeout)
{
    int eventsNum;
    int eventsIdx;
    int bufIdx = 0;
    struct kevent *pollEvent;
    struct SchdpollDesc *pd;
    struct CODEThread *wakeCODEThread;
    struct kevent kevents[SCHDPOLL_EVENT_NUM];
 
    if (pthread_mutex_trylock(&schedule->netpoll.pollMutex) != 0) {
        return 0;
    }
    if (schedule->state == SCHEDULE_SUSPENDING) {
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        return 0;
    }

    eventsNum = InitEventsNum(bufLen);
    eventsNum = NetpollWait(schedule->netpoll.npfd, kevents, eventsNum, timeout);
    if (eventsNum < 0) {
        pthread_mutex_unlock(&schedule->netpoll.pollMutex);
        return eventsNum;
    }

    for (eventsIdx = 0; eventsIdx < eventsNum; ++eventsIdx) {
        pollEvent = &(kevents[eventsIdx]);
        if (pollEvent->flags & EV_ERROR) {
            LOG_ERROR(ERRNO_SCHD_KEVENT_FAILED, "kevent error. fd:%d", pollEvent->ident);
            continue;
        }
        pd = static_cast<struct SchdpollDesc *>(pollEvent->udata);

        switch (pollEvent->filter) {
            case EVFILT_READ:
                wakeCODEThread = SchdpollReady(pd, SHCDPOLL_READ, true);
                break;
            case EVFILT_WRITE:
                wakeCODEThread = SchdpollReady(pd, SHCDPOLL_WRITE, true);
                break;
            default:
                pthread_mutex_unlock(&schedule->netpoll.pollMutex);
                return 0;
        }
        if (wakeCODEThread != nullptr) {
            buf[bufIdx] = wakeCODEThread;
            bufIdx++;
        }
    }

    if (schedule->netpoll.closingPd != nullptr) {
        SchdpollFreePd();
    }

    pthread_mutex_unlock(&schedule->netpoll.pollMutex);
    return bufIdx;
}

#endif

#ifdef __cplusplus
}
#endif
