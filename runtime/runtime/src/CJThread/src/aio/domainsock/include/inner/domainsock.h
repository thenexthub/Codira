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


#ifndef MRT_DOMAINSOCK_H
#define MRT_DOMAINSOCK_H

#include "sock.h"
#include "schdfd_impl.h"
#include "sock_impl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int DomainsockInit(void);

/**
 * @brief create domain socket
 * @param domain        [IN] domain
 * @param type          [IN] socket type
 * @param protocol      [IN] socket protocol
 * @param socketError   [OUT] error code
 * @retval #!=-1 create socket success
 * @retval #-1 create socket fail
 */
SignedSocket DomainsockCreate(int domain, int type, int protocol, int *socketError);

/**
 * @brief bind address for domain socket
 * @param fd            [IN] file descriptor handle
 * @param addr          [IN] address
 * @param addrLen       [IN] address length
 * @retval #0  bind address for domain socket success
 * @retval #!=0 bind address for domain socket fail
 */
int DomainsockBind(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen);

/**
 * @brief bind and listen domain socket
 * @param sockFd        [IN] socket handle
 * @param addr          [IN] address
 * @param backlog       [IN] max number of connections
 * @retval #0  bind and listen domain socket success
 * @retval #!=0 bind and listen domain socket fail
 */
int DomainsockBindListen(SignedSocket sockFd, const struct SockAddr *addr, int backlog);

/**
 * @brief accept domain socket
 * @param inFd          [IN] socket hadnle
 * @param outFd         [OUT] socket handle
 * @param addr          [OUT] address
 * @param timeout       [IN] timeout
 * @retval #0 accept domain socket success
 * @retval #error accept domain socket fail
 */
int DomainsockAccept(SignedSocket inFd, SignedSocket *outFd, struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief bind and connect domain socket
 * @param fd            [IN] file descriptor handle
 * @param addr          [IN] address
 * @param addr_len      [IN] address length
 * @param timeout       [IN] timeout
 * @retval #0  connect success
 * @retval #!=0 the function operation fail
 */
int DomainsockBindConnect(SignedSocket connFd, struct SockAddr *local,
                          struct SockAddr *peer, unsigned long long timeout);

/**
 * @brief connect domain socket
 * @param fd            [IN] socket handle
 * @param addr          [IN] address
 * @param addrLen       [IN] address length
 * @param timeout       [IN] timeout
 * @retval #0  connect domain socket
 * @retval #!=0 connect domain socket fail
 */
int DomainsockConnect(SignedSocket fd, const struct sockaddr *addr,
                      socklen_t addrLen, unsigned long long timeout);

/**
 * @brief disconnect domain socket
 * @param connFd        [IN] socket handle
 * @retval #0 disconnect domain socket success
 * @retval #error disconnect domain socket fail
 */
int DomainsockDisconnect(SignedSocket connFd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_DOMAINSOCK_H */
