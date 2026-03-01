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


#ifndef MRT_NETPOLL_H
#define MRT_NETPOLL_H

#include <pthread.h>
#include "netpoll_common.h"
#include "macro_def.h"

#define MSPERSECOND 1000
#define NSPERMS 1000000

struct NetpollMetaData {
    int kqfd;
};

typedef int FdHandle;
 
typedef void* NetpollFd;
 
NetpollFd NetpollCreate(void);
int NetpollAdd(NetpollFd npfd, int fd, void* data, unsigned int events);
int NetpollDel(NetpollFd npfd, int fd, unsigned int events);
int NetpollWait(NetpollFd npfd, struct kevent* events, int maxevents, int timeoutms);
void NetpollExit(NetpollFd npfd);

#endif /* MRT_NETPOLL_H */
