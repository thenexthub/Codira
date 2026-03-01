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


#ifndef MRT_QUEUE_H
#define MRT_QUEUE_H

#include <atomic>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"
#include "mid.h"
#include "macro_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief 0x10020001
 * The queue is full
 */
#define ERROR_QUEUE_FULL ((MID_QUEUE) | 0x01)

/**
 * @brief 0x10020002
 * The queue is empty
 */
#define ERROR_QUEUE_EMPTY ((MID_QUEUE) | 0x02)

/**
 * @brief 0x10020003
 * During initialization, the queue buffer fails to apply for memory.
 */
#define ERROR_QUEUE_BUF_ALLOC_FAILED ((MID_QUEUE) | 0x03)

/**
 * @brief 0x10020004
 * The input parameter transferred by the function is invalid.
 */
#define ERROR_QUEUE_ARG_INVALID ((MID_QUEUE) | 0x04)

/**
 * @brief 0x10020005
 * The queue->buf is empty, and the queue is not initialized.
 */
#define ERROR_QUEUE_BUF_UNINITIALIZED ((MID_QUEUE) | 0x05)


/**
 * @ingroup queue
 * @struct Queue
 * @brief queue structure
 * Defines the head and tail of a queue, capacity, and queue pointer.
 */
struct Queue {
    std::atomic<unsigned int> head __attribute__((aligned(64)));
    std::atomic<unsigned int> tail __attribute__((aligned(64)));
    unsigned int capacity;
    void **buf;
};

/**
 * @ingroup queue
 * @brief Get queue length
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * @param  queue   [IN]  queue
 * @retval the length of queue
 */
MRT_INLINE static unsigned int QueueLength(struct Queue *queue)
{
    unsigned int head;
    unsigned int tail;

    head = atomic_load(&queue->head);
    tail = atomic_load(&queue->tail);
    return (tail - head);
}

/**
 * @ingroup queue
 * @brief Get queue capacity
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * @param  queue   [IN]  queue
 * @retval the capacity of queue
 */
MRT_INLINE static unsigned int QueueCapacity(struct Queue *queue)
{
    return queue->capacity;
}

/**
 * @ingroup queue
 * @brief Obtains the element from the head of the queue.
 * @par Obtains the header element of the queue and returns a pointer to this element.
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * @param  queue   [IN]  queue
 * @retval Pointer to the header element. If the queue is empty, NULL is returned.
 */
MRT_INLINE static void *QueuePopHead(struct Queue *queue)
{
    unsigned int head;
    unsigned int tail;
    void *res;

    while (1) {
        head = atomic_load(&queue->head);
        tail = atomic_load(&queue->tail);
        if (tail == head) {
            return nullptr;
        }
        res = queue->buf[head % queue->capacity];
        if (atomic_compare_exchange_weak(&queue->head, &head, head + 1)) {
            return res;
        }
        // If the operation fails, contention exists. Wait for a while and perform the operation again.
        usleep(1);
    }
}

/**
 * @ingroup queue
 * @brief Obtains elements from the queue header in batches.
 * @par Obtains queue header elements in batches and returns the number of elements to be obtained.
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * @param  queue   [IN] queue
 * @param  buf   [IN]  buffer
 * @param  bufLen   [IN]  length of buffer
 * @retval Number of elements to be fetched. The maximum value is buf_len.
 */
MRT_INLINE static unsigned int QueuePopHeadBatch(struct Queue *queue, void *buf[], unsigned int bufLen)
{
    unsigned int head;
    unsigned int tail;
    unsigned int popLen;
    unsigned int i;

    if (bufLen == 0) {
        return 0;
    }

    while (1) {
        head = atomic_load(&queue->head);
        tail = atomic_load(&queue->tail);
        if (head == tail) {
            return 0;
        }
        popLen = ((tail - head) > bufLen) ? bufLen : (tail - head);
        for (i = 0; i < popLen; i++) {
            buf[i] = queue->buf[(head + i) % queue->capacity];
        }
        if (atomic_compare_exchange_weak(&queue->head, &head, head + popLen)) {
            return popLen;
        }
        // If the operation fails, contention exists. Wait for a while and perform the operation again.
        usleep(1);
    }
}

/**
 * @ingroup queue
 * @brief Pushes the element to the end of the queue.
 * @par Pushes the element to the end of the queue.
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * Only single-thread insertion is supported.
 * @param  queue   [IN]  queue
 * @param  object   [IN]  object
 * @retval error code
 */
MRT_INLINE static int QueuePushTail(struct Queue *queue, void *object)
{
    unsigned int head;
    unsigned int tail;

    head = atomic_load(&queue->head);
    tail = atomic_load(&queue->tail);
    if ((tail - head) < queue->capacity) {
        queue->buf[tail % queue->capacity] = object;
        atomic_store(&queue->tail, tail + 1);
        return 0;
    }
    return ERROR_QUEUE_FULL;
}

/**
 * @ingroup queue
 * @brief Pushes some elements to the end of the queue.
 * @par Pushes some elements to the end of the queue.
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * Only single-thread insertion is supported.
 * @param  queue   [IN]  queue
 * @param  buf   [IN]  buffer
 * @param  bufLen   [IN]  length of buffer
 * @retval Number of elements that are pushed to the tail of queue. The maximum value is buf_len.
 */
MRT_INLINE static unsigned int QueuePushTailBatch(struct Queue *queue, void *buf[], unsigned int bufLen)
{
    unsigned int head;
    unsigned int tail;
    unsigned int i;

    if (bufLen == 0) {
        return 0;
    }

    head = atomic_load(&queue->head);
    tail = atomic_load(&queue->tail);
    i = 0;
    while ((tail - head) < queue->capacity) {
        queue->buf[tail % queue->capacity] = buf[i];
        tail++;
        i++;
        if (i == bufLen) {
            break;
        }
    }
    atomic_store(&queue->tail, tail);
    return i;
}

/**
 * @ingroup queue
 * @brief Destroy queue
 * @par This API is used to release queue resources. Destroyed queue resources cannot be accessed.
 * Only arrays in the structure are released. The queue structure needs to be released by users.
 * @attention The input parameter queue cannot be null, and the queue must have been initialized.
 * @param  queue   [IN]  queue
 */
MRT_INLINE static void QueueDestroy(struct Queue *queue)
{
    if (queue->buf != nullptr) {
        free(queue->buf);
        queue->buf = nullptr;
    }
    queue->capacity = 0;
    atomic_store(&queue->head, 0u);
    atomic_store(&queue->tail, 0u);
}

/**
 * @ingroup queue
 * @brief Init queue
 * @par Init queue
 * @attention Queue cannot be null and capacity cannot be 0.
 * @param  queue   [IN]  queue
 * @param  capacity   [IN]  capacity
 * @retval error code
 */
MRT_INLINE static int QueueInit(struct Queue *queue, unsigned int capacity)
{
    if (capacity == 0) {
        return ERROR_QUEUE_ARG_INVALID;
    }
    queue->buf = (void **)malloc(sizeof(void *) * (capacity));
    if (queue->buf == nullptr) {
        LOG_ERROR(errno, "malloc failed, size: %u", (sizeof(void *) * capacity));
        return ERROR_QUEUE_BUF_ALLOC_FAILED;
    }
    queue->capacity = capacity;
    atomic_store(&queue->head, 0u);
    atomic_store(&queue->tail, 0u);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* _QUEUE_H */

