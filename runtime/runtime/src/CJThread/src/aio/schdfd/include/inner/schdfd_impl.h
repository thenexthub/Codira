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


#ifndef MRT_SCHDFD_IMPL_H
#define MRT_SCHDFD_IMPL_H

#include "schdpoll.h"
#include "sema.h"
#include "mid.h"
#include "macro_def.h"
#include "external.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief 0x10100000 Failed to initialize resources of the schdfd modul.
 */
#define ERRNO_SCHDFD_INIT_RESOURCE_FAILED ((MID_SCHDFD) | 0x0000)

/**
 * @brief 0x10100001 The size of fd registered by schdfd module exceeds the max value.
 */
#define ERRNO_SCHDFD_FD_OVERLIMIT ((MID_SCHDFD) | 0x0001)

/**
 * @brief 0x10100002 The fd has been registered and it cannot be registered repeatedly.
 */
#define ERRNO_SCHDFD_FD_REGISTED ((MID_SCHDFD) | 0x0002)

/**
 * @brief 0x10100003 Failed to add fd to schdpoll monitoring.
 */
#define ERRNO_SCHDFD_SCHDPOLL_ADD_FAILED ((MID_SCHDFD) | 0x0003)

/**
 * @brief 0x10100004 The current fd is being closed or has been closed.
 */
#define ERRNO_SCHDFD_FD_CLOSING ((MID_SCHDFD) | 0x0004)

/**
 * @brief 0x10100005 The interface is incorrectly used and need to locate the cause based on
 * the log information.
 */
#define ERRNO_SCHDFD_MISUSED ((MID_SCHDFD) | 0x0005)

/**
 * @brief 0x10100006 fd operation timed out.
 */
#define ERRNO_SCHDFD_TIMEOUT ((MID_SCHDFD) | 0x0006)

/**
 * @brief 0x10100003 Failed to add fd to Netpoll monitoring.
 */
#define ERRNO_SCHDFD_NOT_ADD_NETPOLL ((MID_SCHDFD) | 0x0007)

/**
 * @brief 0 Set null timer to schdfd.
 */
#define SCHDFD_TIMER_NULL 0

/**
 * @brief 0xffffffff The maximum number of fd that
 * can be managed by the scheduling I/O is UINT_MAX.
 */
#define SCHDFD_MAX_FD_NUM 0xffffffff

/**
 * @brief 1 << 63 fd closed flag.
 */
#define SCHDFD_CLOSING ((unsigned long long int)1 << 63)

#define SECOND_LEVEL_SHIFT          12
#define FIRST_LEVEL_SHIFT           24
#define FIRST_LEVEL_MASK            0xff
#define SCOND_LEVEL_MASK            0x00fff
#define THIRD_LEVEL_MASK            0x00000fff

#ifdef MRT_WINDOWS
typedef SOCKET SignedSocket;
typedef SOCKET UnsignedSocket;
typedef DWORD SocketFlag;
#else
typedef int SignedSocket;
typedef unsigned int UnsignedSocket;
typedef int SocketFlag;
#endif

// Obtains the primary index of the current fd.
MRT_INLINE static int GetFirstLevel(SignedSocket fd)
{
    return (int)(((UnsignedSocket)fd >> FIRST_LEVEL_SHIFT) & FIRST_LEVEL_MASK);
}

// Obtains the secondary index of the current fd.
MRT_INLINE static int GetSecondLevel(SignedSocket fd)
{
    return (int)(((UnsignedSocket)fd >> SECOND_LEVEL_SHIFT) & SCOND_LEVEL_MASK);
}

// Obtains the level-3 index of the current fd.
MRT_INLINE static int GetThirdLevel(SignedSocket fd)
{
    return (int)((UnsignedSocket)fd & THIRD_LEVEL_MASK);
}

/**
 * @brief Register fd with schdFd management module.
 * @par Description: Register system fd with schdFd management module. For a fd, other APIs of
 * this module can be used only after this API is successfully invoked.
 * @param  fd           [IN]  fd
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdRegister(SignedSocket fd);

/**
 * @brief Add an epoll listening event to the fd registered.
 * @par Description: Add an epoll listening event for the fd registered in schdfd.
 * @attention This function should be called when fd can generate an event. Otherwise, the EPOLLHUP
 * event that may be generated needs to be processed. Such as tcp, the following types of fd
 * can be invoked: fd for listen, fd for connect, and fd for accept.
 * @param  fd           [IN]  fd
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdNetpollAdd(SignedSocket fd);

/**
 * @brief Deregister fd with schdFd management module.
 * @par
 * Description: Deregisters the registered fd from schdFd management module.
 * Before disabling a registered fd, call this API to deregister it.
 * Otherwise, resource leakage may occur.
 * @param  fd           [IN]  Registered fd
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdDeregister(SignedSocket fd);

/**
 * @brief Wakes up blocked fd.
 * @param  fd           [IN]  Registered fd
 */
void WakeallFd(SignedSocket fd);

/**
 * @brief The fd waits for the corresponding event.
 * @par Description: fd waits for the corresponding event.
 * When waiting, codethread park is released. When the event arrives, it is woken up.
 * @attention The same fd and type do not support concurrent invoking.
 * When concurrent calls are required, need to use SchdfdLock to lock and use SchdfdWaitInlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  The types of events that need to be waited for, such as SchdpollEventType
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 * Possible parameter errors or fd being turned off.
 */
int SchdfdWait(SignedSocket fd, SchdpollEventType type);

/**
 * @brief The fd waits for the corresponding event with timeout.
 * @par Description: fd waits for the corresponding event with timeout.
 * When waiting, codethread park is released. When the event arrives, it is woken up.
 * @attention The same fd and type do not support concurrent invoking.
 * When concurrent calls are required, need to use SchdfdLock to lock and use SchdfdWaitInlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  The types of events that need to be waited for, such as SchdpollEventType
 * @param  timeout      [IN]  Timeout, ns
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 * Possible parameter errors or fd being turned off.
 */
int SchdfdWaitTimeout(SignedSocket fd, SchdpollEventType type, unsigned long long timeout);

/**
 * @brief The fd waits for the corresponding event.
 * @par Description: fd waits for the corresponding event in lock.
 * When waiting, codethread park is released. When the event arrives, it is woken up.
 * @attention This interface must be used between SchdfdLock and SchdfdUnlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  The types of events that need to be waited for, such as SchdpollEventType
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 * Possible fd being turned off.
 */
int SchdfdWaitInlock(SignedSocket fd, SchdpollEventType type);

/**
 * @brief The fd waits for the corresponding event with timeout.
 * @par Description: fd waits for the corresponding event with timeout in lock.
 * When waiting, codethread park is released. When the event arrives, it is woken up.
 * @attention This interface must be used between SchdfdLock and SchdfdUnlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  The types of events that need to be waited for, such as SchdpollEventType
 * @param  timeout      [IN]  Timeout, ns
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 * Possible fd being turned off.
 */
int SchdfdWaitInlockTimeout(SignedSocket fd, SchdpollEventType type, unsigned long long timeout);

/**
 * @brief fd sequential operation with lock.
 * @par Description: Ensure the sequential execution of corresponding type operations on fd through
 * locking, and unlock through SchdfdUnlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  Type of the sequential operation to be controlled.
 * SHCDPOLL_READ indicates the read operation, and SHCDPOLL_WRITE indicates the write operation.
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 * It is normal for this interface to fail when fd is closed.
 */
int SchdfdLock(SignedSocket fd, SchdpollEventType type);

/**
 * @brief fd sequential operation with unlock.
 * @par Description: After calling SchdfdLock to lock, call this interface to unlock.
 * This interface can only be called after SchdfdLock returns success.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  Type of the sequential operation to be controlled.
 * SHCDPOLL_READ indicates the read operation, and SHCDPOLL_WRITE indicates the write operation.
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdUnlock(SignedSocket fd, SchdpollEventType type);

/**
 * @brief Register fd in schdFd management module and add epoll listening events for fd.
 * @param  fd           [IN]  fd
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdRegisterAndNetpollAdd(SignedSocket fd);

#ifdef MRT_WINDOWS
/**
 * @brief Waiting for IOCP asynchronous operation to complete.
 * @par Description: When an asynchronous operation is submitted, call this function
 * to wait for the asynchronous operation to complete or the asynchronous operation fails.
 * IOCP operation must be successfully submitted before it can be used.
 * @attention This interface must be used between SchdfdLock and SchdfdUnlock.
 * @param  operation    [IN]  iocp operation
 * @param  timeout      [IN]  Timeout, ns
 * @retval #0 Function operation succeeded.
 * @retval #Non-zero Function fails to be operated and an error code is returned.
 */
int SchdfdIocpWaitInlock(struct IocpOperation *operation, unsigned long long timeout);

/**
 * @brief Retrieve the iocp operation structure corresponding to fd.
 * @par Description: Retrieve the iocp operation structure corresponding to fd
 * for subsequent asynchronous operations.
 * @attention This interface must be used between SchdfdLock and SchdfdUnlock.
 * @param  fd           [IN]  Registered fd
 * @param  type         [IN]  Type of the sequential operation to be controlled.
 * SHCDPOLL_READ indicates the read operation, and SHCDPOLL_WRITE indicates the write operation.
 * @param  buf
 * @param  len
 * @retval #Non NULL Function operation succeeded.
 * @retval #NULL Function fails to be operated.
 */
struct IocpOperation *SchdfdUpdateIocpOperationInlock(SignedSocket fd, SchdpollEventType type,
                                                      const void *buf, unsigned int len);

/**
 * @brief Check if the 'SetFileCompletionElementModes' function can run normally
 * @par Description: If the system supports the 'SetFileCompletionElementModes' function,
 * set the global variable to true
 * @attention To avoid dropping an IO operation to the IOCP waiting queue immediately
 * after it returns a success, use the 'SetFileCompletionNotify Modes' function.
 */
void SchdfdCheckIocpCompleteSkip();

/**
 * @brief Check if the function is enabled: After IO operation returns success immediately,
 * it will not be delivered to the IOCP waiting queue.
 * @par Description: After enabling IO operation and immediately returning success, do not drop it to
 * the IOCP waiting queue. After the function returns success immediately,
 * it can improve the performance of IO operation.
 * @attention If this feature is enabled, asynchronous operations such as recv, send, accept,
 * and connect will immediately return success and will no longer be added to the IOCP waiting queue.
 * @attention If this function is not enabled, the asynchronous operation will be immediately
 * completed and still added to the waiting queue, and later awakened by schdpoll.
 */
bool SchdfdUseSkipIocp();

#else
struct SchdpollDesc *SchdfdPdRemove(int fd);

int SchdfdPdPut(int fd, struct SchdpollDesc *pd);

#endif

int FreeSchdfdTimer(struct SchdfdFd *schdFd, SchdpollEventType type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_SCHDFD_IMPL_H */
