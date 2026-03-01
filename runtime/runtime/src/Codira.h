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


#ifndef CODIRA_H
#define CODIRA_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <functional>
#ifndef _WIN64
#include <csignal>
#endif
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifndef _WIN64
#include <signal.h>
#endif
#endif

/* C++ needs to know that types and declarations are C, not C++. */
#ifdef __cplusplus
#define CODIRA_RT_API_DECLS_BEGIN extern "C" {
#define CODIRA_RT_API_DECLS_END }
#else
#define CODIRA_RT_API_DECLS_BEGIN
#define CODIRA_RT_API_DECLS_END
#endif

#ifndef MRT_EXPORT
#define MRT_EXPORT __attribute__((visibility("default")))
#endif

#ifndef MRT_OPTIONAL_DISABLE_TAIL_CALL
#ifdef __OHOS__
#define MRT_OPTIONAL_DISABLE_TAIL_CALL __attribute__((disable_tail_calls))
#else
#define MRT_OPTIONAL_DISABLE_TAIL_CALL
#endif
#endif

#ifndef MRT_OPTIONAL_BRANCH_PROTECT_NONE
#if (defined(__OHOS__) && defined(__aarch64__)) || (defined(__ANDROID__) && defined(__aarch64__))
#define MRT_OPTIONAL_BRANCH_PROTECT_NONE __attribute__((target("branch-protection=none")))
#else
#define MRT_OPTIONAL_BRANCH_PROTECT_NONE
#endif
#endif

#ifdef __cplusplus
/*
 * @brief CODEThread handle returned by Codira CODEThread scheduler.
 * @attention Currently, the return value of CODEThread module is 'void *',
 * it would be changed into 'int' later.
 */
using CODEThreadHandle = void*;

/*
 * @brief CODEThread handle returned by Codira codethread scheduler.
 * @attention Currently, the return value of CODEThread module is 'void *',
 * it would be changed into 'int' later.
 */
using SemiCODEThreadHandle = void*;

/* Prototype function submitted to Codira codethread scheduler. */
using CODETaskFunc = void*(*)(void*);

/* Key of codethread specific data. */
using CODEThreadKey = unsigned int;

/* Destructor function for the value of codethread specific data. */
using DestructorFunc = void(*)(void*);

/* Post Task function is used for user's event handler system to handle code task. */
using PostTaskFunc = bool(*)(void*);

/* Check for higher priority tasks in event handler system. */
using HasHigherPriorityTaskFunc = bool(*)();

/* Update designate stackInfo */
using UpdateStackInfoFunc = void (*)(unsigned long long, void*, unsigned int);
#else
/*
 * @brief CODEThread handle returned by Codira codethread scheduler.
 * @attention Currently, the return value of CODEThread module is 'void *',
 * it would be changed into 'int' later.
 */
typedef void* CODEThreadHandle;

/*
 * @brief Semi-codethread handle returned by CreateCODEThread.
 * @attention Currently, SemiCODEThreadHandle is provided to lua2code project.
 * It simulates lua codethread's behavior, which means it is different from
 * CODEThreadHandle. Do not mix up CODEThreadHandle and SemiCODEThreadHandle.
 */
typedef void* SemiCODEThreadHandle;

/* Prototype function submitted to Codira codethread scheduler. */
typedef void* (*CODETaskFunc)(void*);

/* Key of codethread specific data. */
typedef unsigned int CODEThreadKey;

/* Destructor function for the value of codethread specific data. */
typedef void (*DestructorFunc)(void*);

/* Post Task function is used for user's event handler system to handle code task. */
typedef bool (*PostTaskFunc)(void*);

/* Check for higher priority tasks in event handler system. */
typedef bool (*HasHigherPriorityTaskFunc)(void);

/* Update designate stackInfo */
typedef void (*UpdateStackInfoFunc)(unsigned long long, void*, unsigned int);
#endif

/*
 * @struct CODEThreadSpecificData
 * @brief Data structure to store the codethread specific data,\n
 * including the key and value of codethread specific data.
 */
struct CODEThreadSpecificData {
    CODEThreadKey key;
    void* value;
};

#ifdef __cplusplus
struct CODEErrorObject {
    const char* name;
    const char* message;
    const char* stack;
};

struct CODEUncaughtExceptionInfo {
    const char* hapPath;
    std::function<void(const char* summary, const CODEErrorObject errorObj)> uncaughtTask;
};
#endif

/* CODEThread state type. */
enum CODEThreadState {
    CODETHREAD_IDLE,
    CODETHREAD_READY,
    CODETHREAD_RUNNING,
    CODETHREAD_PENDING,
    CODETHREAD_SYSCALL,
};

/* Semi CODEThread state type */
enum SemiCODEThreadState {
    SEMI_CODETHREAD_INIT,
    SEMI_CODETHREAD_SUSPENDING,
    SEMI_CODETHREAD_RUNNING,
    SEMI_CODETHREAD_DONE,
};

/* Log level for Codira runtime. */
enum RTLogLevel {
    RTLOG_VERBOSE,
    RTLOG_DEBUG,
    RTLOG_INFO,
    RTLOG_REPORT, // same as INFO, also output to VLOG(REPORT)
    RTLOG_WARNING,
    RTLOG_ERROR,
    RTLOG_FAIL,
    RTLOG_FATAL,
    RTLOG_OFF
};

enum RTErrorCode { E_OK = 0, E_ARGS = -1, E_TIMEOUT = -2, E_STATE = -3, E_FAILED = -4 };

/*
 * @struct HeapParam
 * @brief Data structure for Codira heap configuration parameters,\n
 * including the heap size, region size at runtime, and etc.
 */
struct HeapParam {
    /*
     * The reference value of region size, measured in KB, default to 64 KB, must be in range [4KB, 64KB].
     * It will be set to default value if assigned with 0.
     */
    size_t regionSize;

    /*
     * The maximum size of cangjie heap, measured in KB, default to 256 * 1024 KB, must >= 4MB.
     * It will be set to default value if assigned with 0.
     */
    size_t heapSize;

    /*
     * Threshold used to determine whether a region is exempted (i.e., will not be forwarded).
     * If the percentage of live objects in a region is greater than this value, this region will not be exempted.
     * Default to 0.8, must be in range (0, 1].
     * It will be set to default value if assigned with 0.
     */
    double exemptionThreshold;

    /*
     * A hint to guide collector to release physical memory to OS.
     * heap utilization = heap-used-memory / total-heap-memory.
     * During each gc, collector determines how much memory should be cached,
     * and let the heap utilization be close to this value.
     * Default to 0.80, must be in range (0, 1].
     * It will be set to default value if assigned with 0.
     */
    double heapUtilization;
    /*
     * The ratio to expand heap after each GC.
     * GC is probably triggered more often if this value is set to an improperly small number.
     * Default to 0.15, must > 0.
     * It will be set to default value if assigned with 0.
     */
    double heapGrowth;

    /*
     * The rate of allocating memory from heap.
     * this value is the lower bound of the real allocation rate.
     * allocator maybe wait some time if this value is set with an improperly small number.
     * Mesured in MB/s, default to 10240 MB/s, must be > 0 MB/s.
     * It will be set to default value if assigned with 0.
     */
    double allocationRate;

    /*
     * The maximum wait time when allocating memory from heap.
     * The latter allocation will wait a number of time if the two allocation interval is less than the wait time.
     * The real wait time is the minimum of allocationWaitTime and the wait time calculated from real allocation rate.
     * Measured in ns, default to 1000 ns, must > 0 ns.
     * It will be set to default value if assigned with 0.
     */
    size_t allocationWaitTime;
};

/*
 * @struct GCParam
 * @brief Data structure for Codira garbage collection configuration parameters,\n
 * including the garbage ratio, garbage collection interval and etc.
 */
struct GCParam {
    /*
     * GC will be triggered when heap allocated size is greater than this threshold.
     * Measured in KB, must be > 0.
     */
    size_t gcThreshold;

    /*
     * The threshold used to determine whether to collect from-space during GC.
     * The from-space will be collected if the percentage of the garbage in from space is greater than this threshold.
     * default to 0.5, must be in range [0.1, 1.0].
     */
    double garbageThreshold;

    /*
     * Minimum interval each GC request will be responded. If two adjacent GC requests with
     * interval less than this value, the latter one is ignored.
     * Measured in ns, default to 150 ms, must be > 0 ms.
     * It will be set default value if the value is 0.
     */
    uint64_t gcInterval;

    /*
     * Minimum interval each backup GC request will be responded.
     * Backup GC will be triggered if there is no GC during this interval.
     * Measured in ns, default to 240 s, must be > 0 s.
     * It will be set default value if the value is 0.
     */
    uint64_t backupGCInterval;

    /*
     * Parameters for adjusting the number of GC threads.
     * The number of gc threads is ((the hardware concurrency / this value) - 1).
     * default to 8, must be > 0.
     * It will be set default value if the value is 0.
     */
    int32_t gcThreads;
};

/*
 * @struct LogParam
 * @brief Data structure for Codira Log configuration parameters,\n
 * including the log size, log file level and etc.
 */
struct LogParam {
    /* Logging statements with level less than this value are ignored. Default to ERROR. */
    enum RTLogLevel logLevel;
};

/*
 * @struct ConcurrencyParam
 * @brief Data structure for thread and codethread configuration parameters,\n
 * including the default stack size for threads and codethread, numbers of processors,\n
 * and the maximum number of codethreads.
 */
struct ConcurrencyParam {
    /*
     * Thread stack size. Measured in KB, recommended value is 1 MB, must be > 0.
     * It will be set default value if the value is 0.
     */
    size_t thStackSize;

    /*
     * CODEThread stack size. Measured in KB, recommended value is 64KB, must be in range [64KB, 1GB].
     * It will be set default value if the value is 0.
     */
    size_t coStackSize;

    /*
     * Number of processors, recommended value is consistent with the number of CPU cores, must be > 0.
     * It will be set default value if the value is 0.
     */
    uint32_t processorNum;
};

/*
 * @struct RuntimeParam
 * @brief Data structure for Codira runtime parameters,\n
 * including the config information of heap, garbage collection, codethread and log.
 */
struct RuntimeParam {
    struct HeapParam heapParam;
    struct GCParam gcParam;
    struct LogParam logParam;
    struct ConcurrencyParam coParam;
};

CODIRA_RT_API_DECLS_BEGIN

/* ============ Interfaces to launch Codira runtime environment. ============*/

/*
 * @brief Initialize Codira runtime environment.
 * @par This API will create an instance of cangjie runtime.
 * @attention
 * @param  param  [IN] a structure pointer to config Codira runtime parameters.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode InitCODERuntime(const struct RuntimeParam* param);

/*
 * @brief register arkVM in Runtime
 * @par This API will record a external VM.
 * @attention
 * @param  param  [IN] arkVM address
 * @retval
 */
MRT_EXPORT void RegisterArkVMInRuntime(unsigned long long vm);

/*
 * @brief Create and init a Codira UI scheduler.
 * @par This API will create a single processor ui scheduler and return the scheduler handle.
 * @attention This API could not be used to create default scheduler.
 * @retval Return the scheduler handle if success. Otherwise, return NULL.
 */
MRT_EXPORT void* InitUIScheduler(void);

/*
 * @brief Create and init a Codira UI scheduler.
 * @par This API will start a ui-scheduler that created by `InitUIScheduler`.
 * @attention This API should be invoked on the same thread that `InitUIScheduler` called.
 * @param  timeout  [IN] Timeout interval, in milliseconds. If the value is 0, the timeout interval is not set.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode RunUIScheduler(unsigned long long timeout);

/*
 * @brief Set Codira command line arguments.
 * @par This API will set the command line arguments of Codira program.
 * @attention This API should be used when Codira runtime is initialized.
 * @param  argc  [IN]  the count of arguments.
 * @param  argv  [IN]  the array of arguments.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode SetCODECommandLineArgs(int argc, const char* argv[]);

/*
 * @brief Finish Codira runtime environment.
 * @par This API will stop scheduler and finish cangjie runtime, and allowed to be used only once.
 * @attention Other threads are not allowed to depend on this function invocation result.
 * @param
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode FiniCODERuntime(void);

/*
 * @brief Set Codira runtime initialization flag.
 * @attention
 * @param
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode SetRuntimeInitFlag(void);

/*
 * @brief Set Codira runtime finish flag.
 * @attention
 * @param
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT enum RTErrorCode SetRuntimeFiniFlag(void);
/*
 * @brief Submit a task to Codira codethread scheduler.
 * @par This API will create a Codira codethread and return the codethread handle.
 * @attention The arg fi should be released by API ReleaseHandle.
 * @param  func  [IN]  user defined Codira task function prototype.
 * @param  args  [IN]  arguments of the Codira task function.
 * @retval Return the codethread handle if success. Otherwise, return NULL.
 */
MRT_EXPORT CODEThreadHandle RunCODETask(const CODETaskFunc func, void* args);
MRT_EXPORT CODEThreadHandle RunCODETaskSignal(const CODETaskFunc func, void* args);
/*
 * @brief Submit a task to specify Codira codethread scheduler.
 * @par This API will create a Codira codethread and return the codethread handle.
 * @attention The arg fi should be released by API ReleaseHandle.
 * @param  func     [IN]  user defined Codira task function prototype.
 * @param  args     [IN]  arguments of the Codira task function.
 * @param  schedule [IN]  specify codethread scheduler handle.
 * @retval Return the codethread handle if success. Otherwise, return NULL.
 */
MRT_EXPORT CODEThreadHandle RunCODETaskToSchedule(const CODETaskFunc func, void* args, void* schedule);

/*
 * @brief Submit a task with codethread specific data to Codira codethread scheduler.
 * @par This API will create a Codira codethread with codethread specific data,\n
 * and return the codethread handle.
 * @attention
 * @param  func  [IN]  user defined Codira task function prototype.
 * @param  args  [IN]  arguments of the Codira task function.
 * @param  data  [IN]  pointer to the codethread specific data.
 * @param  num   [IN]  number of codethread specific data, num <= CODETHREAD_KEYS_MAX (now defined to 8 in codethread).
 * @retval Return the codethread handle if success. Otherwise, return NULL.
 */
MRT_EXPORT CODEThreadHandle RunCODETaskWithLocal(const CODETaskFunc func, void* args, struct CODEThreadSpecificData* data,
                                             int num);

/* ===================== Interfaces for Lua2CODE Project. ===================== */

/*
 * @brief Submit a semi-codethread to Codira codethread scheduler.
 * @par This API will create a Codira codethread and return the semi-codethread handle.
 * @attention 1. This API is provided to lua2code project.
 *            2. SemiCODEThreadHandle should be released by API DestroyCODEThread.
 * @param  func  [IN]  user defined Codira task function prototype.
 * @param  args  [IN]  arguments of the Codira task function.
 * @param  data  [IN]  pointer to the codethread specific data.
 * @param  num   [IN]  number of codethread specific data, num <= CODETHREAD_KEYS_MAX (now defined to 8 in codethread).
 * @retval Return the semi-codethread handle if success. Otherwise, return nullptr.
 */
MRT_EXPORT SemiCODEThreadHandle CreateCODEThread(const CODETaskFunc func, void *arg, struct CODEThreadSpecificData* data,
                                             int num);

/*
 * @brief Suspend the execution of the calling semi-codethread, and the latest ResumeCODEThread
 *        in the main thread will return.
 * @attention 1. This API is provided to lua2code project.
 *            2. Only can be called in the semi-codethread's context.
 *            3. SuspendCODEThread will return only after main thread called ResumeCODEThread.
 * @retval Return 0 if success. Otherwise, return an error code.
 */
MRT_EXPORT int SuspendCODEThread(void);

/*
 * @brief Start or continue the execution of semi-codethread co.
 * @par   The first time you resume a codethread, it starts running its body.
 *        If the codethread has suspended, resume restarts it.
 * @attention 1. This API is provided to lua2code project.
 *            2. ResumeCODEThread will return only after co called SuspendCODEThread
 *               or finished its body.
 * @retval Return 0 if success. Otherwise, return an error code.
 */
MRT_EXPORT int ResumeCODEThread(SemiCODEThreadHandle co);

/*
 * @brief Get the state of semi-codethread co.
 * @attention 1. This API is provided to lua2code project.
 * @retval Return #SemiCODEThreadState if success. Otherwise, return an error code.
 */
MRT_EXPORT int GetCODEThreadState(SemiCODEThreadHandle co);

/*
 * @brief Get the result of semi-codethread co.
 * @attention 1. This API is provided to lua2code project.
 *            2. This API should be called after co finished.
 * @retval Return the return value of user func if success. Otherwise, return nullptr
 *         (maybe user func returns nullptr too).
 */
MRT_EXPORT void *GetCODEThreadResult(SemiCODEThreadHandle co);

/*
 * @brief destroy semi-codethread co.
 * @attention 1. This API is provided to lua2code project.
 *            2. This API should be called before co starts or after co finished, to
 *               avoid memory leak.
 *            3. Afther destoryed, co should not be used again.
 * @retval Return 0 if success. Otherwise, return an error code.
 */
MRT_EXPORT int DestoryCODEThread(SemiCODEThreadHandle co);

/*
 * @brief Suspend the schedule
 * @par This API will suspend the schedule, the current tasks continue to execute to the end,
   but does not allow to create new tasks.
 * @attention This API should be used when schedule's state is running.
 * @param
 * @retval Return 0 if successfully suspend the schedule. Otherwise, return an error code.
 */
MRT_EXPORT int SuspendSchedule(void);

/*
 * @brief Resume the schedule
 * @par This API will resume the schedule, allow to create new tasks.
 * @attention This API should be used when schedule's state is suspending.
 * @param
 * @retval Return 0 if successfully resume the schedule. Otherwise, return an error code.
 */
MRT_EXPORT int ResumeSchedule(void);

/*
 * @brief Query whether a task is currently exists.
 * @attention
 * @param
 * @retval Return Ture if any tasks exists. Otherwise, return False.
 */
MRT_EXPORT bool AnyTask(void);

/*
 * @brief The func type which passed into NetPollNotifyAdd func.
 * @attention
 */
typedef unsigned int (*NetPollNotifyFunc)(int fd, int event, void* arg);

// NetPollNotifyAdd and NetPollNotifyDel was used for datacom, not available on Windows for the moment.
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
/*
 * @brief Add one fd into epoll, call the func when events happen.
 * @attention Only accept C func
 * @param  fd     [IN] the fd user want add to epoll
 * @param  events [IN] the events user interest
 * @param  func   [IN] the func to call when events happen
 * @param  arg    [IN] the argument of func
 * @retval Return poll handle if successfully add the fd into epoll. Otherwise, return nullptr.
 */
MRT_EXPORT void* NetPollNotifyAdd(int fd, int events, NetPollNotifyFunc func, void* arg);

/*
 * @brief Delete one fd in epoll.
 * @attention
 * @param  fd     [IN] the fd user want delete in epoll.
 * @param  pd     [IN] the handle which NetPollNotifyAdd return.
 * @retval Return 0 if successfully delete the fd in epoll. Otherwise, return an error code.
 */
MRT_EXPORT int NetPollNotifyDel(int fd, void* pd);
#endif

/*
 * @brief Get the results of the corresponding codethread in blocking mode.
 * @attention
 * @param  handle  [IN]  specified codethread handle.
 * @param  ret     [IN]  results of the given Codira task.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int GetTaskRet(const CODEThreadHandle handle, void** ret);

/*
 * @brief Get the results of the corresponding codethread in blocking mode with timeout.
 * @attention
 * @param  handle  [IN]  specified codethread handle.
 * @param  ret     [IN]  results of the given Codira task.
 * @param  timeout [IN]  given timeout, measured in milliseconds. If `timeout` <= 0,\n
 * its behavior is the same as `GetTaskRet(handle, ret)`.
 * @retval Return 0 if successfully get the returned results of codethread\n
 * within the given timeout. Otherwise, return an error code.
 */
MRT_EXPORT int GetTaskRetWithTimeout(const CODEThreadHandle handle, void** ret, int64_t timeout);

/*
 * @brief Release the corresponding codethread handle.
 * @attention
 * @param  handle  [IN]  specified codethread handle.
 * @retval
 */
MRT_EXPORT void ReleaseHandle(const CODEThreadHandle handle);

/* ============== Interfaces to handle codethread specific data. ============== */

/*
 * @brief Create the key of codethread specific data.
 * @attention If `destructor` is NULL, the user manages the memory and is responsible for the\n
 * free of the value of codethread specific data. If `destructor` is non-null, the codethread\n
 * framework is responsible for destroying the value of codethread specific data. When the codethread\n
 * stops, if the stored pointer of codethread specific data is non-null, and the `destructor` is\
 * non-null, the destruction function will be called. Must be careful not to mix them. Must avoid\n
 * the case that the codethread free again after the user manually free the memory of codethread\n
 * specific data. You can create CODETHREAD_KEYS_MAX(now defined to 8 in codethread)keys at most.
 * @param  key         [IN]  a pointer to the key of codethread specific data.
 * @param  destructor  [IN]  destructor function for the value of codethread specific data.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int CODEThreadKeyCreate(CODEThreadKey* key, DestructorFunc destructor);

/*
 * @brief Set codethread specific data.
 * @par This API stores `value` in the codethread specific data slot identified by `key`.
 * @param  key    [IN]  key of codethread specific data, got from CODEThreadKeyCreate.
 * @param  value  [IN]  value of the codethread specific data.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int CODEThreadSetspecific(CODEThreadKey key, void* value);

/*
 * @brief Get the value of codethread specific data slot identified by key.
 * @param  key  [IN]  key of codethread specific data, got from CODEThreadKeyCreate.
 * @retval Return the current value of codethread specific data slot identified by key\n
 * if OK. Otherwise, return NULL.
 */
MRT_EXPORT void* CODEThreadGetspecific(CODEThreadKey key);

/*
 * @brief Register Event handler Callbacks to global schedule manager.
 * PostTaskFunc is used for user's event handler system to handle code task.
 * HasHigherPriorityTaskFunc is used for checking for higher priority tasks in event handler system.
 * @param  pFunc  [IN]  PostTask function that defined by user.
 * @param  hFunc  [IN]  HasHigherPriorityTask function that defined by user.
 */
MRT_EXPORT void RegisterEventHandlerCallbacks(PostTaskFunc pFunc, HasHigherPriorityTaskFunc hFunc);

/*
 * @brief Register UpdateStackInfo Func to global scheduler manager.
 * UpdateStackInfo is used for update arkts stack info when single model codethread stack changed.\
 * @param  uFunc  [IN]  UpdateStackInfo function that defined by user.
 */
MRT_EXPORT void RegisterStackInfoCallbacks(UpdateStackInfoFunc uFunc);

/* ============= Interfaces to manage Codira dynamic libraries. ============= */

/*
 * @brief Load Codira dynamic shared library.
 * @par This API will load Codira dynamic shared library and parse symbols.
 * @attention This no need to launch Codira runtime environment before calling this API if only need to load library.\n
 * Suggest to use LoadCODELibraryWithInit if there is need to run cangjie func later.
 * @param  libName  [IN]  name of Codira dynamic shared library.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int LoadCODELibrary(const char* libName);

/*
 * @brief Initialize Codira dynamic shared library.
 * @par This API will invoke `package_global_init` to initialize Codira dynamic shared libraries.\n
 * Since `LoadCODELibrary` is provided by `libcangjie-elfloader.so`, which is not dependent with\n
 * Codira runtime, and it will be implemented in elibc later. Therefore, the API needs to be\n
 * temporarily extracted out separately.
 * @attention Make sure Codira runtime environment must have been launched before calling this API.
 * @param  libName  [IN]  name of Codira dynamic shared library.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int InitCODELibrary(const char* libName);

/*
 * @brief Load and initialize Codira dynamic shared library.
 * @par This API will load Codira dynamic shared library, parse symbols, and invoke `package_global_init`\n
 * to initialize Codira dynamic shared libraries.
 * @attention Make sure Codira runtime environment must have been launched before calling this API.
 * @param  libName  [IN]  name of Codira dynamic shared library.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int LoadCODELibraryWithInit(const char* libName);

/*
 * For example (Find Canjie Func address and execute this func):
 * if (InitCODERuntime(&param) != E_OK) {
 *     fprintf(stderr, "failed to init.\n");
 * }
 * if (LoadCODELibraryWithInit(libName) != E_OK) {
 *     fprintf(stderr, "failed to load.\n");
 * }
 * void *funcInCode = FindCODESymbol(libName, "funcInCode");
 * if (funcInCode == NULL) {
 *     fprintf(stderr, "failed to find.\n");
 * }
 * if (RunCODETask((void *(*)(void *))funcInCode, NULL) == NULL) {
 *     fprintf(stderr, "failed to new task.\n");
 * }
 * @brief Find symbol address from Codira dynamic shared library.
 * @par This API takes a dynamic loaded Codira shared object along with a null-terminated symbol\n
 * name, and returns the address where that symbol is loaded into memory.
 * @attention Make sure Codira dynamic shared library has been loaded before calling this API. This no need to\n
 * launch Codira runtime environment and initialize cangjie dynamic shared library if only to find symbol address.
 * However, make sure Codira runtime environment must has been launched and this Codira dynamic shared\n
 * library has been initialized if the finded func symbol need to be executed as example.
 * @param  libName     [IN]  name of Codira dynamic shared library.
 * @param  symbolName  [IN]  symbol name of a Codira function.
 * @retval Return the address where that symbol is loaded into memory if found. Otherwise,\n
 * return NULL.
 */
MRT_EXPORT void* FindCODESymbol(const char* libName, const char* symbolName);

/*
 * @brief Unload Codira dynamic shared library.
 * @attention This interface must be used carefully. Make sure there is no global variables and any objects belonging to
 * this shared library are used before calling this API.
 * @param  libName  [IN]  name of Codira dynamic shared library.
 * @retval Return 0 if OK. Otherwise, return an error code.
 */
MRT_EXPORT int UnloadCODELibrary(const char* libName);

#ifndef _WIN64
/*
 * @struct SignalAction
 * @brief Data structure to store the signal data,\n
 * including signal handler and other signal params.
 */
struct SignalAction {
    union {
        void (*saHandler)(int);
        bool (*saSignalAction)(int, siginfo_t*, void*);
    };
    sigset_t scMask;
    uint64_t scFlags;
};

/* ================= Interfaces to manage singal in Codira. ================= */

/*
 * @brief Add a new handle to the signal queue.
 * @attention The new signal handler will try to execute at the end.
 * @param  signal  [IN]  target signal.
 * @param  sa  [IN]  The signal action to be registered..
 * @retval.
 */
MRT_EXPORT void AddHandlerToSignalStack(int signal, struct SignalAction* sa);

/*
 * @brief Remove an existing handler from the signal queue.
 * @param  signal  [IN]  target signal.
 * @param  fn  [IN]  Signal handler to be removed.
 * @retval.
 */
MRT_EXPORT void RemoveHandlerFromSignalStack(int signal, bool (*fn)(int, siginfo_t*, void*));
#endif

/*
 * @brief Register uncaught exception handler.
 * @param  handle  [IN]  The cangjie uncaught ecxeption info to be registered.
 * @retval.
 */
#ifdef __cplusplus
MRT_EXPORT void RegisterUncaughtExceptionHandler(const CODEUncaughtExceptionInfo& handle);
#endif

/*
 * @brief Parse a mangel symbol into a demangle symbol.
 * @param  functionName  [IN]  mangle symbol.
 * @retval Return demangled name.
 */
MRT_EXPORT char* CODE_MRT_DemangleHandle(const char* functionName);

/*
 * @brief Dump Codira Heap.
 * @param  fd  [IN]  file descriptor.
 * @retval.
 */
MRT_EXPORT void CODE_MRT_DumpHeapSnapshot(int fd);

/*
 * @brief Trigger GC.
 * @retval.
 */
MRT_EXPORT void CODE_MRT_ForceFullGC();

CODIRA_RT_API_DECLS_END

#endif // CODIRA_H
