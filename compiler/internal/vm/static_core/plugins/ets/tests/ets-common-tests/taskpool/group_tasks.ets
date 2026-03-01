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

function returnAbc(): string {
    return (() => 'a')() + (() => 'b')() + (() => 'c')();
}

function returnDef(): string {
    return (() => 'de')() + (() => 'f')();
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

async function testDelayFunc(): Promise<Any> {
    let task = new taskpool.Task(sum, 100, 300);
    let value = await taskpool.executeDelayed(100, task);
    let g = new taskpool.TaskGroup();
    arktest.expectError(() => {
        g.addTask(task);
    }, new Error('taskpool:: taskGroup cannot add seqRunnerTask or executedTask'));
    return value;
}

async function executeDelayedTest() {
    try {
        let res = await launchImpl(testDelayFunc);
        arktest.assertEQ(res, 400);
    } catch (e) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof NullPointerError, true);
    }
    let task = new taskpool.Task(max, 10, 15);
    let g = new taskpool.TaskGroup();
    g.addTask(task);
    arktest.expectError(() => {
        taskpool.executeDelayed(100, task);
    }, new Error('taskpool:: groupTask cannot executeDelayed outside'));
}

function testPeriodFunc(): Any {
    let task = new taskpool.Task(sum, 100, 300);
    taskpool.executePeriodically(200, task);
    let g = new taskpool.TaskGroup();
    arktest.expectError(() => {
        g.addTask(task);
    }, new Error('taskpool:: The interface does not support the periodicTask'));
    taskpool.cancel(task);
    return 'success';
}

async function executePeriodicallyTest() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    try {
        let res1 = await launchImpl(testPeriodFunc);
        arktest.assertEQ(res1, 'success');
    } catch (e) {
        // NOTE(wangzhaoyong, #25035): del try catch when fix stackoverflow and invalid error message
        arktest.assertEQ(e instanceof NullPointerError, true);
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function grouptaskExecutePeriodicallyTest(): void {
    let task = new taskpool.Task(returnAbc);
    let g = new taskpool.TaskGroup();
    g.addTask(task);
    arktest.expectError(() => {
        taskpool.executePeriodically(10, task);
    }, new Error('taskpool:: The concurrent task has been executed and cannot be executed periodically'));
}

async function groupTaskExecuteRepeat() {
    let task : taskpool.Task = new taskpool.Task(sum, 100, 300);
    let group = new taskpool.TaskGroup();
    group.addTask(task);

    let a = await taskpool.execute(group);
    arktest.assertEQ(a[0], 400);
    let res = await taskpool.execute(group);
    arktest.assertEQ(res[0], 400);
}

function taskGroupNestedTaskTest(): void {
    let group = new taskpool.TaskGroup();
    let task : taskpool.Task = new taskpool.Task(()=>{
        taskpool.execute(group);
    })
    group.addTask(task);
    arktest.expectError(() => {
        waitForCompletion<void>(async (): Promise<void> => {
            await taskpool.execute(task);
        })
    }, new Error('taskpool:: groupTask cannot execute outside'));
}

async function runOneGroupTest() {
    let g2 = new taskpool.TaskGroup('g2');
    arktest.assertEQ(g2.name, 'g2');
    let t1 = new taskpool.Task(returnAbc);
    let t2 = new taskpool.Task(returnDef);
    // tasks with params
    let t3 = new taskpool.Task(sum, 10, 20);
    let t4 = new taskpool.Task(max, 80, 30);
    g2.addTask(t1);
    g2.addTask(t2);
    g2.addTask(t3);
    g2.addTask(t4);
    let res2 = await taskpool.execute(g2);
    arktest.assertEQ(res2[0], 'abc');
    arktest.assertEQ(res2[1], 'def');
    arktest.assertEQ(res2[2], 30);
    arktest.assertEQ(res2[3], 80);
    arktest.assertEQ(g2.name, 'g2');
    // Run group without tasks
    let g0 = new taskpool.TaskGroup('g0');
    let res0 = await taskpool.execute(g0);
    arktest.assertEQ(res0.length, 0);
    arktest.assertEQ(g0.name, 'g0');
}

async function runSeveralGroupsTest() {
    let g1 = new taskpool.TaskGroup();
    let g2 = new taskpool.TaskGroup();
    let g3 = new taskpool.TaskGroup();
    // Group 1
    let t1 = new taskpool.Task(returnAbc);
    let t2 = new taskpool.Task(returnDef);
    g1.addTask(t1);
    g1.addTask(t2);
    g1.addTask(returnAbc);
    g1.addTask(sum, 10, 20);
    // Group 2
    g2.addTask(returnDef);
    g2.addTask(returnAbc);
    g2.addTask(max, 80, 30);
    // Execute group 1
    let p1 = taskpool.execute(g1);
    // Group 3
    g3.addTask(returnAbc);
    g3.addTask(new taskpool.Task(returnDef));
    g3.addTask(new taskpool.Task(sum, 100, 60));
    // Execute group 3 and group 2
    let p3 = taskpool.execute(g3);
    let p2 = taskpool.execute(g2);
    // await result for all groups
    let res1 = await p1;
    let res3 = await p3;
    let res2 = await p2;
    // Asserts for group 1
    arktest.assertEQ(res1[0], 'abc');
    arktest.assertEQ(res1[1], 'def');
    arktest.assertEQ(res1[2], 'abc');
    arktest.assertEQ(res1[3], 30);
    arktest.assertEQ(t1.isDone(), true);
    arktest.assertEQ(t2.isDone(), true);
    // Asserts for group 2
    arktest.assertEQ(res2[0], 'def');
    arktest.assertEQ(res2[1], 'abc');
    arktest.assertEQ(res2[2], 80);
    // Asserts for group 3
    arktest.assertEQ(res3[0], 'abc');
    arktest.assertEQ(res3[1], 'def');
    arktest.assertEQ(res3[2], 160);
}

function cancelNonStartedGroupTest() {
    let g = new taskpool.TaskGroup();
    g.addTask(returnAbc);
    g.addTask(returnDef);
    arktest.expectError(() => {
        taskpool.cancel(g);
        taskpool.execute(g);
    }, new Error('taskpool:: taskGroup is not executed or has been executed'));
}

function waitWaiterSignal(event : Event) {
    event.wait();
}

function cancelExecutingGroupTest() {
    let g = new taskpool.TaskGroup();
    let waiter = new Event();
    let task = new taskpool.Task(waitWaiterSignal, waiter);
    g.addTask(task);
    g.addTask(returnDef);
    let p = taskpool.execute(g);
    try {
        taskpool.cancel(g);
        waiter.fire();
    } catch (e) {
        let err = e as Error;
        arktest.assertEQ(err.message, 'taskpool:: taskGroup is not executed or has been executed');
        return;
    }
    arktest.expectError(() => {
        waitForCompletion(async (): Promise<void> => {
            await p;
        })
    }, new Error('taskpool:: taskGroup has been canceled'));
}

function cancelExecutedGroupTest1() {
    let g = new taskpool.TaskGroup();
    g.addTask(returnAbc);
    g.addTask(returnDef);
    arktest.expectError(() => {
        await taskpool.execute(g);
        // Group tasks finished, so 'cancel' should produce the Error
        taskpool.cancel(g);
    }, new Error('taskpool:: taskGroup is not executed or has been executed'));
}

function cancelExecutedGroupTest2() {
    // Cancel the completed taskGroup in the then branch.
    let result: int[] = [1, 1, 1];
    let g = new taskpool.TaskGroup();
    let task = new taskpool.Task(sum, 15, 15);
    g.addTask(sum, 100, 60);
    g.addTask(max, 80, 30);
    g.addTask(task);
    taskpool.execute(g).then((res: object): void => {
        arktest.expectError(() => {
            taskpool.cancel(g);
        }, new Error('taskpool:: taskGroup is not executed or has been executed'));
        result = res as int[];
        arktest.assertEQ(result[0], 160);
        arktest.assertEQ(result[1], 80);
        arktest.assertEQ(result[2], 30);
    });
}

function addExecutedTaskTest() {
    let g = new taskpool.TaskGroup();
    let expectedError = new Error('taskpool:: taskGroup cannot add seqRunnerTask or executedTask');
    // Executed task
    let task = new taskpool.Task(returnAbc);
    taskpool.execute(task);
    arktest.expectError(() => { g.addTask(task) }, expectedError);
    // Executed task via SequenceRunner
    let runnerTask = new taskpool.Task(returnAbc);
    let runner = new taskpool.SequenceRunner();
    runner.execute(runnerTask);
    arktest.expectError(() => { g.addTask(runnerTask) }, expectedError);
}

function addLongTaskTest() {
    let g = new taskpool.TaskGroup();
    let t = new taskpool.LongTask(returnAbc);
    arktest.expectError(() => { g.addTask(t) },
                new Error('taskpool:: The interface does not support the long task'));
}

function addTaskFromGroupTest() {
    let g1 = new taskpool.TaskGroup();
    let t1 = new taskpool.Task(returnAbc);
    g1.addTask(t1);
    let g2 = new taskpool.TaskGroup();
    let t2 = new taskpool.Task(returnDef);
    g2.addTask(t2);
    let expectedError = new Error('taskpool:: taskGroup cannot add groupTask');
    // Add the same task
    arktest.expectError(() => { g1.addTask(t1) }, expectedError);
    // Add task from another group
    arktest.expectError(() => { g1.addTask(t2) }, expectedError);
}

function addDependentTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(returnAbc);
    task1.addDependency(task2);
    let taskGroup = new taskpool.TaskGroup();
    arktest.expectError(() => { taskGroup.addTask(task1) }, new Error('taskpool:: dependent task not allowed.'));
}

function addDependencyForGroupTaskTest() {
    let task1 = new taskpool.Task(returnAbc);
    let taskGroup = new taskpool.TaskGroup();
    taskGroup.addTask(task1);
    let task2 = new taskpool.Task(returnAbc);
    arktest.expectError(() => { task1.addDependency(task2) }, new Error('taskpool:: groupTask cannot addDependency'));
    arktest.expectError(() => { task2.addDependency(task1) }, new Error('taskpool:: groupTask cannot be relied on'));
}

function addTaskWithDifferentParamsTest() {
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
    let test = (a: number, b: number): number => {
        let t = Date.now();
        // mock time-consuming operation
        while ((Date.now() - t) < 500) {
            continue;
        }
        return a + b;
    }
    let task1 = new taskpool.Task(testWithZeroParam);
    let task2 = new taskpool.Task(testWithOneParam, 15);
    let task3 = new taskpool.Task(testWithThreeParams, 10.0, 'task3', 20.0);
    let task4 = new taskpool.Task(testWithFiveParams, 1.0, 2.0, 3.0, true, 'TEST');
    let longTask = new taskpool.LongTask(test, 10.0, 20.0);
    let taskGroup = new taskpool.TaskGroup();
    taskGroup.addTask(task1);
    taskGroup.addTask(task2);
    taskGroup.addTask(task3);
    taskGroup.addTask(task4);
    arktest.expectError(() => { taskGroup.addTask(longTask) },
                new Error('taskpool:: The interface does not support the long task'));
    let res = await taskpool.execute(taskGroup);
    arktest.assertEQ(res[0], 'testWithZeroParam');
    arktest.assertEQ(res[1], 30);
    arktest.assertEQ(res[2], 'testWithThreeParams res: 30');
    arktest.assertEQ(res[3], 'TEST res: 6');
    arktest.assertEQ(task1.isDone(), true);
    arktest.assertEQ(task2.isDone(), true);
    arktest.assertEQ(task3.isDone(), true);
    arktest.assertEQ(task4.isDone(), true);
    arktest.assertEQ(longTask.isDone(), false);
}

function main(): int {
    let taskGroupTestsuite = new arktest.ArkTestsuite('taskpool.TaskGroup');
    taskGroupTestsuite.addAsyncTest('ExecuteDelayedTest', executeDelayedTest);
    taskGroupTestsuite.addAsyncTest('ExecutePeriodicallyTest', executePeriodicallyTest);
    taskGroupTestsuite.addTest('GrouptaskExecutePeriodicallyTest', grouptaskExecutePeriodicallyTest);
    taskGroupTestsuite.addAsyncTest('GroupTaskExecuteRepeatTest', groupTaskExecuteRepeat);
    taskGroupTestsuite.addTest('taskGroupNestedTaskTest', taskGroupNestedTaskTest);
    taskGroupTestsuite.addAsyncTest('RunOneGroupTest', runOneGroupTest);
    taskGroupTestsuite.addAsyncTest('RunSeveralGroupsTest', runSeveralGroupsTest);
    taskGroupTestsuite.addTest('CancelNonStartedGroupTest', cancelNonStartedGroupTest);
    taskGroupTestsuite.addTest('CancelExecutingGroupTest', cancelExecutingGroupTest);
    taskGroupTestsuite.addTest('CancelExecutedGroupTest1', cancelExecutedGroupTest1);
    taskGroupTestsuite.addTest('CancelExecutedGroupTest2', cancelExecutedGroupTest2);
    taskGroupTestsuite.addTest('AddExecutedTaskTest', addExecutedTaskTest);
    taskGroupTestsuite.addTest('AddLongTaskTest', addLongTaskTest);
    taskGroupTestsuite.addTest('AddTaskFromGroupTest', addTaskFromGroupTest);
    taskGroupTestsuite.addTest('AddDependentTaskTest', addDependentTaskTest);
    taskGroupTestsuite.addTest('AddDependencyForGroupTaskTest', addDependencyForGroupTaskTest);
    taskGroupTestsuite.addTest('AddTaskWithDifferentParamsTest', addTaskWithDifferentParamsTest);
    let res = taskGroupTestsuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
