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

#ifndef MRT_SCHEDULE_RENAME_H
#define MRT_SCHEDULE_RENAME_H

#ifdef MRT_USE_CODETHREAD_RENAME
#define CODIRA
#endif
#include "schedule_rename_part2.h"

#ifdef CODIRA

/* schdfd */
#define SchdfdInit                               CODE_SchdfdInit
#define SchdfdIncref                             CODE_SchdfdIncref
#define SchdfdDecref                             CODE_SchdfdDecref
#define SchdfdRegister                           CODE_SchdfdRegister
#define SchdfdWakeall                            CODE_SchdfdWakeAll
#define SchdfdNetpollAdd                         CODE_SchdfdNetpollAdd
#define SchdfdDeregister                         CODE_SchdfdDeregister
#define SchdfdWait                               CODE_SchdfdWait
#define SchdfdWaitTimeout                        CODE_SchdfdWaitTimeout
#define SchdfdWaitInlock                         CODE_SchdfdWaitInlock
#define SchdfdReadTimeout                        CODE_SchdfdReadTimeout
#define SchdfdWriteTimeout                       CODE_SchdfdWriteTimeout
#define SchdfdWaitInlockTimeout                  CODE_SchdfdWaitInlockTimeout
#define SchdfdLock                               CODE_SchdfdLock
#define SchdfdUnlock                             CODE_SchdfdUnlock
#define SchdfdRegisterAndNetpollAdd              CODE_SchdfdRegisterAndNetpollAdd
#define SchdfdPdPut                              CODE_SchdfdPdPut
#define SchdfdPdRemove                           CODE_SchdfdPdRemove
#define g_keepAliceCfg                           CODE_GKeepAliceCfg
/* windows api */
#define g_iocpCompleteSkipFlag                   CODE_GIocpCompleteSkipFlag
#define SchdfdUpdateIocpOperationInlock          CODE_SchdfdUpdateIocpOperationInlock
#define SchdfdIocpWaitInlock                     CODE_SchdfdIocpWaitInlock
#define SchdfdCheckIocpCompleteSkip              CODE_SchdfdSCheckIocpCompleteSkipFlag
#define SchdfdUseSkipIocp                        CODE_SchdfdUseSkipIocp
#define SchdfdFdValidCheck                       CODE_SchdfdValidCheck

/* sock */
#define g_sockErrno                              CODE_GSockErrno
#define g_sockCommHooks                          CODE_GSockCommHooks
#define g_mswsockHooks                           CODE_GMswsockHooks

#define SockErrnoSet                             CODE_SockErrnoSet
#define SockErrnoGet                             CODE_SockErrnoGet
#define SockCommHooksReg                         CODE_SockCommHooksReg
#define SockStrToNetType                         CODE_SockStrToNetType
#define SockHandleParse                          CODE_SockHandleParse
#define SockCreate                               CODE_MRT_SockCreate
#define SockBind                                 CODE_MRT_SockBind
#define SockListen                               CODE_MRT_SockListen
#define SockAcceptTimeout                        CODE_MRT_SockAcceptTimeout
#define SockAccept                               CODE_MRT_SockAccept
#define SockConnectTimeout                       CODE_MRT_SockConnectTimeout
#define SockConnect                              CODE_MRT_SockConnect
#define SockDisconnect                           CODE_MRT_SockDisconnect
#define SockSendTimeout                          CODE_MRT_SockSendTimeout
#define SockSend                                 CODE_MRT_SockSend
#define SockSendNonBlock                         CODE_MRT_SockSendNonBlock
#define SockWaitSendTimeout                      CODE_MRT_SockWaitSendTimeout
#define SockWaitSend                             CODE_MRT_SockWaitSend
#define SockRecvTimeout                          CODE_MRT_SockRecvTimeout
#define SockRecv                                 CODE_MRT_SockRecv
#define SockRecvNonBlock                         CODE_MRT_SockRecvNonBlock
#define SockWaitRecvTimeout                      CODE_MRT_SockWaitRecvTimeout
#define SockWaitRecv                             CODE_MRT_SockWaitRecv
#define SockClose                                CODE_MRT_SockClose
#define SockShutdown                             CODE_MRT_SockShutdown
#define SockKeepAliveSet                         CODE_SockKeepAliveSet
#define SockAddrGet                              CODE_SockAddrGet
#define SockLocalAddrGet                         CODE_MRT_SockLocalAddrGet
#define SockPeerAddrGet                          CODE_MRT_SockPeerAddrGet
#define SockSendtoTimeout                        CODE_SockSendtoTimeout
#define SockSendto                               CODE_MRT_SockSendto
#define SockSendtoNonBlock                       CODE_MRT_SockSendtoNonBlock
#define SockRecvfromTimeout                      CODE_MRT_SockRecvfromTimeout
#define SockRecvfrom                             CODE_SockRecvfrom
#define SockRecvfromNonBlock                     CODE_MRT_SockRecvfromNonBlock
#define SockOptionSet                            CODE_SockOptionSet
#define SockOptionGet                            CODE_SockOptionGet
#define SockAddrGetGeneral                       CODE_SockAddrGetGeneral
#define SockOptionSetGeneral                     CODE_SockOptionSetGeneral
#define SockOptionGetGeneral                     CODE_SockOptionGetGeneral
#define SockCloseGeneral                         CODE_MRT_SockCloseGeneral
#define SockShutdownGeneral                      CODE_SockShutdownGeneral
#define SockRecvGeneral                          CODE_SockRecvGeneral
#define SockSendGeneral                          CODE_SockSendGeneral
#define SockSendGeneral                          CODE_SockSendGeneral
#define SockSendtoGeneral                        CODE_SockSendtoGeneral
#define SockRecvfromGeneral                      CODE_SockRecvfromGeneral
#define SockSendNonBlockGeneral                  CODE_SockSendNonBlockGeneral
#define SockRecvNonBlockGeneral                  CODE_MRT_SockRecvNonBlockGeneral
#define SockSendtoNonBlockGeneral                CODE_MRT_SockSendtoNonBlockGeneral
#define SockRecvfromNonBlockGeneral              CODE_SockRecvfromNonBlockGeneral
#define SockWinStartup                           CODE_SockWinStartup
#define SockLoadMswsockHooks                     CODE_SockLoadMswsockHooks
#define SockMswsockHooksReg                      CODE_SockMswsockHooksReg
#define SockWinInit                              CODE_SockWinInit
#define SockGetLocalDefaultAddr                  CODE_SockGetLocalDefaultAddr

/* tcpsock */
#define TcpsockBind                              CODE_TcpsockBind
#define TcpsockBindListen                        CODE_TcpsockBindListen
#define TcpsockAccept                            CODE_TcpsockAccept
#define TcpsockConnect                           CODE_TcpsockConnect
#define TcpsockCreate                            CODE_TcpsockCreate
#define TcpsockBindConnect                       CODE_TcpsockBindConnect
#define TcpsockKeepAliveSet                      CODE_TcpsockKeepAliveSet
#define TcpsockInit                              CODE_MRT_TcpsockInit
#define TcpsockAcceptSuccessLogWrite             CODE_TcpsockAcceptSuccessLogWrite
#define TcpsockRegisterSocketHooks               CODE_TcpsockRegisterSocketHooks
#define TcpsockWait                              CODE_TcpsockWait
#define TcpsockWaitSend                          CODE_TcpsockWaitSend
#define TcpsockWaitRecv                          CODE_TcpsockWaitRecv
/* windows api */
#define ConnectBindLocal                         CODE_ConnectBindLocal
#define TcpsockAcceptCoreInlock                  CODE_TcpsockAcceptCoreInlock

/* domainsock */
#define DomainsockCreate                         CODE_DomainsockCreate
#define DomainsockBind                           CODE_DomainsockBind
#define DomainsockBindListen                     CODE_DomainsockBindListen
#define DomainsockAccept                         CODE_DomainsockAccept
#define DomainsockConnect                        CODE_DomainsockConnect
#define DomainsockBindConnect                    CODE_DomainsockBindConnect
#define DomainsockInit                           CODE_MRT_DomainsockInit

/* udpsock */
#define UdpsockCreate                            CODE_UdpsockCreate
#define UdpsockBind                              CODE_UdpsockBind
#define UdpsockBindNoListen                      CODE_UdpsockBindNoListen
#define UdpsockConnect                           CODE_UdpsockConnect
#define UdpsockDisconnect                        CODE_UdpsockDisconnect
#define UdpsockBindConnect                       CODE_UdpsockBindConnect
#define UdpsockRegisterSocketHooks               CODE_UdpsockRegisterSocketHooks
#define UdpsockInit                              CODE_MRT_UdpsockInit

/* log */
#define g_logFunc                                CODE_GLogFunc
#define g_logEnable                              CODE_GLogEnable
#define g_logLevel                               CODE_GLogLevel

#define ErrorLeverString                         CODE_ErrorLeverString
#define LogWrite                                 CODE_LogWrite
#define LogRegister                              CODE_LogRegister
#define LogInfoWritable                          CODE_LogInfoWritable

/* netpoll */
#define g_epollRegister                         CODE_GEpollRegister

#define NetpollExit                             CODE_NetpollExit
#define NetpollFnRegister                       CODE_NetpollFnRegister
#define NetpollFnUnregister                     CODE_NetpollFnUnregister
#define NetpollCreateImpl                       CODE_NetpollCreateImpl
#define NetpollCreate                           CODE_NetpollCreate
#define NetpollAdd                              CODE_NetpollAdd
#define NetpollDel                              CODE_NetpollDel
#define NetpollWait                             CODE_NetpollWait
#define NetpollInnerFd                          CODE_NetpollInnerFd
#define NetpollMetaDataInit                     CODE_NetpollMetaDataInit

/* codethread */
#define g_codethread                              CODE_GCODEThread
#define g_schedule                              CODE_GSchedule
#define g_scheduleList                          CODE_GScheduleList
#define g_preemptFlag                           CODE_GPreemptFlag
#define g_codethreadStackReservedSize             CODE_GCODEThreadStackReservedSize

#define GetCODEThreadScheduler                   CODE_GetCODEThreadScheduler
#define RebindCODEThread                         CODE_RebindCODEThread
#define AddSingleModelC2NCount                 CODE_AddSingleModelC2NCount
#define DecSingleModelC2NCount                 CODE_DecSingleModelC2NCount
#define CODEThreadMemFree                        CODE_CODEThreadMemFree
#define CODEThreadFree                           CODE_CODEThreadFree
#define CODEThreadNewId                          CODE_CODEThreadNewId
#define CODEThreadInit                           CODE_CODEThreadInit
#define CODEThreadMemAlloc                       CODE_CODEThreadMemAlloc
#define CODEThreadAlloc                          CODE_CODEThreadAlloc
#define CODEThreadMexit                          CODE_CODEThreadMexit
#define CODEThreadExit                           CODE_CODEThreadExit
#define CODEThreadEntry                          CODE_CODEThreadEntry
#define CODEThreadEntryInitMutator               CODE_CODEThreadEntryInitMutator
#define CODEThreadMake                           CODE_CODEThreadMake
#define CODEThread0Make                          CODE_CODEThread0Make
#define CODEThreadAttrCheck                      CODE_CODEThreadAttrCheck
#define CODEThreadNew                            CODE_CODEThreadNew
#define CODEThreadNewToSchedule                  CODE_CODEThreadNewToSchedule
#define CODEThreadNewToDefault                   CODE_CODEThreadNewToDefault
#define CODEThreadSchdHookRegister               CODE_CODEThreadSchdHookRegister
#define CODEThreadeStateHookRegister             CODE_CODEThreadStateHookRegister
#define CODEThreadMpark                          CODE_CODEThreadMpark
#define CODEThreadPark                           CODE_CODEThreadPark
#define CODEThreadWait                           CODE_CODEThreadWait
#define CODEThreadResumeAndWait                  CODE_CODEThreadResumeAndWait
#define CODEThreadMresched                       CODE_CODEThreadMresched
#define CODEThreadResched                        CODE_CODEThreadResched
#define CODEThreadPreemptResched                 CODE_CODEThreadPreemptResched
#define CODEThreadReady                          CODE_CODEThreadReady
#define CODEThreadAddBatch                       CODE_CODEThreadAddBatch
#define CODEThreadId                             CODE_CODEThreadId
#define CODEThreadGetId                          CODE_CODEThreadGetId
#define CODEThreadGetHandle                      CODE_CODEThreadGetHandle
#define GetCODEThreadStackInfo                   CODE_GetCODEThreadStackInfo
#define CODEThreadSetName                        CODE_CODEThreadSetName
#define CODEThreadGetInfo                        CODE_CODEThreadGetInfo
#define CODEThreadStackReversedSet               CODE_CODEThreadStackReversedSet
#define CODEThreadStackReversedGet               CODE_CODEThreadStackReversedGet
#define CODEThreadStackGuardExpand               CODE_CODEThreadStackGuardExpand
#define CODEThreadStackGuardRecover              CODE_CODEThreadStackGuardRecover
#define CODEThreadAttrInit                       CODE_CODEThreadAttrInit
#define CODEThreadAttrStackSizeSet               CODE_CODEThreadAttrStackSizeSet
#define CODEThreadAttrNameSet                    CODE_CODEThreadAttrNameSet
#define CODEThreadAttrCodeFromCSet                 CODE_CODEThreadAttrCODEFromCSet
#define CODEThreadNewSetLocalData                CODE_CODEThreadNewSetLocalData
#define CODEThreadNewSetAttr                     CODE_CODEThreadNewSetAttr
#define CODEThreadAttrSpecificSet                CODE_CODEThreadAttrSpecificSet
#define CODEBindOSThread                         CODE_BindOSThread
#define CODEUnbindOSThread                       CODE_UnbindOSThread
#define CODEThreadStackGuardGet                  CODE_CODEThreadStackGuardGet
#define CODEThreadStackSizeGet                   CODE_CODEThreadStackSizeGet
#define CODEThreadStackAddrGet                   CODE_CODEThreadStackAddrGet
#define CODEThreadStackBaseAddrGet               CODE_CODEThreadStackBaseAddrGet
#define CODEThreadStackSizeGetByCODEThrd           CODE_CODEThreadStackSizeGetByCODEThrd
#define CODEThreadStackAddrGetByCODEThrd           CODE_CODEThreadStackAddrGetByCODEThrd
#define CODEThreadStackBaseAddrGetByCODEThrd       CODE_CODEThreadStackBaseAddrGetByCODEThrd
#define CODEThreadPreemptOffCntAdd               CODE_CODEThreadPreemptOffCntAdd
#define CODEThreadPreemptOffCntSub               CODE_CODEThreadPreemptOffCntSub
#define CODEThreadAndArgsMemAlloc                CODE_CODEThreadAndArgsMemAlloc
#define CODEThreadStackMemAlloc                  CODE_CODEThreadStackMemAlloc
#define CODEThreadStackMemFree                   CODE_CODEThreadStackMemFree
#define CODEThreadStackAttrInit                  CODE_CODEThreadStackAttrInit
#define CODEThreadDestructorHookRegister         CODE_CODEThreadDestructorHookRegister
#define CODEThreadGetMutatorStatusHookRegister   CODE_CODEThreadGetMutatorStatusHookRegister
#define CODEThreadGetMutator                     CODE_CODEThreadGetMutator
#define CODEThreadSetMutator                     CODE_CODEThreadSetMutator

#define LuaFuncWrap                            CODE_LuaFuncWrap
#define CODEThreadCreate                         CODE_CODEThreadCreate
#define CODEThreadYieldCallback                  CODE_CODEThreadYieldCallback
#define CODEThreadYield                          CODE_CODEThreadYield
#define CODEThreadResume                         CODE_CODEThreadResume
#define CODEThreadStateGet                       CODE_CODEThreadStateGet
#define CODEThreadDestroy                        CODE_CODEThreadDestroy
#define CODEThreadResultGet                      CODE_CODEThreadResultGet
#define CODEThreadGetArg                         CODE_CODEThreadGetArg
#define CODEThreadStackAdjust                    CODE_CODEThreadStackAdjust
#define CODEThreadStackGrow                      CODE_CODEThreadStackGrow
#define CODEThreadOldStackFree                   CODE_CODEThreadOldStackFree
#define CODEThreadSetStackGrow                   CODE_CODEThreadSetStackGrow

/* codethread_key */
#define g_codethreadKeys                         CODE_GCODEThreadKeys

#define CODEThreadKeyCreateInner                 CODE_CODEThreadKeyCreateInner
#define CODERegisterSubStackInfoCallbacks        CODE_CODERegisterSubStackInfoCallbacks
#define CODERegisterExternalVMInRuntime          CODE_CODERegisterExternalVMInRuntime
#define CODEThreadSetspecificInner               CODE_CODEThreadSetSpecificInner
#define CODEThreadGetspecificInner               CODE_CODEThreadGetSpecificInner
#define CODEThreadKeysClean                      CODE_CODEThreadKeysClean

/* processor */
#define g_randSeed                              CODE_GRandSeed

#define ProcessorGlobalWrite                    CODE_ProcessorGlobalWrite
#define ProcessorLocalWriteBatch                CODE_ProcessorLocalWriteBatch
#define ProcessorGlobalRead                     CODE_ProcessorGlobalRead
#define ProcessorLocalRead                      CODE_ProcessorLocalRead
#define ProcessorLocalWrite                     CODE_ProcessorLocalWrite
#define ProcessorQueueSteal                     CODE_ProcessorQueueSteal
#define ProcessorCODEThreadSteal                  CODE_ProcessorCODEThreadSteal
#define ProcessorRelease                        CODE_ProcessorRelease
#define ProcessorStopWithLastCheck              CODE_ProcessorStopWithLastCheck
#define RandSeedInit                            CODE_RandSeedInit
#define RandomPseudo                            CODE_RandomPseudo
#define ProcessorTimerSteal                     CODE_ProcessorTimerSteal
#define ProcessorSteal                          CODE_ProcessorSteal
#define ProcessorSearchingSteal                 CODE_ProcessorSearchingSteal
#define ProcessorSearchingGlobal                CODE_ProcessorSearchingGlobal
#define ProcessorTimerCheck                     CODE_ProcessorTimerCheck
#define ProcessorCODEThreadGet                    CODE_ProcessorCODEThreadGet
#define ProcessorSchedule                       CODE_ProcessorSchedule
#define ProcessorInit                           CODE_ProcessorInit
#define ProcessorFree                           CODE_ProcessorFree
#define ProcessorNonDefaultScheduleWake         CODE_ProcessorNonDefaultScheduleWake
#define ProcessorWake                           CODE_ProcessorWake
#define ProcessorAlloc                          CODE_ProcessorAlloc
#define ProcessorThreadExit                     CODE_ProcessorThreadExit
#define ProcessorGetspecific                    CODE_ProcessorGetSpecific
#define ProcessorSetspecific                    CODE_ProcessorSetSpecific
#define ProcessorGetInfoAll                     CODE_ProcessorGetInfoAll
#define ProcessorFreelistPush                   CODE_ProcessorFreelistPush
#define ProcessorFreelistPop                    CODE_ProcessorFreelistPop
#define ProcessorFreelistPut                    CODE_ProcessorFreelistPut
#define ProcessorFreelistGet                    CODE_ProcessorFreelistGet
#define ProcessorStartBoundCODEThread             CODE_ProcessorStartBoundCODEThread
#define ProcessorStopBoundCODEThread              CODE_ProcessorStopBoundCODEThread
#define ProcessorNewId                          CODE_ProcessorNewId
#define ProcessorId                             CODE_ProcessorId
#define ProcessorCanSpin                        CODE_ProcessorCanSpin

/* schdpoll */
#define SchdpollInit                            CODE_SchdpollInit
#define SchdpollAdd                             CODE_SchdpollAdd
#define SchdpollDel                             CODE_SchdpollDel
#define SchdpollWaitPark                        CODE_SchdpollWaitPark
#define SchdpollWait                            CODE_SchdpollWait
#define SchdpollReady                           CODE_SchdpollReady
#define SchdpollAcquireCallback                 CODE_SchdpollAcquireCallback
#define SchdpollAcquire                         CODE_SchdpollAcquire
#define SchdpollCallbackCODEThread                CODE_SchdpollCallbackCODEThread
#define SchdpollNotifyCallback                  CODE_SchdpollNotifyCallback
#define SchdpollCallbackAdd                     CODE_SchdpollCallbackAdd
#define SchdpollNotifyAdd                       CODE_SchdpollNotifyAdd
#define SchdpollNotifyDel                       CODE_SchdpollNotifyDel
#define SchdpollInnerFd                         CODE_SchdpollInnerFd
#define SchdpollFreePd                          CODE_SchdpollFreePd

/* schedule */
#define g_schdAttr                              CODE_GSchdAttr
#define g_scheduleManager                       CODE_GScheduleManager
#define g_timerHookFunc                         CODE_GTimerHookFunc
#define g_pageSize                              CODE_GPageSize
#define g_tryExit                               CODE_GTryExit

#define SetSchedulerState                       CODE_SetSchedulerState
#define ScheduleAttributeGet                    CODE_ScheduleAttributeGet
#define ScheduleAttrInit                        CODE_ScheduleAttrInit
#define ScheduleAttrCostackSizeSet              CODE_ScheduleAttrCostackSizeSet
#define ScheduleAttrThstackSizeSet              CODE_ScheduleAttrThstackSizeSet
#define ScheduleAttrProcessorNumSet             CODE_ScheduleAttrProcessorNumSet
#define ScheduleAttrStackProtectSet             CODE_ScheduleAttrStackProtectSet
#define ScheduleAttrStackGrowSet                CODE_ScheduleAttrStackGrowSet
#define ScheduleAttrRegisterFuncSet             CODE_ScheduleAttrRegisterFuncSet
#define ScheduleRecursiveLockCreate             CODE_ScheduleRecursiveLockCreate
#define ScheduleProcessorInit                   CODE_ScheduleProcessorInit
#define ScheduleThreadInit                      CODE_ScheduleThreadInit
#define ScheduleGfreelistInit                   CODE_ScheduleGfreelistInit
#define ScheduleCODEThreadInit                    CODE_ScheduleCODEThreadInit
#define ScheduleThread0Init                     CODE_ScheduleThread0Init
#define ScheduleAttrCheck                       CODE_ScheduleAttrCheck
#define ScheduleAlloc                           CODE_ScheduleAlloc
#define ScheduleThread0Fini                     CODE_ScheduleThread0Fini
#define ScheduleCODEThreadFini                    CODE_ScheduleCODEThreadFini
#define ScheduleThreadFini                      CODE_ScheduleThreadFini
#define ScheduleFree                            CODE_ScheduleFree
#define ScheduleFini                            CODE_ScheduleFini
#define ScheduleManagerInit                     CODE_ScheduleManagerInit
#define ScheduleManagerDestroy                  CODE_ScheduleManagerDestroy
#define ScheduleNew                             CODE_ScheduleNew
#define ScheduleNetpollInit                     CODE_ScheduleNetpollInit
#define ScheduleNetpollDestroy                  CODE_ScheduleNetpollDestroy
#define ScheduleStart                           CODE_ScheduleStart
#define ScheduleExistTask                       CODE_ScheduleExistTask
#define ScheduleStartNoWait                     CODE_ScheduleStartNoWait
#define ScheduleNonDefaultThreadExit            CODE_ScheduleNonDefaultThreadExit
#define ScheduleThreadsFree                     CODE_ScheduleThreadsFree
#define ScheduleProcessorFree                   CODE_ScheduleProcessorFree
#define ScheduleSchmonExit                      CODE_ScheduleSchmonExit
#define ScheduleCODEThreadFree                    CODE_ScheduleCODEThreadFree
#define ScheduleNetpollExit                     CODE_ScheduleNetpollExit
#define ScheduleAnyCODEThreadRunning              CODE_ScheduleAnyCODEThreadRunning
#define ScheduleNonDefaultFree                  CODE_ScheduleNonDefaultFree
#define ScheduleProcessorSkipFFI                CODE_ScheduleProcessorSkipFFI
#define ScheduleAllNonDefaultExit               CODE_ScheduleAllNonDefaultExit
#define ScheduleProcessorExit                   CODE_ScheduleProcessorExit
#define ScheduleExitMode                        CODE_ScheduleExitMode
#define ScheduleStop                            CODE_ScheduleStop
#define ScheduleStopOutside                     CODE_ScheduleStopOutside
#define ScheduleTryExit                         CODE_ScheduleTryExit
#define ScheduleClean                           CODE_ScheduleClean
#define ScheduleGlobalWrite                     CODE_ScheduleGlobalWrite
#define ScheduleAnyCODEThread                     CODE_ScheduleAnyCODEThread
#define ScheduleCODEThreadCount                   CODE_ScheduleCODEThreadCount
#define ScheduleCODEThreadCountPublic             CODE_ScheduleCODEThreadCountPublic
#define ScheduleRunningOSThreadCount            CODE_ScheduleRunningOSThreadCount
#define SchdProcessorHookRegister               CODE_SchdProcessorHookRegister
#define SchdSchmonHookRegister                  CODE_SchdSchmonHookRegister
#define SchdExitHookRegister                    CODE_SchdExitHookRegister
#define SchdCheckExistenceHookRegister          CODE_SchdCheckExistenceHookRegister
#define SchdCheckReadyHookRegister              CODE_SchdCheckReadyHookRegister
#define ScheduleTimerHookRegister               CODE_ScheduleTimerHookRegister
#define ScheduleGfreelistPush                   CODE_ScheduleGfreelistPush
#define ScheduleGfreelistPop                    CODE_ScheduleGfreelistPop
#define ScheduleGfreelistGet                    CODE_ScheduleGfreelistGet
#define ScheduleListAdd                         CODE_ScheduleListAdd
#define ScheduleListRemove                      CODE_ScheduleListRemove
#define ScheduleAllThreadListAdd                CODE_ScheduleAllThreadListAdd
#define ScheduleAllCODEThreadListAdd              CODE_ScheduleAllCODEThreadListAdd
#define ScheduleAllCODEThreadListRemove           CODE_ScheduleAllCODEThreadListRemove
#define ScheduleAllCODEThreadVisitImpl            CODE_ScheduleAllCODEThreadVisitImpl
#define ScheduleAllCODEThreadVisit                CODE_ScheduleAllCODEThreadVisit
#define ScheduleAllCODEThreadVisitMutator         CODE_ScheduleAllCODEThreadVisitMutator
#define ScheduleAnyCODEThreadOrTimer              CODE_ScheduleAnyCODEThreadOrTimer
#define ScheduleLockAll                         CODE_ScheduleLockAll
#define ScheduleUnlockAll                       CODE_ScheduleUnlockAll
#define ScheduleSuspend                         CODE_ScheduleSuspend
#define ScheduleResume                          CODE_ScheduleResume
#define SchedulePreemptCheck                    CODE_SchedulePreemptCheck
#define ScheduleIsRunning                       CODE_ScheduleIsRunning
#define ScheduleSetToCurrentThread              CODE_ScheduleSetToCurrentThread
#define ScheduleGetProcessorNum                 CODE_ScheduleGetProcessorNum
#define ScheduleTraceDlclose                    CODE_ScheduleTraceDlclose
#define ScheduleStartTrace                      CODE_ScheduleStartTrace
#define ScheduleStopTrace                       CODE_ScheduleStopTrace
#define ScheduleDumpTrace                       CODE_ScheduleDumpTrace
#define ScheduleGetTraceReader                  CODE_ScheduleGetTraceReader
#define ScheduleTraceEventOrigin                CODE_ScheduleTraceEventOrigin
#define ScheduleTraceEvent                      CODE_ScheduleTraceEvent

/* windows global value */
#define g_localTlsOffset                        CODE_GLocalTlsOffset

/* tls dynmic */
#define ScheduleGetTlsHookRegister              CODE_ScheduleGetTlsHookRegister
#define g_getTlsFunc                            CODE_GGetTlsFunc

/* schmon */
#define SchmonPreemptSyscall                    CODE_SchmonPreemptSyscall
#define SchmonPreemptRunning                    CODE_SchmonPreemptRunning
#define SchmonProcessorPreemptCheck             CODE_SchmonProcessorPreemptCheck
#define SchmonCheckAllprocessors                CODE_SchmonCheckAllProcessors
#define SchmonRemovelistClear                   CODE_SchmonRemovelistClear
#define SchmonCODEThreadPoolClean                CODE_SchmonCODEThreadPoolClean
#define SchmonEntry                             CODE_SchmonEntry
#define SchmonStart                             CODE_SchmonStart

/* thread */
#define ThreadSleep                              CODE_ThreadSleep
#define ThreadCoreBind                           CODE_ThreadCoreBind
#define ThreadPreemptFlagInit                    CODE_ThreadPreemptFlagInit
#define ThreadEntry                              CODE_ThreadEntry
#define ThreadOsnew                              CODE_ThreadOsNew
#define ThreadCreate                             CODE_ThreadCreate
#define ThreadFromSchedule                       CODE_ThreadFromSchedule
#define ThreadAlloc                              CODE_ThreadAlloc
#define ThreadAllocBindProcessor                 CODE_ThreadAllocBindProcessor
#define ThreadStop                               CODE_ThreadStop
#define ThreadBindProcessor                      CODE_ThreadBindProcessor

/* codethread_context.S */
#define CODEThreadMcall                           CODE_CODEThreadMcall
#define CODEThreadMcallNosave                     CODE_CODEThreadMcallNosave
#define CODEThreadExecute                         CODE_CODEThreadExecute
#define CODEThreadContextInit                     CODE_CODEThreadContextInit
#define CODEThreadContextGet                      CODE_CODEThreadContextGet
#define CODEThreadContextSet                      CODE_CODEThreadContextSet

/* timer */
#define g_timerNum                               CODE_GTimerNum

#define TimerDeadlineCheck                       CODE_TimerDeadlineCheck
#define TimerShiftUp                             CODE_TimerShiftUp
#define TimerShiftDown                           CODE_TimerShiftDown
#define TimerDoAdd                               CODE_TimerDoAdd
#define TimerHeapInit                            CODE_TimerHeapInit
#define TimerUpdateTimer0Ddl                     CODE_TimerUpdateTimer0Ddl
#define TimerRemove                              CODE_TimerRemove
#define TimerHeapTopAdjust                       CODE_TimerHeapTopAdjust
#define TimerAdd                                 CODE_TimerAdd
#define TimerStopAheadStateAdjust                CODE_TimerStopAheadStateAdjust
#define TimerStopStateAdjust                     CODE_TimerStopStateAdjust
#define TimerStop                                CODE_TimerStop
#define TimerResetDoChange                       CODE_TimerResetDoChange
#define TimerResetHeapAdjust                     CODE_TimerResetHeapAdjust
#define TimerResetStatusAdjust                   CODE_TimerResetStatusAdjust
#define TimerReset                               CODE_TimerReset
#define TimerTriggerHeapStateAdjust              CODE_TimerTriggerHeapStateAdjust
#define TimerTriggerHeapAdjust                   CODE_TimerTriggerHeapAdjust
#define TimerTimer0Run                           CODE_TimerTimer0Run
#define TimerTimer0StateAdjust                   CODE_TimerTimer0StateAdjust
#define TimerTimer0StateAdjustAndRun             CODE_TimerTimer0StateAdjustAndRun
#define TimerStoppedWaitingAdjust                CODE_TimerStoppedWaitingAdjust
#define TimerStoppedStateAdjust                  CODE_TimerStoppedStateAdjust
#define TimerStoppedClear                        CODE_TimerStoppedClear
#define TimerTriggerNowCheck                     CODE_TimerTriggerNowCheck
#define TimerTrigger                             CODE_TimerTrigger
#define TimerHeapInitAndAdd                      CODE_TimerHeapInitAndAdd
#define TimerInit                                CODE_TimerInit
#define TimerNew                                 CODE_TimerNew
#define TimerSleepCoreadyCallback                CODE_TimerSleepCoreadyCallback
#define TimerSleepCoparkCallback                 CODE_TimerSleepCoparkCallback
#define TimerSleep                               CODE_TimerSleep
#define TimerTryStop                             CODE_TimerTryStop
#define TimerExit                                CODE_TimerExit
#define TimerSchmonCheck                         CODE_TimerSchmonCheck
#define TimerCheckReady                          CODE_TimerCheckReady
#define TimerNum                                 CODE_TimerNum
#define ScheduleTimerHookInit                    CODE_ScheduleTimerHookInit
#define TimerRelease                             CODE_TimerRelease
#define TimerGetHeap                             CODE_TimerGetHeap
#define TimerStoppedDoRemove                     CODE_TimerStoppedDoRemove

/* basetime */
#define CurrentNanotimeGet                       CODE_CurrentNanotimeGet
#define CurrentCPUTicks                          CODE_CurrentCPUTicks

/* waitqueue */
#define WaitqueueNew                             CODE_WaitqueueNew
#define WaitqueueParkUnlock                      CODE_WaitqueueParkUnlock
#define WaitqueueCallback                        CODE_WaitqueueCallback
#define WaitqueuePark                            CODE_WaitqueuePark
#define WaitqueueWakeOne                         CODE_WaitqueueWakeOne
#define WaitqueueWakeAll                         CODE_WaitqueueWakeAll
#define WaitqueueDelete                          CODE_WaitqueueDelete
#define WaitqueueNodeNew                         CODE_WaitqueueNodeNew
#define WaitqueuePush                            CODE_WaitqueuePush
#define WaitqueueCallbackCheck                   CODE_WaitqueueCallbackCheck
#define WaitqueueWakePreparation                 CODE_WaitqueueWakePreparation
#define WaitqueueGetWaitNum                      CODE_WaitqueueGetWaitNum

/* syscall_impl */
#define SyscallEnter                             CODE_SyscallEnter
#define SyscallFastExit                          CODE_SyscallFastExit
#define SyscallExit0                             CODE_SyscallExit0
#define SyscallExit                              CODE_SyscallExit

/* syscall_linux */
#define SyscallRead                              CODE_SyscallRead
#define SyscallWrite                             CODE_SyscallWrite
#define SyscallReadv                             CODE_SyscallReadv
#define SyscallWritev                            CODE_SyscallWritev
#define SyscallClose                             CODE_SyscallClose
#define SyscallFcntl                             CODE_SyscallFcntl
#define SyscallDup                               CODE_SyscallDup
#define SyscallDup3                              CODE_SyscallDup3
#define SyscallSleep                             CODE_SyscallSleep
#define SyscallUsleep                            CODE_SyscallUsleep
#define SyscallNanosleep                         CODE_SyscallNanoSleep
#define SyscallConnect                           CODE_SyscallConnect
#define SyscallRecv                              CODE_SyscallRecv
#define SyscallRecvfrom                          CODE_SyscallRecvFrom
#define SyscallRecvmsg                           CODE_SyscallRecvMsg
#define SyscallSend                              CODE_SyscallSend
#define SyscallSendto                            CODE_SyscallSendTo
#define SyscallSendmsg                           CODE_SyscallSendMsg
#define SyscallGetsockopt                        CODE_SyscallGetSockOpt
#define SyscallSetsockopt                        CODE_SyscallSetSockOpt
#define SyscallGetaddrinfo                       CODE_SyscallGetAddrInfo
#define SyscallGetnameinfo                       CODE_SyscallGetNameInfo
#define SyscallFreeaddrinfo                      CODE_SyscallFreeAddrInfo
#define SyscallOpen                              CODE_SyscallOpen
#define SyscallDup2                              CODE_SyscallDup2
#define SyscallPoll                              CODE_SyscallPoll
#define SyscallSelect                            CODE_SyscallSelect

#endif

/**
 * @ingroup mid
 * @brief generate module number
 * @param  x   [IN]  module number
 */
#define MID_MAKE(x) ((0x1000 + (x)) << 16)

#endif // MRT_SCHEDULE_RENAME_H
