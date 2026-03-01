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

package std.core;

import { launch } from "std/concurrency";
import { InteropSerializeHelper } from "std/interop";

type Task = () => void;
type ErrorHandlerType = (error: Error) => void;

/**
 * @enum defines the worker priority
 */
export enum WorkerPriority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_DEFAULT = 2,
    PRIORITY_HIGH = 3,
}

const INVALID_WORKER_ID = -1;
const MAIN_WORKER_ID = 0;

/**
 * This class is needed to run Task in exclusive mode:
 * elaunch method creates a new worker and this task will be scheduled on it.
 * All child coroutines will be scheduled on the same worker.
 * A JS runtime is also created if the needInterop parameter is set to true
 */
class ExclusiveLauncher {
    /**
     * Run task in exclusive mode
     * Creates a new worker
     *
     * @param { Task } task will be scheduled on created worker
     * @param { string } name the name of the worker
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     * @returns { int } the id of the worker
     */
    public static elaunch(task: Task,  name: string, needInterop: boolean = false): int {
        return ExclusiveLauncher.elaunch(task as Object, needInterop, name);
    }

    private static native elaunch(task: Object, needInterop: boolean, name: string): int;
};

/**
 * Helper class for ExclusiveLauncher that allows to use
 * the same exclusive worker for several tasks
 */
export class EAWorker {
    private worker: InternalWorker;
    private priority: WorkerPriority = WorkerPriority.PRIORITY_DEFAULT;
    private loopStarted = new AtomicFlag(false);
    private loopJoined = new AtomicFlag(false);
    private initialTask: Task = () => {};
    private static workers = new containers.ConcurrentHashMap<int, EAWorker>();
    private isTaskpool = false;

    /**
     * Create EAWorker instance
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(needInterop: boolean = false) {
        this("EAWorker", needInterop);
    }

    /**
     * Create EAWorker instance with specified name
     * @param { string } name The name of the worker
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(name: string, needInterop: boolean = false) {
        this.worker = needInterop ? new InteropWorker(name) : new StaticWorker(name);
    }

    /**
     * Create EAWorker instance with specified task
     * @param { Task } task The task function to execute
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(task: Task, needInterop: boolean = false) {
        this("EAWorker", task, needInterop);
    }

    /**
     * Create EAWorker instance with specified name and task
     * @param { string } name The name of the worker
     * @param { Task } task The task function to execute
     * @param { boolean } [needInterop=false] true if need to create JS runtime
     */
    constructor(name: string, task: Task, needInterop: boolean = false) {
        this.worker = needInterop ? new InteropWorker(name) : new StaticWorker(name);
        this.initialTask = task;
    }

    // NOTE(panferovi): make this constructor private
    protected constructor(worker: MainWorker) {
        this.worker = worker;
        this.loopStarted.set(true);
        EAWorker.workers.set(this.worker.getWorkerId(), this);
    }

    /**
     * Create EAWorker instance with specified name and supportInterop and whether it is a taskpool worker
     * Internal usage only
     * @param { string } name The name of the worker
     * @param { Object } task The initial task to execute
     * @param { boolean } supportInterop true if need to create JS runtime
     * @param { boolean } isTaskpool true if the worker is a taskpool worker
     */
    protected constructor(task: Object, name: string, supportInterop: boolean = false, isTaskpool: boolean = false) {
        this.isTaskpool = isTaskpool;
        this.initialTask = task as Task;
        this.worker = supportInterop ? new InteropWorker(name) : new StaticWorker(name);
    }

    /**
     * Get all registered EAWorker instances.
     *
     * @returns { containers.ConcurrentHashMap<int, EAWorker> }
     *   A concurrent hash map containing all EAWorker instances,
     *   keyed by their integer identifiers.
     */
    private static getWorkers(): containers.ConcurrentHashMap<int, EAWorker> {
        return EAWorker.workers;
    }

    /**
     * Get the worker identifier
     * @returns { int } The unique identifier of the worker
     */
    public getWorkerId(): int | undefined {
        let wId = this.worker.getWorkerId();
        return wId != INVALID_WORKER_ID ? wId : undefined;
    }

    /**
     * Get the name of the worker
     * @returns { string } The name of the worker
     */
    public getName(): string {
        return this.worker.getName();
    }

    /**
     * Start the worker execution
     * Begins processing tasks in the worker thread
     */
    public start(): void {
        if ((this.loopStarted.get() || this.loopJoined.get()) && !this.isMain()) {
            throw new Error("Worker already started or quit");
        }
        this.worker.start(() => {
            EAWorker.getWorkers().set(CoroutineExtras.getWorkerId(), this);
            let wrappedInitialTask = this.worker.wrapTask<void>(this.initialTask);
            wrappedInitialTask();
        });
        this.worker.setPriority(this.priority);
        this.loopStarted.set(true);
    }

    /**
     * Check if the worker is currently alive and running
     * @returns { boolean } True if the worker is alive, false otherwise
     */
    public isAlive(): boolean {
        return this.loopStarted.get() && !this.loopJoined.get();
    }

    /**
     * Set the priority of the worker
     * @param { WorkerPriority } priority The new priority value for the worker
     */
    public setPriority(priority: WorkerPriority) {
        this.priority = priority;
        if (this.loopStarted.get()) {
            this.worker.setPriority(priority);
        }
    }

    /**
     * Get the current priority of the worker
     * @returns { WorkerPriority } The current priority value of the worker
     */
    public getPriority(): WorkerPriority {
        return this.priority;
    }

    /**
     * Quit the worker
     * Waits for pending tasks to complete before stopping the worker
     */
    public quit(): void {
        this.join().Await();
    }

    /**
     * Set the uncaught exception handler for the worker
     * @param { (error: Error) => void } handler The exception handler function
     */
    public setUncaughtExceptionHandler(handler: (error: Error) => void): void {
        this.worker.setErrorHandler(handler);
    }

    /**
     * Get the current uncaught exception handler
     * @returns { (error: Error) => void } The current exception handler function
     */
    public getUncaughtExceptionHandler(): ErrorHandlerType | undefined {
        return this.worker.getErrorHandler();
    }

    /**
     * Get the main worker instance
     * @returns { EAWorker } The main worker instance
     */
    public static main(): EAWorker {
        return mainEAWorker;
    }

    /**
     * Get the current worker instance
     * @returns { EAWorker } The current worker instance
     */
    public static current(): EAWorker | undefined {
        let currentWorker = EAWorker.getWorkers().get(CoroutineExtras.getWorkerId());
        if (currentWorker !== undefined && currentWorker.isTaskpool) {
            throw new Error("Can not get current worker in taskpool");
        }
        return currentWorker;
    }

    /**
     * Run task in EAWorker
     * Note: it has no effect if EAWorker was stoped
     *
     * @param { Function } task function to execute
     * @param { ...Any } args argumnents for task to execute with
     * @returns { Job<R> } Job that represents the task
     */
    public run<R> (task: Function, ...args: FixedArray<Any>): Job<R> {
        if (!this.loopStarted.get()) {
            throw new Error("Can not run task when worker is not started");
        }
        return this.worker.run<R>(task, ...args);
    }

    public static postToMain<R>(coroFun: Function, launchParams: LaunchParams, ...args: FixedArray<Any>) : Job<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<Any>, launchParams.abortFlag);
    }

    public static postToMain<R>(coroFun: Function, ...args: FixedArray<Any>) : Job<R> {
        return EAWorker.postTaskToMain<R>(coroFun, args as FixedArray<Any>, false);
    }
    /**
     * Post task to main worker
     * Note: it has no effect if EAWorker was stoped
     */
    private static postTaskToMain<R>(coroFun: Function, args: FixedArray<Any>, abort: boolean) : Job<R> {
        let wrappedTask = (): R => {
            return coroFun.unsafeCall(...args) as R;
        }
        let lParams: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [MAIN_WORKER_ID]),
            abortFlag: abort
        };
        return launch<R, () => R>(wrappedTask, lParams);
    }

    /**
     * Stop EAWorker
     * This method closes run queue and detaches worker
     * Note: it's required to call join for EAWorker
     */
    public join(): Job<void> {
        if (!this.loopStarted.get()) {
             throw new Error("Cannot stop worker before start");
        }
        let finalTask = () => { EAWorker.getWorkers().delete(this.worker.getWorkerId()); };
        let joinJob = this.worker.join(finalTask);
        this.loopStarted.set(false);
        this.loopJoined.set(true);
        return joinJob;
    }

    /**
     * Post task to EAWorker
      @param { Task } task The task to post
     */
    public postTask(task: Task):void {
        if (this.isMain()) {
            EAWorker.postToMain<void>(task);
            return;
        }
        if (!this.loopStarted.get()) {
            throw new Error("Can not post task when worker is not started");
        }
        this.worker.run<void>(task);
    }

    private isMain() {
        return this === EAWorker.main();
    }
};

export abstract class InternalWorker {
    private readonly name: string;
    protected workerId: int;
    private errorHandler: ErrorHandlerType | undefined = undefined;

    constructor(name: string, workerId: int) {
        this.name = name;
        this.workerId = workerId;
    }

    run<R>(task: Function, ...args: FixedArray<Any>): Job<R> {
        return this.runImpl<R>((): R => { return task.unsafeCall(...args) as R; });
    }

    abstract start(initialTask: Task);

    abstract join(finalTask: Task): Job<void>;

    getName(): string {
        return this.name;
    }

    getWorkerId(): int {
        return this.workerId;
    }

    setPriority(priority: WorkerPriority) {
        this.runImpl<void>(() => { InternalWorker.setCurrentWorkerPriority(priority); });
    }

    setErrorHandler(handler: ErrorHandlerType) {
        this.errorHandler = handler;
    }

    getErrorHandler(): ErrorHandlerType | undefined {
        return this.errorHandler;
    }

    public wrapTask<R>(task: () => R): () => R {
        let wrappedTask = (): R => {
            try {
                return task();
            } catch (e) {
                if (this.getErrorHandler() != undefined) {
                    // NOTE(csaba.osztrogonac): remove as cast once e has Error type
                    this.getErrorHandler()!(e as Error);
                }
                throw e;
            }
        };
        return wrappedTask;
    }

    protected runImpl<R>(task: () => R): Job<R> {
        let wGroup: LaunchParams = {workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EXACT_ID, [this.workerId])};
        return launch<R, () => R>(this.wrapTask<R>(task), wGroup);
    }

    /**
     * Set the priority of the worker
     * @param { int } priority The priority value for the worker
     */
    private static native setCurrentWorkerPriority(priority: int): void;

    protected static native join(workerId: int, finalTask: Object): void;
}

class InteropWorker extends InternalWorker {
    constructor(name: string) {
        super(name, INVALID_WORKER_ID);
    }

    start(initialTask: Task) {
        this.workerId = ExclusiveLauncher.elaunch(initialTask, this.getName(), true);
    }

    public override run<R>(task: Function, ...args: FixedArray<Any>): Job<R> {
        let serializeHelper = new InteropSerializeHelper(args);
        let wrappedTask = (): R => {
            return task.unsafeCall(...serializeHelper.deserialize()) as R;
        }
        return super.runImpl<R>(wrappedTask);
    }

    public join(finalTask: Task): Job<void> {
        let joinJob = new CompletableJob<void>();
        let finTask = () => {
            finalTask();
            joinJob.finish();
        }
        InternalWorker.join(this.workerId, finTask as Object);
        return joinJob;
    }
}

class StaticWorker extends InternalWorker {
    private joiningEvent = new Event();

    constructor(name: string) {
        super(name, INVALID_WORKER_ID);
    }

    start(initialTask: Task) {
        let initTask = () => {
            initialTask();
            this.joiningEvent.wait();
        }
        this.workerId = ExclusiveLauncher.elaunch(initTask, this.getName(), false);
    }

    public join(finalTask: Task): Job<void> {
        let joinJob = this.runImpl(finalTask);
        this.joiningEvent.fire();
        return joinJob;
    }
}

class MainEAWorker extends EAWorker {
    constructor() {
        super(new MainWorker());
    }
}

export class MainWorker extends InternalWorker {
    constructor() {
        super("main", MAIN_WORKER_ID);
    }

    start(initialTask: Task) {
        throw new Error("Main worker cannot be started");
    }

    join(finalTask: Task): Job<void> {
        throw new Error("Main worker cannot be stopped");
    }

    override setPriority(priority: WorkerPriority) {
        throw new Error("Main worker cannot set priority");
    }

    override setErrorHandler(handler: ErrorHandlerType) {
        throw new Error("Main worker cannot have uncaught exception handler");
    }

    override getErrorHandler(): ErrorHandlerType | undefined {
        return undefined;
    }
}

const mainEAWorker: EAWorker;
