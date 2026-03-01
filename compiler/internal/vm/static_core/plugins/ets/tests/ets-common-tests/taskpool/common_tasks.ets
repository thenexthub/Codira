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

import { CoroutineExtras, AtomicFlag } from 'std/debug/concurrency';

function returnAbc(): string {
    return (() => 'a')() + (() => 'b')() + (() => 'c')();
}

function returnFailAbc(): string {
    throw new Error('Concurrent function failed');
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
}

function longFunc(duration: int): string {
    let t = Date.now();
    while ((Date.now() - t) < duration) {
        continue;
    }
    return 'success';
}

function testCancelFunc(duration: int): boolean {
    let t = Date.now();
    while((Date.now() - t) < duration) {
        continue;
    }
    if (taskpool.Task.isCanceled()) {
        return false;
    }
    return true;
}

async function promiseCase(duration: int): Promise<string> {
    let p: Promise<string> = new Promise<string>((resolve: (value: string) => void, reject: (reason: Error) => void) => {
        let t = Date.now();
        while ((Date.now() - t) < duration) {
            continue;
        }
        resolve('success');
    });
    return p;
}

function delayWithYield(duration: int): void {
    let t = Date.now();
    while ((Date.now() - t) < duration) {}
    Coroutine.Schedule();
}

function launchImpl(func: () => Any): Promise<Any> {
    const job: Job<Any> = launch<Any, () => Any>(
        func
    );
    return new Promise<Any>((resolve, reject) => {
        try {
            let res = job.Await();
            resolve(res);
        } catch (e) {
            reject(e as Error);
        }
    });
}

class Event {
    constructor() {
        this.promise = new Promise<boolean>((resolve: (val: boolean) => void) => {
            this.resolveFn = resolve;
        })
        this.job = new CompletableJob<boolean>();
        this.promise.then((result: boolean) => {
            this.job.finish(result);
        }).catch((e: Error) => {
            this.job.fail(e);
        });
    }

    public wait(): boolean {
        return this.job.Await();
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
    private job: CompletableJob<boolean>;
}

function testDelayFunc(): Any {
    let task1 = new taskpool.Task(returnFailAbc);
    let res: string = '';
    let isTask1Finished = new Event();
    taskpool.executeDelayed(100, task1).catch((e: Any) => {
        res = (e as Error).message;
        isTask1Finished.fire();
    })
    isTask1Finished.wait();
    return res;
}

async function taskExecuteDelayedTest(): Promise<void> {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res1 = await launchImpl(testDelayFunc);
        arktest.assertEQ(res1, 'Concurrent function failed');
        let task2 = new taskpool.Task(longFunc, 50);
        arktest.expectError(() => {
            await taskpool.executeDelayed(-100, task2);
        }, new Error('taskpool:: The delayTime is less than zero'));
    } catch (e) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof NullPointerError, true);
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

async function getDelayedTaskInfoTest(): Promise<void> {
    const taskName = 'testDelayedtaskInfo'
    let task = new taskpool.Task(taskName, longFunc, 50);
    let p = taskpool.executeDelayed(100, task);

    let taskpoolInfo = taskpool.getTaskPoolInfo();
    let state : int = taskpool.State.WAITING;
    for (const taskInfo of taskpoolInfo.taskInfos) {
        if (taskInfo.name === taskName) {
            state = taskInfo.state.valueOf();
        }
    }
    arktest.assertEQ(taskpool.State.DELAYED.valueOf(), state);

    arktest.assertEQ(await p, 'success');

    taskpoolInfo = taskpool.getTaskPoolInfo();
    let delayedExists = true;
    for (const taskInfo of taskpoolInfo.taskInfos) {
        if (taskInfo.name === taskName && taskInfo.state === taskpool.State.DELAYED) {
            delayedExists = false;
        }
    }
    arktest.assertTrue(delayedExists);
}

async function getDelayedTaskInfoWithCancelTest(): Promise<void> {
    const taskName = 'testDelayedCanceltaskInfo'
    let task = new taskpool.Task(taskName, longFunc, 50);
    let p = taskpool.executeDelayed(100, task);

    taskpool.cancel(task);
    let taskpoolInfo = taskpool.getTaskPoolInfo();
    let state : int = taskpool.State.WAITING;
    for (const taskInfo of taskpoolInfo.taskInfos) {
        if (taskInfo.name === taskName) {
            state = taskInfo.state.valueOf();
        }
    }
    arktest.assertEQ(taskpool.State.CANCELED.valueOf(), state);
    arktest.expectError(() => {
        await p;
    }, new Error('taskpool:: task has been canceled'));
}

let verifyCount = 0;
function verifySendDataRes(a:int):void{
    arktest.assertEQ(a, 10)
    verifyCount++;
}

function verifySendDataResWithRestParameter(...args:FixedArray<Any>) {
    let arg1 = args[0] as Array<int>;
    let arg2 = args[1] as Array<string>;
    arktest.assertEQ(arg1[0], 1);
    arktest.assertEQ(arg1[1], 2);
    arktest.assertEQ(arg2[0], 'hello');
    arktest.assertEQ(arg2[1], 'world');
}

async function testOnReceiveDataRestParameter():Promise<void> {
    let task = new taskpool.Task(() => {
        let arg = new Array<int>(1, 2);
        taskpool.Task.sendData(arg, new Array<string>('hello', 'world'));
    })
    task.onReceiveData(verifySendDataResWithRestParameter)
    await taskpool.execute(task);
}

async function taskOnReceiveData():Promise<void> {
    clearCount();
    let task = new taskpool.Task(() => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(10);
    });
    task.onReceiveData(verifySendDataRes);
    await taskpool.execute(task);
    arktest.assertEQ(verifyCount, 4);
}

function clearCount():void {
    verifyCount = 0;
}

async function taskSendData():Promise<void> {
    clearCount();
    let task = new taskpool.Task(() => {
        taskpool.Task.sendData(10);
        taskpool.Task.sendData(undefined);
        taskpool.Task.sendData(null);
        taskpool.Task.sendData('string');
        taskpool.Task.sendData(1.0);
    });

    task.onReceiveData((): void => {
        verifyCount++
    });
    await taskpool.execute(task);
    arktest.assertEQ(verifyCount, 5);
}

async function taskSendDataInAsyncFunction():Promise<void> {
    clearCount();
    let task = new taskpool.Task(async() => {
        taskpool.Task.sendData(10);
        await Promise.resolve();
        taskpool.Task.sendData('string');
        taskpool.Task.sendData(undefined);
        taskpool.Task.sendData(null);
        taskpool.Task.sendData(1.0);
    });

    task.onReceiveData((): void => {
        verifyCount++
    });
    await taskpool.execute(task);
    arktest.assertEQ(verifyCount, 5);
}

async function taskExecutePeriodicallyTest(): Promise<void> {
    let task1 = new taskpool.Task(returnAbc);
    arktest.expectError(() => {
        taskpool.executePeriodically(-100, task1);
    }, new Error('taskpool:: The period value is less than zero'));
    let res = await taskpool.execute(task1);
    arktest.assertEQ(res, 'abc');
    arktest.expectError(() => {
        taskpool.executePeriodically(100, task1);
    }, new Error('taskpool:: The concurrent task has been executed and cannot be executed periodically'));
}

async function taskWithDependencyExecutePeriodicallyTest(): Promise<void> {
    let task1 = new taskpool.Task(sum, 5, 25);
    let task2 = new taskpool.Task(max, 36, 12);
    task1.addDependency(task2);
    arktest.expectError(() => {
        taskpool.executePeriodically(10, task1);
    }, new Error('taskpool:: the task with dependency cannot executePeriodically'));
    arktest.expectError(() => {
        taskpool.executePeriodically(100, task2);
    }, new Error('taskpool:: the task with dependency cannot executePeriodically'));
}

async function testTaskpoolInfo() {
    // Check non empty threadInfo without tasks
    let emptyThreadInfo = taskpool.getTaskPoolInfo();
    let flagForTask = new Event();
    let isRunTask = new Event();
    // When taskpool using launch we do not have default worker for it
    // So the behavior is little bit different with taskpool using EAWorker
    if (!CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        arktest.assertNE(emptyThreadInfo.threadInfos.length, 0);
    }
    let funcForTask = (): string => {
        isRunTask.fire();
        flagForTask.wait();
        return 'result'
    };
    let task = new taskpool.Task('task_name', funcForTask);
    let info1 = taskpool.getTaskPoolInfo();
    // Obtain the worker pool info, and the behavior is the same as 1.0 taskpool
    if (!CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        arktest.assertNE(info1.threadInfos.length, 0);
    }
    let p = taskpool.execute(task, taskpool.Priority.HIGH);
    // wait for task started
    isRunTask.wait();
    let info2 = taskpool.getTaskPoolInfo();
    // Notify to continue task
    flagForTask.fire();
    // Check current priority of the worker
    //here we cannot guarantee the task has been dispatched to the first worker
    let compared: boolean = false;
    for (let threadInfo of info2.threadInfos) {
        if (threadInfo.taskIds !== undefined) {
            arktest.assertEQ(threadInfo!.priority, taskpool.Priority.HIGH);
            compared = true;
            break;
        }
    }
    arktest.assertEQ(compared, true);
    arktest.assertEQ(info2.taskInfos.length, 1);
    arktest.assertNE(info2.threadInfos.length, 0);
    let isNonEmptyWorker: boolean = false;
    for (let threadInfo of info2.threadInfos) {
        arktest.assertLT(0, threadInfo.tid);
        isNonEmptyWorker = isNonEmptyWorker || (threadInfo.taskIds != undefined);
    }
    arktest.assertEQ(isNonEmptyWorker, true);
    // NOTE(ipetrov, #20349): uncomment when frontend fix enum comparison
    arktest.assertEQ(info2.taskInfos[0].state.valueOf(), taskpool.State.RUNNING.valueOf());
    arktest.assertEQ(info2.taskInfos[0].name, 'task_name');
    arktest.assertEQ(await p, 'result');
}

function namedTaskFunction(flagForTask:Event, isRunTask:Event):string {
    isRunTask.fire();
    flagForTask.wait();
    return 'result'
}

function testTaskpoolInfoOfNoneNameTask() {
    // Check non empty threadInfo without tasks
    let emptyThreadInfo = taskpool.getTaskPoolInfo();
    let flagForTask = new Event();
    let isRunTask = new Event();
    // When taskpool using launch we do not have default worker for it
    // So the behavior is little bit different with taskpool using EAWorker
    if (!CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        arktest.assertNE(emptyThreadInfo.threadInfos.length, 0);
    }
    let task = new taskpool.Task(namedTaskFunction, flagForTask, isRunTask);
    let info1 = taskpool.getTaskPoolInfo();
    // Obtain the worker pool info, and the behavior is the same as 1.0 taskpool
    if (!CoroutineExtras.isTaskpoolUsingLaunchMode()) {
        arktest.assertNE(info1.threadInfos.length, 0);
    }
    let p = taskpool.execute(task, taskpool.Priority.HIGH);
    // wait for task started
    isRunTask.wait();
    let info2 = taskpool.getTaskPoolInfo();
    // Notify to continue task
    flagForTask.fire();
    // Check current priority of the worker
    let checked = false;
    info2.threadInfos.forEach(threadInfo => {
        if (threadInfo.taskIds != undefined) {
            checked = true;
            arktest.assertEQ(threadInfo.priority, taskpool.Priority.HIGH);
        }
    });
    arktest.assertEQ(checked, true);
    arktest.assertEQ(info2.taskInfos.length, 1);
    arktest.assertNE(info2.threadInfos.length, 0);
    let isNonEmptyWorker: boolean = false;
    for (let threadInfo of info2.threadInfos) {
        arktest.assertLT(0, threadInfo.tid);
        isNonEmptyWorker = isNonEmptyWorker || (threadInfo.taskIds != undefined);
    }
    arktest.assertEQ(isNonEmptyWorker, true);
    // NOTE(ipetrov, #20349): uncomment when frontend fix enum comparison
    arktest.assertEQ(info2.taskInfos[0].state.valueOf(), taskpool.State.RUNNING.valueOf());
    arktest.assertEQ(info2.taskInfos[0].name, 'namedTaskFunction');
    arktest.assertEQ(waitForCompletion(async () => {
        let res = await p;
        return res;
    }), 'result');
}

function testTaskPoolInfoasyncawaitTask() {
    const caseName : string = 'testasyncawaitTask';
    let flagforTask = new Event();
    let isRunTask = new Event();
    let asyncfuncforTask = async (ms : int) : Promise<int> => {
        const promise = await new Promise<int>(resolve => {
            isRunTask.fire();
            flagforTask.wait();
            resolve(ms);
        })
        return promise;
    };
    let task : taskpool.Task = new taskpool.Task(caseName, asyncfuncforTask, 3000);
    taskpool.execute(task, taskpool.Priority.HIGH);
    isRunTask.wait();
    let taskpoolInfo: taskpool.TaskPoolInfo =  taskpool.getTaskPoolInfo();
    let state = 0;
    for(const taskInfo of taskpoolInfo.taskInfos) {
        if(taskInfo.name == caseName) {
            state = taskInfo.state.valueOf();
        }
    }
    arktest.assertEQ(state, taskpool.State.RUNNING.valueOf());
    flagforTask.fire();
}

function returnNumber(): int {
    return 100;
}

async function executeNestedTask():Promise<int> {
    let count = 0; 
    for(let a = 0; a < 10; a++) {
        let res = await taskpool.execute(returnNumber);
        arktest.assertEQ(res, 100);
        count++;
    }
    return count
}

async function exceedContinueTaskTest() {
    let res = await taskpool.execute(executeNestedTask);
    arktest.assertEQ(res, 10)
}

async function oneTaskExecutionTest() {
    let Sum = (): int => {
        return 10 + 20;
    }
    // Task with lambda
    let lambdaTask = new taskpool.Task(Sum);
    let result = await taskpool.execute(lambdaTask);
    arktest.assertEQ(result, 30);
    arktest.assertEQ(lambdaTask.isDone(), true);
    // Task with function
    let funcTask = new taskpool.Task(returnAbc);
    let res = await taskpool.execute(funcTask);
    arktest.assertEQ(res, 'abc');
    arktest.assertEQ(funcTask.isDone(), true);
    // Repeated execution is allowed for common tasks
    let secondRunResult = await taskpool.execute(funcTask);
    arktest.assertEQ(secondRunResult, 'abc');
    arktest.assertEQ(funcTask.isDone(), true);
    // Tasks with params
    let task1 = new taskpool.Task(sum, 10, 20);
    let res1 = await taskpool.execute(task1);
    arktest.assertEQ(res1, 30);
    arktest.assertEQ(task1.isDone(), true);
    let task2 = new taskpool.Task(max, 80, 60);
    let res2 = await taskpool.execute(task2);
    arktest.assertEQ(res2, 80);
    arktest.assertEQ(task2.isDone(), true);
}

async function callbackTest() {
    let onEnqueuedStr = '';
    let onEnqueuedExpectedStr = 'enqueue callback is done';
    let onEnqueuedCallback = () => { onEnqueuedStr = onEnqueuedExpectedStr; };
    let onStartExecutionStr = '';
    let onStartExecutionExpepctedStr = 'start callback is done';
    let onStartExecutionCallback = () => { onStartExecutionStr = onStartExecutionExpepctedStr; };
    let onExecutionSucceededStr = '';
    let onExecutionSucceededExpectedStr = 'success callback is done';
    let onExecutionSucceededCallback = () => { onExecutionSucceededStr = onExecutionSucceededExpectedStr; };
    let onExecutionFailedStr = '';
    let onExecutionFailedExpectedStr = 'Concurrent function failed, fail callback is done';
    let onExecutionFailedCallback = (e: Error) => { onExecutionFailedStr = e.message + ', fail callback is done'; };
    // Successed task
    let task1 = new taskpool.Task(returnAbc);
    task1.onEnqueued(onEnqueuedCallback);
    task1.onStartExecution(onStartExecutionCallback);
    task1.onExecutionSucceeded(onExecutionSucceededCallback);
    task1.onExecutionFailed(onExecutionFailedCallback);
    let res1 = await taskpool.execute(task1);
    arktest.assertEQ(res1, 'abc');
    arktest.assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
    arktest.assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
    arktest.assertEQ(onExecutionSucceededStr, onExecutionSucceededExpectedStr);
    arktest.assertEQ(onExecutionFailedStr, '');
    onEnqueuedStr = '';
    onStartExecutionStr = '';
    onExecutionSucceededStr = '';
    // Successed task with params
    let task2 = new taskpool.Task(sum, 10, 20);
    task2.onEnqueued(onEnqueuedCallback);
    task2.onStartExecution(onStartExecutionCallback);
    task2.onExecutionSucceeded(onExecutionSucceededCallback);
    task2.onExecutionFailed(onExecutionFailedCallback);
    let res2 = await taskpool.execute(task2);
    arktest.assertEQ(res2, 30);
    arktest.assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
    arktest.assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
    arktest.assertEQ(onExecutionSucceededStr, onExecutionSucceededExpectedStr);
    arktest.assertEQ(onExecutionFailedStr, '');
    // Failed task
    onEnqueuedStr = '';
    onStartExecutionStr = '';
    onExecutionSucceededStr = '';
    let failedTask = new taskpool.Task(returnFailAbc);
    failedTask.onEnqueued(onEnqueuedCallback);
    failedTask.onStartExecution(onStartExecutionCallback);
    failedTask.onExecutionSucceeded(onExecutionSucceededCallback);
    failedTask.onExecutionFailed(onExecutionFailedCallback);
    arktest.expectError(() => { await taskpool.execute(failedTask) },
                new Error('Concurrent function failed'));
    arktest.assertEQ(onEnqueuedStr, onEnqueuedExpectedStr);
    arktest.assertEQ(onStartExecutionStr, onStartExecutionExpepctedStr);
    arktest.assertEQ(onExecutionSucceededStr, '');
    arktest.assertEQ(onExecutionFailedStr, onExecutionFailedExpectedStr);
    // Add callbacks for executed task
    let executedTask = new taskpool.Task(returnAbc);
    let expectedCallbackError = new Error('taskpool:: The executed task does not support the registration of listeners.');
    taskpool.execute(executedTask);
    arktest.expectError(() => { executedTask.onEnqueued(() => {}); }, expectedCallbackError);
    arktest.expectError(() => { executedTask.onStartExecution(() => {}); }, expectedCallbackError);
    arktest.expectError(() => { executedTask.onExecutionSucceeded(() => {}); }, expectedCallbackError);
    arktest.expectError(() => { executedTask.onExecutionFailed((e: Error) => {}); }, expectedCallbackError);
}

async function isCancelTest() {
    arktest.assertEQ(taskpool.Task.isCanceled(), false);
    let t = new taskpool.Task(returnAbc);
    let res = await taskpool.execute(t);
    arktest.assertEQ(res, 'abc');
    arktest.assertEQ(taskpool.Task.isCanceled(), false);
    // Check canceling of exeuting task
    // CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN); 
    let isCanceled = new Event();
    let isRunning = new Event();
    let waitForCancel = new Event();
    let funcForTask = (): boolean => {
        isRunning.fire();
        waitForCancel.wait();
        // Set isCanceled value from taskpool
        isCanceled.fire(taskpool.Task.isCanceled());
        return true;
    };
    let task = new taskpool.Task(funcForTask);
    let p = taskpool.execute(task);
    isRunning.wait();
    arktest.assertEQ(task.isDone(), false);
    taskpool.cancel(task);
    waitForCancel.fire();
    let wasCanceled = isCanceled.wait();
    // Check isCanceled from taskpool
    arktest.assertEQ(wasCanceled, true);
    arktest.expectError(() => { await p }, new Error('taskpool:: task has been canceled'));
}

async function isCancelPromiseTest() {
    arktest.assertEQ(taskpool.Task.isCanceled(), false);
    let t = new taskpool.Task(returnAbc);
    let res = await taskpool.execute(t);
    arktest.assertEQ(res, 'abc');
    arktest.assertEQ(taskpool.Task.isCanceled(), false);
    // Check canceling of exeuting Promise task
    // CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN); 
    let isCanceled = new Event();
    let isRunning = new Event();
    let waitForCancel = new Event();
    let funcForPromiseTask = async (): Promise<boolean> => {
        isRunning.fire();
        waitForCancel.wait();
        // Set isCanceled value from taskpool
        isCanceled.fire(taskpool.Task.isCanceled());
        return true;
    };
    let task = new taskpool.Task(funcForPromiseTask);
    let p = taskpool.execute(task);
    isRunning.wait();
    arktest.assertEQ(task.isDone(), false);
    taskpool.cancel(task);
    waitForCancel.fire();
    let wasCanceled = isCanceled.wait();
    // Check isCanceled from taskpool
    arktest.assertEQ(wasCanceled, true);
    arktest.expectError(() => { await p }, new Error('taskpool:: task has been canceled'));
}

async function cancelNonExecutingTaskTest() {
    let expectedError = new Error('taskpool:: task is not executed or has been executed');
    arktest.expectError(() => {
        let t = new taskpool.Task(returnAbc);
        taskpool.cancel(t);
        taskpool.execute(t);
    }, expectedError);
    arktest.expectError(() => {
        let t = new taskpool.Task(returnAbc);
        await taskpool.execute(t);
        taskpool.cancel(t);
    }, expectedError);
    // tasks with params
    arktest.expectError(() => {
        let t = new taskpool.Task(sum, 10, 20);
        taskpool.cancel(t);
        taskpool.execute(t);
    }, expectedError);
    arktest.expectError(() => {
        let t = new taskpool.Task(sum, 10, 20);
        await taskpool.execute(t);
        taskpool.cancel(t);
    }, expectedError);
}

async function cancelExecutingTaskTest():Promise<void> {
    let isExecuting = new Event();
    let lambdaFunc = (): string => {
        isExecuting.wait();
        return 'abc';
    };
    let task1 = new taskpool.Task(lambdaFunc);
    let p1 = taskpool.execute(task1);
    taskpool.cancel(task1);
    isExecuting.fire();
    arktest.expectError(() => { await p1 }, new Error('taskpool:: task has been canceled'));
    let res = await taskpool.execute(task1);
    arktest.assertEQ(res, 'abc');
    let task2 = new taskpool.Task(testCancelFunc, 300);
    let val: int = 0;
    let isExecuted = new Event();
    taskpool.execute(task2).then((value: Any): void => {
        val = 1;
        isExecuted.fire();
    }).catch((e: Any): void => {
        val = 2;
        isExecuted.fire();
    });
    await promiseCase(50);
    try {
        taskpool.cancel(task2);
    } catch (e) {
        let err = e as Error;
        arktest.assertEQ(err.message, 'taskpool:: task is not executed or has been executed');
        return undefined;
    }
    isExecuted.wait();
    arktest.assertEQ(task2.isDone(), false);
    // The canceled task result returns through the catch branch, not the then branch.
    arktest.assertEQ(val, 2);
}

async function checkIsDoneAndCancelTaskTest() {
    let isCanceled: AtomicFlag = new AtomicFlag(false);
    let isRunning: AtomicFlag = new AtomicFlag(false);
    let waitCancel: AtomicFlag = new AtomicFlag(true);
    let funcForTask = (duration: int): boolean => {
        isRunning.set(true);
        while(waitCancel.get()) {}
        // mock time-consuming operation
        longFunc(duration);
        if (taskpool.Task.isCanceled()) {
            isCanceled.set(true);
            isRunning.set(false);
            return false;
        }
        // mock time-consuming operation
        longFunc(duration);
        isRunning.set(false);
        return true;
    };
    let task1 = new taskpool.Task(funcForTask, 50);
    let p1 = taskpool.execute(task1);
    while (!isRunning.get()) {}
    // Cancel the executing task will not throw an error.
    arktest.expectNoThrow(() => {
        if (!task1.isDone()) {
            taskpool.cancel(task1);
        } else {
            throw new Error('The task isDone status is wrong.');
        }
    });
    waitCancel.set(false);
    while (isRunning.get()) {}
    arktest.assertEQ(isCanceled.get(), true);
    arktest.expectError(() => { await p1 }, new Error('taskpool:: task has been canceled'));
    let task2 = new taskpool.Task(testCancelFunc, 100);
    arktest.expectError(() => {
        await taskpool.execute(task2);
        // If isDone is true, cancel the task will throw an error.
        if (task2.isDone()) {
            taskpool.cancel(task2);
        }
    }, new Error('taskpool:: task is not executed or has been executed'));
}

async function dependentTasksTest() {
    let resultStr: string = '';
    let func1 = (): string => {
        resultStr += '1';
        return resultStr;
    };
    let func2 = (a: string): string => {
        resultStr += a;
        return resultStr;
    }
    let func3 = (): string => {
        resultStr += '3';
        return resultStr;
    }
    let task1 = new taskpool.Task(func1);
    let task2 = new taskpool.Task(func2, '2');
    let task3 = new taskpool.Task(func3);
    task1.addDependency(task2);
    task2.addDependency(task3);
    let p1 = taskpool.execute(task1);
    let p2 = taskpool.execute(task2);
    let p3 = taskpool.execute(task3);
    let res2 = await p2;
    arktest.assertEQ(res2, '32');
    let res3 = await p3;
    arktest.assertEQ(res3, '3');
    let res1 = await p1;
    arktest.assertEQ(res1, '321');
    arktest.assertEQ(resultStr, '321');
    // Empty dependencies
    arktest.expectError(() => {
        let task = new taskpool.Task(returnAbc);
        task.addDependency();
    }, new Error('taskpool:: addDependency has no params.'));
    arktest.expectError(() => {
        let task = new taskpool.Task(returnAbc);
        task.removeDependency();
    }, new Error('taskpool:: removeDependency has no params.'));
}

async function dependentTasksWithParamsTest() {
    let func1 = (str: string): string => {
        return str;
    };
    let func2 = (num: int): int => {
        let t = Date.now();
        while ((Date.now() - t) < 100) {}
        return num;
    }
    let func3 = (duration: int): string => {
        let t = Date.now();
        while ((Date.now() - t) < duration) {}
        return 'Task3';
    }
    let task1 = new taskpool.Task(func1, '1');
    let task2 = new taskpool.Task(func2, 222);
    let task3 = new taskpool.Task(func3, 500);
    task1.addDependency(task2);
    task2.addDependency(task3);
    task1.removeDependency(task2);
    task2.removeDependency(task3);
    let p1 = taskpool.execute(task1);
    let p2 = taskpool.execute(task2);
    let p3 = taskpool.execute(task3);
    let res1 = await p1;
    arktest.assertEQ(res1, '1');
    let res2 = await p2;
    arktest.assertEQ(res2, 222);
    let res3 = await p3;
    arktest.assertEQ(res3, 'Task3');
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    let task4 = new taskpool.Task(func3, 100);
    let task5 = new taskpool.Task(func3, 200);
    let task6 = new taskpool.Task(func3, 100);
    let task7 = new taskpool.Task(func3, 200);
    // one task depends on multiple tasks
    task4.addDependency(task5, task6, task7);
    let p4 = taskpool.execute(task4);
    taskpool.execute(task5);
    taskpool.execute(task6);
    taskpool.execute(task7);
    await p4;
    arktest.assertEQ(task5.isDone(), true);
    arktest.assertEQ(task6.isDone(), true);
    arktest.assertEQ(task7.isDone(), true);
}

async function circularDependencyTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    let task3 = new taskpool.Task(returnAbc);
    let task4 = new taskpool.Task(returnAbc);
    let task5 = new taskpool.Task(returnAbc);
    let task6 = new taskpool.Task(returnAbc);
    let expectedError = new Error('taskpool:: There is a circular dependency');
    arktest.expectNoThrow(() => {
        task1.addDependency(task2);
        task2.addDependency(task3);
        task3.addDependency(task4);
        task4.addDependency(task5);
        task5.addDependency(task6);
    });
    arktest.expectError(() => {
        // circular dependency
        task6.addDependency(task1);
    }, expectedError);
    arktest.expectError(() => { task1.addDependency(task1) }, expectedError);
    arktest.expectError(() => { task1.removeDependency(task3) }, new Error('taskpool:: The dependency does not exist'));
}

async function dependencyExecutedTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(sum, 10, 20);
    arktest.expectError(() => { task1.removeDependency(task2) }, new Error('taskpool:: task has no dependency'));
    let res1 = await taskpool.execute(task2);
    arktest.assertEQ(res1, 30);
    arktest.expectError(() => { task1.addDependency(task2) }, new Error('taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot be relied on'));
    arktest.expectError(() => { task2.addDependency(task1) }, new Error('taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot addDependency'));
    let task3 = new taskpool.Task(returnAbc);
    let task4 = new taskpool.Task(max, 30, 10);
    let expectedAgainExecuteError = new Error('taskpool:: executedTask with dependency cannot execute again');
    task1.addDependency(task3);
    let res2 = await taskpool.execute(task3);
    arktest.assertEQ(res2, 'abc');
    arktest.expectError(() => { task1.removeDependency(task3); }, new Error('taskpool:: cannot removeDependency on a dependent and executed task'));
    arktest.expectError(() => { task1.removeDependency(task4); }, new Error('taskpool:: task has no dependency'));
    let res3 = await taskpool.execute(task1);
    arktest.assertEQ(res3, 'abc');
    arktest.expectError(() => { await taskpool.execute(task1) }, expectedAgainExecuteError);
    arktest.expectError(() => { await taskpool.execute(task3) }, expectedAgainExecuteError);
    // Try to remove dependency after execute
    let isRuning1 = new Event();
    let isRuning2 = new Event();
    let eventTask1 = () =>{
        isRuning1.fire();
    }
    let eventTask2 = () =>{
        isRuning2.fire();
    }
    let task5 = new taskpool.Task(eventTask1);
    let task6 = new taskpool.Task(eventTask2);
    task5.addDependency(task6);
    taskpool.execute(task5);
    arktest.expectError(() => { task5.removeDependency(task6); }, new Error('taskpool:: executedTask cannot removeDependency'));
    taskpool.execute(task6);
    isRuning1.wait();
    isRuning2.wait();
}

async function sendReceiveDataTest() {
    let res = 0;
    let recvCallback = (): string => {
        res += 10;
        return '';
    };
    let sendDataFunc = (): string => {
        taskpool.Task.sendData();
        return 'abc';
    };
    let t1 = new taskpool.Task(sendDataFunc);
    t1.onReceiveData(recvCallback);
    let t2 = new taskpool.Task(sendDataFunc);
    t2.onReceiveData(recvCallback);
    let t3 = new taskpool.Task(sendDataFunc);
    t3.onReceiveData(recvCallback);
    let res1 = await taskpool.execute(t1);
    let res2 = await taskpool.execute(t2);
    arktest.assertEQ(res1, 'abc');
    arktest.assertEQ(res2, 'abc');
    arktest.assertEQ(res, 20);
    let r = await taskpool.execute(t3);
    arktest.assertEQ(r, 'abc');
    arktest.assertEQ(res, 30);
}

async function sendDataAndCancelTest() {
    // Cancel the task after the sendData is executed on the taskpool task.
    let value = 0;
    let recvCallback = (): string => {
        value += 10;
        return '';
    };
    let isRunning = new Event();
    let sendDataFunc = (): string => {
        isRunning.fire();
        taskpool.Task.sendData();
        let t = Date.now();
        while ((Date.now() - t) < 200) {
            continue;
        }
        if (taskpool.Task.isCanceled()) {
            return 'def';
        }
        t = Date.now();
        while ((Date.now() - t) < 200) {
            continue;
        }
        taskpool.Task.sendData();
        return 'abc';
    };
    let task = new taskpool.Task(sendDataFunc);
    task.onReceiveData(recvCallback);
    let p = taskpool.execute(task);
    isRunning.wait();
    try {
        taskpool.cancel(task);
    } catch (err) {
        let e = err as Error;
        arktest.assertEQ(e.message, 'taskpool:: task is not executed or has been executed');
        return undefined;
    }
    arktest.expectError(() => { await p }, new Error('taskpool:: task has been canceled'));
    // When the task is canceled, recvCallback will no longer be executed.
    arktest.assertEQ(value, 10);
}

async function tasksWithDifferentParamsTest() {
    let testWithZeroParam = (): string => {
        return 'testWithZeroParam';
    }
    let testWithOneParam = (num: int): int => {
        return num * 2;
    }
    let testWithThreeParams = (a: number, str: string, b: number): string => {
        let sum: number = a + b;
        return 'testWithThreeParams res: ' + sum;
    }
    let testWithFiveParams = (a: number, b: number, c: number, flag: boolean, str: string): string => {
        let res: number = 0;
        res = flag ? (a + b + c) : (a * b * c);
        return str + ' res: ' + res;
    }
    let testWith16Params = (a: number, b: number, c: number, d: number, e: number, f: number, g: number, h: number,
                            i: number, j: number, k: number, l: number, m: number, n: number, o: number, p: number): number => {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    }
    let task1 = new taskpool.Task(testWithZeroParam);
    let task2 = new taskpool.Task(testWithOneParam, 15);
    let task3 = new taskpool.Task(testWithThreeParams, 10.0, 'task3', 20.0);
    let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, 'TEST');
    let task5 = new taskpool.Task(testWith16Params, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                                    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
    let res1 = await taskpool.execute(task1);
    let res2 = await taskpool.execute(task2);
    let res3 = await taskpool.execute(task3);
    let res4 = await taskpool.execute(task4);
    let res5 = await taskpool.execute(task5);
    arktest.assertEQ(res1, 'testWithZeroParam');
    arktest.assertEQ(res2, 30);
    arktest.assertEQ(res3, 'testWithThreeParams res: 30');
    arktest.assertEQ(res4, 'TEST res: 6');
    arktest.assertEQ(res5, 16.0);
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    arktest.assertEQ(task4.isDone(), true);
    arktest.assertEQ(task5.isDone(), true);
}

async function taskPoolInTaskPoolTest() {
    // execute the taskpool task in the taskpool task
    let res: int = 0;
    let innerFunc = (a: int): int => {
        let t = Date.now();
        res += a;
        while ((Date.now() - t) < 50) {
            continue;
        }
        return a + 10;
    };
    let outerFunc = (): string => {
        let innerTask = new taskpool.Task(innerFunc, 10);
        let innerRes = await taskpool.execute(innerTask);
        arktest.assertEQ(innerRes, 20);
        res += 20;
        return '';
    };
    let task = new taskpool.Task(outerFunc);
    await taskpool.execute(task);
    arktest.assertEQ(res, 30);
    arktest.assertEQ(task.isDone(), true);
}

async function tasksExecutionTest() {
    // execute a large number of taskpool tasks
    let num: int = 0;
    // count was reduced due to stack limits on arm32
    let count: int = 10;
    let taskArray: Array<taskpool.Task> = new Array<taskpool.Task>();
    for (let i: int = 0; i < count; ++i) {
        let task: taskpool.Task = new taskpool.Task(sum, 10, 20);
        taskArray.push(task);
    }
    let allExecuted = new Event();
    for (let i: int = 0; i < count; ++i) {
        taskpool.execute(taskArray[i]).then((value: Any): void => {
            num++;
            if (num == count) {
                allExecuted.fire();
            }
        })
    }
    allExecuted.wait();
    arktest.assertEQ(taskArray[0].isDone(), true);
    arktest.assertEQ(taskArray[5].isDone(), true);
    arktest.assertEQ(num, count);
}

async function taskDurationTest() {
    let task1 = new taskpool.Task(longFunc, 200);
    arktest.assertEQ(task1.ioDuration, 0);
    arktest.assertEQ(task1.cpuDuration, 0);
    arktest.assertEQ(task1.totalDuration, 0);
    let res1 = await taskpool.execute(task1);
    arktest.assertEQ(res1, 'success');
    arktest.assertNE(task1.cpuDuration, 0);
    arktest.assertNE(task1.totalDuration, 0);
    let task2 = new taskpool.Task(longFunc, 1000);
    arktest.assertEQ(task2.ioDuration, 0);
    arktest.assertEQ(task2.cpuDuration, 0);
    arktest.assertEQ(task2.totalDuration, 0);
    let res2 = await taskpool.execute(task2);
    arktest.assertEQ(res2, 'success');
    arktest.assertNE(task2.cpuDuration, 0);
    arktest.assertNE(task2.totalDuration, 0);
}

async function taskAssigmentTest() {
    let task1WorkerId = 0;
    let task2WorkerId = 0;
    let task1 = new taskpool.Task(() => {
        longFunc(50);
        task1WorkerId = CoroutineExtras.getWorkerId();
    });
    let task2 = new taskpool.Task(() => {
        longFunc(50);
        task2WorkerId = CoroutineExtras.getWorkerId();
    });
    let p1 = taskpool.execute(task1);
    let p2 = taskpool.execute(task2);
    await p1;
    await p2;
    arktest.assertNE(task1WorkerId, 0);
    arktest.assertNE(task2WorkerId, 0);
}

class ArgTestClass {
    a1: string = 'a1';
    a2: number = 2.0;
}

function taskArgumentsTest() {
    let taskFunc = (p1: string, p2: number, p3: int, p4: ArgTestClass) => { };
    let argTestObj = new ArgTestClass();
    let task = new taskpool.Task(taskFunc, 'p1', 2.0, 2, argTestObj);
    let arguments = task.arguments as Array<Any>;
    arktest.assertEQ(arguments.length, 4);
    arktest.assertEQ(arguments[0], 'p1');
    arktest.assertEQ(arguments[1], 2.0);
    arktest.assertEQ(arguments[2], 2);
    arktest.assertEQ(arguments[3], argTestObj);
}

async function dependencyThenCallbackExecuteSequenceTest() {
    let checkPoints = new Array<number>();
    checkPoints.push(1);
    let task1 = new taskpool.Task('task1', () => {
        checkPoints.push(2);
    })
    let task2 = new taskpool.Task('task2', () => {
        checkPoints.push(4);
    })
    task2.addDependency(task1);
    let p2 = taskpool.execute(task2).then((res: Any) => {
        checkPoints.push(5);
    })
    let p1 = taskpool.execute(task1).then((res: Any) => {
        checkPoints.push(3);
        taskpool.cancel(task2);
    });
    await p1;
    let infos = taskpool.getTaskPoolInfo().taskInfos;
    arktest.assertEQ(infos.length, 1);
    arktest.assertEQ(infos[0].state, taskpool.State.CANCELED);
    arktest.assertEQ(checkPoints.length, 3);
}

async function dependencyErrorTaskTest() {
    let checkPoints = new Array<number>();
    checkPoints.push(1);
    let task1 = new taskpool.Task('task1', () => {
        checkPoints.push(2);
        throw new Error('error');
    })
    let task2 = new taskpool.Task('task2', () => {
        checkPoints.push(3);
    })
    task2.addDependency(task1);
    let p2 = taskpool.execute(task2).then((res: Any):void => {
        checkPoints.push(4);
    })
    arktest.expectError(() => {
        await taskpool.execute(task1);
    }, new Error('error'));
    await p2;
    arktest.assertEQ(checkPoints.length, 4);
}

async function delay(ms: int) {
    await new Promise<void>((res, rej) => {
        setTimeout(() => {
            res(undefined);
        }, ms);
    });
}

async function dependencyCancelTaskTest() {
    let checkPoints = new Array<number>();
    let task1 = new taskpool.Task('task1', () => {
        checkPoints.push(1);
    });
    let task2 = new taskpool.Task('task2', () => {
        checkPoints.push(2);
    });
    let task3 = new taskpool.Task('task3', () => {
        checkPoints.push(3);
    });
    task2.addDependency(task1);
    task3.addDependency(task2);
    taskpool.execute(task3);
    taskpool.execute(task2);
    await taskpool.execute(task1).then(() => {
        taskpool.cancel(task2);
    });
    await delay(50);
    arktest.assertEQ(checkPoints.length, 1);
    arktest.assertEQ(checkPoints[0], 1);
}

async function doubleCancelByDependencyTest() {
    let checkPoints = new Array<number>();
    let task1 = new taskpool.Task('task1', () => {
        checkPoints.push(1);
    });
    let task2 = new taskpool.Task('task2', () => {
        checkPoints.push(2);
    });
    let task3 = new taskpool.Task('task3', () => {
        checkPoints.push(3);
    });
    task2.addDependency(task1);
    task3.addDependency(task2);
    taskpool.execute(task3);
    taskpool.execute(task2);
    taskpool.cancel(task3);
    await taskpool.execute(task1).then(() => {
        taskpool.cancel(task2);
    });
    await delay(50);
    arktest.assertEQ(checkPoints.length, 1);
    arktest.assertEQ(checkPoints[0], 1);
}

let isExecuted: AtomicFlag = new AtomicFlag(false);
let maxCount: int = max(4, CoroutineExtras.getTaskPoolWorkersLimit());
let executedCount: int = 0;
function expandTest() {
    while (!isExecuted.get()) {}
}

function reset() {
    isExecuted.set(false);
    executedCount = 0;
}

function taskpoolExpandTest() {
    for (let i = 1; i <= 20; ++i) {
        taskpool.execute(expandTest, 10).then(() => {
            executedCount++;
        })
    }
    while (CoroutineExtras.getTaskPoolWorkersNum() < maxCount) {
        delayWithYield(10);
    }
    isExecuted.set(true);
    while (executedCount != 20) {
        delayWithYield(10);
    }
    arktest.assertTrue(CoroutineExtras.getTaskPoolWorkersNum() > 2);
    reset();
}

function taskpoolMaxWorkersTest() {
    maxCount = max(10, CoroutineExtras.getTaskPoolWorkersLimit());
    CoroutineExtras.setTaskPoolWorkersLimit(maxCount);
    for (let i = 1; i <= 50; ++i) {
        taskpool.execute(expandTest, 10).then(() => {
            executedCount++;
        })
    }
    while (CoroutineExtras.getTaskPoolWorkersNum() < maxCount) {
        delayWithYield(10);
    }
    isExecuted.set(true);
    while (executedCount != 50) {
        delayWithYield(10);
    }
    arktest.assertEQ(CoroutineExtras.getTaskPoolWorkersNum(), maxCount);
    reset();
}

async function taskpoolRetriggerShrinkTest() {
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(100000);
    CoroutineExtras.setTaskPoolIdleThreshold(200000);
    CoroutineExtras.retriggerTaskPoolShrink();
    for (let i = 1; i <= 20; ++i) {
        taskpool.execute(expandTest).then(() => {
            executedCount++;
        })
    }
    while(CoroutineExtras.getTaskPoolWorkersNum() < maxCount) {
        delayWithYield(10);
    }
    isExecuted.set(true);
    while (executedCount != 20) {
        delayWithYield(10);
    }
    await taskpool.execute(longFunc, 200);
    let num = CoroutineExtras.getTaskPoolWorkersNum();
    arktest.assertEQ(num, maxCount);
    reset();
}

async function taskpoolShrinkTest() {
    let expandedNum: int = CoroutineExtras.getTaskPoolWorkersNum();
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(50);
    CoroutineExtras.setTaskPoolIdleThreshold(50);
    CoroutineExtras.retriggerTaskPoolShrink();
    await taskpool.execute(longFunc, 200);
    let shrinkedNum = CoroutineExtras.getTaskPoolWorkersNum();
    arktest.assertTrue(expandedNum > shrinkedNum);
    reset();
    // avoid impact other tests
    CoroutineExtras.setTaskPoolTriggerShrinkInterval(30000);
    CoroutineExtras.setTaskPoolIdleThreshold(60000);
    CoroutineExtras.retriggerTaskPoolShrink();
}

async function testExecuteUndefined() {
    let callback = (arg?: int) => {
        arktest.assertEQ(arg, undefined);
    }
    let task = new taskpool.Task(callback, undefined);
    await taskpool.execute(task);
}

class TestFoo {
    static total: number = 0;
    static tag: string = "foo";

    name: string;
    age: number;
    active: boolean;
    nums: number[];

    constructor(name: string, age: number) {
        this.name = name;
        this.age = age;
        this.active = true;
        this.nums = [];
        TestFoo.total++;
    }

    public describe(): string {
        return `${this.name}:${this.age}:${this.active}`;
    }

    public static create(name: string): TestFoo {
        return new TestFoo(name, 0);
    }
}

async function useTestFoo() {
    let task = new taskpool.Task(TestFoo.create, 'test');
    let res = await taskpool.execute(task) as TestFoo;
    let type = Class.from<TestFoo>();
    res.nums.push(1, 2, 3);
    arktest.assertEQ(res.describe(), 'test:0:true');
    arktest.assertEQ(RuntimeDebug.countInstancesOfClass(type), 1);
}

function testTaskpoolReturnObjWithGC() {
    waitForCompletion(useTestFoo);

    let type = Class.from<TestFoo>();
    let instanceCount = RuntimeDebug.countInstancesOfClass(type);
    if (instanceCount != 0) {
        const gcId = GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE);
        if (gcId > 0) {
            GC.waitForFinishGC(gcId);
        }
    }

    arktest.assertEQ(RuntimeDebug.countInstancesOfClass(type), 0);
}

function executeAfterExecutePeriodically() {
    let task = new taskpool.Task(sum, 1, 2);
    taskpool.executePeriodically(100, task);
    arktest.expectError(() => {
        taskpool.execute(task);
    }, new Error("taskpool:: the periodicTask cannot execute again"));
    taskpool.cancel(task);
}

function periodicTaskAddDepedency() {
    let task1 = new taskpool.Task(sum, 1, 2);
    let task2 = new taskpool.Task(sum, 3, 4);
    taskpool.executePeriodically(100, task1);
    arktest.expectError(() => {
        task1.addDependency(task2);
    }, new Error("taskpool:: the periodic task cannot have a dependency"));
    taskpool.cancel(task1);
}

function periodicTaskAddedAsDepedency() {
    let task1 = new taskpool.Task(sum, 1, 2);
    let task2 = new taskpool.Task(sum, 3, 4);
    taskpool.executePeriodically(100, task1);
    arktest.expectError(() => {
        task2.addDependency(task1);
    }, new Error("taskpool:: the periodic task cannot have a dependency"));
    taskpool.cancel(task1);
}

async function executeOneTaskThreetimes(){
    let checkPoints = new containers.LinkedBlockingQueue<int>();
    let isTaskFinished = new Event();
    let task = new taskpool.Task('executeOneTaskThreetimes', ()=> {
        checkPoints.push(1);
        if (checkPoints.size == 3) {
            isTaskFinished.fire();
        }
    });
    taskpool.execute(task);
    taskpool.execute(task);
    taskpool.execute(task);
    isTaskFinished.wait();
    arktest.assertEQ(checkPoints.size, 3);
}

async function executeDelayedOneTaskThreetimes(){
    let checkPoints = new containers.LinkedBlockingQueue<int>();
    let isTaskFinished = new Event();
    let task = new taskpool.Task('executeDelayedOneTaskThreetimes', () => {
        checkPoints.push(1);
        if (checkPoints.size == 3) {
            isTaskFinished.fire();
        }
    });
    taskpool.executeDelayed(100, task);
    taskpool.executeDelayed(100, task);
    taskpool.executeDelayed(100, task);
    isTaskFinished.wait();
    arktest.assertEQ(checkPoints.size, 3);
}

async function cancelAsyncRunnerWaittingTask(){
    let catchCalled = false;
    let runner = new taskpool.AsyncRunner(1);
    let event = new Event();
    let blockTask = (): void => {
        event.wait();
        return;
    }

    let emptyTask = (): void => {
        return;
    }

    let task1 = new taskpool.Task(blockTask);
    let task2 = new taskpool.Task(blockTask);
    let p1 = runner.execute(task1);
    let p2 = runner.execute(task2).catch((e: Error) => {
        catchCalled = true;
        arktest.assertEQ(e.message, 'taskpool:: asyncRunner task has been canceled');
    });
    taskpool.cancel(task2);
    event.fire();
    await p1;
    await p2;
    arktest.assertEQ(catchCalled, true);
}

function main(): int {
    let commonTaskSuite = new arktest.ArkTestsuite('taskpool.CommonTask');
    let commonTaskSuiteDisabled = new arktest.ArkTestsuite('taskpool.CommonTask.DISABLED');
    commonTaskSuite.addAsyncTest('GetTaskpoolInfoTest', testTaskpoolInfo);
    commonTaskSuite.addTest('GetTaskpoolInfoOfNoneNameTaskTest', testTaskpoolInfoOfNoneNameTask);
    commonTaskSuite.addAsyncTest('ExceedContinueTaskCountTest', exceedContinueTaskTest);
    commonTaskSuite.addAsyncTest('OneTaskExecutionTest', oneTaskExecutionTest);
    commonTaskSuite.addAsyncTest('CallbackTest', callbackTest);
    commonTaskSuite.addAsyncTest('IsCancelTest', isCancelTest);
    commonTaskSuite.addAsyncTest('IsCancelPromiseTest', isCancelPromiseTest);
    commonTaskSuite.addAsyncTest('CancelNonExecutingTaskTest', cancelNonExecutingTaskTest);
    commonTaskSuite.addAsyncTest('CancelExecutingTaskTest', cancelExecutingTaskTest);
    commonTaskSuite.addAsyncTest('CheckIsDoneAndCancelTaskTest', checkIsDoneAndCancelTaskTest);
    commonTaskSuite.addAsyncTest('DependentTasksTest', dependentTasksTest);
    // #24758: Test is incorrect and flaky fails
    commonTaskSuiteDisabled.addAsyncTest('DependentTasksWithParamsTest', dependentTasksWithParamsTest);
    commonTaskSuite.addAsyncTest('TestExecuteUndefined', testExecuteUndefined);
    commonTaskSuite.addAsyncTest('CircularDependencyTest', circularDependencyTest);
    commonTaskSuite.addAsyncTest('DependencyExecutedTaskTest', dependencyExecutedTaskTest);
    commonTaskSuite.addAsyncTest('SendReceiveDataTest', sendReceiveDataTest);
    // #32450: Test is incorrect
    commonTaskSuiteDisabled.addAsyncTest('SendDataAndCancelTest', sendDataAndCancelTest);
    commonTaskSuite.addAsyncTest('TasksWithDifferentParamsTest', tasksWithDifferentParamsTest);
    commonTaskSuite.addAsyncTest('TaskPoolInTaskPoolTest', taskPoolInTaskPoolTest);
    commonTaskSuite.addAsyncTest('TasksExecutionTest', tasksExecutionTest);
    commonTaskSuite.addAsyncTest('TaskDurationTest', taskDurationTest);
    commonTaskSuite.addAsyncTest('TaskExecuteDelayedTest', taskExecuteDelayedTest);
    commonTaskSuite.addAsyncTest('getDelayedTaskInfoTest', getDelayedTaskInfoTest);
    commonTaskSuite.addAsyncTest('getDelayedTaskInfoWithCancelTest', getDelayedTaskInfoWithCancelTest);
    commonTaskSuite.addAsyncTest('TaskExecutePeriodicallyTest', taskExecutePeriodicallyTest);
    commonTaskSuite.addAsyncTest('TaskWithDependencyExecutePeriodicallyTest', taskWithDependencyExecutePeriodicallyTest);
    commonTaskSuite.addAsyncTest('TaskOnReceiveDataTest', taskOnReceiveData);
    commonTaskSuite.addAsyncTest('TaskSendDataTest', taskSendData);
    commonTaskSuite.addAsyncTest('TaskSendDataInAsyncFunction', taskSendDataInAsyncFunction);
    commonTaskSuite.addAsyncTest('testOnReceiveDataRestParameter', testOnReceiveDataRestParameter);
    commonTaskSuite.addAsyncTest('taskAssigmentTest', taskAssigmentTest);
    commonTaskSuite.addTest('taskArgumentsTest', taskArgumentsTest);
    //#32501: flaky issue
    commonTaskSuiteDisabled.addAsyncTest('DependencyThenCallbackExecuteSequenceTest', dependencyThenCallbackExecuteSequenceTest);
    commonTaskSuiteDisabled.addAsyncTest('dependencyErrorTaskTest', dependencyErrorTaskTest);
    commonTaskSuiteDisabled.addAsyncTest('dependencyCancelTaskTest', dependencyCancelTaskTest);
    commonTaskSuiteDisabled.addAsyncTest('doubleCancelByDependencyTest', doubleCancelByDependencyTest);
    commonTaskSuiteDisabled.addTest('taskpoolExpandTest', taskpoolExpandTest);
    commonTaskSuiteDisabled.addTest('taskpoolMaxWorkersTest', taskpoolMaxWorkersTest);
    commonTaskSuiteDisabled.addAsyncTest('taskpoolRetriggerShrinkTest', taskpoolRetriggerShrinkTest);
    commonTaskSuiteDisabled.addAsyncTest('taskpoolShrinkTest', taskpoolShrinkTest);
    commonTaskSuite.addAsyncTest('TestExecuteUndefined', testExecuteUndefined);
    commonTaskSuite.addTest('testTaskpoolReturnObjWithGC', testTaskpoolReturnObjWithGC);
    commonTaskSuite.addTest('executeAfterExecutePeriodically', executeAfterExecutePeriodically);
    commonTaskSuite.addTest('periodicTaskAddDepedency', periodicTaskAddDepedency);
    commonTaskSuite.addTest('periodicTaskAddedAsDepedency', periodicTaskAddedAsDepedency);
    commonTaskSuite.addAsyncTest('executeOneTaskThreetimes', executeOneTaskThreetimes);
    commonTaskSuite.addAsyncTest('executeDelayedOneTaskThreetimes',executeDelayedOneTaskThreetimes);
    commonTaskSuite.addAsyncTest('cancelAsyncRunnerWaittingTask', cancelAsyncRunnerWaittingTask);
    let res = commonTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
