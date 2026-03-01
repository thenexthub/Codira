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


#ifndef MRT_RAWSOCK_H
#define MRT_RAWSOCK_H

#include "sock.h"
#include "sock_impl.h"
#include "schdfd_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief init raw socket module
 * @retval #0 init raw socket success
 * @retval #error init raw socket fail
 */
int RawsockInit(void);

/**
 * @brief register socket hooks
 * @param hooks         [OUT] sock_comm_hooks_s pointer
 */
void RawsockRegisterSocketHooks(struct SockCommHooks *hooks);

/**
 * @brief create raw socket。
 * @param domain        [IN] domain
 * @param type          [IN] type
 * @param protocol      [IN] protocol
 * @param socketError   [OUT] error
 * @retval #-1 create raw socket success.
 * @retval #error create raw socket fail.
 */
SignedSocket RawsockCreate(int domain, int type, int protocol, int *socketError);

/**
 * @brief bind address for raw socket
 * @param sockFd        [IN] socket handle
 * @param addr          [IN] address
 * @param backlog       [IN] max number of connections
 * backlog is meaningless because the bind and listen method of raw socket are separated
 * @retval #0 bind address for raw socket success.
 * @retval #error bind address for raw socket fail.
 */
int RawsockBind(SignedSocket sockFd, const struct SockAddr *addr, int backlog);

/**
 * @brief listen for raw socket
 * @param sockFd        [IN] socket handle
 * @param backlog       [IN] max number of connections
 * backlog is meaningless because the bind and listen method of raw socket are separated
 * @retval #0 listen raw socket success
 * @retval #error listen raw socket fail
 */
int RawsockListen(SignedSocket sockFd, int backlog);

/**
 * @brief accept raw socket
 * @param inFd          [IN] socket handle
 * @param outFd         [OUT] socket handle
 * @param addr          [IN] address
 * @param timeout       [IN] timeout
 * @retval #0 accept raw socket success
 * @retval #error accept raw socket fail
 */
int RawsockAccept(SignedSocket inFd, SignedSocket *outFd, struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief bind and connect for raw socket
 * @param connFd        [IN] socket handle
 * @param local         [IN] bind address
 * @param peer          [IN] connect address
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockBindConnect(SignedSocket connFd, struct SockAddr *local,
                       struct SockAddr *peer, unsigned long long timeout);

/**
 * @brief connect for raw socket
 * @param fd            [IN] socket handle
 * @param addr          [IN] address
 * @param addrLen       [IN] address length
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockConnect(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout);

/**
 * @brief  linux raw socket wait event
 * @param fd            [OUT] socket handle
 * @param type          [IN] event type
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockWait(SignedSocket fd, SchdpollEventType type, unsigned long long timeout);

/**
 * @brief  linux raw socket wait send
 * @param fd            [OUT] socket handle
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockWaitSend(SignedSocket fd, unsigned long long timeout);

/**
 * @brief  linux raw socket wait recv
 * @param fd            [OUT] socket handle
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockWaitRecv(SignedSocket fd, unsigned long long timeout);

/**
 * @brief disconnect for raw socket
 * @param connFd        [IN] socket handle
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int RawsockDisconnect(SignedSocket connFd);

#ifdef __cplusplus
}
#endif

#endif /* MRT_RAWSOCK_H */
