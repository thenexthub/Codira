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


#ifndef MRT_NETPOLL_COMMON_H
#define MRT_NETPOLL_COMMON_H

#include "mid.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief 0x10090000 netpoll repeated initialization
 */
#define ERRNO_NETPOLL_INITED ((MID_NETPOLL) | 0x0000)

/**
 * @brief 0x10090001 netpoll invalid arg
 */
#define ERRNO_NETPOLL_ARG_INVAILD ((MID_NETPOLL) | 0x0001)

/**
 * @brief 0x10090003 netpoll repeated epoll registration
 */
#define ERRNO_NETPOLL_REGISTED ((MID_NETPOLL) | 0x0003)

/**
 * @brief 0x10090004 netpoll is not initialized
 */
#define ERRNO_NETPOLL_UNINIT ((MID_NETPOLL) | 0x0004)

/**
 * @brief create and init NetpollMetaData
 * @retval NetpollMetaData pointer
 * @retval NULL
 */
struct NetpollMetaData *NetpollMetaDataInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_NETPOLL_COMMON_H */
