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


#ifndef MRT_SOCK_H
#define MRT_SOCK_H

#include "mid.h"
#include "schdfd_impl.h"
#ifdef MRT_WINDOWS
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief 0x100C0000 invalid arg
 */
#define ERRNO_SOCK_ARG_INVALID ((MID_SOCK) | 0x0000)

/**
 * @brief 0x100C0001 unsupported
 */
#define ERRNO_SOCK_NOT_SUPPORTED ((MID_SOCK) | 0x0001)

/**
 * @brief 0x100C0002 the function is not supported
 */
#define ERRNO_SOCK_NOT_REGISTERED ((MID_SOCK) | 0x0002)

/**
 * @brief 0x100C0003 invalid socket handle
 */
#define ERRNO_SOCK_HANDLE_INVALID ((MID_SOCK) | 0x0003)

/**
 * @brief 0x100C0004 The number of handles opened by the process exceeds the upper limit: 65535
 */
#define ERRNO_SOCK_FD_NUM_OVER_LIMIT ((MID_SOCK) | 0x0004)

/**
 * @brief 0x100C0005 the socket operation times out.
 */
#define ERRNO_SOCK_TIMEOUT ((MID_SOCK) | 0x0005)

/**
 * @brief 0x100C0007 linux tcp sock recv/send return EAGAIN
 */
#define ERRNO_SOCK_EAGAIN ((MID_SOCK) | 0x0007)

/**
 * socket addr
 */
struct SockAddr {
    struct sockaddr_storage *sockaddr;
    socklen_t addrLen;
};

/**
 * keepalive cfg
 */
struct SockKeepAliveCfg {
    unsigned int keepAlive; /* whether to open keep alive：0 is close, 1 is open */
    unsigned int idle;       /* if no data is exchanged during idle(seconds), the probe is performed. */
    unsigned int interval;   /* interval for sending probe packets: interval(s) */
    unsigned int count;      /* number of times that detection packets are sent. If all detection packets
                              * times out, the connection is invalid. */
};

/**
 * @brief Error code indicating that the sock interface fails to be obtained.
 * @par If invoking the sock interface fails, call this interface immediately before invoking other
 * interfaces to obtain the corresponding error code.
 * @retval #errorNo
 */
int SockErrnoGet(void);

/**
 * @brief create socket
 * @param domain     [IN] Protocol suite.
 * @param type       [IN] Socket type. When net is set to tcp, the parameter must be set to SOCK_FLOW,
 * When net is set to domain, the supported input parameter values are SOCK_FLOW and SOCK_DPGA.
 * @param protocol   [IN] Protocol type
 * @param net        [IN] net type. Currently, tcp, udp, domain, and raw are supported.
 * @retval #>=0 The creation is successful.
 * @retval #-1 The function fails to be operated. You can call #SockErrnoGet to obtain the error code.
 */
long long SockCreate(int domain, int type, int protocol, const char *net);

/**
 * @brief Create a socket and listen to the corresponding address. (The raw socket is bound but not listened.)
 * @par Description: Create a socket and bind the corresponding address, TCP socket, and raw socket of
 * SOCK_FLOW. The listen operation is performed.
 * @param sock     [IN] Socket handle
 * @param addr     [IN] Pointer and length of the address to be bound
 * @param backlog  [IN] This parameter is valid when the TYPE type of the TCP or domain protocol is SOCK_FLOW,
 * In the Linux environment, the actual value of backlog is the smaller value between backlog and
 * "/proc/sys/net/core/somaxconn"。 In the Windows operating system, the backlog is changed to a value
 * ranging from 200 to 65535.
 * @retval # Valid handle. If the function is successfully operated, the newly created sock handle is returned.
 * @retval #-1 If the function fails to be operated, call #SockErrnoGet to obtain the error code.
 */
long long SockBind(long long sock, struct SockAddr *addr, int backlog);

/**
 * @brief Listening address.
 * @par Description: socket listening address, which is used only for raw sockets.
 * @param sock     [IN] Socket handle
 * @param backlog  [IN] backlog
 * @retval # A valid handle (non-1 value) is successfully operated and the newly created sock handle is returned.
 * @retval #-1 The function fails to be operated. You can call #SockErrnoGet to obtain the error code.
 */
long long SockListen(long long sock, int backlog);

/**
 * @brief Used for a protocol with a connection to receive and establish a connection.
 * @par Description: Only connection-based protocols are supported. The peer end initiates a connect and
 * receives a connection from the peer end. This interface blocks the codethread but does not block the
 * thread when waiting for connection access.
 * @attention For the same sock, this interface does not support multiple processes. In addition, the
 * raw socket can also invoke this interface because the raw socket is transparently transmitted.
 * The sock of any protocol type can be used as an input parameter. For example, if the sock protocol is UDP,
 * a failure message is returned immediately.
 * @param sock     [IN] SockBind Returned SOCK handle.
 * @param addr     [OUT] Pointer and length of the peer address.
 * @retval # A valid handle. It is successfully operated and the sock handle of the new connection is returned.
 * @retval #-1 The function fails to be operated. You can call #SockErrnoGet to obtain the error code.
 */
long long SockAccept(long long sock, struct SockAddr *addr);

/**
 * @brief Used for a protocol with a connection to receive and establish a connection.
 * @par Description: Compared with the sockAccept interface, this interface supports the function of
 * waiting fora timeout response. If a connection request is received from the peer end within the
 * duration specified by timeout(ns), a normal response is returned. If no connection is received,
 * -1 is returned and the obtained error code is ERRNO_SOCK_TIMEOUT(0x100C0005) . If timeout is
 * set to 0. When there is no connection at the bottom layer, a timeout is returned immediately,
 * which is equivalent to a non-blocking interface. Use sockAccept if you want to keep blocking
 * reception without timeout.
 * @attention For the same sock, this interface does not support multiple processes. In addition,
 * the raw socket can also invoke this interface because the raw socket is transparently transmitted.
 * The sock of any protocol type may be used as an input parameter. If the sock protocol is UDP,
 * a failure message is returned immediately.
 * @param sock     [IN] SockBind Returned SOCK handle.
 * @param addr     [OUT] Peer address.
 * @param timeout  [IN] Waiting timeout interval, in ns.
 * @retval #valid handle
 * @retval #-1
 */
long long SockAcceptTimeout(long long sock, struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief Initiate a connection request to the peer end.
 * @par Description: Initiate a connection to the address pointed to by peerAddr. This interface
 * blocks the codethread but does not block the thread when the connection is successfully established.
 * @attention tcp socket Failed to execute this function. You need to close the socket, re-create
 * the socket, and try to connect to it.
 * @attention udp socket: This function can be invoked to set the peer address. The peer address
 * is returned immediately. After this interface is invoked, UDP can only use send and recv.
 * @param sock         [IN] Socket handle
 * @param localAddr    [IN] Pointer and length of the local address to be bound. NULL is supported.
 * If NULL is input, the local address is randomly selected by the system.
 * @param peerAddr     [IN] Pointer and length of the peer address to be connected
 * @retval #invalid handle
 * @retval #-1
 */
long long SockConnect(long long sock, struct SockAddr *localAddr, struct SockAddr *peerAddr);

/**
 * @brief Initiate a connection request to the peer end. version of sockConnect with timeout
 * @par Description: Compared with SockConnect, this interface supports the function of waiting for
 * a response upon timeout. If the connection is successfully initiated within the duration specified
 * by timeout(ns), a message is returned. If the connection fails, -1 is returned and the obtained
 * error code is ERRNO_SOCK_TIMEOUT(0x100C0005) . If the value of timeout is 0, the value is returned
 * directly. The value of timeout cannot be too small because the three-way handshake takes a long time.
 * If you want to keep blocking reception without timeout, use SockConnect.
 * @attention The connect implementation of the tcp socket uses the non-block connect. After this
 * function is successfully executed, the socket status bit is not set to SS_CONNECTED. The result
 * of the last connection is returned. If the connection is successful twice consecutively, you are
 * not advised to connect the connection again.
 * @param sock      [IN] Socket handle
 * @param localAddr [IN] Pointer and length of the local address to be bound. The value NULL can be
 * input. If NULL is input, the local address is randomly selected by the system.
 * @param peerAddr  [IN] Pointer and length of the peer address to be connected.
 * @param timeout   [IN] Waiting timeout interval, in ns.
 * @retval #invalid handle
 * @retval #-1
 */
long long SockConnectTimeout(long long sock, struct SockAddr *localAddr,
                             struct SockAddr *peerAddr, unsigned long long timeout);

/**
 * @brief SockDisconnect
 * @par   Disconnect by setting the socket address to AF_UNSPEC.
 * @param sock          [IN]  socket handle
 * @retval #0
 * @retval #-1
 */
int SockDisconnect(long long sock, bool isIPv6);

/**
 * @brief socket send for windows
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @retval #>0
 * @retval #-1
 */
int SockSend(long long sock, const void *buf, unsigned int len, SocketFlag flags);

/**
 * @brief It is used to send messages through a connection protocol in Windows.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  timeout      [IN]  timeout, ns。
 * @retval #>0
 * @retval #-1
 */
int SockSendTimeout(long long sock, const void *buf, unsigned int len, SocketFlag flags, unsigned long long timeout);

/**
 * @brief It is used to send messages through a connection protocol in Linux. SockSend does not block codethread
 * @par Currently, this interface is used only in the TCP socket and raw socket scenarios in the Linux.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @retval #>0
 * @retval #-1
 */
int SockSendNonBlock(long long sock, const void *buf, unsigned int len, SocketFlag flags);

/**
 * @brief Function for waiting for the write event in Linux
 * @par Description: This interface is used only for TCP sockets in Linux.
 * This interface is invoked after the EAGAIN error code is returned. Invoking this interface blocks the codethread.
 * @param sock     [IN] SOCK handle of an established connection.
 * @retval #=0
 * @retval #-1
 */
int SockWaitSend(long long sock);

/**
 * @brief SockWaitSend with timeout
 * @par This interface can be invoked only by the TCP socket in the Linux environment after the socksend interface
 * is invoked and the EAGAIN error code is returned. Invoking this interface will block codethread.
 * @param  sock         [IN]  handle
 * @param  timeout      [IN]  timeout
 * @retval #=0
 * @retval #-1
 */
int SockWaitSendTimeout(long long sock, unsigned long long timeout);

/**
 * @brief SockRecv
 * @par Only connection-based protocols are supported. Messages are received from the peer end of the connection
 * corresponding to the sock. This interface blocks the codethread but does not block the codethread when waiting
 * for a message.
 * @attention For the same sock, this interface does not support multiple codethreads.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
* @retval #>0 Number of bytes successfully received.
* @retval #-1 The function fails to be operated. You can call #SockErrnoGet to obtain the error code.
* @retval #0 The peer end is disconnected. The #SockClose interface needs to be invoked to close the sock handle.
 */
int SockRecv(long long sock, void *buf, unsigned int len, SocketFlag flags);

/**
 * @brief SockRecvTimeout
 * @par Compared with SockRecv, this interface has the function of waiting for a timeout response.
 * If a message is received within the duration specified by timeout(ns), the system returns a
 * message normally. If no message is received, the system returns -1 when the timeout period
 * expires. If the value of timeout is 0, a timeout message is returned immediately when no message
 * is received from the bottom layer, which is equivalent to a non-blocking interface. Use
 * SockRecv if you want to keep blocking reception without timeout.
 * @attention For the same sock, this interface does not support multiple processes or multiple processes
 * together with SockRecv.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer length
 * @param  len          [IN]  buffer
 * @param  flags        [IN]  flags
 * @param  timeout      [IN]  timeout, ns。
* @retval #>0 Number of bytes successfully received.
* @retval #-1 The function fails to be operated. You can call #SockErrnoGet to obtain the error code.
* @retval #0 The peer end is disconnected. The #SockClose interface needs to be invoked to close the sock handle.
 */
int SockRecvTimeout(long long sock, void *buf, unsigned int len, SocketFlag flags, unsigned long long timeout);

/**
 * @brief It is used to receive messages through a connection protocol in Linux. SockRecv does not block codethread.
 * @attention
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer length
 * @param  len          [IN]  buffer
 * @param  flags        [IN]  flags
 * @retval #>0
 * @retval #-1
 */
int SockRecvNonBlock(long long sock, void *buf, unsigned int len, SocketFlag flags);

/**
 * @brief wait read events in linux
 * @par This interface is invoked only by the TCP socket in the Linux environment after the sockrecv interface
 * is invoked and the EAGAIN error code is returned. Invoking this interface will block the codethread.
 * @param  sock         [IN]  socket handle
 * @retval #=0
 * @retval #-1
 */
int SockWaitRecv(long long sock);

/**
 * @brief SockWaitRecv with timeout
 * @par This interface is invoked only by the TCP socket in the Linux environment after the sockrecv interface
 * is invoked and the EAGAIN error code is returned. Invoking this interface will block the codethread.
 * @param  sock         [IN]  socket handle
 * @param  timeout      [IN]  timeout
 * @retval #=0
 * @retval #-1
 */
int SockWaitRecvTimeout(long long sock, unsigned long long timeout);

/**
 * @brief close sock handle
 * @par Close the socket and cleans up resources, and disconnects if the sock has established a connection.
 * @param  sock         [IN]  sock handle
 * @retval #0
 * @retval #-1
 */
int SockClose(long long sock);

/**
 * @brief close sock handle
 * @par Close the socket and cleans up resources, and disconnects if the sock has established a connection.
 * @param  sock         [IN]  sock handle
 * @retval #0
 * @retval #-1
 */
int SockShutdown(long long sock);

/**
 * @brief The keep alive function of the connection protocol is enabled.
 * @par To set the keep alive heartbeat detection function for a connection, set the returned sock
 * handle after SockConnect or SockAccept is successfully invoked.
 * @param  sock         [IN]  socket handle
 * @param  cfg          [IN]  keep alive args
 * @retval #0
 * @retval #-1
 */
int SockKeepAliveSet(long long sock, const struct SockKeepAliveCfg *cfg);

/**
 * @brief Obtains the local address bound to the sock.
 * @par Obtains the local address bound to the sock and saves the address in the addr file. If the sock
 * is not bound to a local address, you can call this API to obtain the local IP address and
 * port number assigned by the kernel after the connection is successful. If the specified port
 * number is 0, you can call this API to obtain the local port number assigned by the kernel.
 * @param  sock         [IN]  socket handle
 * @param  addr         [OUT]  addr
 * @retval #0
 * @retval #-1
 */
int SockLocalAddrGet(long long sock, struct SockAddr *addr);

/**
 * @brief Obtain the peer IP address of the sock connection.
 * @par For a sock that has established a connection, this interface is invoked to obtain the peer address
 * and save the address in the addr file.
 * @param  sock         [IN]  sock handle
 * @param  addr         [OUT] buf
 * @retval #0
 * @retval #-1
 */
int SockPeerAddrGet(long long sock, struct SockAddr *addr);

/**
 * @brief sendto with timeout.
 * @par Used to send messages through UDP and raw sockets. The interface is not normally blocked.
 * TCP does not support this method.
 * @attention If the UDP and raw sockets use SockConnect, only SockSend can be used.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [IN]  address
 * @param  timeout      [IN]  timeout, ns
 * @retval #>=0
 * @retval #-1
 */
int SockSendtoTimeout(long long sock, const void *buf, unsigned int len, SocketFlag flags,
                      const struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief sendto with timeout.
 * @par Used to send messages through UDP and raw sockets. The interface is not normally blocked.
 * TCP does not support this method.
 * @attention If the UDP and raw sockets use SockConnect, only SockSend can be used.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [IN]  address
 * @retval #>=0
 * @retval #-1
 */
int SockSendto(long long sock, const void *buf, unsigned int len, SocketFlag flags, const struct SockAddr *addr);

/**
 * @brief sendto non-blocking codethread
 * @par Used to send raw socket. The interface is not normally blocked.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [IN]  address
 * @retval #>=0
 * @retval #-1
 */
int SockSendtoNonBlock(long long sock, const void *buf, unsigned int len,
                       SocketFlag flags, const struct SockAddr *addr);

/**
 * @brief Used for udp and RawSocket to receive messages with timeout.
 * @par Used for udp and RawSocket to receive messages. This interface does not block codethreads.
 * @attention For the same sock, this interface does not support multiple codethreads.
 * @attention If the udp and raw sockets use SockConnect, only SockRecv can be used.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [OUT]  address. Support ipv4, for example: "ipv4:127.0.0.1:8080"
 * @param  timeout      [IN]  timeout, ns。
 * @retval #>=0
 * @retval #-1
 */
int SockRecvfromTimeout(long long sock, void *buf, unsigned int len, SocketFlag flags,
                        struct SockAddr *addr, unsigned long long timeout);

/**
 * @brief Used for udp and RawSocket to receive messages.
 * @par Used for udp and RawSocket to receive messages. This interface does not block codethreads.
 * @attention For the same sock, this interface does not support multiple codethreads.
 * @attention If the udp and raw sockets use SockConnect, only SockRecv can be used.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [IN]  address
 * @retval #>=0
 * @retval #-1
 */
int SockRecvfrom(long long sock, void *buf, unsigned int len, SocketFlag flags, struct SockAddr *addr);

/**
 * @brief It is used by rawsocket to receive messages in Linux. recvfrom nonblocking codethread.
 * @par It is used by rawsocket to receive messages. It doesn't block codethreads.
 * @attention For the same sock, this interface does not support multiple codethreads.
 * @attention If the raw sockets use SockConnect, only SockRecv can be used.
 * @param  sock         [IN]  socket handle
 * @param  buf          [IN]  buffer
 * @param  len          [IN]  buffer length
 * @param  flags        [IN]  flags
 * @param  addr         [IN]  address
 * @retval #>=0
 * @retval #-1
 */
int SockRecvfromNonBlock(long long sock, void *buf, unsigned int len, SocketFlag flags, struct SockAddr *addr);

/**
 * @brief Set socket options for sock
 * @param  sock         [IN]  socket handle
 * @param  level        [IN] level
 * @param  optname      [IN] option namae
 * @param  optval       [IN] option value
 * @param  optlen       [IN] option length
 * @retval #0
 * @retval #-1
 */
int SockOptionSet(long long sock, int level, int optname, const void *optval, int optlen);

/**
 * @brief Get socket options for sock
 * @param  sock         [IN]  socket handle
 * @param  level        [IN] level
 * @param  optname      [IN] option namae
 * @param  optval       [IN] option value
 * @param  optlen       [IN] option length
 * @retval #0
 * @retval #-1
 */
int SockOptionGet(long long sock, int level, int optname, void *optval, int *optlen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_SOCK_H */
