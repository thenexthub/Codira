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


#ifndef MRT_EXTERNAL_H
#define MRT_EXTERNAL_H

#include <atomic>
#include "pthread.h"
#ifdef MRT_WINDOWS
#include <winsock2.h>
#include <windows.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// The number of layers of 3D array slots is 256.
#define SCHDFD_SLOTS_MAX_LAYER 0x100

// The maximum number of fd supported in each row in the 3D array slot is 4096.
#define SCHDFD_SLOTS_LINE_MAX_FD_NUM 0x1000

// The number of rows for 3D array slots is 4096.
#define SCHDFD_SLOTS_LAYER_MAX_LINE_NUM 0x1000

#define WAITQUEUE_MAX_SIZE 4

#define SEMA_MAX_SIZE 2

/**
* @brief waitqueue external structure
* @attention The mutex is exposed to prevent the mutex size from being
* different in different operating systems.
*/
struct Waitqueue {
    pthread_mutex_t mutex;
    long long int data[WAITQUEUE_MAX_SIZE];
};

/**
 * @brief sema external structure
 * Defines the value in the semaphore and a waiting queue.
 */
struct Sema {
    struct Waitqueue queue;
    long long int data[SEMA_MAX_SIZE];
};

/**
 * @brief Netpoll state
 */
enum NetpollState {
    NETPOLL_TO_ADD = 0,     // To add netpoll listening events.
    NETPOLL_ADDING,         // Adding netpoll listening events.
    NETPOLL_ADDED,          // Added netpoll listening events.
    NETPOLL_CLOSING,        // fd is closing.
};

/**
 * @brief Schdpoll event type
 */
enum SchdpollEventType {
    SHCDPOLL_READ = 0,      // Readable events, including disconnect and error events
    SHCDPOLL_WRITE = 1      // Writable events, including disconnect and error events
};

struct CODEThreadDesc {
    std::atomic<uintptr_t> readWaiter;    // Read object waiting on the PD
    std::atomic<uintptr_t> writeWaiter;   // Write object waiting on the PD
};

#ifdef MRT_WINDOWS
struct SchdpollDesc {
    struct CODEThreadDesc codethread;  // codethread descriptor
};

struct IocpOperation {
    OVERLAPPED overlapped;          /* IOCP overlapped struct. It must be placed in the
                                       first field of this structure. The field pointer
                                       is converted into a structure pointer in schdpoll. */

    struct SchdpollDesc *pd;        // Schdpoll descriptor
    enum SchdpollEventType type;    // Schdpoll event tyep
    WSABUF iocpBuf;                 // Buffer for read or wirte
    DWORD transBytes;               // Number of bytes passed when a read/write completes normally

    SOCKET rawFd;                   // Windows raw socket
    int error;                      // Error code when the IOCP is complete
    std::atomic<bool> iocpComplete;       // Indicates whether the IOCP is completed normally
};
#endif

/**
 * @brief fd management structure
 */
struct SchdfdFd {
    std::atomic<NetpollState> netpollState;      // netpoll state
    struct Sema readSema;         // Read semaphore, used to control the sequence of read operations on fd
    struct Sema writeSema;        // Write semaphore, used to control the sequence of write operations on fd
    struct SchdpollDesc *pd;      // schdpoll descriptor
    std::atomic<uintptr_t> readTimer;
    std::atomic<uintptr_t> writeTimer;
#ifdef MRT_WINDOWS
    struct IocpOperation readOperation;  // IOCP read operation structure
    struct IocpOperation writeOperation; // IOCP write operation structure
#endif
};

/**
 * @brief fd managent structure slot
 * Stores the SchdfdFd and manages the life cycle through the reference counting.
 * Performance optimization: The size of the slot structure is added to 64 bytes
 * to avoid pseudo-sharing and improve concurrency performance.
 */
struct SchdfdSlot {
    std::atomic<unsigned long long> ref;        // reference counting of SchdFd
    struct SchdfdFd *schdFd;
};

/**
 * @brief fd global management array
 */
struct SchdfdManager {
    pthread_mutex_t initLock;                            // slots init lock
    struct SchdfdSlot **slots[SCHDFD_SLOTS_MAX_LAYER];   /* Global three-dimensional array of fd slots.
                                                            The maximum number of three-dimensional
                                                            arrays is 256 x 4096 x 4096. */
};

struct SchdfdManager* SchdfdManagerInit();
void FreeSchdfdManager(struct SchdfdManager *schdfdManager);
void FreeSchdfdFd(struct SchdfdFd *schdfdFd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
