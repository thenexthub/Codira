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

function returnAbc() {
    return (() => 'a')() + (() => 'b')() + (() => 'c')();
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
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

async function delay(ms: int) {
    await new Promise<void>((res, rej) => {
        setTimeout(() => {
            res(undefined);
        }, ms);
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

async function testDelayFunc(): Promise<Any> {
    let task = new taskpool.Task(max, 80, 90);
    let res = await taskpool.executeDelayed(50, task);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        runner.execute(task);
    }, new Error('taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask'));
    return res;
}

async function executeDelayedTest(): Promise<void> {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res = await launchImpl(testDelayFunc);
        arktest.assertEQ(res, 90);
    } catch (e) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof StackOverflowError, true);
    }
    let runner = new taskpool.SequenceRunner();
    let task = new taskpool.Task(sum, 25, 35);
    await runner.execute(task);
    arktest.expectError(() => {
        taskpool.executeDelayed(100, task);
    }, new Error('taskpool:: seqRunnerTask cannot executeDelayed outside'));
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

async function executePeriodicallyTest() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let runner = new taskpool.SequenceRunner();
    let task1 = new taskpool.Task(returnAbc);
    await runner.execute(task1);
    arktest.expectError(() => {
        taskpool.executePeriodically(100, task1);
    }, new Error('taskpool:: The concurrent task has been executed and cannot be executed periodically'));
    try {
        let res2 = await launchImpl(testPeriodFunc);
        arktest.assertEQ(res2, 'success');
    } catch (e) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof LinkerUnresolvedClassError, true);
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function testPeriodFunc(): Any {
    let task = new taskpool.Task(sum, 80, 90);
    taskpool.executePeriodically(200, task);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        runner.execute(task);
    }, new Error('taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask'));
    taskpool.cancel(task);
    return 'success';
}

async function orderTest() {
    let resultStr: string = '';
    let func1 = (): string => {
        resultStr += '1';
        return resultStr;
    };
    let func2 = (): string => {
        resultStr += '2';
        return resultStr;
    }
    let func3 = (): string => {
        resultStr += '3';
        return resultStr;
    }
    let runner = new taskpool.SequenceRunner();
    let task1 = new taskpool.Task(func1);
    let task2 = new taskpool.Task(func2);
    let task3 = new taskpool.Task(func3);
    let p1 = runner.execute(task1);
    let p2 = runner.execute(task2);
    let p3 = runner.execute(task3);
    let res2 = await p2;
    arktest.assertEQ(res2, '12');
    let res3 = await p3;
    arktest.assertEQ(res3, '123');
    let res1 = await p1;
    arktest.assertEQ(res1, '1');
    arktest.assertEQ(resultStr, '123');
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    // Execute LongTask
    let longTask = new taskpool.LongTask(returnAbc);
    let res = await runner.execute(longTask);
    arktest.assertEQ(res, 'abc');
}

function executeCommonTest() {
    let runner = new taskpool.SequenceRunner();
    let task = new taskpool.Task(returnAbc);
    taskpool.execute(task);
    arktest.expectError(() => { runner.execute(task) },
                new Error('taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask'));
}

function executeSeqRunnerTaskTest() {
    let runner1 = new taskpool.SequenceRunner();
    let runner2 = new taskpool.SequenceRunner();
    let task = new taskpool.Task(returnAbc);
    runner1.execute(task);
    arktest.expectError(() => { runner2.execute(task) },
                new Error('taskpool:: sequenceRunner cannot execute seqRunnerTask or executedTask'));
}

function executeGroupTaskTest() {
    let runner = new taskpool.SequenceRunner();
    let group = new taskpool.TaskGroup();
    let task = new taskpool.Task(returnAbc);
    group.addTask(task);
    arktest.expectError(() => { runner.execute(task) },
                new Error('taskpool:: sequenceRunner cannot execute groupTask'));
}

async function cancelNonStartedTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        taskpool.cancel(task);
        runner.execute(task);
    }, new Error('taskpool:: task is not executed or has been executed'));
    // tasks with params
    let t1 = new taskpool.Task(sum, 10, 20);
    let res1 = await runner.execute(t1);
    arktest.assertEQ(res1, 30);
    let t2 = new taskpool.Task(max, 80, 30);
    let res2 = await runner.execute(t2);
    arktest.assertEQ(res2, 80);
}

async function cancelExecutingTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    let p = runner.execute(task);
    try {
        taskpool.cancel(task);
    } catch (e) {
        let err = e as Error;
        arktest.assertEQ(err.message, 'taskpool:: sequenceRunner task has been executed');
        return undefined;
    }
    arktest.expectError(() => { let res = waitForCompletion<Any>(() => p); }, new Error('taskpool:: sequenceRunner task has been canceled'));
}

async function cancelExecutedTaskTest() {
    let task = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => {
        await runner.execute(task);
        taskpool.cancel(task);
    }, new Error('taskpool:: sequenceRunner task has been executed'));
    // tasks with params
    let t1 = new taskpool.Task(sum, 10, 20);
    let res1 = await runner.execute(t1);
    arktest.assertEQ(res1, 30);
    let t2 = new taskpool.Task(max, 80, 30);
    let res2 = await runner.execute(t2);
    arktest.assertEQ(res2, 80);
}

function addDependentTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    task1.addDependency(task2);
    let runner = new taskpool.SequenceRunner();
    arktest.expectError(() => { runner.execute(task1) }, new Error('taskpool:: sequenceRunner dependent task not allowed.'));
}

function addDependencyExecutedTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    runner.execute(task2);
    arktest.expectError(() => { task1.addDependency(task2) }, new Error('taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot be relied on'));
    arktest.expectError(() => { task2.addDependency(task1) }, new Error('taskpool:: asyncRunnerTask or seqRunnerTask or executedTask cannot addDependency'));
}

async function taskWithDifferentParamsTest() {
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
        if (flag) {
            res = a + b + c;
        } else {
            res = a * b * c;
        }
        return str + ' res: ' + res;
    }

    let awake_waiter: (val: int)=>void = ()=>{};
    let waiter = new Promise<int>((res, rej)=>{awake_waiter = res;})
    let test = async (a: number, b: number): Promise<number> => {
        await waiter;
        return a + b;
    }
    let task1 = new taskpool.Task(testWithZeroParam);
    let task2 = new taskpool.Task(testWithOneParam, 25);
    let task3 = new taskpool.Task(testWithThreeParams, 10.0, 'task3', 20.0);
    let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, 'TEST');
    let longTask = new taskpool.LongTask(test, 10.0, 20.0);
    let runner = new taskpool.SequenceRunner();
    let res1 = await runner.execute(task1);
    let res2 = await runner.execute(task2);
    let res3 = await runner.execute(task3);
    let res4 = await runner.execute(task4);
    awake_waiter(0);
    let res5 = await runner.execute(longTask);
    arktest.assertEQ(res1, 'testWithZeroParam');
    arktest.assertEQ(res2, 50);
    arktest.assertEQ(res3, 'testWithThreeParams res: 30');
    arktest.assertEQ(res4, 'TEST res: 6');
    arktest.assertEQ(res5, 30);
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    arktest.assertEQ(task4.isDone(), true);
    arktest.assertEQ(longTask.isDone(), true);
}

async function testSequenceRunnerexecuteorder(){
    let count : int = 0;
    let runner = new taskpool.SequenceRunner();
    let result : int[] = [];
    let isRunning = new Event();
    for(let i : int = 0; i < 10; i++){
        let task = new taskpool.Task(`task_${i}`, () => {
            await delay(30);
            result.push(i);
            count++;
            if(count >= 10){
                isRunning.fire();
            }
        });
        runner.execute(task);
    }
    isRunning.wait();
    for(let i : int = 0; i< 10; i++) {
        arktest.assertEQ(result[i], i);
    }
}

async function sameNameSharedRunnerTest() {
    let resultStr: string = '';
    let func = (str: string): string => {
        resultStr += str;
        return resultStr;
    };
    
    const sharedName = 'shared_runner';
    let runner1 = new taskpool.SequenceRunner(sharedName);
    let task1 = new taskpool.Task(func, 'a');
    let task2 = new taskpool.Task(func, 'b');
    let p1 = runner1.execute(task1);
    let p2 = runner1.execute(task2);

    let runner2 = new taskpool.SequenceRunner(sharedName);
    let task3 = new taskpool.Task(func, 'c');
    let task4 = new taskpool.Task(func, 'd');
    let p3 = runner2.execute(task3);
    let p4 = runner2.execute(task4);

    await Promise.all<Any>([p1, p2, p3, p4]);
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    arktest.assertEQ(task4.isDone(), true);
    arktest.assertEQ(resultStr, 'abcd');
}

async function cancelSequenceRunnerWaittingTask(){
    let catchCalled = false;
    let runner = new taskpool.SequenceRunner();
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
        arktest.assertEQ(e.message, 'taskpool:: sequenceRunner task has been canceled');
    });
    taskpool.cancel(task2);
    event.fire();
    await p1;
    await p2;
    arktest.assertEQ(catchCalled, true);
}

function main(): int {
    let seqRunnerSuite = new arktest.ArkTestsuite('taskpool.SequenceRunner');
    let seqRunnerSuiteDisabled = new arktest.ArkTestsuite('taskpool.SequenceRunner.DISABLED');
    seqRunnerSuite.addAsyncTest('ExecuteDelayedTest', executeDelayedTest);
    seqRunnerSuite.addAsyncTest('ExecutePeriodicallyTest', executePeriodicallyTest);
    // #27912
    seqRunnerSuiteDisabled.addAsyncTest('OrderTest', orderTest);
    seqRunnerSuite.addTest('ExecuteCommonTest', executeCommonTest);
    seqRunnerSuite.addTest('ExecuteSeqRunnerTaskTest', executeSeqRunnerTaskTest);
    seqRunnerSuite.addTest('ExecuteGroupTaskTest', executeGroupTaskTest);
    seqRunnerSuite.addAsyncTest('CancelNonStartedTaskTest', cancelNonStartedTaskTest);
    seqRunnerSuite.addAsyncTest('CancelExecutingTaskTest', cancelExecutingTaskTest);
    seqRunnerSuite.addAsyncTest('CancelExecutedTaskTest', cancelExecutedTaskTest);
    seqRunnerSuite.addTest('AddDependentTaskTest', addDependentTaskTest);
    seqRunnerSuite.addTest('AddDependencyExecutedTaskTest', addDependencyExecutedTaskTest);
    seqRunnerSuite.addAsyncTest('TaskWithDifferentParamsTest', taskWithDifferentParamsTest);
    seqRunnerSuite.addAsyncTest('TestSequenceRunnerExecuteOrder', testSequenceRunnerexecuteorder);
    seqRunnerSuite.addAsyncTest('sameNameSharedRunnerTest', sameNameSharedRunnerTest);
    seqRunnerSuite.addAsyncTest('cancelSequenceRunnerWaittingTask', cancelSequenceRunnerWaittingTask);
    let res = seqRunnerSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
