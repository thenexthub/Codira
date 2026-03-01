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


#include <cerrno>
#include <cstdint>
#include "schdfd_impl.h"
#include "schedule_impl.h"
#include "processor.h"
#include "timer_impl.h"
#include "securec.h"
#include "log.h"
#include "external.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MRT_WINDOWS
const int PROTOCOL_BUF_SIZE = 32;
bool g_iocpCompleteSkipFlag = false;
#endif

/* init Shcdfd management structure. */
int SchdfdInit(int layerIndex, int lineIndex)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    int ret;
    unsigned int size;

    if (schdfdManager->slots[layerIndex] != nullptr && schdfdManager->slots[layerIndex][lineIndex] != nullptr) {
        return 0;
    }

    pthread_mutex_lock(&schdfdManager->initLock);
    if (schdfdManager->slots[layerIndex] != nullptr && schdfdManager->slots[layerIndex][lineIndex] != nullptr) {
        pthread_mutex_unlock(&schdfdManager->initLock);
        return 0;
    }

    if (schdfdManager->slots[layerIndex] == nullptr) {
        size = (unsigned int)(sizeof(struct SchdfdSlot *) * SCHDFD_SLOTS_LAYER_MAX_LINE_NUM);
        schdfdManager->slots[layerIndex] = reinterpret_cast<struct SchdfdSlot **>(malloc(size));
        if (schdfdManager->slots[layerIndex] == nullptr) {
            ret = errno;
            pthread_mutex_unlock(&schdfdManager->initLock);
            LOG_ERROR(ret, "malloc failed, size: %u, layerIndex: %d", size, layerIndex);
            return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
        }
        memset_s(schdfdManager->slots[layerIndex], size, 0, size);
    }
    size = (unsigned int)(sizeof(struct SchdfdSlot) * SCHDFD_SLOTS_LINE_MAX_FD_NUM);
    schdfdManager->slots[layerIndex][lineIndex] = reinterpret_cast<struct SchdfdSlot *>(malloc(size));
    if (schdfdManager->slots[layerIndex][lineIndex] == nullptr) {
        ret = errno;
        pthread_mutex_unlock(&schdfdManager->initLock);
        LOG_ERROR(ret, "malloc failed, size: %u, lineIndex: %d", size, lineIndex);
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }
    memset_s(schdfdManager->slots[layerIndex][lineIndex], size, 0, size);

    pthread_mutex_unlock(&schdfdManager->initLock);
    return 0;
}

/* Obtain SchdfdFd based on fd and increase the reference count.
 * Internal interface, no need to determine the legality of fd.
 */
struct SchdfdFd *SchdfdIncref(SignedSocket fd, bool close)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdSlot *slot;
    struct SchdfdFd *schdFd;
    unsigned long long int ref;
    unsigned long long int closeFlag = 0;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

    slot = &schdfdManager->slots[layerIndex][lineIndex][fdIndex];
    schdFd = slot->schdFd;
    // fd has not been initialized or has been closed, return directly
    if (schdFd == nullptr) {
        return nullptr;
    }

    if (close) {
        closeFlag = SCHDFD_CLOSING;
    }
    while (1) {
        ref = atomic_load(&slot->ref);
        // If the close flag is set, the acquisition fails and returns directly.
        if ((ref & SCHDFD_CLOSING) != 0) {
            return nullptr;
        }
        // If the close flag is not set, add 1 to the reference count through CAS.
        if (atomic_compare_exchange_weak(&slot->ref, &ref, (ref | closeFlag) + 1)) {
            return schdFd;
        }
    }
}

/* Reduce the reference count of fd, it must be used after SchdfdIncref.
 * Internal interface, no need to determine the legality of fd.
 */
void SchdfdDecref(SignedSocket fd, bool close)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdSlot *slot;
    unsigned long long int ref;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

    slot = &schdfdManager->slots[layerIndex][lineIndex][fdIndex];
    while (1) {
        ref = atomic_load(&slot->ref);
        // The reference count is 0, indicating that it was not used after SchdfdIncrof.
        if ((ref & (~SCHDFD_CLOSING)) == 0) {
            LOG_ERROR(ERRNO_SCHDFD_MISUSED, "SchdfdDecref must used after SchdfdIncref");
            return;
        }
        // If closing process is not marked as closed, SchdfdIncrof did not fill in true.
        if (close && ((ref & SCHDFD_CLOSING) == 0)) {
            LOG_ERROR(ERRNO_SCHDFD_MISUSED, "close arg must be the same in incref and decref");
            return;
        }

        if ((!close) || ((ref - 1) == SCHDFD_CLOSING)) {
            // 1. Not in close process, the reference count will be successfully reduced and returned directly.
            // 2. In close process, the reference count must be reduced to 0 before it can be returned.
            if (atomic_compare_exchange_weak(&slot->ref, &ref, ref - 1)) {
                return;
            }
        } else {
            // 3. In close process and the reference count is not 0, execute CODEThreadResched
            // and wait for the next judgment
            CODEThreadResched();
        }
    }
}

#ifdef MRT_WINDOWS
static int IocpOperationInit(struct SchdfdFd *schdFd, SignedSocket fd)
{
    int ret;
    if (g_iocpCompleteSkipFlag) {
        // If the IO operation returns success immediately, it will not be delivered to the IOCP
        // waiting queue and will be returned directly.
        ret = SetFileCompletionNotificationModes((HANDLE)fd,
                                                 FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);
        if (ret == 0) {
            ret = (int)GetLastError();
            LOG_ERROR(ret, "SetFileCompletionNotificationModes failed, fd %llu registered", fd);
            return ret;
        }
    }

    schdFd->readOperation.pd = schdFd->pd;
    schdFd->writeOperation.pd = schdFd->pd;
    schdFd->readOperation.type = SHCDPOLL_READ;
    schdFd->writeOperation.type = SHCDPOLL_WRITE;
    schdFd->writeOperation.rawFd = fd;
    schdFd->readOperation.rawFd = fd;
    return 0;
}
#endif

/* init SchdfdFd struct */
static int SchdfdFdInit(struct SchdfdFd *schdFd)
{
    int error = SemaNew(&schdFd->readSema, 1);
    if (error) {
        LOG_ERROR(ERRNO_SCHDFD_INIT_RESOURCE_FAILED, "SemaNew failed");
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }
    error = SemaNew(&schdFd->writeSema, 1);
    if (error) {
        LOG_ERROR(ERRNO_SCHDFD_INIT_RESOURCE_FAILED, "SemaNew failed");
        SemaDelete(&schdFd->readSema);
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }
    schdFd->readTimer = SCHDFD_TIMER_NULL;
    schdFd->writeTimer = SCHDFD_TIMER_NULL;
    return 0;
}

int SchdfdRegister(SignedSocket fd)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    int ret;
    struct SchdfdFd *schdFd;
    struct SchdfdSlot *slot;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

#ifdef MRT_WINDOWS
    if (fd > SCHDFD_MAX_FD_NUM) {
        LOG_ERROR(ERRNO_SCHDFD_FD_OVERLIMIT, "fd %llu overlimit %u", fd, UINT_MAX);
        return ERRNO_SCHDFD_FD_OVERLIMIT;
    }
#endif
    if (SchdfdInit(layerIndex, lineIndex) != 0) {
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }

    slot = &schdfdManager->slots[layerIndex][lineIndex][fdIndex];
    if (slot->schdFd != nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_FD_REGISTED, "fd %llu registered", fd);
        return ERRNO_SCHDFD_FD_REGISTED;
    }

    schdFd = (struct SchdfdFd *)malloc(sizeof(struct SchdfdFd));
    if (schdFd == nullptr) {
        LOG_ERROR(errno, "malloc failed, size: %u", sizeof(struct SchdfdFd));
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }
    memset_s(schdFd, sizeof(struct SchdfdFd), 0, sizeof(struct SchdfdFd));
    ret = SchdfdFdInit(schdFd);
    if (ret != 0) {
        free(schdFd);
        return ret;
    }

    slot->schdFd = schdFd;
    atomic_store(&slot->ref, 0ULL);
    return 0;
}

/* Wake up all possible codethread that may block the fd */
void SchdfdWakeall(struct SchdfdFd *schdFd)
{
    struct CODEThread *readCODEThread;
    struct CODEThread *writeCODEThread;

    if (atomic_load(&schdFd->netpollState) == NETPOLL_CLOSING) {
        LOG_INFO(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd not yet add to netpoll");
        return;
    }
    readCODEThread = SchdpollReady(schdFd->pd, SHCDPOLL_READ, false);
    writeCODEThread = SchdpollReady(schdFd->pd, SHCDPOLL_WRITE, false);
    if (readCODEThread != nullptr) {
        ProcessorLocalWrite(readCODEThread);
        ProcessorWake(readCODEThread->schedule, nullptr);
    }
    if (writeCODEThread != nullptr) {
        ProcessorLocalWrite(writeCODEThread);
        ProcessorWake(writeCODEThread->schedule, nullptr);
    }
}

int SchdfdFdValidCheck(SignedSocket fd)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);

#ifdef MRT_WINDOWS
    if (fd > SCHDFD_MAX_FD_NUM) {
        LOG_ERROR(ERRNO_SCHDFD_FD_OVERLIMIT, "fd %llu overlimit %u", fd, UINT_MAX);
        return ERRNO_SCHDFD_FD_OVERLIMIT;
    }
#endif
    if (schdfdManager->slots[layerIndex] == nullptr || schdfdManager->slots[layerIndex][lineIndex] == nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_FD_CLOSING, "fd %llu not registered", fd);
        return ERRNO_SCHDFD_FD_CLOSING;
    }

    return 0;
}

int SchdfdNetpollAdd(SignedSocket fd)
{
    struct SchdfdFd *schdFd;
    int ret;
    NetpollState netpollState;

    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    schdFd = SchdfdIncref(fd, false);
    if (schdFd == nullptr) {
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    // After introducing raw sockets, there are several scenarios here:
    // 1. If netpollState is in the pending state, change the state to adding state
    // 2. If netpollState is in the process of adding, wait for a addition to complete until the state changes to added
    // 3. NetpollState is in the closing state, netpolllad operation is not allowed, error code returned
    while (true) {
        netpollState = atomic_load(&schdFd->netpollState);
        if (netpollState == NETPOLL_ADDED) {
            SchdfdDecref(fd, false);
            return 0;
        } else if (netpollState == NETPOLL_CLOSING) {
            SchdfdDecref(fd, false);
            return ERRNO_SCHDFD_SCHDPOLL_ADD_FAILED;
        } else if (netpollState == NETPOLL_TO_ADD &&
                   atomic_compare_exchange_strong(&schdFd->netpollState, &netpollState, NETPOLL_ADDING)) {
            break;
        }
    }

    schdFd->pd = SchdpollAdd((FdHandle)fd);
    if (schdFd->pd == nullptr) {
        atomic_store(&schdFd->netpollState, NETPOLL_TO_ADD);
        SchdfdDecref(fd, false);
        return ERRNO_SCHDFD_SCHDPOLL_ADD_FAILED;
    }
#ifdef MRT_WINDOWS
    // This function must be called after initializing SchdFd ->pd
    ret = IocpOperationInit(schdFd, fd);
    if (ret != 0) {
        atomic_store(&schdFd->netpollState, NETPOLL_TO_ADD);
        SchdfdDecref(fd, false);
        return ret;
    }
#endif
    atomic_store(&schdFd->netpollState, NETPOLL_ADDED);
    SchdfdDecref(fd, false);

    return 0;
}

int SchdfdDeregister(SignedSocket fd)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdFd *schdFd;
    int ret;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);
    NetpollState expectState = NETPOLL_TO_ADD;
    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    schdFd = SchdfdIncref(fd, true);
    if (schdFd == nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_FD_CLOSING, "fd %u not registered or double closed", fd);
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    while (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED &&
           !atomic_compare_exchange_strong(&schdFd->netpollState, &expectState, NETPOLL_CLOSING)) {
        expectState = NETPOLL_TO_ADD;
    }
    SchdfdWakeall(schdFd);
    SchdfdDecref(fd, true);

    // release SchdFd
    schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd = nullptr;
    // The netpoll_flag represents whether the fd has executed NetpollAdd,
    // and if true, the NetpollDel needs to be performed.
    SchdpollDel(ScheduleGet(), (FdHandle)fd, schdFd->pd, schdFd->netpollState);
    SemaDelete(&schdFd->readSema);
    SemaDelete(&schdFd->writeSema);
    free(schdFd);
    return 0;
}

void WakeallFd(SignedSocket fd)
{
    struct SchdfdFd *schdFd;
    int ret;
    NetpollState expectState = NETPOLL_TO_ADD;
    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return;
    }

    schdFd = SchdfdIncref(fd, false);
    if (schdFd == nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_FD_CLOSING, "fd %u not registered or double closed", fd);
        return;
    }
    while (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED &&
           !atomic_compare_exchange_strong(&schdFd->netpollState, &expectState, NETPOLL_CLOSING)) {
        expectState = NETPOLL_TO_ADD;
    }
    SchdfdWakeall(schdFd);
    SchdfdDecref(fd, false);
}

/* SchdfdWait cannot be called concurrently between the same fd and type,
 * and may require calling SchdfdLock/SchdfdUnlock to lock during concurrency.
 * SchdfdWaitInlock can be called for better performance.
 */
int SchdfdWait(SignedSocket fd, SchdpollEventType type)
{
    int ret;
    struct SchdfdFd *schdFd;

    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    schdFd = SchdfdIncref(fd, false);
    if (schdFd == nullptr) {
        return ERRNO_SCHDFD_FD_CLOSING;
    }

    if (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED) {
        LOG_ERROR(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd %u not yet add to netpoll", fd);
        SchdfdDecref(fd, false);
        return ERRNO_SCHDFD_NOT_ADD_NETPOLL;
    }

    if (!SchdpollWait(schdFd->pd, type)) {
        SchdfdDecref(fd, false);
        return ERRNO_SCHDFD_FD_CLOSING;
    }

    SchdfdDecref(fd, false);
    return 0;
}

/* SchdfdWaitInlock is used between SchdfdLock/SchdfdUnlock
 * and can omit parameter checking and atomic operations.
 */
int SchdfdWaitInlock(SignedSocket fd, SchdpollEventType type)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdFd *schdFd;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

    schdFd = reinterpret_cast<struct SchdfdFd *>(schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd);
    if (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED) {
        LOG_ERROR(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd %u not yet add to netpoll", fd);
        return ERRNO_SCHDFD_NOT_ADD_NETPOLL;
    }
    if (!SchdpollWait(schdFd->pd, type)) {
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    return 0;
}

void SchdfdReadTimeout(struct SchdfdFd *schdFd)
{
    struct CODEThread *readCODEThread;

    // Timer timeout is also considered as a ready event, and it is up to the party
    // who started the timer to determine which situation it is.
    readCODEThread = SchdpollReady(schdFd->pd, SHCDPOLL_READ, true);
    (void)FreeSchdfdTimer(schdFd, SHCDPOLL_READ);
    if (readCODEThread != nullptr) {
        ProcessorLocalWrite(readCODEThread);
        ProcessorWake(readCODEThread->schedule, nullptr);
    }
}

void SchdfdWriteTimeout(struct SchdfdFd *schdFd)
{
    struct CODEThread *writeCODEThread;

    writeCODEThread = SchdpollReady(schdFd->pd, SHCDPOLL_WRITE, true);
    (void)FreeSchdfdTimer(schdFd, SHCDPOLL_WRITE);
    if (writeCODEThread != nullptr) {
        ProcessorLocalWrite(writeCODEThread);
        ProcessorWake(writeCODEThread->schedule, nullptr);
    }
}

/* SchdfdWait with timeout. */
int SchdfdWaitTimeout(SignedSocket fd, SchdpollEventType type, unsigned long long timeout)
{
    struct SchdfdFd *schdFd;
    int ret;
    TimerHandle timer;
    TimerFunc callback = (type == SHCDPOLL_READ) ? (TimerFunc)SchdfdReadTimeout : (TimerFunc)SchdfdWriteTimeout;

    if (timeout == 0) {
        return ERRNO_SCHDFD_TIMEOUT;
    }

    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    schdFd = SchdfdIncref(fd, false);
    if (schdFd == nullptr) {
        return ERRNO_SCHDFD_FD_CLOSING;
    }

    if (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED) {
        SchdfdDecref(fd, false);
        LOG_ERROR(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd %u not yet add to netpoll", fd);
        return ERRNO_SCHDFD_NOT_ADD_NETPOLL;
    }

    timer = TimerNew(timeout, 0, callback, schdFd);
    if (timer == nullptr) {
        SchdfdDecref(fd, false);
        LOG_ERROR(ERRNO_SCHDFD_INIT_RESOURCE_FAILED, "timer init failed");
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }

    if (!SchdpollWait(schdFd->pd, type)) {
        (void) TimerTryStop(timer);
        SchdfdDecref(fd, false);
        TimerRelease(timer);
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    // Timer stop fail indicates that the current timeout has occurred and returns a timeout error.
    // Otherwise, it is a real ready event that has occurred and returns OK.
    ret = TimerTryStop(timer);
    TimerRelease(timer);
    if (ret != 0) {
        SchdfdDecref(fd, false);
        return ERRNO_SCHDFD_TIMEOUT;
    }

    SchdfdDecref(fd, false);
    return 0;
}

/* SchdfdWaitInlock with timeout. */
int SchdfdWaitInlockTimeout(SignedSocket fd, SchdpollEventType type, unsigned long long timeout)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdFd *schdFd;
    TimerHandle timer;
    TimerFunc callback = (type == SHCDPOLL_READ) ? (TimerFunc)SchdfdReadTimeout : (TimerFunc)SchdfdWriteTimeout;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

    if (timeout == 0) {
        return ERRNO_SCHDFD_TIMEOUT;
    }

    schdFd = reinterpret_cast<struct SchdfdFd *>(schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd);
    if (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED) {
        LOG_ERROR(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd %u not yet add to netpoll", fd);
        return ERRNO_SCHDFD_NOT_ADD_NETPOLL;
    }
    timer = TimerNew(timeout, 0, callback, schdFd);
    if (timer == nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_INIT_RESOURCE_FAILED, "timer init failed");
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }
    if (type == SHCDPOLL_READ) {
        atomic_store(&schdFd->readTimer, reinterpret_cast<uintptr_t>(timer));
    } else {
        atomic_store(&schdFd->writeTimer, reinterpret_cast<uintptr_t>(timer));
    }
    // In the case of close, close error will be returned regardless of whether the timer has timed out or not.
    if (!SchdpollWait(schdFd->pd, type)) {
        (void)FreeSchdfdTimer(schdFd, type);
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    int ret = FreeSchdfdTimer(schdFd, type);
    if (ret == -1) {
        return ERRNO_SCHDFD_TIMEOUT;
    }

    return ret;
}

int SchdfdLock(SignedSocket fd, SchdpollEventType type)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdFd *schdFd;
    struct Sema *sema;
    int ret;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);

    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    // Lock and unlock must be used together, so decref is in unlock to reduce the number of atomic operations.
    schdFd = SchdfdIncref(fd, false);
    if (schdFd == nullptr) {
        return ERRNO_SCHDFD_FD_CLOSING;
    }

    if (type == SHCDPOLL_READ) {
        sema = &schdFd->readSema;
    } else {
        sema = &schdFd->writeSema;
    }

    ret = SemaAcquire(sema, false);
    if (ret != 0) {
        LOG_ERROR(ret, "SemaAcquire failed");
        SchdfdDecref(fd, false);
        return ret;
    }

    // After receiving the semaphore, check if fd is turned off. If it is turned off, return failure.
    if ((schdfdManager->slots[layerIndex][lineIndex][fdIndex].ref & SCHDFD_CLOSING) != 0) {
        SemaRelease(sema);
        SchdfdDecref(fd, false);
        return ERRNO_SCHDFD_FD_CLOSING;
    }
    return 0;
}

int SchdfdUnlock(SignedSocket fd, SchdpollEventType type)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdfdFd *schdFd;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);
    struct Sema *sema;
    int ret;

    ret = SchdfdFdValidCheck(fd);
    if (ret != 0) {
        return ret;
    }

    schdFd = reinterpret_cast<struct SchdfdFd *>(schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd);
    if (schdFd == nullptr) {
        // schdFd cannot be nullptr.
        // If it is nullptr, it means it is not used in conjunction with lock and an error log should be recorded.
        LOG_ERROR(ERRNO_SCHDFD_MISUSED, "fd is free, must used after SchdfdLock");
        return ERRNO_SCHDFD_MISUSED;
    }

    if (type == SHCDPOLL_READ) {
        sema = &schdFd->readSema;
    } else {
        sema = &schdFd->writeSema;
    }

    ret = SemaRelease(sema);
    if (ret != 0) {
        LOG_ERROR(ret, "SemaRelease failed");
    }

    SchdfdDecref(fd, false);
    return ret;
}

int SchdfdRegisterAndNetpollAdd(SignedSocket fd)
{
    int ret = SchdfdRegister(fd);
    if (ret != 0) {
#ifdef MRT_WINDOWS
        (void)closesocket(fd);
#else
        (void)close(fd);
#endif
        LOG_ERROR(ret, "sockFd SchdfdRegister failed!");
        return ret;
    }
    ret = SchdfdNetpollAdd(fd);
    if (ret != 0) {
        SchdfdDeregister(fd);
#ifdef MRT_WINDOWS
        (void)closesocket(fd);
#else
        (void)close(fd);
#endif
        LOG_ERROR(ret, "sockFd SchdfdNetpollAdd failed!");
        return ret;
    }
    return 0;
}

#ifdef MRT_WINDOWS
/* Requesting to cancel IOCP operation will result in the following four situations
 * 1.The request was not submitted in a timely manner, and the IO operation was completed normally.
 *   Considered as a normal IO operation, returns 0
 * 2.Operation successfully cancelled, return -1. If IO is abnormal, update the error code.
 * 3.An unexpected error occurred while canceling the operation, returning -1. Update error code.
 * 4.When entering this function, the IO operation has actually been completed,
 *   but the timer callback and SchdpollAcquire are concurrent, resulting in being judged as timeout.
 * */
static int IocpCancelInlock(struct IocpOperation *operation, int *errorCode)
{
    int ret;
    SchdpollEventType type;
    SignedSocket fd;
    std::atomic<uintptr_t> *waiter;
    uintptr_t state;

    // When the timer callback function is concurrent with SchdpollAcquire,
    // it may cause IO operations to be completed but judged as timeout.
    // If the IO asynchronous operation has been completed normally, calling SchdpollWait later will not wake it up.
    if (atomic_load(&operation->iocpComplete) == true) {
        // Scenario 4: The IO operation has actually been completed,
        // but it was judged as timeout by the timer callback function
        return 0;
    }

    type = operation->type;
    fd = operation->rawFd;

    if (type == SHCDPOLL_READ) {
        waiter = &operation->pd->codethread.readWaiter;
    } else {
        waiter = &operation->pd->codethread.writeWaiter;
    }
    // Before CancelIoEx, set the PD status to PDD_NOWAIT to ensure that
    // calling SchdpollWait below will result in a normal park.
    state = PD_CLOSING;
    atomic_compare_exchange_strong(waiter, &state, PD_NOWAIT);

    // Cancel the asynchronous operation previously submitted.
    ret = CancelIoEx((HANDLE)fd, &operation->overlapped);
    if (ret == 0) { // Scenario 1 or Scenario 3
        ret = (int)GetLastError();
        if (ret != ERROR_NOT_FOUND) { // Scenario 3
            *errorCode = ret;
            LOG_ERROR(ret, "CancelIoEx failed, fd: %llu", fd);
            return -1;
        }
    }
    // Waiting for the cancellation operation to complete,
    // it will be awakened by SchdpollAcquire in situations 1 and 2.
    SchdpollWait(operation->pd, type);

    if (operation->error != 0) { // Scenario 2
        ret = operation->error;
        if (ret != WSA_OPERATION_ABORTED) {
            *errorCode = ret;
            LOG_ERROR(ret, "WSAGetOverlappedResult failed, fd: %llu", fd);
        }
        return -1;
    }

    return 0; // Scenario 1
}

/* Waiting for asynchronous IO operation to complete. Success returns 0, failure returns error code. */
int SchdfdIocpWaitInlock(struct IocpOperation *operation, unsigned long long timeout)
{
    int ret;
    int cancelRet;
    SchdpollEventType type;
    SignedSocket fd;
    type = operation->type;
    fd = operation->rawFd;

    if (timeout == (unsigned long long)-1) {
        ret = SchdfdWaitInlock(fd, type);
    } else {
        ret = SchdfdWaitInlockTimeout(fd, type, timeout);
    }

    if (ret == 0) {
        if (operation->error != 0) {
            ret = operation->error;
            LOG_ERROR(ret, "WSAGetOverlappedResult failed, fd: %llu", fd);
            return ret;
        }
        return 0;
    }

    cancelRet = IocpCancelInlock(operation, &ret);
    if (cancelRet == -1) {
        return ret;
    }

    return 0;
}

struct IocpOperation *SchdfdUpdateIocpOperationInlock(SignedSocket fd, SchdpollEventType type,
                                                      const void *buf, unsigned int len)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct IocpOperation *operation = nullptr;
    struct SchdfdFd *schdFd;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);
    std::atomic<uintptr_t> *waiter;

    schdFd = (struct SchdfdFd *)schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd;

    if (atomic_load(&schdFd->netpollState) != NETPOLL_ADDED) {
        LOG_ERROR(ERRNO_SCHDFD_NOT_ADD_NETPOLL, "fd %u not yet add to netpoll", fd);
        return nullptr;
    }

    if (type == SHCDPOLL_READ) {
        operation = &schdFd->readOperation;
        waiter = &schdFd->pd->codethread.readWaiter;
    } else if (type == SHCDPOLL_WRITE) {
        operation = &schdFd->writeOperation;
        waiter = &schdFd->pd->codethread.writeWaiter;
    } else {
        return nullptr;
    }

    atomic_store(waiter, PD_NOWAIT);
    atomic_store(&operation->iocpComplete, false);
    operation->iocpBuf.buf = (char *)buf;
    operation->iocpBuf.len = static_cast<unsigned long>(len);
    operation->error = 0;
    memset_s(&operation->overlapped, sizeof(OVERLAPPED), 0, sizeof(OVERLAPPED));

    return operation;
}

void SchdfdCheckIocpCompleteSkip()
{
    int infoNum;
    int protocolIdx;
    WSAPROTOCOL_INFOA *protocolInfo;
    int protocol_buf_size = sizeof(WSAPROTOCOL_INFOA) * PROTOCOL_BUF_SIZE;

    protocolInfo = (WSAPROTOCOL_INFOA *)malloc(protocol_buf_size);
    if (protocolInfo == nullptr) {
        LOG_ERROR(errno, "malloc failed, size: %u", protocol_buf_size);
        return;
    }

    // Check if the current system can run the SetFileCompletionElementModes function normally.
    infoNum = WSAEnumProtocols(nullptr, protocolInfo, (DWORD *)&protocol_buf_size);
    if (infoNum == SOCKET_ERROR) {
        free(protocolInfo);
        return;
    }

    for (protocolIdx = 0; protocolIdx < infoNum; ++protocolIdx) {
        if ((protocolInfo[protocolIdx].dwServiceFlags1 & XP1_IFS_HANDLES) == 0) {
            free(protocolInfo);
            return;
        }
    }
    free(protocolInfo);
    g_iocpCompleteSkipFlag = true;
}

bool SchdfdUseSkipIocp()
{
    return g_iocpCompleteSkipFlag;
}

#else

/* Put fd pd key value pairs. */
int SchdfdPdPut(int fd, struct SchdpollDesc *pd)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);
    struct SchdfdFd *schdFd;
    if (SchdfdInit(layerIndex, lineIndex) != 0) {
        LOG_ERROR(ERRNO_SCHDFD_INIT_RESOURCE_FAILED, "SchdfdInit failed.fd : %d", fd);
        return ERRNO_SCHDFD_INIT_RESOURCE_FAILED;
    }

    if (schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd != nullptr) {
        LOG_ERROR(ERRNO_SCHDFD_FD_REGISTED, "fd %d registered", fd);
        return ERRNO_SCHDFD_FD_REGISTED;
    }

    schdFd = (struct SchdfdFd *)malloc(sizeof(struct SchdfdFd));
    if (schdFd == nullptr) {
        LOG_ERROR(-1, "malloc failed");
        return -1;
    }
    schdFd->pd = pd;
    schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd = schdFd;
    return 0;
}

/* Remove fd pd key value pairs. */
struct SchdpollDesc *SchdfdPdRemove(int fd)
{
    struct SchdfdManager *schdfdManager = g_scheduleManager.schdfdManager;
    struct SchdpollDesc *pd;
    struct SchdfdFd *schdFd;
    int layerIndex = GetFirstLevel(fd);
    int lineIndex = GetSecondLevel(fd);
    int fdIndex = GetThirdLevel(fd);
    if (SchdfdInit(layerIndex, lineIndex) != 0) {
        return nullptr;
    }
    schdFd = schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd;
    if (schdFd == nullptr) {
        return nullptr;
    }
    pd = schdFd->pd;
    free(schdFd);
    schdfdManager->slots[layerIndex][lineIndex][fdIndex].schdFd = nullptr;
    return pd;
}

#endif

int FreeSchdfdTimer(struct SchdfdFd *schdFd, SchdpollEventType type)
{
    int ret = 0;
    uintptr_t old = 0;
    std::atomic<uintptr_t> *timer;
    uintptr_t newState = SCHDFD_TIMER_NULL;
    if (type == SHCDPOLL_READ) {
        timer = &schdFd->readTimer;
    } else {
        timer = &schdFd->writeTimer;
    }
 
    while (1) {
        old = atomic_load(timer);
        if (old == newState) {
            return -1;
        }
 
        if (atomic_compare_exchange_weak(timer, &old, newState)) {
            ret = TimerTryStop(reinterpret_cast<void *>(old));
            TimerRelease(reinterpret_cast<void *>(old));
            if (ret != 0) {
                ret = ERRNO_SCHDFD_TIMEOUT;
            }
            return ret;
        }
    }
}

#ifdef __cplusplus
}
#endif
