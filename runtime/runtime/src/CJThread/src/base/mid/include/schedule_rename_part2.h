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


#ifndef MRT_SCHEDULE_RENAME_PART2_H
#define MRT_SCHEDULE_RENAME_PART2_H


#ifdef CODIRA

/* sema */
#define SemaNew                                  CODE_SemaNew
#define SemaTryAcquire                           CODE_SemaTryAcquire
#define SemaAcquire                              CODE_SemaAcquire
#define SemaRelease                              CODE_SemaRelease
#define SemaDelete                               CODE_SemaDelete
#define SemaGetValue                             CODE_SemaGetValue

/* rawsock */
#define RawsockInit                               CODE_MRT_RawsockInit
#define RawsockRegisterSocketHooks                CODE_RawsockRegisterSocketHooks
#define RawsockCreate                             CODE_RawsockCreate
#define RawsockBind                               CODE_RawsockBind
#define RawsockListen                             CODE_RawsockListen
#define RawsockAccept                             CODE_RawsockAccept
#define RawsockBindConnect                        CODE_RawsockBindConnect
#define RawsockConnect                            CODE_RawsockConnect
#define RawsockWait                               CODE_RawsockWait
#define RawsockWaitSend                           CODE_RawsockWaitSend
#define RawsockWaitRecv                           CODE_RawsockWaitRecv
#define RawsockDisconnect                         CODE_RawsockDisconnect
/* windows api */
#define RawsockIsConnectionOriented               CODE_RawsockIsConnectionOriented
#define RawsockGetLocalDefaultAddr                CODE_RawsockGetLocalDefaultAddr
#define RawsockAcceptCoreInlock                   CODE_RawsockAcceptCoreInlock
#define RawsockConnectOriented                    CODE_RawsockConnectOriented

/* trace */
#define TraceFini                                 CODE_TraceFini
#define TraceStart                                CODE_TraceStart
#define TraceFullQueue                            CODE_TraceFullQueue
#define TraceParkUnlock                           CODE_TraceParkUnlock
#define TraceStop                                 CODE_TraceStop
#define TraceByte                                 CODE_TraceByte
#define TraceUint64                               CODE_TraceUint64
#define TraceFlush                                CODE_TraceFlush
#define TraceStackId                              CODE_TraceStackId
#define TraceBufAquire                            CODE_TraceBufAquire
#define TraceBufRelease                           CODE_TraceBufRelease
#define TraceEvent                                CODE_TraceEvent
#define TraceStackTableDump                       CODE_TraceStackTableDump
#define TraceDump                                 CODE_TraceDump
#define TraceReaderAvailable                      CODE_TraceReaderAvailable
#define TraceReaderGet                            CODE_TraceReaderGet
#define TraceRegister                             CODE_TraceRegister
#define TraceDeregister                           CODE_TraceDeregister

#endif

#endif // MRT_SCHEDULE_RENAME_PART2_H
