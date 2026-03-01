/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.concurrency;

import { InteropSerializeHelper } from "std/interop";

export namespace taskpool {
    const TASKPOOL_ERROR_NOT_IN_TASKPOOL: int = 10200022;
    const TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED: int = 10200024;
    const TASKPOOL_ERROR_CIRCULAR_DEPENDENCY: int = 10200026;
    const TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST: int = 10200027;
    const TASKPOOL_ERROR_DELAY_TIME_NEGATIVE: int = 10200028;
    const TASKPOOL_ERROR_LISTENER_REGISTRATION: int = 10200034;
    const TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY: int = 10200070;
    const TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY: int = 10200071;
    const TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE: int = 10200072;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY: int = 10200073;
    const TASKPOOL_ERROR_GROUP_TASK_RELIED_ON: int = 10200074;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE: int = 10200075;
    const TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED: int = 10200076;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP: int = 10200077;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED: int = 10200078;
    const TASKPOOL_ERROR_SEQ_RUNNER_OR_ASYNC_RUNNER_TASK_RELIED_ON: int = 10200079;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE: int = 10200080;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED: int = 10200081;
    const TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED: int = 10200082;
    const TASKPOOL_ERROR_LONG_TASK_MULTIPLE_EXECUTIONS: int = 10200083;
    const TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED: int = 10200084;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC: int = 10200085;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC: int = 10200086;
    const TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG: int = 10200087;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK: int = 10200088;
    const TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP: int = 10200089;
    const TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC: int = 10200090;
    const TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED: int = 10200091;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_DELAYED: int = 10200092;
    const TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN: int = 10200093;
    const TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED: int = 10200094;
    const TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER: int = 10200095;
    const TASKPOOL_ERROR_PERIOD_NEGATIVE: int = 10200096;
    const TASKPOOL_ERROR_NO_DEPENDENCY: int = 10200097;
    const TASKPOOL_ERROR_TASK_CANCELED: int = 10200098;
    const TASKPOOL_ERROR_GROUP_CANCELED: int = 10200099;
    const TASKPOOL_ERROR_SEQ_RUNNER_CANCELED: int = 10200100;
    const TASKPOOL_ERROR_NO_PARAMS: int = 10200101;
    const TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP: int = 10200102;
    const TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER: int = 10200103;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_GROUP: int = 10200104;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_EXECUTED: int = 10200105;
    const TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_SQUENCE: int = 10200106;
    const TASKPOOL_ERROR_PERIODIC_TASK_ASYNC_RUNNER: int = 10200107;
    const TASKPOOL_ERROR_ASYNC_RUNNER_PARAM: int = 10200108;
    const TASKPOOL_ERROR_ASYNC_RUNNER_WAITING_QUEUE_DROPPED: int = 10200109;
    const TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED: int = 10200110;
    const TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTED: int = 10200111;
    const TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_DELAYED: int = 10200112;
    const TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY: int = 10200113;

    class GlobalQueueWorker extends EAWorker {
        public static closingTask: Task = new Task("closingTask", (): void => {});

        constructor (supportInterop: boolean) {
            super((() => this.workerBody()) as Object, "taskpool", supportInterop, true);
            initGlobalTaskQueues();
            this.needInterop = supportInterop;
            this.start();
            this.workerId = this.getWorkerId()!.toInt();
        }

        constructor () {
            super("managerWorker", false);
            this.start();
        }

        private workerBody() {
            while (this.isWorkerActive()) {
                let task = getTaskFromGlobalQueue(this); // Now handles blocking internally to avoid CPU busy waiting
                if (task === undefined) {
                    continue;
                }
                removeFromIdleWorkers(this);

                const internalTask = InternalTask.of(task);
                this.currentPriority = internalTask?.priority;
                this.currentTasks.add(task);
                let count = getNewTaskTotalCount();
                this.idleTime = 0;

                try {
                    let res: Any = null;
                    res = internalTask?.execute(this);
                    ConcurrencyHelpers.lockGuard(internalTask!.taskMutex, () => {
                        internalTask!.currentDeferred!.resolvePromise(res);
                        internalTask!.currentDeferred = undefined;
                        if (internalTask!.pendingDeferreds!.length > 0) {
                            internalTask!.currentDeferred = internalTask!.pendingDeferreds!.shift();
                        }
                    });
                    //clear reference
                    res = null;
                } catch (e) {
                    ConcurrencyHelpers.lockGuard(internalTask!.taskMutex, () => {
                        // if task is seq task may rejected inside the execution, so here using ? to avoid null pointer
                        internalTask!.currentDeferred?.rejectPromise(e);
                        internalTask!.currentDeferred = undefined;
                        if (internalTask!.pendingDeferreds!.length > 0) {
                            internalTask!.currentDeferred = internalTask!.pendingDeferreds!.shift();
                        }
                    });
                } finally {
                    // this is to mock ArkTS-dyn behavior
                    EAWorker.postToMain<void>(() => {
                        setTimeout(() => {
                            internalTask!.notifyDependencies();
                        }, 1);
                    });
                }
            }
        }

        closeWorker() {
            this.setWorkerActive(false);
            ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                // notiy all to make sure the right one can be closed
                // other workers' idle time will update
                ConcurrencyHelpers.condVarNotifyAll(globalTaskCondVar, globalTaskQueuesMutex);
            });
            this.join();
        }

        public isWorkerActive(): boolean {
            let res: boolean = true;
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                res = this.isActive;
            });
            return res;
        }
        
        public setWorkerActive(active: boolean) {
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                this.isActive = active;
            });
        }
        public notifyNewTask() {
            ConcurrencyHelpers.lockGuard(this.isAliveMutex, () => {
                ConcurrencyHelpers.condVarNotifyOne(this.newTaskCondVar, this.isAliveMutex);
            });
        }

        // For managerWorker to expand.
        triggerExpand(): void {
            let expandJob = this.run<Any>((): Any => {
                let targetNum: int = 0;
                ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                    if (totalExecCount.get() != 0) {
                        let averageDuration: number = totalExecTime.get() / totalExecCount.get();
                        let result: number = (averageDuration * waitingTasksNum.get()) / TASK_DURATION;
                        targetNum = (min(result, waitingTasksNum.get())).toInt();
                    } else {
                        let waitingNum: int = waitingTasksNum.get();
                        targetNum = (min(waitingNum, STEP_SIZE)).toInt();
                    }
                });
                if (targetNum == 0) {
                    return 0;
                }
                ConcurrencyHelpers.lockGuard(workersMutex, () => {
                    let idleWorkersNum: int = 0;
                    for (let w : GlobalQueueWorker of workers) {
                        if(w.currentTasks.size.toInt() === 0) {
                            idleWorkersNum++;
                        }
                    }
                    
                    if (workers.size < workersLimit && idleWorkersNum < targetNum) {
                        let step: int = (Math.ceil(min(workersLimit - workers.size,
                                                   min(workersLimit, targetNum) - idleWorkersNum))).toInt();
                        for (let i = 0; i < step; ++i) {
                            let w: GlobalQueueWorker = new GlobalQueueWorker(isTaskpoolSupportInterop);
                            if (this.isAlive()) {
                                workers.add(w);
                            } else {
                                w.closeWorker();
                            }
                        }
                    }
                });
                return 1;
            });
        }

        // For managerWorker to shrink.
        private triggerShrink(): void {
            let shrinkJob = this.run<void>((): void => {
                this.timerId = setInterval(() => {
                    let runningWorkersNum: int = 0;
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        for (let w: GlobalQueueWorker of workers) {
                            if(w.currentTasks.size.toInt() !== 0) {
                                runningWorkersNum++;
                            }
                        }
                    });
                    let targetNum: int = runningWorkersNum;
                    ConcurrencyHelpers.lockGuard(taskCountMutex, () => {
                        if (totalExecCount.get() != 0) {
                            let averageDuration: number = totalExecTime.get() / totalExecCount.get();
                            let result: number = (averageDuration * waitingTasksNum.get()) / TASK_DURATION;
                            targetNum += (min(waitingTasksNum.get(), result)).toInt();
                        } else {
                            targetNum += min(2, waitingTasksNum.get());
                        }
                    });
                    ConcurrencyHelpers.lockGuard(workersMutex, () => {
                        if (workers.size > WORKERS_MINIMUM && workers.size > targetNum) {
                            targetNum = max(WORKERS_MINIMUM, targetNum);
                        } else {
                            return;
                        }
                        let step = min(SHRINK_STEP, workers.size - targetNum);
                        let count: int = 0;
                        for(let worker: GlobalQueueWorker of workers) {
                            if (count >= step ) {
                                break;
                            }
                            if (worker.idleTime == 0) {
                                continue;
                            }
                            if (Chrono.nanoNow() - worker.idleTime < idleThreshold) {
                                continue;
                            }
                            worker.closeWorker();
                            workers.delete(worker);
                            count++;
                        }
                        
                    })
                }, triggerInterval);
            });
        }

        // For managerWorker to retrigger shrink.
        retriggerShrink(): void {
            clearInterval(this.timerId);
            this.triggerShrink();
        }

        // For managerWorker to stop and release.
        stopManagerWorker(): void {
            clearInterval(this.timerId);
            this.join();
        }

        public isAliveMutex = ConcurrencyHelpers.mutexCreate();
        public newTaskCondVar = ConcurrencyHelpers.condVarCreate();

        private needInterop: boolean = false;
        public isActive: boolean = true;
        public workerId: int = -1;
        public currentPriority?: Priority = Priority.MEDIUM;
        public currentTasks: Set<Task> = new Set<Task>();
        public idleTime: long = 0;
        static workerCount: int = 0;
        private timerId: int;
    }

    /**
     * @enum defines the task priority
     * @see taskpool.Task
     */
    export enum Priority {
        /// the task priority is high
        HIGH = 0,
        /// the task priority is medium, default priority
        MEDIUM = 1,
        /// the task priority is low
        LOW = 2,
        /// the task priority is idle, only be executed if there are no other priorities
        IDLE = 3
    }

    /**
     * @enum defines the task state
     * @see taskpool.TaskInfo
     */
    export enum State {
        /// the task state is waiting
        WAITING = 1,

        /// the task state is running
        RUNNING = 2,

        /// the task state is canceled
        CANCELED = 3,

        /// the task state is delayed
        DELAYED = 4
    }

    enum InternalState {
        /// the task is not found in the global task pool
        NOT_FOUND = 0,

        /// the task state is waiting
        WAITING = 1,

        /// the task state is running
        RUNNING = 2,

        /// the task state is canceled
        CANCELED = 3,

        /// the task state is delayed
        DELAYED = 4,

        /// the task state is finished
        FINISHED = 5,

        /// the task is ending
        ENDING = 6,
    }

    /**
     * The type of callback to be registered
     */
    export type CallbackFunction = () => void;

    /**
     * The type of callback with error code to be registered
     */
    export type CallbackFunctionWithError = (e: Error) => void;

    class InternalTask {
        constructor(task: Task) {
            this.taskMutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
            this.dependentTasks = new containers.ConcurrentSet<Task>();
            this.task = task;
        }

        static of(task: Task) {
            return (task.internalTask as InternalTask);
        }

        /**
         * @returns true if the task has been completed, false - otherwise
         */
        isDoneInner(): boolean {
            return this.state.get() == InternalState.FINISHED.valueOf();
        }

        /**
         * Check that the task can be executed as common task
         * @throws Error if the task can not be executed as common task
         * @see taskpool.execute
         */
        checkExecution(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_OUTSIDE, "taskpool:: groupTask cannot execute outside");
            } else if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_OUTSIDE, "taskpool:: seqRunnerTask cannot execute outside");
            } else if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            } else if (this.isSubmitted && this.task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_LONG, "taskpool:: The long task can only be executed once");
            } else if (this.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_AGAIN, "taskpool:: the periodicTask cannot execute again");
            }
        }

        /**
         * Check that the task can be executed as delayed task
         * @throws Error if the task can not be executed as delayed task
         * @see taskpool.executeDelayed
         */
        checkExecutionForDelayedTask(delayTime: int): void {
            if (delayTime < 0) {
                throw new Error(TASKPOOL_ERROR_DELAY_TIME_NEGATIVE, "taskpool:: The delayTime is less than zero");
            }
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_EXECUTE_DELAYED, "taskpool:: groupTask cannot executeDelayed outside");
            }
            if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: seqRunnerTask cannot executeDelayed outside");
            }
            if (this.asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTE_DELAYED, "taskpool:: asyncRunnerTask cannot executeDelayed outside");
            }
            if (this.isSubmitted && (this.isDependent || this.dependentTasks.size != 0)) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_DEPENDENCY_EXECUTE, "taskpool:: executedTask with dependency cannot execute again");
            }
            if (this.isSubmitted && (this.task instanceof LongTask)) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_MULTIPLE_EXECUTIONS, "taskpool:: Multiple executions of LongTask are not supported in the executeDelayed");
            }
            if (this.isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_EXECUTE_DELAYED, "taskpool:: the periodicTask cannot executeDelayed");
            }
        }

        /**
         * Check that the task can be executed as period task
         * @throws Error if the task can not be executed as period task
         * @see taskpool.executeDelayed
         */
        checkExecutionForPeriodTask(period: int): void {
            if (period < 0) {
                throw new Error(TASKPOOL_ERROR_PERIOD_NEGATIVE, "taskpool:: The period value is less than zero");
            }
            if (this.isSubmitted || this.isPeriodicTask || this.groupId != 0 || this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_PERIODIC, "taskpool:: The concurrent task has been executed and cannot be executed periodically");
            }
            if (this.isDependent || this.dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_EXECUTE_PERIODIC, "taskpool:: the task with dependency cannot executePeriodically");
            }
        }

        private waitForDependencies(): void {
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            while (this.taskDependenciesCount > 0) {
                ConcurrencyHelpers.condVarWait(this.condVar, this.taskMutex);
            }
            // Only one instance of the task can be running
            if (this.isRunning) {
                ConcurrencyHelpers.condVarWait(this.condVar, this.taskMutex);
            }
            this.isRunning = true;
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
        }

        notifyDependencies(): void {
            for (const dependentTask: Task of this.dependentTasks) {
                let internalDepTask = InternalTask.of(dependentTask);
                ConcurrencyHelpers.lockGuard(internalDepTask.taskMutex, () => {
                    internalDepTask.taskDependenciesCount--;
                    ConcurrencyHelpers.condVarNotifyOne(internalDepTask.condVar, internalDepTask.taskMutex);
                });

            }
            ConcurrencyHelpers.mutexLock(this.taskMutex);
            this.isRunning = false;
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.taskMutex);
            ConcurrencyHelpers.mutexUnlock(this.taskMutex);
            // Notify waiting workers using condition variable
            for (let i = 0; i < this.dependentTasks.size; i++ ) {
                ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                    ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                });
            }
        }

        getDuration(): int {
            return ((Chrono.nanoNow() - this.startTime) / Chrono.NS_PER_MS).toInt();
        }

        execute(currentWorker : GlobalQueueWorker | undefined = undefined): Any {
            initWorkerPoolSync();
            this.waitForDependencies();
            if (this.state.compareAndSwap(State.WAITING, State.RUNNING) == State.CANCELED) {
                this.notifyDependencies();
                const count = allTasks.get(this.task) ?? 0;
                if (count <= 1) {
                    allTasks.delete(this.task);
                } else {
                    allTasks.set(this.task, count - 1);
                }
                this.state.set(State.CANCELED);
                this.tryTriggerRunner();
                this.throwCancelTaskError();
            }
            this.callUserCallback(this.onStartCallback);
            let result: Any = null;
            let executionSucceeded: boolean = false;
            this.startTime = Chrono.nanoNow();
            const startCpuTime: long = Chrono.getCpuTime();
            try {
                setTaskId(this.task.taskId);
                result = this.task.taskFunction();
                if (result instanceof Promise) {
                    result.then(() => {
                        executionSucceeded = true;
                    }).catch((e) => {
                        try {
                            this.onFailCallback?.(e as Error);
                        } catch (callbackError) {}
                        throw e;
                    });
                } else {
                    executionSucceeded = true;
                }
            } catch (e) {
                try {
                    // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                    this.onFailCallback?.(e as Error);
                } catch (callbackError) {}
                throw e;
            } finally {
                // We cannot reliably calculate ioDuration in 1.2 due to possible coroutine switches.
                // And we should set ioDuration = 0, totalDuration = cpuDuration here
                this.task.cpuDuration = ((Chrono.getCpuTime() - startCpuTime) / Chrono.NS_PER_MS).toInt();
                this.task.totalDuration = this.task.cpuDuration;
                this.task.ioDuration = 0;
                // Process the task finishing
                let taskFinishedhandle = () : void => {
                    if (isTaskPoolUseLaunch) {
                        this.notifyDependencies();
                    } else {
                        totalExecTime.fetchAndAdd(this.getDuration());
                        totalExecCount.fetchAndAdd(1);
                    }
                    const count = allTasks.get(this.task) ?? 0;
                    if (count <= 1) {
                        allTasks.delete(this.task);
                    } else {
                        allTasks.set(this.task, count - 1);
                    }
                    if (currentWorker !== undefined) {
                        currentWorker!.currentTasks.delete(this.task);
                        currentWorker!.currentPriority = undefined;
                    }

                    if (this.state.get() != State.CANCELED) {
                        this.state.set(InternalState.FINISHED.valueOf());
                    }
                    try { 
                        if (this.state.get() == State.CANCELED) {
                            this.throwCancelTaskError();
                        } else if (executionSucceeded) {
                            this.callUserCallback(this.onSuccessCallback);
                        }
                    } catch (e) {
                        try {
                            this.onFailCallback?.(e as Error);
                        } catch (callbackError) {}
                        throw e;
                    } finally {
                        this.tryTriggerRunner();
                    }
                };
                if (result instanceof Promise) {
                    return result.finally(() => {
                        taskFinishedhandle();
                    });
                } else {
                    taskFinishedhandle();
                }
            }
            return result;
        }

        /// @throws Error for the canceled task
        private throwCancelTaskError(): void {
            if (this.groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_CANCELED, "taskpool:: taskGroup has been canceled");
            }
            if (this.seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_CANCELED, "taskpool:: sequenceRunner task has been canceled");
            }
            if (this.asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled");
            }
            throw new Error(TASKPOOL_ERROR_TASK_CANCELED, "taskpool:: task has been canceled");
        }

        private callUserCallback(callback: CallbackFunction | undefined): void {
            try {
                callback?.();
            } catch (e) {}
        }

        /// The task is added to taskpool for execution
        enqueue(priority?: Priority): void {
            priority = priority === undefined ? Priority.MEDIUM : priority;
            this.priority = priority;
            this.state.set(State.WAITING);
            if (!isTaskPoolUseLaunch) {
                initGlobalTaskQueues();
                globalTaskQueue?.get(priority)?.push(this.task);
                let count = allTasks.get(this.task) ?? 0;
                allTasks.set(this.task, count + 1);
                const newWaitingTasksNum = waitingTasksNum.fetchAndAdd(1);
                if (newWaitingTasksNum > workers.size * 2) {
                    tryTriggerExpand();
                }
                // Notify waiting workers using condition variable
                ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                    this.callUserCallback(this.onEnqueueCallback);
                    ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                });
            } else {
                this.callUserCallback(this.onEnqueueCallback);
            }
        }

        /**
         * @see taskpool.execute
         * @see taskpool.SequenceRunner.execute
         * @see taskpool.AsyncRunner.execute
         */

        /// trigger next runner task to execute
        tryTriggerRunner(): void {
            // handle sequenceRunner
            if (this.seqId != 0) {
                let nextTask: Task | undefined = undefined;
                ConcurrencyHelpers.lockGuard(this.runnerImpl.seqMutex, () => {
                    if (this.runnerImpl.tasks.length == 0) {
                        this.runnerImpl.isRunning = false;
                    } else {
                        nextTask = this.runnerImpl.tasks.shift();
                        while (nextTask !== undefined && InternalTask.of(nextTask!).state.get() == (State.CANCELED.valueOf())) {
                            const nextInternalTask = InternalTask.of(nextTask!);
                            ConcurrencyHelpers.lockGuard(nextInternalTask.taskMutex, () => {
                                nextInternalTask.currentDeferred!.rejectPromise(new Error(TASKPOOL_ERROR_SEQ_RUNNER_CANCELED, "taskpool:: sequenceRunner task has been canceled"));
                                nextInternalTask.currentDeferred = undefined;
                            });
                            nextTask = this.runnerImpl.tasks.shift();
                        }
                        if (nextTask === undefined) {
                            this.runnerImpl.isRunning = false;
                        }
                    }
                });
                if (nextTask !== undefined) {
                    InternalTask.of(nextTask!).enqueue(this.runnerImpl.priority);
                }
            }

            // handle asyncRunner
            if (this.asyncId != 0) {
                const asyncRunner = AsyncRunnerImpl.getAsyncRunnerById(this.asyncId);
                if (asyncRunner) {
                    asyncRunner.handleTaskCompletionFromTryTriggerRunner();
                }
            }

            if (this.groupId != 0) {
                let group: TaskGroup | undefined = allGroups.get(this.groupId);
                if (group === undefined) {
                    return;
                }
                GroupState.of(group).taskCount.fetchAndSub(1);
                if (GroupState.of(group).state_.get() != InternalState.RUNNING.valueOf()) {
                    GroupState.of(group).state_.set(InternalState.RUNNING.valueOf());
                }
                if (GroupState.of(group).taskCount.get() == 0) {
                    GroupState.of(group).state_.set(InternalState.FINISHED.valueOf());
                    allGroups.delete(this.groupId);
                }
            }
        }


        clearPendingPromiseDeferredUnsafe() {
            while (this.pendingDeferreds.length > 0) {
                this.pendingDeferreds.shift();
            }
        }

        /// id of group which contains the task, 0 means the task is not part of any group
        groupId: int = 0;
        /// id of sequence runner which ran the task, 0 means task was not ran via any sequence runner
        seqId: int = 0;
        /// id of async runner which ran the task, 0 means task was not ran via any async runner
        asyncId: int = 0;
        /// the task has been submitted into taskpool
        isSubmitted: boolean = false;
        /// the task depends on another task
        isDependent: boolean = false;
        /// Count of tasks which should be executed before this task
        taskDependenciesCount: int = 0;
        /// Set of tasks which can be executed after this task
        dependentTasks: containers.ConcurrentSet<Task>;
        /// identifier of worker which starts to execute the task
        workerId: int = -1;
        /// an instance of the task is running
        isRunning: boolean = false;
        /// start time of the task
        private startTime: long = 0;
        /// the task has been executed periodically
        isPeriodicTask: boolean = false;
        /// identifier of task which is executeDelayed or executePeriodically
        timerId: int = -1;
        /// id of the coroutine that executes task
        coroId: int = -1;

        // NOTE(ipetrov, #19949): Change to private when bug in frontend will be fixed
        taskMutex: Object;
        condVar: Object;

        currentDeferred: PromiseDeferred | undefined = undefined;
        pendingDeferreds: Array<PromiseDeferred> = new Array<PromiseDeferred>();

        priority: Priority;
        state: AtomicInt = new AtomicInt(InternalState.NOT_FOUND);

        runnerImpl: SeqRunnerImpl;
        runnerAsyncImpl: AsyncRunnerImpl;

        onReceiveCallback?: Function;
        dataMutex: Object = ConcurrencyHelpers.mutexCreate();

        onEnqueueCallback?: CallbackFunction;
        onStartCallback?: CallbackFunction;
        onFailCallback?: CallbackFunctionWithError;
        onSuccessCallback?: CallbackFunction;
        private task: Task;
    }
    
    class PromiseDeferred {
        constructor(resolve: (value: Any) => void, reject: (e: Error) => void) {
            this.resolve = resolve;
            this.reject = reject;
        }

        public resolvePromise(value: Any): void {
            if (this.resolve !== undefined) {
                this.resolve(value);
            } else {
                throw new Error("PromiseDeferred: resolve is undefined");
            }
        }

        public rejectPromise(e: Error): void {
            if (this.reject !== undefined) {
                this.reject(e);
            } else {
                throw new Error("PromiseDeferred: reject is undefined");
            }
        }

        public resolve: (value: Any) => void;
        public reject: (e: Error) => void;
    }

    /**
     * @class Task provides an interface to create a task to execute in the taskpool
     */
    export class Task {
        /**
         * Create a Task instance
         *
         * @param { string } name The name of task
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...Any } args arguments for function to execute with
         */
        constructor(name: string, func: Function, ...args: FixedArray<Any>) {
            this.name = name;
            this.arguments = Array.from(args);
            if (isTaskpoolSupportInterop) {
                let serializeHelper = new InteropSerializeHelper(args);
                this.taskFunction = (): Any => {
                    let deserializedArgs = serializeHelper.deserialize();
                    return func.unsafeCall(...deserializedArgs);
                };
            } else {
                this.taskFunction = (): Any => {
                    return func.unsafeCall(...args);
                };
            }
            this.totalDuration = 0;
            this.ioDuration = 0;
            this.cpuDuration = 0;
            // Each task has unique identifier
            this.taskId = Task.generateId();
            this.internalTask = new InternalTask(this);
        }

        /**
         * Create a Task instance
         *
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...Any } args arguments for function to execute with
         */
        constructor(func: Function, ...args: FixedArray<Any>) {
            this(func.name, func, ...args);
        }

        /**
         * Check current running task is canceled or not
         *
         * @returns { boolean } true if current running task is canceled, false - otherwise
         */
        static isCanceled(): boolean {
            let result: boolean = false;
            let currentTask = getCurrentTask();
            if (currentTask !== undefined) {
                result = InternalTask.of(currentTask!).state.get() == (State.CANCELED.valueOf());
            }
            return result;
        }

        /**
         * Register a callback and call it when the task is enqueued
         *
         * @param { CallbackFunction } callback Callback to be registered and executed when the task is enqueued
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onEnqueued(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onEnqueueCallback = callback;
        }

        /**
         * Register a callback and call it before the task execution
         *
         * @param { CallbackFunction } callback Callback to be registered and executed before the task execution
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onStartExecution(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onStartCallback = callback;
        }

        /**
         * Register a callback and call it when the task fails to execute
         *
         * @param { CallbackFunctionWithError } callback Callback to be registered and executed when the task fails to execute
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionFailed(callback: CallbackFunctionWithError): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onFailCallback = callback;
        }

        /**
         * Register a callback and call it when the task successfully executed
         *
         * @param { CallbackFunction } callback Callback to be registered and executed when the task successfully executed
         * @throws Error if task is executed. It does not support the registration of listeners
         */
        onExecutionSucceeded(callback: CallbackFunction): void {
            this.throwIfCallbackCannotBeAdded();
            InternalTask.of(this).onSuccessCallback = callback;
        }

        /**
         * Add dependencies on the task array for this task
         * @param { Task[] } tasks An array of dependent tasks
         * @throws Error if
         *     - no parameters
         *     - this task or argument tasks executed (as common or via SequenceRunner)
         *     - task or argument tasks were added to any group
         *     - argument tasks add a circular dependency
         */
        addDependency(...tasks: Task[]): void {
            if (tasks.length == 0) {
                throw new Error(TASKPOOL_ERROR_NO_PARAMS, "taskpool:: addDependency has no params.");
            }
            if (InternalTask.of(this).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY, "taskpool:: the periodic task cannot have a dependency");
            }
            if (InternalTask.of(this).isSubmitted || InternalTask.of(this).seqId != 0 || InternalTask.of(this).asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_ADD_DEPENDENCY, "taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot addDependency");
            }
            if (InternalTask.of(this).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_DEPENDENCY, "taskpool:: groupTask cannot addDependency");
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).isPeriodicTask) {
                    throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ADD_DEPENDENCY, "taskpool:: the periodic task cannot have a dependency");
                }
                if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0 || InternalTask.of(this).asyncId != 0) {
                    throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_OR_ASYNC_RUNNER_TASK_RELIED_ON, "taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot be relied on");
                }
                if (InternalTask.of(task).groupId != 0) {
                    throw new Error(TASKPOOL_ERROR_GROUP_TASK_RELIED_ON, "taskpool:: groupTask cannot be relied on");
                }
                if (this.taskId == task.taskId || this.hasTaskDFS(task)) {
                    throw new Error(TASKPOOL_ERROR_CIRCULAR_DEPENDENCY, "taskpool:: There is a circular dependency");
                }
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).dependentTasks.has(this)) {
                    continue;
                }
                InternalTask.of(task).dependentTasks.add(this);
                InternalTask.of(this).taskDependenciesCount++;
            }
            InternalTask.of(this).isDependent = (InternalTask.of(this).taskDependenciesCount != 0);
        }

        /**
         * Remove dependencies on the task array for this task
         * @param tasks An array of dependent tasks
         * @throws Error if
         *     - no parameters
         *     - this task or argument tasks executed
         *     - this task or argument tasks have not dependencies
         *     - this task does not depend on an argument task
         */
        removeDependency(...tasks: Task[]): void {
            if (tasks.length == 0) {
                throw new Error(TASKPOOL_ERROR_NO_PARAMS, "taskpool:: removeDependency has no params.");
            }
            if (!InternalTask.of(this).isDependent) {
                throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
            }
            if (InternalTask.of(this).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: executedTask cannot removeDependency");
            }
            for (const task: Task of tasks) {
                if (InternalTask.of(task).dependentTasks.size == 0 && !InternalTask.of(task).isDependent) {
                    throw new Error(TASKPOOL_ERROR_NO_DEPENDENCY, "taskpool:: task has no dependency");
                }
                if (InternalTask.of(task).isSubmitted) {
                    throw new Error(TASKPOOL_ERROR_EXECUTED_TASK_REMOVE_DEPENDENCY, "taskpool:: cannot removeDependency on a dependent and executed task");
                }
                if (!InternalTask.of(task).dependentTasks.has(this)) {
                    throw new Error(TASKPOOL_ERROR_DEPENDENCY_NOT_EXIST, "taskpool:: The dependency does not exist");
                }
            }
            for (let task: Task of tasks) {
                InternalTask.of(task).dependentTasks.delete(this);
                InternalTask.of(this).taskDependenciesCount--;
            }
            InternalTask.of(this).isDependent = (InternalTask.of(this).taskDependenciesCount != 0);
        }

        /**
         * @returns true if the task has been completed, false - otherwise
         */
        isDone(): boolean {
            return InternalTask.of(this).isDoneInner();
        }

        /**
         * Send data to host side and trigger the registered callback
         * @param { ...Any } args arguments for the registered callback
         * @throws Error if the function is not called from a taskpool task
         * @throws Error if the callback is not registered
         */
        static sendData(...args: FixedArray<Any>): void {
            const item: Task | undefined = getCurrentTask();
            if (item === undefined) {
                throw new Error(TASKPOOL_ERROR_NOT_IN_TASKPOOL, "taskpool:: The function is not called in the TaskPool thread");
            }
            const task: Task = item;
            if (InternalTask.of(task).onReceiveCallback === undefined) {
                throw new Error(TASKPOOL_ERROR_CALLBACK_NOT_REGISTERED, "taskpool:: The callback is not registered on the host side");
            }
            task.createPromise(InternalTask.of(task).onReceiveCallback!, ...args);
        }

        /**
         * Register a callback for this task to receive and handle data from the taskpool task
         */
        onReceiveData(callback?: Function): void {
            InternalTask.of(this).onReceiveCallback = callback;
        }

        /// Concurrent function to execute in taskpool
        taskFunction: () => Any;

        /// The concurrent function arguments
        arguments?: Any[];

        /// Task name
        name: string;

        /// Task identity
        taskId: int;

        /// Total duration of task execution
        totalDuration: int;

        /// IO duration of task execution
        ioDuration: int;

        /// CPU duration of task execution
        cpuDuration: int;

        /**
         * @see taskpool.Task.addDependency
         */
        private hasTaskDFS(targetTask: Task): boolean {
            for (const dependentTask: Task of InternalTask.of(this).dependentTasks) {
                if (dependentTask.taskId == targetTask.taskId) {
                    return true;
                }
                if (dependentTask.hasTaskDFS(targetTask)) {
                    return true;
                }
            }
            return false;
        }

        private throwIfCallbackCannotBeAdded() {
            if (InternalTask.of(this).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_LISTENER_REGISTRATION, "taskpool:: The executed task does not support the registration of listeners.");
            }
        }

        private createPromise(callback : Function, ...args: FixedArray<Any> ) : Promise<void> {
            return new Promise<void>((resolve) => {
                    callback.unsafeCall(...args);
          })
        }

        /**
         * Generate new unique identifier for a new task
         * @see Task.constructor
         */
        private static native generateId(): int;

        /// ----- Internal implementation part -----
        readonly internalTask: Any;
    }

    /**
     * @class LongTask provides an interface to create a long executing task.
     * The such task can be executed only once
     * @extends Task
     */
    export class LongTask extends Task {
        /**
         * Create a LongTask instance
         * @param { string } name The name of long task
         * @param { Function } func Concurrent function to execute in the taskpool
         */
        constructor(name: string, func: Function, ...args: FixedArray<Any>) {
            super(name, func, ...args);
        }

        /**
         * Create a LongTask instance
         * @param { Function } func Concurrent function to execute in the taskpool
         * @param { ...Any } args arguments for func to execute with
         */
        constructor(func: Function, ...args: FixedArray<Any>) {
            super("", func, ...args);
        }
    }

    class GroupState {
        constructor() {
            this.id = GroupState.generateGroupId();
            this.tasks = new Array<Task>();
        }

        static of(group: TaskGroup) {
            return group.state as GroupState;
        }

        /**
         * Generate unique identifier for the group
         * @see TaskGroup.constructor
         * @return unique group identifier for a new Taskgroup instance
         */
        private static native generateGroupId(): int;

        readonly id: int;
        tasks: Array<Task>;
        isTaskAlreadySubmitted: boolean = false;
        state_: AtomicInt = new AtomicInt(InternalState.NOT_FOUND);
        taskCount: AtomicInt = new AtomicInt(0);
    }

    /**
     * @class TaskGroup provides an interface to create a task group
     */
    export class TaskGroup {
        /**
         * Create a TaskGroup instance
         */
        constructor() {
            this("");
        }

        /**
         * Create a TaskGroup instance
         * @param { string } name The name of TaskGroup
         */
        constructor(name: string) {
            this.name = name;
            this.state = new GroupState();
        }

        /**
         * Add a concurrent function into task group
         *
         * @param { Function } func the concurrent function to add to the task group
         * @param { ...Any } args arguments for func to execute with
         */
        addTask(func: Function, ...args: FixedArray<Any>): void {
            this.addTask(new Task(func, ...args));
        }

        /**
         * Add a task into task group
         *
         * @param { Task } task the task to add to the task group
         * @throws Error if
         *     - task was already added to any group
         *     - task is LongTask
         *     - task was executed outside the group
         *     - task executed via SequenceRunner
         */
        addTask(task: Task): void {
            this.checkExecution(task);
            InternalTask.of(task).groupId = GroupState.of(this).id;
            GroupState.of(this).tasks.push(task);
            GroupState.of(this).taskCount.fetchAndAdd(1);
        }

        private checkExecution(task: Task): void {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_GROUP, "taskpool:: taskGroup cannot add groupTask");
            } else if (task instanceof LongTask) {
                throw new Error(TASKPOOL_ERROR_LONG_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the long task");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_NOT_SUPPORTED, "taskpool:: The interface does not support the periodicTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_GROUP_TASK_ADD_SEQ_RUNNER, "taskpool:: taskGroup cannot add seqRunnerTask or executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: dependent task not allowed.");
            }
        }

        /**
         * TaskGroup name
         */
        name: string;

        /// ----- Internal implementation part -----
        readonly state: Any;
    }

    class SeqRunnerImpl {
        constructor(id: int) {
            this.id = id;
            this.seqPromise = Promise.resolve<Any>(new Object());
            this.tasks = new Array<Task>();
            this.seqMutex = ConcurrencyHelpers.mutexCreate();
            this.isRunning = false;
        }

        priority: Priority;
        isRunning: boolean;
        tasks: Array<Task>;
        seqMutex: Object;

        readonly id: int;
        seqPromise: Promise<Any>;
    }

    /**
     * @class SequenceRunner provides a queue, in which all tasks are executed in sequence
     */
    export class SequenceRunner {
        /**
         * Create a SequenceRunner instance
         */
        constructor(priority?: Priority) {
            this("", priority);
        }

        constructor(name: string, priority?: Priority) {
            this.impl = SequenceRunner.getOrCreateSeqRunnerImpl(name);
            this.impl.priority = priority === undefined ? Priority.MEDIUM : priority;
        }

        execute(task: Task): Promise<Any> {
            initWorkerPoolSync();
            this.checkExecution(task);

            const internalTask = InternalTask.of(task);
            internalTask.seqId = this.impl.id;
            internalTask.runnerImpl = this.impl;
            taskSubmitted(task);
            internalTask.isSubmitted = true;

            if (isTaskPoolUseLaunch) {
                internalTask.enqueue(this.impl.priority);
                const taskRunner = (value: Any): Any => {
                    return internalTask.execute();
                };
                this.impl.seqPromise = this.impl.seqPromise.then<Any>(taskRunner, (error) => {
                    taskRunner(undefined);
                    return new Promise<never>((resolve, reject) => {});
                } );
                return this.impl.seqPromise;
            }

            const promise = new Promise<Any>((resolve, reject) => {
                const newDeferred = new PromiseDeferred(resolve, reject);
                ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
                    if (internalTask.currentDeferred === undefined) {
                        internalTask.currentDeferred = newDeferred;
                    } else {
                        internalTask.pendingDeferreds.push(newDeferred);
                    }
                });
            });

            ConcurrencyHelpers.lockGuard(this.impl.seqMutex, () => {
                if (this.impl.isRunning) {
                    this.impl.tasks.push(task);
                    InternalTask.of(task).state.set(InternalState.WAITING);
                } else {
                    this.impl.isRunning = true;
                    internalTask.enqueue(this.impl.priority);
                }
            });
            return promise;
        }

        private static native generateSeqRunnerId(): int;

        private static getOrCreateSeqRunnerImpl(name: string): SeqRunnerImpl {
            if (name == "") {
                return new SeqRunnerImpl(SequenceRunner.generateSeqRunnerId());
            }
            ConcurrencyHelpers.mutexLock(SequenceRunner.mutex);
            let impl: SeqRunnerImpl | undefined = SequenceRunner.namedRunners.get(name);
            if (impl === undefined) {
                // No sequence runner with passed name, so create instance and set to named runners set
                impl = new SeqRunnerImpl(SequenceRunner.generateSeqRunnerId());
                SequenceRunner.namedRunners.set(name, impl);
            }
            ConcurrencyHelpers.mutexUnlock(SequenceRunner.mutex);
            return impl!;
        }

        private checkExecution(task: Task) {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_GROUP, "taskpool:: sequenceRunner cannot execute groupTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_EXECUTE_EXECUTED, "taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: sequenceRunner dependent task not allowed.");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_SEQUENCE_RUNNER, "taskpool:: sequenceRunner cannot execute the periodicTask");
            } else if (InternalTask.of(task).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            }
        }

        private impl: SeqRunnerImpl;

        static namedRunners: Map<string, SeqRunnerImpl> = new Map<string, SeqRunnerImpl>();
        static mutex: Object = ConcurrencyHelpers.mutexCreate();
    }

    class AsyncRunnerImpl {
        constructor(id: int, runningCapacity: int, waitingCapacity: int) {
            this.id = id;
            this.runningCapacity = runningCapacity;
            this.waitingCapacity = waitingCapacity;
            this.runningCount = 0;
            this.waitingQueue = new Array<Task>();
            this.implInnerMutex = ConcurrencyHelpers.mutexCreate();
        }

        static getAsyncRunnerById(id: int): AsyncRunnerImpl | undefined {
            ConcurrencyHelpers.mutexLock(AsyncRunnerImpl.implMutex);
            let r = AsyncRunnerImpl.asyncRunners.get(id);
            ConcurrencyHelpers.mutexUnlock(AsyncRunnerImpl.implMutex);
            return r;
        }

        /**decrement runningCount and poll tasks from the waiting queue
         * @see Internal.tryTriggerRunner()
         */
        handleTaskCompletionFromTryTriggerRunner(): void {
            let toEnqueue: Array<Task> = new Array<Task>();
            ConcurrencyHelpers.mutexLock(this.implInnerMutex);
            try {
                if (this.runningCount > 0) {
                    this.runningCount--;
                }
                // poll tasks from the waiting queue if available
                while (this.waitingQueue.length > 0 && this.runningCount < this.runningCapacity) {
                    let nextTask = this.waitingQueue.shift();
                    if (nextTask == undefined) break;
                    if (InternalTask.of(nextTask).state.get() == State.CANCELED.valueOf()) {
                        ConcurrencyHelpers.lockGuard(InternalTask.of(nextTask).taskMutex, () => {
                            InternalTask.of(nextTask).currentDeferred!.rejectPromise(
                                new Error(TASKPOOL_ERROR_ASYNC_RUNNER_CANCELED, "taskpool:: asyncRunner task has been canceled")
                            );
                            InternalTask.of(nextTask!).currentDeferred = undefined;
                        });
                        continue;
                    }
                    this.runningCount++;
                    toEnqueue.push(nextTask);
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.implInnerMutex);
            }

            for (let t of toEnqueue) {
                const taskInternalTask = InternalTask.of(t);
                taskInternalTask.enqueue(taskInternalTask.priority);
            }
        }

        readonly id: int;
        readonly runningCapacity: int;
        readonly waitingCapacity: int;

        private runner: AsyncRunner;

        runningCount: int;
        waitingQueue: Array<Task>;
        implInnerMutex: Object;
        static implMutex: Object = ConcurrencyHelpers.mutexCreate();
        // Find the corresponding asyncRunnerImpl by task.asyncID
        static asyncRunners: Map<number, AsyncRunnerImpl> = new Map<number, AsyncRunnerImpl>();
    }

    /**
     * @class The AsyncRunner class provides an interface to create an Asyncrunner.
     */
    export class AsyncRunner {
        /**
         * Create a AsyncRunner instance.
         * @param runningCapacity The maximum task execution capacity.
         * @param waitingCapacity The waiting task capacity, 0 is default, means no limit on waiting task capacity.
         */
        constructor(runningCapacity: int, waitingCapacity?: int) {
            this(AsyncRunner.ANONYMOUS_NAME, runningCapacity, waitingCapacity);
        }

        /**
         * Create or get a AsyncRunner instance by name.
         * @param name AsyncRunner's name, if name is the same, will return the same AsyncRunner.
         * @param runningCapacity The maximum task execution capacity.
         * @param waitingCapacity The waiting task capacity, 0 is default, means no limit on waiting task capacity.
         */
        constructor(name: string | undefined, runningCapacity: int, waitingCapacity?: int) {
            const rcap: int = runningCapacity;
            if (runningCapacity == undefined || runningCapacity <= 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_PARAM, "taskpool:: runningCapacity must be a positive number");
            }
            const wcap: int = waitingCapacity == undefined ? 0 : waitingCapacity;
            if (waitingCapacity != undefined && wcap < 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_PARAM, "taskpool:: watingCapacity must be a non-negative number");
            }

            this.impl = AsyncRunner.getOrCreateAsyncRunnerImpl(name, rcap, wcap);
            this.priority = Priority.MEDIUM;

            ConcurrencyHelpers.lockGuard(AsyncRunner.mutex, () => {
                AsyncRunnerImpl.asyncRunners.set(this.impl.id, this.impl);
            });
        }

        /**
         * Execute a concurrent function.
         * @param task The task to be executed
         * @param priority Task execution priority, MEDIUM is default.
         * @returns
         */
        execute(task: Task, priority?: Priority): Promise<Any> {
            initWorkerPoolSync();
            this.checkExecution(task);

            const internalTask = InternalTask.of(task);
            internalTask.runnerAsyncImpl = this.impl;
            internalTask.asyncId = this.impl.id;
            taskSubmitted(task);
            internalTask.isSubmitted = true;

            const promise = new Promise<Any>((resolve, reject) => {
                const newDeferred = new PromiseDeferred(resolve, reject);
                ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
                    if (internalTask.currentDeferred === undefined) {
                        internalTask.currentDeferred = newDeferred;
                    } else {
                        internalTask.pendingDeferreds.push(newDeferred);
                    }
                });
            });

            ConcurrencyHelpers.mutexLock(this.impl.implInnerMutex);
            try {
                if (this.impl.runningCount < this.impl.runningCapacity) {
                    this.impl.runningCount = this.impl.runningCount + 1;
                    internalTask.enqueue(priority == undefined ? this.priority : priority);
                } else {
                    // If waitingCapacity == 0, treat it's waitingQueue's length as unlimited. Task is added to tail.
                    InternalTask.of(task).state.set(InternalState.WAITING);
                    if (this.impl.waitingCapacity == 0) {
                        this.impl.waitingQueue.push(task);
                    } else {
                        if (this.impl.waitingCapacity > 0 && this.impl.waitingQueue.length >= this.impl.waitingCapacity) {
                            // the waiting queue exceeds capacity, discard the head task.
                            const head = this.impl.waitingQueue.shift();
                            if (head != undefined) {
                                const headInternalTask = InternalTask.of(head!);
                                if(headInternalTask.currentDeferred !== undefined) {
                                    headInternalTask.currentDeferred!.rejectPromise(new Error(TASKPOOL_ERROR_ASYNC_RUNNER_WAITING_QUEUE_DROPPED, "taskpool:: waiting queue dropped head"));
                                }
                                headInternalTask.isSubmitted = false;
                            }
                        }
                        this.impl.waitingQueue.push(task);
                    }
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.impl.implInnerMutex);
            }

            return promise;
        }

        private static native generateAsyncRunnerId(): int;

        private static getOrCreateAsyncRunnerImpl(name: string | undefined, runningCapacity: int, waitingCapacity: int): AsyncRunnerImpl {
            if (name == AsyncRunner.ANONYMOUS_NAME) {
                return new AsyncRunnerImpl(AsyncRunner.generateAsyncRunnerId(), runningCapacity, waitingCapacity);
            }
            ConcurrencyHelpers.mutexLock(AsyncRunner.mutex);
            let impl: AsyncRunnerImpl | undefined = AsyncRunner.namedRunners.get(name);
            if (impl === undefined) {
                // No asyncRunner with passed name, so create instance and set to named runners map.
                impl = new AsyncRunnerImpl(AsyncRunner.generateAsyncRunnerId(), runningCapacity, waitingCapacity);
                AsyncRunner.namedRunners.set(name, impl);
            }
            ConcurrencyHelpers.mutexUnlock(AsyncRunner.mutex);
            return impl!;
        }

        private checkExecution(task: Task) {
            if (InternalTask.of(task).groupId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_GROUP, "taskpool:: asyncRunner cannot execute groupTask");
            } else if (InternalTask.of(task).isSubmitted || InternalTask.of(task).asyncId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_EXECUTED, "taskpool:: asyncRunner cannot execute executedTask");
            } else if (InternalTask.of(task).isDependent || InternalTask.of(task).dependentTasks.size != 0) {
                throw new Error(TASKPOOL_ERROR_DEPENDENT_TASK_NOT_ALLOWED, "taskpool:: asyncRunner dependent task not allowed.");
            } else if (InternalTask.of(task).isPeriodicTask) {
                throw new Error(TASKPOOL_ERROR_PERIODIC_TASK_ASYNC_RUNNER, "taskpool:: asyncRunner cannot execute the periodicTask");
            } else if (InternalTask.of(task).isSubmitted) {
                throw new Error(TASKPOOL_ERROR_TASK_ALREADY_EXECUTED_BASIC, "taskpool:: task has been executed");
            } else if (InternalTask.of(task).seqId != 0) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_EXECUTE_SQUENCE, "taskpool:: asyncRunner cannot execute seqenceRunnerTask");
            }
        }

        private impl: AsyncRunnerImpl;
        private priority: Priority;
        private static readonly ANONYMOUS_NAME: string | undefined = undefined;

        // Reuse AsyncRunner instance by a specified name
        private static namedRunners: Map<string | undefined, AsyncRunnerImpl> = new Map<string | undefined, AsyncRunnerImpl>();
        private static mutex: Object = ConcurrencyHelpers.mutexCreate();
    }

    /**
     * @class represents internal information about task in taskpool
     */
    export class TaskInfo {
        /// Unique identifier of task
        taskId: int = 0;

        /// Task state in taskpool
        state: State = State.WAITING;

        /// Duration of task exeuction
        duration?: int;

        /// Task name
        name: string = "";
    }

    /**
     * @class represents internal information about worker thread with taskpool tasks
     */
    export class ThreadInfo {
        /// Worker thread id
        tid: int = -1;

        /// Running task identifiers list on current worker thread
        taskIds?: int[];

        /// Thread priority
        priority?: Priority;
    }

    /**
     * @class represents internal information about taskpool
     */
    export class TaskPoolInfo {
        /// Array of threads information with taskpool tasks
        threadInfos: ThreadInfo[] = new ThreadInfo[0];

        /// Array of taskpool tasks information
        taskInfos: TaskInfo[] = new TaskInfo[0];
    }

    /**
     * Create task and execute
     *
     * @param { Function } func a concurrent function to create task from
     * @param { ...Any } args arguments for func to execute with
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(func: Function, ...args: FixedArray<Any>): Promise<Any> {
        initWorkerPoolSync();
        if (isTaskPoolUseLaunch) {
            return launchPromise(func, ...args);
        }
        let task = new Task(func.name, func, ...args);
        
        return execute(task);
    }

    /**
     * Execute a concurrent task with default priority
     *
     * @param task The task to execute
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(task: Task): Promise<Any>{
        return executeImpl(task);
    }

    /**
     * Execute a concurrent task with specified priority
     *
     * @param task The task to execute
     * @param { Priority } priority for the task
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function execute(task: Task, priority: Priority): Promise<Any>{
        return executeImpl(task, priority);
    }

    function executeImpl(task: Task, priority?: Priority): Promise<Any> {
        initWorkerPoolSync();
        const internalTask = InternalTask.of(task);
        internalTask.checkExecution();

        internalTask.isSubmitted = true;

        if (isTaskPoolUseLaunch) {
            return launchPromise(internalTask.execute);
        }

        const promise = new Promise<Any>((resolve, reject) => {
            const newDeferred = new PromiseDeferred(resolve, reject);
            ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
                if (internalTask.currentDeferred === undefined) {
                    internalTask.currentDeferred = newDeferred;
                } else {
                    internalTask.pendingDeferreds.push(newDeferred);
                }
            });
        });

        internalTask.enqueue(priority);
        return promise;
    }

    /**
     * Execute a concurrent task group
     *
     * @param { TaskGroup } group The task group for execution
     * @param { Priority } [priority] priority for tasks in the group
     * @returns { Promise<Array<Any>> } Promise for array of results of executed tasks from the group
     */
    export function execute(group: TaskGroup, priority?: Priority): Promise<Array<Any>> {
        initWorkerPoolSync();
        const tasksCount = GroupState.of(group).tasks.length.toInt();
        if (tasksCount == 0) {
            return Promise.resolve<Array<Any>>(new Array<Any>());
        }
        taskGroupSubmitted(GroupState.of(group).id, tasksCount);
        GroupState.of(group).isTaskAlreadySubmitted = true;
        const promises: Array<Promise<Any>> = new Array<Promise<Any>>();
        allGroups.set(GroupState.of(group).id, group);
        GroupState.of(group).state_.set(InternalState.WAITING.valueOf());
        GroupState.of(group).tasks.forEach((task: Task) => {
            InternalTask.of(task).isSubmitted = true;
            if (isTaskPoolUseLaunch) {
                promises.push(launchPromise(InternalTask.of(task).execute));
            } else {
                const promise = new Promise<Any>((resolve, reject) => {
                    const newDeferred = new PromiseDeferred(resolve, reject);
                    const internalTask = InternalTask.of(task);
                    ConcurrencyHelpers.lockGuard(internalTask.taskMutex, () => {
                        if (internalTask.currentDeferred === undefined) {
                            internalTask.currentDeferred = newDeferred;
                        } else {
                            internalTask.pendingDeferreds.push(newDeferred);
                        }
                    });
                });
                InternalTask.of(task).enqueue(priority);
                promises.push(promise);
            }
        });
        return Promise.all<Any>(promises);
    }

    /**
     * Delayed execution of a concurrent task with default priority
     *
     * @param { int } delayTime timeout in ms to execute task after
     * @param { Task } task The task to execute
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function executeDelayed(delayTime: int, task: Task): Promise<Any> {
        return executeDelayedImpl(delayTime, task);
    }

    /**
     * Delayed execution of a concurrent task with specified priority
     *
     * @param { int } delayTime timeout in ms to execute task after
     * @param { Task } task The task to execute
     * @param { Priority } priority specified priority for task
     * @returns { Promise<Any> } Promise for result of executed task
     */
    export function executeDelayed(delayTime: int, task: Task, priority: Priority): Promise<Any> {
        return executeDelayedImpl(delayTime, task, priority);
    }

    function executeDelayedImpl(delayTime: int, task: Task, priority?: Priority): Promise<Any> {
        initWorkerPoolSync();
        InternalTask.of(task).checkExecutionForDelayedTask(delayTime);
        InternalTask.of(task).isSubmitted = true;
        ConcurrencyHelpers.lockGuard(mutex, () => {
            pendingDelayed.add(task);
        });
        InternalTask.of(task).state.set(InternalState.DELAYED.valueOf());
        return new Promise<Any>((resolve, reject) => {
            InternalTask.of(task).timerId = setTimeout(() => {
                ConcurrencyHelpers.lockGuard(mutex, () => {
                    pendingDelayed.delete(task);
                });
                if (InternalTask.of(task).timerId == -1 || isCanceled(task)) {
                    ConcurrencyHelpers.lockGuard(mutex, () => {
                        pendingDelayedCanceled.delete(task);
                    });
                    reject(new Error("taskpool:: task has been canceled"));
                }
                if (isTaskPoolUseLaunch) {
                    launchImpl(task, priority).then((res: Any) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                } else {
                    const p = new Promise<Any>((resolve, reject) => {
                        const newDeferred = new PromiseDeferred(resolve, reject);
                        ConcurrencyHelpers.lockGuard(InternalTask.of(task).taskMutex, () => {
                            if (InternalTask.of(task).currentDeferred === undefined) {
                                InternalTask.of(task).currentDeferred = newDeferred;
                            } else {
                                InternalTask.of(task).pendingDeferreds.push(newDeferred);
                            }
                        });
                    })
                    InternalTask.of(task).state.set(InternalState.WAITING.valueOf());
                    InternalTask.of(task).enqueue(priority);
                    p.then((res: Any) => {
                        resolve(res);
                    }).catch((e) => {
                        reject(e);
                    })
                }
            }, delayTime);
        });
    }

    /**
     * Execute a task periodically with default priority
     *
     * @param { int } period period in ms to execute task
     * @param { Task } task The task to execute
     */
    export function executePeriodically(period: int, task: Task): void {
        return executePeriodicallyImpl(period, task);
    }

    /**
     * Execute a task periodically with specified priority
     *
     * @param { int } period period in ms to execute task
     * @param { Task } task The task to execute
     * @param { Priority } priority priority for task
     */
    export function executePeriodically(period: int, task: Task, priority: Priority): void {
        return executePeriodicallyImpl(period, task, priority);
    }

    function executePeriodicallyImpl(period: int, task: Task, priority?: Priority): void {
        initWorkerPoolSync();
        InternalTask.of(task).checkExecutionForPeriodTask(period);
        InternalTask.of(task).isSubmitted = true;
        InternalTask.of(task).isPeriodicTask = true;
        InternalTask.of(task).timerId = setInterval(() => {
            if (InternalTask.of(task).timerId == -1) {
                return;
            }
            taskSubmitted(task);
            if (isTaskPoolUseLaunch) {
                launchImpl(task, priority);
            } else {
                const p = new Promise<Any>((resolve, reject) => {
                    const newDeferred = new PromiseDeferred(resolve, reject);
                    ConcurrencyHelpers.lockGuard(InternalTask.of(task).taskMutex, () => {
                        if (InternalTask.of(task).currentDeferred === undefined) {
                            InternalTask.of(task).currentDeferred = newDeferred;
                        } else {
                            InternalTask.of(task).pendingDeferreds.push(newDeferred);
                        }
                    });
                })
                InternalTask.of(task).enqueue(priority);
            }
        }, period);
    }

    /**
     * Cancel a concurrent task
     *
     * @param task The task for canceling
     * @throws Error if the task does not exist when it is canceled
     * @see Task.isCancel
     */
    export function cancel(task: Task): void {
        tryCancelDependentTasks(task);
        let currentState = InternalTask.of(task).state.get();
        // 1. task has been canceled skip it
        // 2. task is waiting or running, cancel it
        if (currentState == InternalState.CANCELED.valueOf()) {
            return;
        }
        let timerId = -1;
        if (InternalTask.of(task).timerId !== -1) {
            timerId = InternalTask.of(task).timerId;
            if (InternalTask.of(task).isPeriodicTask) {
                clearInterval(InternalTask.of(task).timerId);
            }
            InternalTask.of(task).timerId = -1;
        }
        if (currentState == InternalState.DELAYED.valueOf()) {
            InternalTask.of(task).state.set(State.CANCELED); 
            return;
        }
        if (currentState == InternalState.RUNNING.valueOf()){
            InternalTask.of(task).state.set(State.CANCELED);
            return;
        }
        if (currentState == InternalState.WAITING.valueOf()) {
            InternalTask.of(task).state.set(State.CANCELED);
            return;
        }
        if (timerId != -1) {
            // canceled task set state to canceled, avoid duplicate execution
            InternalTask.of(task).state.set(State.CANCELED);
            return;
        }
        if (InternalTask.of(task).seqId != 0) {
            if (currentState != InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_SEQ_RUNNER_TASK_EXECUTED, "taskpool:: sequenceRunner task has been executed");
            }
            InternalTask.of(task).state.set(State.CANCELED.valueOf());
        } else if (InternalTask.of(task).asyncId != 0) {
            if (currentState != InternalState.NOT_FOUND.valueOf()) {
                throw new Error(TASKPOOL_ERROR_ASYNC_RUNNER_TASK_EXECUTED, "taskpool:: asyncRunner task has been executed")
            }
            InternalTask.of(task).state.set(State.CANCELED.valueOf());
        }
        if (currentState == InternalState.NOT_FOUND.valueOf() || currentState == InternalState.FINISHED.valueOf()) {
            throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_TASK, "taskpool:: task is not executed or has been executed");
        }
    }

    /**
     * Cancel a concurrent task group
     *
     * @param group The group for canceling
     * @throws Error if the task group does not exist when it is canceled
     */
    export function cancel(group: TaskGroup): void {
        let groupState = GroupState.of(group).state_.get();
        if (groupState != (InternalState.RUNNING.valueOf()) && groupState != (InternalState.WAITING.valueOf())) {
            throw new Error(TASKPOOL_ERROR_TASK_NOT_EXECUTED_GROUP, "taskpool:: taskGroup is not executed or has been executed");
        }
        GroupState.of(group).state_.set(State.CANCELED.valueOf());
        GroupState.of(group).tasks.forEach((task: Task) => {
            InternalTask.of(task).state.set(State.CANCELED.valueOf());
        });
    }

    /**
     * Terminate a long task.
     *
     * @param longTask The long task for terminating
     * @note This method is needed for compatibilty with OHOS API. For coroutines in static ArkTS it is not required
     */
    export function terminateTask(longTask: LongTask): void {}

    /**
     * @returns { TaskPoolInfo } taskpool internal information about tasks and threads
     */
    export function getTaskPoolInfo(): TaskPoolInfo {
        initWorkerPoolSync();
        const taskInfos: Array<TaskInfo> = new Array<TaskInfo>();
        const threadInfos: Array<ThreadInfo> = new Array<ThreadInfo>();
        ConcurrencyHelpers.mutexLock(mutex);
        // collect all tasks waiting and running
        for (let task of allTasks.keys()) {
            if (InternalTask.of(task).state.get() == InternalState.FINISHED.valueOf()) {
                // should not enter this branch, task will be removed from allTasks map after finished
                continue;
            }
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            let state = InternalTask.of(task).state.get();
            if (state == 1) {
                taskInfo.state = State.WAITING;
            } else if (state == 2) {
                taskInfo.state = State.RUNNING;
            } else if (state == 3) {
                taskInfo.state = State.CANCELED;
            } else if (state == 4) {
                taskInfo.state = State.DELAYED;
            }
            taskInfos.push(taskInfo);
        }
        // Collect delay and canceled tasks info
        pendingDelayed.forEach((task: Task) => {
            const taskInfo: TaskInfo = new TaskInfo();
            taskInfo.taskId = task.taskId;
            taskInfo.name = task.name;
            let state = InternalTask.of(task).state.get();
            if (state == State.WAITING.valueOf()) {
                taskInfo.state = State.WAITING;
            } else if (state == State.RUNNING.valueOf()) {
                taskInfo.state = State.RUNNING;
            } else if (state == State.CANCELED.valueOf()) {
                taskInfo.state = State.CANCELED;
            } else if (state == State.DELAYED.valueOf()) {
                taskInfo.state = State.DELAYED;
            }
            taskInfos.push(taskInfo);
        });
        // Collect worker threads info
        if (!isTaskPoolUseLaunch) {
            workers.forEach(worker=>{
                const threadInfo: ThreadInfo = new ThreadInfo();
                threadInfo.tid = worker.workerId;
                threadInfo.priority = worker!.currentPriority;
                let tasks = worker.currentTasks;
                const tasksCount = tasks.size.toInt();
                if (tasksCount == 0) {
                    threadInfos.push(threadInfo);
                    return;
                }
                const taskIds: int[] = new int[tasksCount];
                let currentIndex: int = 0;
                tasks.forEach((task: Task) => {
                    taskIds[currentIndex] = task.taskId;
                    currentIndex++;
                });
                threadInfo.taskIds = taskIds;
                threadInfos.push(threadInfo);
            })
        } else {
            workerToTasks.forEach((tasks: Set<Task>, workerId: int) => {
                const threadInfo: ThreadInfo = new ThreadInfo();
                threadInfo.tid = workerId;
                const tasksCount: int = tasks.size.toInt();
                if (tasksCount == 0) {
                    threadInfos.push(threadInfo);
                    return;
                }
                const taskIds: int[] = new int[tasksCount];
                let currentIndex: int = 0;
                tasks.forEach((task: Task) => {
                    taskIds[currentIndex] = task.taskId;
                    currentIndex++;
                    // priority of running tasks can be regard as the priority of current thread's priority
                    // current only when task has been executed the worker can execute other task
                    if (runningTasks.has(task) && threadInfo.priority === undefined) {
                        threadInfo.priority = InternalTask.of(task).priority;
                    }
                });
                threadInfo.taskIds = taskIds;
                threadInfos.push(threadInfo);
            });
        }
        ConcurrencyHelpers.mutexUnlock(mutex);
        const Info: TaskPoolInfo = new TaskPoolInfo();
        const taskInfosRes: TaskInfo[] = new TaskInfo[taskInfos.length];
        const threadInfosRes: ThreadInfo[] = new ThreadInfo[threadInfos.length];
        taskInfos.forEach((taskInfo: TaskInfo, index: int) => {
            taskInfosRes[index.toInt()] = taskInfo;
        });
        threadInfos.forEach((threadInfo: ThreadInfo, index: int) => {
            threadInfosRes[index.toInt()] = threadInfo;
        });
        Info.taskInfos = taskInfosRes;
        Info.threadInfos = threadInfosRes;
        return Info;
    }

    /// ----- Internal implementation part -----

    /**
     * @brief Submit task to the taskpool on the execute method
     * @see taskpool.execute
     * @see taskpool.SequenceRunner.execute
     * @see taskpool.AsyncRunner.execute
     * @param task submitting task
     */
    function taskSubmitted(task: Task): void {
        const count: int | undefined = waitingTasks.get(task);
        waitingTasks.set(task, count == undefined ? 1 : count + 1);
    }

    /**
     * @brief Submit group of tasks to the taskpool on the execute method
     * @see taskpool.execute
     * @param groupId identifier of the submitted group
     * @param tasksCount count of tasks in the passed group
     */
    function taskGroupSubmitted(groupId: int, tasksCount: int): void {
        const count: int | undefined = waitingGroupTasks.get(groupId);
        waitingGroupTasks.set(groupId, count == undefined ? tasksCount : count + tasksCount);
    }

    function popFromMap<T>(item: T, currentTasksMap: containers.ConcurrentHashMap<T, int>, possibleTasksMap: containers.ConcurrentHashMap<T, int>, cancelSet: containers.ConcurrentSet<T>): boolean {
        const instancesOfTheTask: int = currentTasksMap.get(item)! - 1;
        if (instancesOfTheTask == 0) {
            // This is last instance in the map
            currentTasksMap.delete(item);
        } else {
            currentTasksMap.set(item, instancesOfTheTask);
        }
        if (cancelSet.has(item)) {
            if (instancesOfTheTask == 0 && !possibleTasksMap.has(item)) {
                // No more instances in the taskpool, so delete from cancelSet
                cancelSet.delete(item);
            }
            return true; // was marked as canceled
        }
        return false;
    }

    function pushToRunning(task: Task): void {
        const runningInstancesOfTheTask: int | undefined = runningTasks.get(task);
        runningTasks.set(task, runningInstancesOfTheTask == undefined ? 1 : runningInstancesOfTheTask + 1);
        if (InternalTask.of(task).groupId != 0) {
            const runningTasksOfTheGroup: int | undefined = runningGroupTasks.get(InternalTask.of(task).groupId);
            runningGroupTasks.set(InternalTask.of(task).groupId, runningTasksOfTheGroup == undefined ? 1 : runningTasksOfTheGroup + 1);
        }
        // NOTE(ipetrov, #20208): CoroutineExtras is debug functionality, maybe need to create a separate intrinsic
        const currentCoroId: int = CoroutineExtras.getCoroutineId()
        InternalTask.of(task).coroId = currentCoroId;
        taskIdToTask.set(task.taskId, task);
        const currentWorkerId: int = CoroutineExtras.getWorkerId();
        InternalTask.of(task).workerId = currentWorkerId;
        const workerTasks: Set<Task> = workerToTasks.get(currentWorkerId, /* default */ new Set<Task>());
        workerTasks.add(task);
        workerToTasks.set(currentWorkerId, workerTasks);
    }

    function getCurrentTaskUnsafe(): Task | undefined {
        let taskId = getTaskId();
        let currTask = taskIdToTask.get(taskId);
        return currTask;
    }

    function getCurrentTask(): Task | undefined {
        let task: Task | undefined = undefined;
        let taskId = getTaskId();
        let workerId = CoroutineExtras.getWorkerId();
        let currentWorker = filter<GlobalQueueWorker>(workers, (worker: GlobalQueueWorker) => worker.workerId == workerId);
        if (currentWorker.length != 0) {
            // if currentWorker exists, we can only get one worker
            filter<Task>(currentWorker[0].currentTasks, (t: Task) => t.taskId == taskId).some((t: Task)=>{
                task = t;
                return true;
            })
        }
        return task;
    }

    function filter<T>(s: Set<T>, cb: (t: T, idx: int, arr: Array<T>) => boolean): Array<T> {
        return Array.from(s).filter(cb);
    }

    function isCancel(): boolean {
        ConcurrencyHelpers.mutexLock(mutex);
        const item: Task | undefined = getCurrentTaskUnsafe();
        if (item === undefined) {
            ConcurrencyHelpers.mutexUnlock(mutex);
            // No task on current coroutine
            return false;
        }
        const result: boolean = isCanceled(item!);
        ConcurrencyHelpers.mutexUnlock(mutex);
        return result;
    }

    function isCanceled(task: Task): boolean {
        return pendingDelayedCanceled.has(task) || tasksToBeCanceled.has(task) || groupsToBeCanceled.has(InternalTask.of(task).groupId);
    }

    function tryCancelDependentTasks(task: Task): void {
        for(const dependentTask: Task of InternalTask.of(task).dependentTasks) {
            try {
                cancel(dependentTask);
            } catch (e) {
                // The error can be ignored.
            }
        }
    }

    function hasTask(task: Task): boolean {
        ConcurrencyHelpers.mutexLock(mutex);
        const result: boolean = waitingTasks.has(task) || runningTasks.has(task);
        ConcurrencyHelpers.mutexUnlock(mutex);
        return result;
    }

    /// Stop all taskpool workers for debug.
    function stopAllWorkers(): void {
        if (isTaskPoolUseLaunch) {
            return;
        }
        managerWorker?.stopManagerWorker();
        
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            workers.forEach((worker: GlobalQueueWorker) => {
                worker.closeWorker();
            });
            workers.clear();
        });
    }

    /**
     * @brief Set taskpool trigger shrink interval in taskpool test suite for debug.
     * @param interval The trigger shrink interval value.
     */
    function setTaskPoolTriggerShrinkInterval(interval: int): void {
        triggerInterval = interval;
    }

    /**
     * @brief Set taskpool idle threshold in taskpool test suite for debug.
     * @param threshold The idle threshold value.
     */
    function setTaskPoolIdleThreshold(threshold: int): void {
        idleThreshold = threshold;
    }

    /**
     * @brief Get current taskpool workers num for debug.
     * @returns TaskPool workers num.
     */
    function getTaskPoolWorkersNum(): int {
        let num = 0;
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            num = workers.size;
        });
        return num;
    }

    /**
     * @brief Set taskpool workers limit in taskpool test suite for debug.
     * @param limit TaskPool workers limit.
     */
    function setTaskPoolWorkersLimit(limit: int): void {
        if (limit <= 0) {
            return;
        }
        workersLimit = limit;
    }

    /**
     * @brief Retrigger taskpool shrink in taskpool test suite for debug.
     */
    function retriggerTaskPoolShrink(): void {
        managerWorker?.retriggerShrink();
    }

    /**
     * @brief Increase workers for debug.
     * @param num target workers num.
     */
    function increaseWorkersToN(num: int): void {
        if (isTaskPoolUseLaunch) {
            return;
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            let workersNum = workers.size;
            if (workersNum > num) {
                return;
            }
            for (let i = workersNum; i < num; ++i) {
                let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
        })
    }

    /**
     * @brief Initialize taskpool worker pool in manager worker avoid blocking main thread for long time
     * @see taskpool.execute
     * @see taskpool.Task.constructor
     * @see taskpool.TaskGroup.constructor
     * @see taskpool.SequenceRunner.constructor
     * @see taskpool.AsyncRunner.constructor
     */
    function initWorkerPool(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch || isInitTaskLaunched.get()) {
                return;
        }

        let initTask = ()=> {
            ConcurrencyHelpers.lockGuard(workersMutex, () => {
                if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                    return;
                }

                if (managerWorker === undefined) {
                    // Create managerWorker, and start to trigger shrink.
                    managerWorker = new GlobalQueueWorker();
                    managerWorker.retriggerShrink();
                }

                // Init taskpool worker pool.
                for (let i = 0; i < initialWorkersNumber; ++i) {
                    let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                    workers.add(w);
                }
                for(let i = 0; i < waitingTasksNum.get(); i++) {
                    ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
                        ConcurrencyHelpers.condVarNotifyOne(globalTaskCondVar, globalTaskQueuesMutex);
                    });
                }
                isWorkersInitialized.set(true);
            })
        }
        launch<void, ()=>void>(initTask);
        isInitTaskLaunched.set(true);

    }

    /**
     * @brief Initialize taskpool worker sequential mode for avoid missing task
     */
    function initWorkerPoolSync(): void {
        if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
        }
        ConcurrencyHelpers.lockGuard(workersMutex, () => {
            if (isWorkersInitialized.get() || isTaskPoolUseLaunch) {
                return;
            }

            if (managerWorker === undefined) {
                // Create managerWorker, and start to trigger shrink.
                managerWorker = new GlobalQueueWorker();
                managerWorker!.retriggerShrink();
            }

            for (let i = 0; i < initialWorkersNumber; ++i) {
                let w = new GlobalQueueWorker(isTaskpoolSupportInterop);
                workers.add(w);
            }
            isWorkersInitialized.set(true);
        });
    }

    /**
     * @brief Try to trigger expand.
     * @see taskpool.notifyExecuteTask
     */
    function tryTriggerExpand(): void {
        if (workers.size >= workersLimit) {
            return;
        }
        managerWorker?.triggerExpand();
    }

    /**
     * @brief Update taskpool worker state.
     * @see taskpool.TaskPoolWorker.performTask
     */
    function removeFromIdleWorkers(worker: GlobalQueueWorker): void {
        for (let w: GlobalQueueWorker of idleWorkers) {
            if (worker == w) {
                idleWorkers.delete(w);
                w.idleTime = 0;
                return;
            }
        }
    }

    /**
     * @brief Initialize taskpool taskQueues.
     * @see taskpool.Task.enqueue
     */
    function initTaskQueues(): void {
        if (isTaskQueuesInitialized) {
            return;
        }
        taskQueues = new Map<Priority, Array<Task>>();
        highTaskQueue = new Array<Task>();
        mediumTaskQueue = new Array<Task>();
        lowTaskQueue = new Array<Task>();
        idleTaskQueue = new Array<Task>();
        taskQueues.set(Priority.HIGH, highTaskQueue);
        taskQueues.set(Priority.MEDIUM, mediumTaskQueue);
        taskQueues.set(Priority.LOW, lowTaskQueue);
        taskQueues.set(Priority.IDLE, idleTaskQueue);
        isTaskQueuesInitialized = true;
    }

    function initGlobalTaskQueues(): void {
        if (isGlobalTaskQueuesInitialized.get()) {
            return;
        }
        globalTaskQueue = new Map<Priority, containers.LinkedBlockingQueue<Task>>();
        globalTaskQueue.set(Priority.HIGH, new containers.LinkedBlockingQueue<Task>());
        globalTaskQueue.set(Priority.MEDIUM, new containers.LinkedBlockingQueue<Task>());
        globalTaskQueue.set(Priority.LOW, new containers.LinkedBlockingQueue<Task>());
        globalTaskQueue.set(Priority.IDLE, new containers.LinkedBlockingQueue<Task>());

        isGlobalTaskQueuesInitialized.set(true);
    }

    /**
     * @brief Get task from new task queues with priority logic
     * Uses condition variable to wait when no tasks are available to avoid busy waiting
     * @returns Task if task has no dependency, undefined - otherwise.
     */
    function getTaskFromGlobalQueue(worker: GlobalQueueWorker): Task | undefined {
        // First try non-blocking priority selection
        // here get task with attempts is used to avoid missing tasks
        let task = getTaskWithAttempts();
        if (task != undefined) {
            waitingTasksNum.fetchAndSub(1);
            return task;
        }
        
        // No tasks available, wait on condition variable to avoid CPU busy waiting
        // Only wait if worker is active
        if (!worker.isWorkerActive()) {
            return undefined;
        }
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            // Double-check after acquiring lock to avoid race condition
            task = getTaskWithAttempts();
            if (task === undefined && worker.isWorkerActive()) {
                worker.idleTime = Chrono.nanoNow();
                Coroutine.Schedule();
                idleWorkers.add(worker);
                ConcurrencyHelpers.condVarWait(globalTaskCondVar, globalTaskQueuesMutex);
            }
        });
        // After waking up, try again (task might still be undefined due to spurious wakeup)
        if (task != undefined) {
            waitingTasksNum.fetchAndSub(1);
            return task;
        }
        return undefined;
    }

    function getTaskWithAttempts(): Task | undefined {
        let attempts = continuousExecutionCount;
        while (attempts > 0) {
            let task = selectTaskByPriorityNonBlocking();
            if (task != undefined) {
                return task;
            }
            attempts--;
        }
        return undefined;
    } 
    /**
     * @brief Non-blocking priority-based task selection using LinkedBlockingQueue.poll()
     * Uses poll() instead of pop() to avoid blocking
     */
    function selectTaskByPriorityNonBlocking(): Task | undefined {
        let task: Task | undefined = undefined;
        
        // HIGH priority tasks (with continuous execution limit)
        if (getGlobalTaskQueueSize(Priority.HIGH) > 0) {
            if (globalHighTaskExecuteCount.get() < continuousExecutionCount) {
                globalHighTaskExecuteCount.fetchAndAdd(1);
                task = getGlobalTaskImplNonBlocking(Priority.HIGH);
            } else {
                globalHighTaskExecuteCount.set(0);
            }
        }
        
        // MEDIUM priority tasks (with continuous execution limit)
        if (getGlobalTaskQueueSize(Priority.MEDIUM) > 0 && task === undefined) {
            if (globalMediumTaskExecuteCount.get() < continuousExecutionCount) {
                globalMediumTaskExecuteCount.fetchAndAdd(1);
                task = getGlobalTaskImplNonBlocking(Priority.MEDIUM);
            } else {
                globalMediumTaskExecuteCount.set(0);
            }
        }
        
        // LOW priority tasks
        if (getGlobalTaskQueueSize(Priority.LOW) > 0 && task === undefined) {
            task = getGlobalTaskImplNonBlocking(Priority.LOW);
        }
        
        // IDLE priority tasks (non-blocking check)
        if (getGlobalTaskQueueSize(Priority.IDLE) > 0 && task === undefined) {
            task = getGlobalTaskImplNonBlocking(Priority.IDLE);
        }
        
        return task;
    }


    /**
     * @brief Non-blocking implementation helper using poll() instead of pop()
     * @param priority The priority queue to get task from
     * @returns Task if found and executable, undefined otherwise
     */
    function getGlobalTaskImplNonBlocking(priority: Priority): Task | undefined {
        let queue = globalTaskQueue.get(priority);
        if (queue == undefined || queue.size == 0) {
            return undefined;
        }
        
        let task = queue.poll(); // Non-blocking poll instead of blocking pop
        if (task == undefined) {
            return undefined;
        }
        // Check if task has dependencies
        if (InternalTask.of(task!).taskDependenciesCount == 0) {
            return task;
        } else {
            // Put back dependent task at the end of queue
            queue.push(task);
        }
        return undefined;
    }

    /**
     * @brief Get total count of tasks across all new task queues
     * @returns Total number of tasks waiting
     */
    function getNewTaskTotalCount(): int {
        let total = 0;
        total += getGlobalTaskQueueSize(Priority.HIGH);
        total += getGlobalTaskQueueSize(Priority.MEDIUM);
        total += getGlobalTaskQueueSize(Priority.LOW);
        total += getGlobalTaskQueueSize(Priority.IDLE);
        return total;
    }

    /**
     * @brief Get size of specific priority queue
     * @param priority The priority queue to check
     * @returns Number of tasks in the queue
     */
    function getGlobalTaskQueueSize(priority: Priority): int {
        let queue = globalTaskQueue.get(priority);
        return queue?.size ?? 0;
    }

    /**
     * @brief Notify waiting workers that new tasks are available
     */
    function notifyNewTaskAvailable(): void {
        ConcurrencyHelpers.lockGuard(globalTaskQueuesMutex, () => {
            ConcurrencyHelpers.condVarNotifyAll(globalTaskCondVar, globalTaskQueuesMutex);
        });
    }

    /**
     * @brief The worker gets an executable task.
     * @returns Task if task has no dependency, undefined - otherwise.
     * @see taskpool.TaskPoolWorker.performTask
     */
    function getTask(): Task | undefined {
        let task: Task | undefined = undefined;
        ConcurrencyHelpers.lockGuard(taskQueuesMutex, () => {
            if ((highTaskQueue?.length != 0) && (highTaskExecuteCount < continuousExecutionCount)) {
                highTaskExecuteCount++;
                task = getTaskImpl(highTaskQueue);
                return;
            }
            highTaskExecuteCount = 0;
            if ((mediumTaskQueue?.length != 0) && (mediumTaskExecuteCount < continuousExecutionCount)) {
                mediumTaskExecuteCount++;
                task = getTaskImpl(mediumTaskQueue);
                return;
            }
            mediumTaskExecuteCount = 0;
            if (lowTaskQueue?.length != 0) {
                task = getTaskImpl(lowTaskQueue);
                return;
            }
            if (idleTaskQueue?.length != 0 && highTaskQueue?.length == 0
                && mediumTaskQueue?.length == 0 && lowTaskQueue?.length == 0) {
                task = getTaskImpl(idleTaskQueue);
                return;
            }
        })
        return task;
    }

    function getTaskImpl(tasksQueue: Array<Task>): Task | undefined {
        let task: Task | undefined = undefined;
        let len = tasksQueue.length
        for (let i = 0; i < len; ++i) {
            task = tasksQueue.shift();
            if (InternalTask.of(task!).taskDependenciesCount == 0) {
                break;
            } else {
                let t: Task = task as Task;
                tasksQueue.push(t);
                task = undefined;
            }
        }
        return task;
    }

    /**
     * @brief Get the number of tasks to be executed .
     * @returns tasks num.
     * @see taskpool.TaskPoolWorker.notifyTaskFinished
     */
    function getTaskNum(): int {
        let tasksNum: int = 0;
        ConcurrencyHelpers.lockGuard(taskQueuesMutex, () => {
            for (const taskQueue of taskQueues.values()) {
                tasksNum += taskQueue.length;
            }
        })
        return tasksNum;
    }

    /**
     * @brief Submit delayed task or period task to the taskpool on the execute method.
     * @see taskpool.executeDelayed
     * @see taskpool.Task.executePeriodTask
     * @param task submitting task
     * @param priority task execution priority
     */
    function launchImpl(task: Task, priority?: Priority): Promise<Any> {
        const job: Job<Any> = launch<Any, () => Any>(
            InternalTask.of(task).execute
        );
        let promise = new Promise<Any>((resolve, reject) => {
            try {
                let res = job.Await();
                resolve(res);
            } catch(e) {
                // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                reject(e as Error);
            }
        });
        return promise;
    }

    native function isUsingLaunch(): boolean;
    native function isSupportingInterop(): boolean;
    native function getTaskPoolWorkersLimit(): int;
    native function getTaskId(): int;
    native function setTaskId(taskid: int): void;

    // managed mutex for taskpool containers below
    const mutex = ConcurrencyHelpers.mutexCreate();
    // value is count of tasks
    const waitingTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    const runningTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    const pendingDelayed: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    const pendingDelayedCanceled: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    const tasksToBeCanceled: containers.ConcurrentSet<Task> = new containers.ConcurrentSet<Task>();
    // key is group id, value is count of associated tasks with this group
    const waitingGroupTasks: Map<int, int> = new Map<int, int>();
    const runningGroupTasks: Map<int, int> = new Map<int, int>();
    const groupsToBeCanceled: Set<int> = new Set<int>();
    // value is executing task on the associated taskId
    const taskIdToTask: Map<int, Task> = new Map<int, Task>();
    // value is set of executing tasks on this worker
    const workerToTasks: Map<WorkerId, Set<Task>> = new Map<WorkerId, Set<Task>>();
    // Count of pending tasks per worker
    const workerPendingTasksCount: Map<WorkerId, Int> = new Map<WorkerId, Int>();

    const workersMutex = ConcurrencyHelpers.mutexCreate();
    let workers: Set<GlobalQueueWorker> = new Set<GlobalQueueWorker>();
    let idleWorkers: containers.ConcurrentSet<GlobalQueueWorker> = new containers.ConcurrentSet<GlobalQueueWorker>();
    let isWorkersInitialized: AtomicFlag = new AtomicFlag(false);
    let isInitTaskLaunched: AtomicFlag = new AtomicFlag(false);

    const taskQueuesMutex = ConcurrencyHelpers.mutexCreate();
    let isTaskQueuesInitialized: boolean = false;
    let isGlobalTaskQueuesInitialized: AtomicFlag = new AtomicFlag(false);
    let taskQueues: Map<Priority, Array<Task>>;
    let globalTaskQueue: Map<Priority, containers.LinkedBlockingQueue<Task>>;

    let highTaskQueue: Array<Task>;
    let mediumTaskQueue: Array<Task>;
    let lowTaskQueue: Array<Task>;
    let idleTaskQueue: Array<Task>;
    let highTaskExecuteCount: int = 0;
    let mediumTaskExecuteCount: int = 0;
    
    // New task queue management variables
    const globalTaskQueuesMutex = ConcurrencyHelpers.mutexCreate();
    const globalTaskCondVar = ConcurrencyHelpers.condVarCreate();
    const globalTaskCounterMutex = ConcurrencyHelpers.mutexCreate();
    let globalHighTaskExecuteCount: AtomicInt = new AtomicInt(0);
    let globalMediumTaskExecuteCount: AtomicInt = new AtomicInt(0);

    // all tasks in the taskpool
    let allTasks: containers.ConcurrentHashMap<Task, int> = new containers.ConcurrentHashMap<Task, int>();
    // manage the taskGroup
    let allGroups: containers.ConcurrentHashMap<int, TaskGroup> = new containers.ConcurrentHashMap<int, TaskGroup>();

    const isTaskPoolUseLaunch: boolean = isUsingLaunch();
    const isTaskpoolSupportInterop: boolean = isSupportingInterop();

    const initialWorkersNumber: int = 2;
    const continuousExecutionCount: int = 5;

    const taskCountMutex = ConcurrencyHelpers.mutexCreate();
    let waitingTasksNum: AtomicInt = new AtomicInt(0);
    let totalExecCount: AtomicInt = new AtomicInt(0);
    let totalExecTime: AtomicInt = new AtomicInt(0);
    let managerWorker: GlobalQueueWorker | undefined = undefined;
    let triggerInterval: int = 30000; // 30s is the default shrink interval.
    let idleThreshold: int = 60000; // If the worker is idle for more than 60s, it will be released.
    let workersLimit: int = max(4, getTaskPoolWorkersLimit()); // 4 is the minimum of workers limit.
    const STEP_SIZE: int = 2;
    const SHRINK_STEP: int = 4; // 4: try to release 4 threads every time.
    const TASK_DURATION: int = 100; // 100: 100ms.
    const WORKERS_MINIMUM: int = 1; // 1: minimum thread num when idle.
}

function launchPromise(f: Function, ...args: FixedArray<Any>): Promise<Any> {
    let resolver: (((v: Any | PromiseLike<Any>) => void) | undefined) = undefined;
    let rejecter: (((error: Error) => void) | undefined) = undefined;
    let p = new Promise<Any>((res, rej) => {
        resolver = res
        rejecter = rej;
    })
    let cb = () => {
        try {
            let v = f.unsafeCall(...args)
            resolver!(v)
        } catch (e) {
            // NOTE(csaba.osztrogonac): remove as cast once e has Error type
            rejecter!(e as Error)
        }
    }
    launch<void, () => void>(cb)
    return p;
}
