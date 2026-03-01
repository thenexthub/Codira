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
#include <sys/event.h>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include "log.h"
#include "netpoll_common.h"
#include "macro_def.h"
#include "netpoll.h"

NetpollFd NetpollCreate(void)
{
    struct NetpollMetaData *metaData = NetpollMetaDataInit();
    if (metaData == nullptr) {
        return nullptr;
    }
 
    metaData->kqfd = kqueue();
    if (metaData->kqfd == -1) {
        LOG_ERROR(errno, "kqueue create failed");
        free(metaData);
        return nullptr;
    }
 
    int error = fcntl(metaData->kqfd, F_SETFD, FD_CLOEXEC);
    if (error == -1) {
        LOG_ERROR(errno, "fcntl FD_CLOEXEC failed");
        return nullptr;
    }
    return metaData;
}
 
int NetpollAdd(NetpollFd npfd, int fd, void *data, unsigned int events)
{
    struct kevent event;
    struct NetpollMetaData *metaData = reinterpret_cast<struct NetpollMetaData *>(npfd);
    if (metaData == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return ERRNO_NETPOLL_UNINIT;
    }
 
    /* Initializes an event and adds it to kqueue.
       If the length of eventlist is 0, only events are registered and returned immediately.
    */
    EV_SET(&event, fd, events, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, data);
    int ret = kevent(metaData->kqfd, &event, 1, nullptr, 0, nullptr);
    if (event.flags & EV_ERROR) {
        LOG_ERROR(event.data, "netpoll add failed, kqfd: %d, fd: %d", metaData->kqfd, fd);
        return -1;
    }
    if (ret < 0) {
        LOG_ERROR(errno, "kevent failed, kqfd: %d, fd: %d", metaData->kqfd, fd);
        return ret;
    }
    return 0;
}
 
/* kevent will be deleted automatically from kqueue when fd is closed. */
int NetpollDel(NetpollFd npfd, int fd, unsigned int events)
{
    struct kevent event;
    struct NetpollMetaData *metaData = reinterpret_cast<struct NetpollMetaData *>(npfd);
    if (metaData == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return ERRNO_NETPOLL_UNINIT;
    }

    EV_SET(&event, fd, events, EV_DELETE, 0, 0, nullptr);
    int ret = kevent(metaData->kqfd, &event, 1, nullptr, 0, nullptr);
    if (event.flags & EV_ERROR) {
        LOG_ERROR(event.data, "netpoll del failed, fd: %d, event type: %u", fd, events);
        return -1;
    }
    if (ret < 0) {
        LOG_ERROR(errno, "kevent failed, fd: %d, event type: %u", fd, events);
        return ret;
    }
    return 0;
}
 
int NetpollWait(NetpollFd npfd, struct kevent *events, int maxevents, int timeoutms)
{
    int eventsNum;
    struct timespec timeout;
    struct NetpollMetaData *meta = reinterpret_cast<struct NetpollMetaData *>(npfd);
 
    if (meta == nullptr) {
        LOG_ERROR(ERRNO_NETPOLL_UNINIT, "netpoll uninited");
        return -1;
    }
 
    timeout.tv_sec = timeoutms / MSPERSECOND;
    timeout.tv_nsec = (timeoutms % MSPERSECOND) * NSPERMS;
    /* Waits for and listens to previously registered events within timeout period. */
    eventsNum = kevent(meta->kqfd, nullptr, 0, events, maxevents, &timeout);
    if (eventsNum == -1) {
        LOG_ERROR(errno, "kevent failed");
        return -1;
    }
    return eventsNum;
}
 
void NetpollExit(NetpollFd npfd)
{
    struct NetpollMetaData *metaData = reinterpret_cast<struct NetpollMetaData *>(npfd);
    if (npfd == nullptr) {
        return;
    }
    free(metaData);
}
