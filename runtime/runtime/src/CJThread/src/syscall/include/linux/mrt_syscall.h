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

#ifndef MRT_CODETHREAD_SYSCALL_H
#define MRT_CODETHREAD_SYSCALL_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include "mid.h"
#include "mrt_syscall_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief syscall readv
 * @par Reads data from the FD and saves it to the cache chain.
 * @param  fileds       [IN]  fd
 * @param  iov          [OUT] Stores the cache chain for reading data
 * @param  count        [IN]  Number of caches in the cache chain.
 * @retval Returns the length of the read upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallReadv(int fileds, const struct iovec *iov, int iovcnt);

/**
 * @brief syscall writev
 * @par Write the data in the cache chain to the specified fd.
 * @param  fileds       [IN]  fd
 * @param  iov          [OUT] Save the cache chain to be written with data
 * @param  iovcnt       [IN]  The number of caches in the cache chain
 * @retval Returns the length of the write upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallWritev(int fileds, const struct iovec *iov, int iovcnt);

/**
 * @brief syscall fcntl
 * @par Change the nature of the specified file.
 * @param  argNum       [IN]  Total number of parameters. The maximum is 3.
 * @param  fd           [IN]  fd, arg1
 * @param  cmd          [IN]  operation instruction, arg 2,
 * with a maximum of one parameter attached.
 * @retval Returns the corresponding value upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallFcntl(int fd, int cmd, ...);

/**
 * @brief sysacll dup
 * @par copy a fd
 * @attention  The copied new fd is the minimum value currently available.
 * @param  oldfd        [IN]  fd
 * @retval Returns the new fd upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallDup(int oldfd);

/**
 * @brief syscall dup2
 * @par Copy an fd using the specified new fd
 * @param  oldfd        [IN]  old fd
 * @param  newfd        [IN]  new fd
 * @retval Returns the new fd upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallDup2(int oldfd, int newfd);

/**
 * @brief sysacll dup3
 * @par Copy an fd using the specified new fd
 * @param  oldfd        [IN]  old fd
 * @param  newfd        [IN]  new fd
 * @param  flags        [IN]  flag
 * @retval Returns the new fd upon success,
 * -1 upon failure, and errno is set as the error code.
 */
int SyscallDup3(int oldfd, int newfd, int flags);

/**
 * @brief sysacll nanosleep
 * @par sleep
 * @param  req          [IN]  duration
 * @param  rem          [OUT]  If awakened in advance, save the remaining time.
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallNanosleep(const struct timespec *req, struct timespec *rem);

/**
 * @brief sysacll connect
 * @par connect socket to the address
 * @param  sockfd       [IN]  socket fd
 * @param  addr         [IN]  target address
 * @param  addrlen      [IN]  address length
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief sysacll recv
 * @par recv for socket
 * @param  sockfd       [IN]  socket fd
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flag
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallRecv(int sockfd, void *buf, size_t len, int flags);

/**
 * @brief sysacll recvfrom
 * @par recvfrom for socket
 * @param  sockfd       [IN]  socket fd
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flag
 * @param  srcAddr      [IN]  src address
 * @param  addrlen      [IN]  address length
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallRecvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *srcAddr, socklen_t *addrlen);

/**
 * @brief syscall recvmsg
 * @par recvmsg for socket
 * @param  sockfd       [IN]  socket fd
 * @param  msg          [IN]  An information handle that stores information
 * @param  flags        [IN]  flag
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallRecvmsg(int sockfd, struct msghdr *msg, int flags);

/**
 * @brief syscall send
 * @par send for socket
 * @param  sockfd       [IN]  socket fd
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flag
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallSend(int sockfd, const void *buf, size_t len, int flags);

/**
 * @brief sysacll sendto
 * @par sendto for socket
 * @param  sockfd       [IN]  socket fd
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flag
 * @param  destAddr     [IN]  target address
 * @param  addrlen      [IN]  address length
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallSendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *destAddr, socklen_t addrlen);

/**
 * @brief syscall sendmsg
 * @par sendmsg for socket
 * @param  sockfd       [IN]  socket fd
 * @param  msg          [IN]  An information handle that stores information
 * @param  flags        [IN]  flag
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallSendmsg(int sockfd, const struct msghdr *msg, int flags);

/**
 * @brief sysacll poll
 * @par Monitor a set of file descriptors to see if IO operations can be performed.
 * @param  fds          [IN]  fd array
 * @param  nfds         [IN]  fd number
 * @param  timeout          [IN]  timeout
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallPoll(struct pollfd *fds, unsigned long nfds, int timeout);

/**
 * @brief sysacll select
 * @par Monitor whether multiple sets of file descriptors can perform IO operations.
 * @param  nfds         [IN]  fd array
 * @param  readfds      [IN]  Readable fd for monitoring
 * @param  writefds     [IN]  Writable fd for monitoring
 * @param  exceptfds    [IN]  The monitored fd will experience exceptions
 * @param  timeout      [IN]  timeout
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallSelect(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds,
                  fd_set *__restrict exceptfds, struct timeval *__restrict timeout);

/**
 * @brief sysacll getsockopt
 * @par Get an option for a socket
 * @param  sockfd       [IN]  fd
 * @param  level        [IN]  protocol level
 * @param  optname      [IN]  option name
 * @param  optval       [IN]  option value
 * @param  optlen       [IN]  length of option value
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallGetsockopt(int sockfd, int level, int optname, void *__restrict optval,
                      socklen_t *__restrict optlen);

/**
 * @brief syscall setsockopt
 * @par Set an option for a socket
 * @param  sockfd       [IN]  fd
 * @param  level        [IN]  protocol level
 * @param  optname      [IN]  option name
 * @param  optval       [IN]  option value
 * @param  optlen       [IN]  length of option value
 * @retval Returns 0 upon success, -1 upon failure, and errno is set as the error code.
 */
int SyscallSetsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_SYSCALL_H */
