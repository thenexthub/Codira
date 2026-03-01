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


#ifndef MRT_CODETHREAD_H
#define MRT_CODETHREAD_H

#include <stdbool.h>
#include <atomic>
#ifdef __OHOS__
#include <vector>
#endif
#include <limits.h>
#include "schedule.h"
#include "list.h"
#include "thread.h"
#include "codethread_context.h"
#include "Mutator/Mutator.h"
#include "base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define CODETHREAD_KEYS_MAX 9
#define CODETHREAD_ARG_ALIGN (16)
#define CODETHREAD_INIT_ID (ULLONG_MAX)

#define CODETHREAD_SANITIZER_CONTEXT_OFFSET (16)

/**
* @brief The address is aligned upward.
* @param addr     [IN] Value to be aligned.
* @param align    [IN] Number of bytes to be aligned.
*/
#define STACK_ADDR_ALIGN_UP(addr, align) ((((uintptr_t)(addr) + (uintptr_t)((align) - 1)) & ~(uintptr_t)((align) - 1)))

/**
* @brief The address is aligned downwards.
* @param addr     [IN] Value to be aligned.
* @param align    [IN] Number of bytes to be aligned.
*/
#define STACK_ADDR_ALIGN_DOWN(addr, align) ((((uintptr_t)(addr)) & ~(uintptr_t)((align) - 1)))

/**
 * @brief Internal management structure of codethread local variables
 */
struct CODEThreadKeyInternal {
    std::atomic<unsigned int> count;
    std::atomic<uintptr_t> keyDestructor[CODETHREAD_KEYS_MAX];
};

/**
 * @brief Structure of the codethread stack
 * low address--------------------------------high address
 * -----------------------------------------
 * |              |                        |
 * | protect page |      codethread stack   |
 * |              |                        |
 * -----------------------------------------
 */
struct CODEThreadStack {
    char *protectAddr;                 /* low address of the stack protection page */
    size_t totalSize;                  /* totalSize includes protection page size, stack size */
    char *stackTopAddr;                /* stack top address */
    char *stackGuard;                  /* threshold for determining stack overflow. It is
                                        * stackTopAddr+Stack_REVERSED(a certain number of bytes
                                        * are reserved) */
    char *stackBaseAddr;               /* Stack bottom address of the applied stack.
                                        * It equals stackTopAddr+stackSize. Note: The address
                                        * range is an open interval. An address smaller than
                                        * stackBaseAddr belongs to the stack, and an address
                                        * equal to stackBaseAddr does not belong to the stack. */
    size_t stackSize;                  /* Specifies the stack size for creating a codethread,
                                        * excluding the stack protection size. */
    char *codethreadStackBaseAddr;       /* Actual stack bottom of codethread stack. It is equal
                                        * to stackAddr+stackAlign and is 16 bytes down. */
    unsigned int stackGrowCnt;         /* whether to enable codethread stack scaling.
                                        * The value 0 indicates that stack scaling is enabled,
                                        * and other values indicate that disabled. */
};

struct StackInfo {
    unsigned long long stackLimit = 0ULL;
    unsigned long long lastLeaveFrame = 0ULL;
};

/**
 * @brief codethread allocation type
 */
enum CODEThreadBuf {
    LOCAL_BUF,      /* The local cache control block is preferentially used. */
    GLOBAL_BUF,     /* The global cache control block is preferentially used. */
    NO_BUF          /* The cache control block is not used. codethread0 only. */
};

/**
 * @brief Internal structure of codethread attributes
 * @attention The size of the structure cannot exceed ATTR_MAX_SIZE
 */
struct CODEThreadAttrInner {
    size_t stackSize;                          /* stack size */
    bool codeFromC;                              /* create codethread directly from side C */
    bool named;
    char name[CODETHREAD_NAME_SIZE];             /* codethread name */
    bool hasSpecificData;
    void *specificData[CODETHREAD_KEYS_MAX];     /* local data */
};

/**
 * @brief codethread structure
 */
struct CODEThread {
    struct Dulink schdDulink;                /* Global scheduling queue for codethreads link to  */
    struct Thread *thread;                   /* The current thread. Note that the offset of
                                              * this field in the structure is used in the
                                              * codethread_context assembly. Therefore, do not
                                              * change the offset. */
    struct CODEThreadContext context;          /* The codethread context. Note that the offset of
                                              * this field in the structure is used in the
                                              * codethread_context assembly. Therefore, do not
                                              * change the offset. */
    std::atomic<CODEThreadState> state;        /* codethread state */
    MapleRuntime::Mutator *mutator;          /* mutator, use for gc */
    struct Schedule *schedule;               /* scheduler */
    struct Dulink allCODEThreadDulink;         /* Global manafement queue connected to all codethreads */
    struct Dulink codeSingleModeThreadDulink;
    struct CODEThreadStack stack;              /* codethread stack */
    CODEThreadFunc func;
    void *argStart;                          /* The header of the codethread stack is used to
                                              * store parameters, pointing to the start address
                                              of the parameters. */
    unsigned int argSize;
    int result;
    void *localData[CODETHREAD_KEYS_MAX];      /* codethread local data */
    struct Thread *boundThread;              /* bound thread of codethread */
    unsigned int preemptOffCnt;              /* the count bit to disable the preemption
                                              * function of the current codethread. If the
                                              * value is not 0, preemption is prohibited. */
    int coErrno;                             /* error code */
    bool isLuaCODEThread;
    unsigned long long int id;               /* codethread id. The offset of this field in the
                                              * structure is used for lock optimization in llvm.
                                              Do not change it. */
    char name[CODETHREAD_NAME_SIZE];           /* codethread name */
    bool isCODEThread0;
#ifdef __OHOS__
    unsigned int singleModelC2NCount;
    struct StackInfo stackInfo;
#endif
};

/**
 * @brief lua codethread is only used in lua2code
 */
struct LuaCODEThread {
    CODEThreadHandle codethread;
    LuaCODEThreadFunc func;
    void *arg;
    void *result;
    struct Semaphore sem;
    int state;
    struct CODEThreadAttr attrUser;
};

/**
 * @brief arg structure, which is used to transfer parameters of CODEThreadAlloc.
 */
struct ArgAttr {
    const void *argStart;                   /* arg pointer */
    size_t argSize;                         /* arg length */
};

/**
 * @brief Stack structure, which stores partial stack information.
 */
struct StackAttr {
    size_t stackSizeAlign;                  /* Size of the codethread stack to be allocated,
                                             * which needs to be page aligned. */
    bool stackGrow;                         /* Whether to enable stack scaling. */
};

/**
 * @brief Release the memory of the codethread.
 * @par Description: Releases the memory of the codethread, including the stack memory and the
 * internal memory of the codethread structure.
 * @param codethread    [IN] codethread to be released
 * @retval none.
 */
void CODEThreadMemFree(struct CODEThread *codethread);

/**
 * @brief Put the codethread control block into the free list.
 * @par Description: Release the codethread control block to the free list.
 * @param codethread    [IN] codethread to be operated
 * @param reuse    [IN] Whether to reuse
 * @retval none.
 */
void CODEThreadFree(struct CODEThread *codethread, bool reuse);

/**
 * @brief Allocate codethread control blocks.
 * @par Description: Allocates space for the codethread control block, initializes the codethread
 * stack information, and initializes the parameter start address and other fields.
 * @attention allocates extra space for the codethread control block for 16-byte alignment.
 * @param schedule    [IN] Home scheduling framework.
 * @param argStart    [IN] Start address of the codethread parameter.
 * @param argSize    [IN] Number of codethread parameters.
 * @param stackSize    [IN] Size of the codethread stack.
 * @param coBuf    [IN] If co_buf is true, search for the cache control block in the current
 * processor and increase the codethread count.
 * If co_buf is false, malloc is performed directly and the codethread count is not increased.
* @retval codethread
 */
struct CODEThread *CODEThreadAlloc(struct Schedule *schedule, struct ArgAttr *argAttr,
                               struct StackAttr *stackAttr, CODEThreadBuf coBuf);

/**
 * @brief Invoke the registered destructor to clear the local variables of the codethread.
 * codethread is the codethread to be cleared.
 * @par When a non-null pointer is stored in a local variable and a registered function is not
 * null, the function is called.
 * @attention Do not mix. Do not let the codethread free again after the user frees the memory.
 * @param codethread    [IN] codethread pointer
 */
void CODEThreadKeysClean(struct CODEThread *codethread);

struct CODEThread* CODEThreadBuild(ScheduleHandle schedule, const struct CODEThreadAttr *attrUser, CODEThreadFunc func,
                               const void *argStart, unsigned int argSize, bool isSignal = false);

/**
 * @brief Add codethreads to the queue in batches.
 * @par is used to invoke the netpoll function to add codethreads to the running queue in batches.
 * The codethread queue has been set to the ready state before this function is invoked.
 * @param list    [IN] List of codethreads added to the queue
 * @param num    [IN] Number of codethreads added to the queue
 * @retval 0 or error code
 */
int CODEThreadAddBatch(CODEThreadHandle *list, unsigned int num);

/**
 * @brief Generate codethread0 context
 * @param  codethread0        [IN]  codethread0
 */
void CODEThread0Make(struct CODEThread *codethread0);

/**
 * @brief Park the current codethread, execute the callback function, schedule the next codethread.
 * @par Park the current codethread, go to codethread0, and execute the callback function. If the
 * callback function returns 0, the callback function is successful. Schedule the next codethread.
 * If the return value of the callback function is not 0, the callback function fails and the
 * previous codethread is rolled back.
 * @param func    [IN] Callback executed before the codethread stops
 * @param waitReason    [IN] Reason for park, which is used for trace.
 * @param arg    [IN] Input callback parameter
 * @retval 0 or error code
 */
int CODEThreadPark(ParkCallbackFunc func, TraceEvent waitReason, void *arg);

/**
 * @brief Apply for the codethread stack memory.
 * @param schedule    [IN] Scheduler to which the codethread belongs
 * @param codethread    [IN] codethread structure
 * @param stackSizeAlign    [IN] New stack size (page alignment)
 * @param totalSize    [OUT] Total size of the codethread stack memory. If stackProtect is not
 * enabled, the value is equal to stackSizeAlign. An extra page of memory is displayed when
 * stackProtect is enabled.
 * @retval stack address
 */
char *CODEThreadStackMemAlloc(struct Schedule *schedule, struct CODEThread *codethread,
                            size_t stackSizeAlign, size_t *totalSize);

/**
 * @brief Free codethread stack
 * @param  codethread         [IN]  codethread
 * @param  stackTopAddr     [IN]  stack start address
 * @param  stackTotalSize   [IN]  stack total size
 */
void CODEThreadStackMemFree(struct CODEThread *codethread, char *stackTopAddr, size_t stackTotalSize);

/**
 * @brief Adjust the codethread stack and replace the old codethread stack with the new one.
 * @param codethread [IN] codethread structure
 * @param newStackSizeAlign[IN] New stack size (page alignment)
 * @retval If the operation is successful, the offsets of the new stack and old stack are
 * returned. If the operation fails, -1 is returned.
 */
intptr_t CODEThreadStackAdjust(struct CODEThread *codethread, size_t newStackSizeAlign);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_CODETHREAD_H */
