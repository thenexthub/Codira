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


#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include "securec.h"
#include "log.h"
#include "netpoll_common.h"
#include "netpoll.h"

#ifdef __cplusplus
extern "C" {
#endif

/* g_epollRegister Prevent concurrent registration and initialization processes */
struct NetpollRegister g_epollRegister = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .epoll = {
        .createFn  = epoll_create,
        .ctlFn = epoll_ctl,
        .waitFn = epoll_wait,
        .closeFn = close
    },
    // After any Netpoll is initialized or registered, the interface cannot be registered.
    .inited = 0
};

/* Netpoll Exit */
void NetpollExit(NetpollFd npfd)
{
    struct NetpollMetaData *meta = (struct NetpollMetaData *)npfd;

    if (meta == nullptr) {
        return;
    }
    g_epollRegister.epoll.closeFn(meta->epfd);
    g_epollRegister.inited = 0;
    free(meta);
}

/* Netpoll register. Must be registered before module initialization. */
int NetpollFnRegister(NetpollCreateFn createFn, NetpollCtlFn ctlFn, NetpollWaitFn waitFn,
                      NetpollCloseFn closeFn)
{
    if (createFn == nullptr || ctlFn == nullptr || waitFn == nullptr || closeFn == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_ARG_INVAILD, "arg is nullptr");
        return ERRNO_NETPOLL_ARG_INVAILD;
    }

    pthread_mutex_lock(&g_epollRegister.mutex);

    // Repeated registration or registration after module initialization is not supported.
    if (g_epollRegister.inited != 0) {
        pthread_mutex_unlock(&g_epollRegister.mutex);
        LOG_ERROR(ERRNO_NETPOLL_REGISTED, "double registered");
        return ERRNO_NETPOLL_REGISTED;
    }

    g_epollRegister.epoll.createFn = createFn;
    g_epollRegister.epoll.ctlFn = ctlFn;
    g_epollRegister.epoll.waitFn = waitFn;
    g_epollRegister.epoll.closeFn = closeFn;
    g_epollRegister.inited = 1;

    pthread_mutex_unlock(&g_epollRegister.mutex);

    return 0;
}

/* Deregister the epoll interface. Internal interface, used only by the test framework. */
void NetpollFnUnregister(void)
{
    pthread_mutex_lock(&g_epollRegister.mutex);
    g_epollRegister.epoll.createFn = epoll_create;
    g_epollRegister.epoll.ctlFn = epoll_ctl;
    g_epollRegister.epoll.waitFn = epoll_wait;
    g_epollRegister.epoll.closeFn = close;
    g_epollRegister.inited = 0;
    pthread_mutex_unlock(&g_epollRegister.mutex);
}

/* Create the global public epoll_fd. */
int NetpollCreateImpl(struct NetpollMetaData *metaData)
{
    metaData->epfd = g_epollRegister.epoll.createFn(EPOLL_CAPACITY);
    if (metaData->epfd == -1) {
        LOG_ERROR(errno, "epoll create failed");
        return errno;
    }
    return 0;
}

/* Module initialization interface. After initialization, the bottom-layer epoll implementation
 * cannot be modified.
 */
NetpollFd NetpollCreate(void)
{
    struct NetpollMetaData *metaData;
    int error;

    metaData = NetpollMetaDataInit();
    if (metaData == nullptr) {
        return nullptr;
    }

    pthread_mutex_lock(&g_epollRegister.mutex);
    error = NetpollCreateImpl(metaData);
    if (error != 0) {
        pthread_mutex_unlock(&g_epollRegister.mutex);
        free(metaData);
        return nullptr;
    }
    g_epollRegister.inited = 1;
    pthread_mutex_unlock(&g_epollRegister.mutex);

    return metaData;
}

/* Add fd to Netpoll monitoring. */
int NetpollAdd(NetpollFd npfd, int fd, void *data, unsigned int events)
{
    struct epoll_event event;
    struct NetpollMetaData *meta = (struct NetpollMetaData *)npfd;
    int ret;

    if (meta == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return ERRNO_NETPOLL_UNINIT;
    }

    event.data.ptr = data;
    event.events = events | EPOLLET;
    ret = g_epollRegister.epoll.ctlFn(meta->epfd, EPOLL_CTL_ADD, fd, &event);
    if (ret != 0) {
        ret = errno;
        LOG_ERROR(ret, "netpoll add failed, epfd: %d, fd: %d", meta->epfd, fd);
        return ret;
    }
    return 0;
}

/* Remove the fd from the Netpoll monitoring. */
int NetpollDel(NetpollFd npfd, int fd)
{
    struct epoll_event event;
    struct NetpollMetaData *meta = (struct NetpollMetaData *)npfd;
    int ret;

    if (meta == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return ERRNO_NETPOLL_UNINIT;
    }

    ret = g_epollRegister.epoll.ctlFn(meta->epfd, EPOLL_CTL_DEL, fd, &event);
    if (ret != 0) {
        ret = errno;
        LOG_ERROR(ret, "netpoll del failed, epfd: %d, fd: %d", meta->epfd, fd);
        return ret;
    }
    return 0;
}

int NetpollWait(NetpollFd npfd, struct epoll_event *events, int maxevents, int timeoutms)
{
    int eventsNum;
    struct NetpollMetaData *meta = (struct NetpollMetaData *)npfd;

    if (meta == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return -1;
    }

    // The return caused by the interrupt needs to be waited.
    do {
        eventsNum = g_epollRegister.epoll.waitFn(meta->epfd, events, maxevents, timeoutms);
    } while (eventsNum == -1 && errno == EINTR);

    if (eventsNum == -1) {
        LOG_ERROR(errno, "netpoll wait failed, epfd: %d, maxevents: %d", meta->epfd, maxevents);
    }
    return eventsNum;
}

#ifdef __cplusplus
}
#endif
