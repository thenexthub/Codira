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


#ifndef MRT_MID_H
#define MRT_MID_H

#include "schedule_rename.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* Modules are identified by 0x10010000 and 0x10020000.
 * The last four bits are reserved for module error codes.
 */
#define MID_LIST                 MID_MAKE(1)
#define MID_QUEUE                MID_MAKE(2)
#define MID_LOG                  MID_MAKE(3)
#define MID_SCHEDULE             MID_MAKE(4)
#define MID_COMUTEX              MID_MAKE(5)
#define MID_CHANNEL              MID_MAKE(6)
#define MID_ASMCALL              MID_MAKE(7)
#define MID_CODECLANCALL           MID_MAKE(8)
#define MID_NETPOLL              MID_MAKE(9)
#define MID_SCHMON               MID_MAKE(10)
#define MID_TIMER                MID_MAKE(11)
#define MID_SOCK                 MID_MAKE(12)
#define MID_WAITQUEUE            MID_MAKE(13)
#define MID_SEMA                 MID_MAKE(14)
#define MID_CHAN                 MID_MAKE(15)
#define MID_SCHDFD               MID_MAKE(16)
#define MID_SERVICE              MID_MAKE(17)
#define MID_RPC                  MID_MAKE(18)
#define MID_HANDLE_POOL          MID_MAKE(19)
#define MID_YTLS                 MID_MAKE(20)
#define MID_TRACE                MID_MAKE(21)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_MID_H */
