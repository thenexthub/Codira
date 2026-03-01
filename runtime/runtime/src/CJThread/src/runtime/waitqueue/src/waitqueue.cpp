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


#include <pthread.h>
#include <atomic>
#include <climits>
#include "waitqueue_impl.h"
#include "timer_impl.h"
#include "log.h"
#include "codethread.h"

#ifdef __cplusplus
extern "C" {
#endif

int WaitqueueNew(struct Waitqueue *wq)
{
    struct WaitqueueInner *queue = reinterpret_cast<struct WaitqueueInner *>(wq);
    int error;

    if (queue == nullptr) {
        return ERRNO_SCHD_ARG_INVALID;
    }
    LinkInit(&queue->head);
    queue->tail = nullptr;
    error = pthread_mutex_init(&queue->mutex, nullptr);
    if (error) {
        LOG_ERROR(error, "mutex init failed");
        return error;
    }

    queue->nwait = 0;
    queue->timerNwait = 0;
    return 0;
}

int WaitqueueDelete(struct Waitqueue *wq)
{
    struct WaitqueueInner *queue = reinterpret_cast<struct WaitqueueInner *>(wq);
    if (queue == nullptr) {
        return ERRNO_QUEUE_DOESNT_EXIST;
    }

    pthread_mutex_destroy(&queue->mutex);
    return 0;
}

int WaitqueueParkUnlock(void *arg, CODEThreadHandle handle)
{
    (void)handle;
    struct WaitqueueInner *queue;
    int error;

    queue = (struct WaitqueueInner *)arg;
    error = pthread_mutex_unlock(&queue->mutex);
    if (error) {
        return error;
    }
    return 0;
}

/*
 * The waitqueue transfers the callback function to the timer. It fetches a codethread from the queue
 * and puts it into the running queue.
 */
void WaitqueueCallback(void *arg)
{
    int error;
    struct WaitqueueNode *node = (struct WaitqueueNode *)arg;
    struct WaitqueueInner *queue = node->queue;

    error = pthread_mutex_lock(&queue->mutex);
    if (error) {
        LOG_ERROR(error, "lock wq lock after wq released");
    }

    if (node->listNode.next == nullptr) {
        // When the linked list is empty after the node is taken out,
        // the tail node is left empty. Otherwise, point to the correct tail node.
        if (queue->nwait == 1) {
            queue->tail = nullptr;
        } else {
            queue->tail = node->listNode.prev;
        }
    }
    LinkRemove(&node->listNode);
    // The codethread that invokes this function must be set with the timer.
    // If there is no timer or codethread, an error is reported.
    if (node->timer == nullptr) {
        LOG_ERROR(ERRNO_TIMER_STATE_INVALID, "timer's ptr is wrong");
        free(node);
        pthread_mutex_unlock(&queue->mutex);
        return;
    }
    // This indicates that the node is not fetched in advance and there is a timer.
    // The waiting queue length is -1 and the codethread in the node is woken up.
    queue->nwait--;
    TimerRelease(node->timer);
    node->timer = nullptr;
    *node->timeout = 1;
    CODEThreadReady((CODEThreadHandle)node->codethread);
    free(node);
    error = pthread_mutex_unlock(&queue->mutex);
    if (error) {
        LOG_ERROR(error, "unlock wq lock after wq released");
    }
    atomic_fetch_sub(&queue->timerNwait, 1u);
    return;
}

int WaitqueuePark(struct Waitqueue *wq, long long ns,
                  WqCallbackFunc callbackFunc, void *arg, bool head)
{
    struct WaitqueueNode *node = nullptr;
    struct WaitqueueInner *queue = reinterpret_cast<struct WaitqueueInner *>(wq);
    int ret;
    // Indicates whether the system is woken up due to timeout.
    int isTimeout = 0;

    if (queue == nullptr) {
        return ERRNO_QUEUE_DOESNT_EXIST;
    }
    if (ns <= 0) {
        return ERRNO_WQ_DURATION_INVALID;
    }
    pthread_mutex_lock(&queue->mutex);
    ret = WaitqueuePush(queue, callbackFunc, arg, &node, head);
    if (ret != 0) {
        pthread_mutex_unlock(&queue->mutex);
        return ret;
    }
    // ns is LLONG_MAX, indicating that there is no timeout period.
    // The current codethread waits until it is woken up by another codethread.
    if (ns == static_cast<long long>(LLONG_MAX)) {
        CODEThreadPark(WaitqueueParkUnlock, TRACE_EV_CODETHREAD_BLOCK_SYNC, queue);
    } else {
        // There is a timeout period, and a timer is set.
        atomic_fetch_add(&queue->timerNwait, 1u);
        node->timeout = &isTimeout;
        node->timer = TimerNew(ns, 0, (TimerFunc) WaitqueueCallback, node);
        CODEThreadPark(WaitqueueParkUnlock, TRACE_EV_CODETHREAD_BLOCK_SYNC, queue);
        if (isTimeout) {
            return WAIT_QUEUE_TIMEOUT;
        }
    }

    return 0;
}

int WaitqueueWakeOne(struct Waitqueue *wqHandle, WqCallbackFunc callbackFunc, void *arg)
{
    struct Link *listNode;
    struct WaitqueueNode *wqNode;
    struct WaitqueueInner *queue = reinterpret_cast<struct WaitqueueInner *>(wqHandle);
    int ret;

    if (queue == nullptr) {
        return ERRNO_QUEUE_DOESNT_EXIST;
    }
    pthread_mutex_lock(&queue->mutex);
    ret = WaitqueueWakePreparation(queue, callbackFunc, arg);
    if (ret != 0) {
        pthread_mutex_unlock(&queue->mutex);
        return ret;
    }

    // Obtains the head node of the linked list.
    listNode = (&queue->head)->next;
    wqNode = LINK_ENTRY(listNode, struct WaitqueueNode, listNode);

    // If the head node has a timer, try to disable the timer. If it fails, try to wake up the next node.
    while (wqNode->timer != nullptr && TimerTryStop(wqNode->timer) != 0) {
        if (listNode->next == nullptr) {
            pthread_mutex_unlock(&queue->mutex);
            return ERRNO_QUEUE_IS_EMPTY;
        }
        listNode = listNode->next;
        wqNode = LINK_ENTRY(listNode, struct WaitqueueNode, listNode);
    }
    if (listNode->next == nullptr) {
        if (queue->nwait == 1) {
            queue->tail = nullptr;
        } else {
            queue->tail = listNode->prev;
        }
    }
    LinkRemove(listNode);
    LinkInit(listNode);
    queue->nwait--;
    // Wake up the codethread. If the timer is not set for the node, release the node.
    CODEThreadReady(wqNode->codethread);
    pthread_mutex_unlock(&queue->mutex);

    if (wqNode->timer != nullptr) {
        TimerRelease(wqNode->timer);
        atomic_fetch_sub(&queue->timerNwait, 1u);
    }
    free(wqNode);
    return 0;
}

int WaitqueueWakeAll(struct Waitqueue *wqHandle, WqCallbackFunc callbackFunc,
                     void *arg)
{
    struct Link *listNode;
    struct WaitqueueNode *wqNode;
    int ret;
    struct WaitqueueInner *queue = reinterpret_cast<struct WaitqueueInner *>(wqHandle);
    unsigned int len;

    if (queue == nullptr) {
        return ERRNO_QUEUE_DOESNT_EXIST;
    }
    pthread_mutex_lock(&queue->mutex);
    ret = WaitqueueWakePreparation(queue, callbackFunc, arg);
    if (ret != 0) {
        pthread_mutex_unlock(&queue->mutex);
        return ret;
    }
    len = queue->nwait;
    while (len > 0) {
        // Obtain the head node. If the head node has a timer, disable the timer.
        listNode = (&queue->head)->next;
        wqNode = LINK_ENTRY(listNode, struct WaitqueueNode, listNode);
        while (wqNode->timer != 0 && TimerTryStop(wqNode->timer) == ERROR_TIMER_IS_NOT_WAITING) {
            len--;
            // There are no more nodes to wake up. Exit directly.
            if (len == 0) {
                pthread_mutex_unlock(&queue->mutex);
                return 0;
            }
            listNode = listNode->next;
            wqNode = LINK_ENTRY(listNode, struct WaitqueueNode, listNode);
        }
        len--;
        if (listNode->next == nullptr) {
            if (queue->nwait == 1) {
                queue->tail = nullptr;
            } else {
                queue->tail = listNode->prev;
            }
        }
        LinkRemove(listNode);
        LinkInit(listNode);
        wqNode = LINK_ENTRY(listNode, struct WaitqueueNode, listNode);
        // Wake up the codethread. If the timer is not set for the node, release the node.
        CODEThreadReady(wqNode->codethread);
        if (wqNode->timer != nullptr) {
            TimerRelease(wqNode->timer);
            atomic_fetch_sub(&queue->timerNwait, 1u);
        }
        queue->nwait--;
        free(wqNode);
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

struct WaitqueueNode *WaitqueueNodeNew(struct CODEThread *codethread, struct WaitqueueInner *queue)
{
    struct WaitqueueNode *node = (struct WaitqueueNode *)malloc(sizeof(struct WaitqueueNode));

    if (node == nullptr) {
        return nullptr;
    }
    node->codethread = codethread;
    node->timer = nullptr;
    node->queue = queue;
    node->timeout = nullptr;
    return node;
}

int WaitqueuePush(struct WaitqueueInner *queue, WqCallbackFunc callbackFunc,
                  void *arg, struct WaitqueueNode **sp, bool toHead)
{
    struct CODEThread *codethread;
    int ret;

    codethread = CODEThreadGet();
    if (queue->nwait == UINT32_MAX) {
        return ERRNO_WQ_MAX_NUM;
    }
    queue->nwait++;
    ret = WaitqueueCallbackCheck(callbackFunc, arg);
    if (ret != 0) {
        queue->nwait--;
        return ret;
    }
    // Adds the current codethread to the waiting queue.
    // When the time is up, it will be woken up. Or it's possible to be awakened early.
    *sp = WaitqueueNodeNew(codethread, queue);
    if (*sp == nullptr) {
        queue->nwait--;
        return ERRNO_MALLOC_FAILED;
    }
    if (queue->nwait == 1) {
        // Before insertion, the linked list is empty.
        // In this case, tail is NULL, and header insertion is used.
        LinkPushHead(&queue->head, &((*sp)->listNode));
        queue->tail = &((*sp)->listNode);
    } else if (toHead) {
        LinkPushHead(&queue->head, &((*sp)->listNode));
    } else {
        LinkPushTail(queue->tail, &((*sp)->listNode));
        queue->tail = &((*sp)->listNode);
    }

    return 0;
}

int WaitqueueCallbackCheck(WqCallbackFunc callbackFunc, void *arg)
{
    bool callbackResult = false;

    if (callbackFunc != nullptr) {
        callbackResult = callbackFunc(arg);
        if (callbackResult) {
            return ERRNO_CALLBACK_RETURN_TRUE;
        }
    }
    return 0;
}

int WaitqueueWakePreparation(const struct WaitqueueInner *queue, WqCallbackFunc callbackFunc,
                             void *arg)
{
    if (WaitqueueCallbackCheck(callbackFunc, arg) != 0) {
        return ERRNO_CALLBACK_RETURN_TRUE;
    }
    if (queue->nwait == 0) {
        return ERRNO_QUEUE_IS_EMPTY;
    }
    return 0;
}

unsigned int WaitqueueGetWaitNum(const struct Waitqueue *wq)
{
    const struct WaitqueueInner *queue = reinterpret_cast<const struct WaitqueueInner *>(wq);

    if (queue == nullptr) {
        return 0;
    }
    return atomic_load(&queue->nwait);
}

#ifdef __cplusplus
}
#endif
