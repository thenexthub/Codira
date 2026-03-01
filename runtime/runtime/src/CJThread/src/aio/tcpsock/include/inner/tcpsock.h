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


#ifndef MRT_TCPSOCK_H
#define MRT_TCPSOCK_H

#include "sock.h"
#include "sock_impl.h"
#include "schdfd_impl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief init tcpsock module
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockInit(void);

/**
 * @brief register socket hooks
 * @param hooks         [OUT] sock_comm_hooks_s pointer
 */
void TcpsockRegisterSocketHooks(struct SockCommHooks *hooks);

/**
 * @brief tcp KeepAliveSet
 * @param fd            [IN] socket handle
 * @param cfg           [IN] address
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockKeepAliveSet(SignedSocket fd, const struct SockKeepAliveCfg *cfg);

/**
 * @brief create tcp Socket。
 * @param domain        [IN] domain
 * @param type          [IN] type
 * @param protocol      [IN] protocol
 * @param socketError   [OUT] error
 * @retval #-1
 * @retval #socket handle
 */
SignedSocket TcpsockCreate(int domain, int type, int protocol, int *socketError);

/**
 * @brief tcp create and bind for tcpsocket
 * @param sockFd        [IN] socket handle
 * @param addr          [IN] address
 * @param backlog       [IN] max number of connections
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockBindListen(SignedSocket sockFd, const struct SockAddr *addr, int backlog);

/**
 * @brief accept for tcp scoket
 * @param inFd          [IN] socket handle
 * @param outFd         [IN] socket handle
 * @param addr          [IN] address
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockAccept(SignedSocket inFd, SignedSocket *outFd, struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief bind and connect for tcp socket
 * @param connFd        [IN] socket handle
 * @param local         [IN] bind address
 * @param peer          [IN] connect address
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockBindConnect(SignedSocket connFd, struct SockAddr *local,
                       struct SockAddr *peer, unsigned long long timeout);

/**
 * @brief connect for tcp socket
 * @param fd            [IN] socket handle
 * @param addr          [IN] address
 * @param addrLen       [IN] address length
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockConnect(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout);

/**
 * @brief bind address for tcp socket
 * @param fd            [IN] socket handle
 * @param addr          [IN] address
 * @param addrLen       [IN] address length
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockBind(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen);

/**
 * @brief  linux tcp socket wait send events
 * @param fd            [OUT] socket handle
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockWaitSend(SignedSocket fd, unsigned long long timeout);

/**
 * @brief  linux tcp socket wait recv events
 * @param fd            [OUT] socket handle
 * @param timeout       [IN] timeout
 * @retval #0 The function is executed successfully
 * @retval #error Failed to execute the function
 */
int TcpsockWaitRecv(SignedSocket fd, unsigned long long timeout);

void TcpsockAcceptSuccessLogWrite(struct sockaddr *addr, int inFd, int fd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* MRT_TCPSOCK_H */
