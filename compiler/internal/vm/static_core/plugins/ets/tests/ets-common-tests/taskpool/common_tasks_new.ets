/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { CoroutineExtras } from 'std/debug/concurrency';

function returnAbc(): string {
    return (() => 'a')() + (() => 'b')() + (() => 'c')();
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

async function cancelInSuccessCallback(){
    let syncSuccessCallbackCalled: boolean = false;
    let asyncSuccessCallbackCalled: boolean = false;
    let timeoutPromise = async (duration: int): Promise<void> => {
        await new Promise<void>((resolve) => {
            setTimeout(() => {
                resolve(undefined);
            }, duration);
        });
    }

    let task1 = new taskpool.Task(returnAbc);
    let task2 = new taskpool.Task(timeoutPromise, 100);
    task1.onExecutionSucceeded(() => {
        syncSuccessCallbackCalled = true;
        arktest.expectError(() => {
            taskpool.cancel(task1);
        }, new Error('taskpool:: task is not executed or has been executed'));
    });
    task2.onExecutionSucceeded(() => {
        asyncSuccessCallbackCalled = true;
        arktest.expectError(() => {
            taskpool.cancel(task2);
        }, new Error('taskpool:: task is not executed or has been executed'));
    });

    await taskpool.execute(task1);
    arktest.assertEQ(syncSuccessCallbackCalled, true);
    await taskpool.execute(task2);
    arktest.assertEQ(asyncSuccessCallbackCalled, true);
}

function getcurrentTask() {
    let fun1 = () => {
        waitForCompletion(() => {
            return new Promise<void>((res) => { setTimeout(() => res(undefined), 10); });
        });
        // if we use currentExecutingTasks, t2 will hold currentExecutingTasks because t2's timeout is 20ms(t1 is 10ms).
        // so the sendData() -> getcurrentTask() -> currentExecutingTasks will get task t2;
        taskpool.Task.sendData(1)
    }
    let t1: taskpool.Task = new taskpool.Task(fun1)
    t1.onReceiveData((res: int) => {
        arktest.assertEQ(res, 1);
    })
    let p1 = taskpool.execute(t1);

    let fun2 = () => {
        waitForCompletion(() => {
            return new Promise<void>((res) => { setTimeout(() => res(undefined), 20); });
        });
    }

    // two t2 tasks can ensure that there is at least one t2'sworker assigned to t1' worker
    for(let i = 0; i < 2; ++i) {
        let t2: taskpool.Task = new taskpool.Task(fun2)
        taskpool.execute(t2);
    }

    arktest.expectNoThrow(() => {
        waitForCompletion(() => {
            return p1;
        });
    });
}

async function onExecutionFailedtestAsyncfun(){
    let catchSuccess : int = 0;
    let event = new Event();
    let funca = async() : Any => {
        throw new Error('Error');
    }
    let task = new taskpool.Task(funca);
    task.onExecutionFailed((e : Error) => { 
       catchSuccess++;
       event.fire();
    });
    taskpool.execute(task);
    event.wait();
    arktest.assertEQ(catchSuccess, 1);
}

async function onExecutionFailedtestmultAsyncfun(){
    let catchSuccess : int = 0;
    let event = new Event();
    let funca = async() : Any => {
        throw new Error('Error');
    };
    let task1 = new taskpool.Task(funca);
    let task2 = new taskpool.Task(funca);
    let task3 = new taskpool.Task(funca);
    task3.addDependency(task2);
    task2.addDependency(task1);
    task1.onExecutionFailed((e : Error) => {
       catchSuccess++;
    });
    task2.onExecutionFailed((e : Error) => {
       catchSuccess++;
    });
    task3.onExecutionFailed((e : Error) => {
       catchSuccess++;
       event.fire();
    });
    taskpool.execute(task1);
    taskpool.execute(task2);
    taskpool.execute(task3);
    event.wait();
    arktest.assertEQ(catchSuccess,3);
}

async function onFailCallbackExceptionTest(){
    let callbackCalled: boolean = false;
    let errorCaught: boolean = false;
    let event = new Event();
    let errorFunc = async() : Promise<void> => {
        throw new Error('Original Error');
    };
    let task = new taskpool.Task(errorFunc);
    task.onExecutionFailed((e: Error) => {
        callbackCalled = true;
        throw new Error('Callback Error');
    });
    arktest.expectError(()=>{await taskpool.execute(task);}, new Error('Original Error'));
    arktest.assertEQ(callbackCalled, true);
}

async function multipleFailCallbackExceptionTest(){
    let callbacksCalled: int = 0;
    let errorsCaught: int = 0;
    let errorFunc = async() : Promise<void> => {
        throw new Error('Error');
    };

    let task1 = new taskpool.Task(errorFunc);
    let task2 = new taskpool.Task(errorFunc);

    task1.onExecutionFailed((e: Error) => {
        callbacksCalled++;
        throw new Error('Callback Error 1');
    });

    task2.onExecutionFailed((e: Error) => {
        callbacksCalled++;
        throw new Error('Callback Error 2');
    });

    arktest.expectError(()=>{await taskpool.execute(task1);}, new Error('Error'));
    arktest.expectError(()=>{await taskpool.execute(task2);}, new Error('Error'));
    arktest.assertEQ(callbacksCalled, 2);
}

function main(): int {
    let commonTaskSuite = new arktest.ArkTestsuite('taskpool.CommonTask');
    commonTaskSuite.addAsyncTest('cancelInSuccessCallback', cancelInSuccessCallback);
    commonTaskSuite.addTest('getcurrentTask', getcurrentTask);
    commonTaskSuite.addAsyncTest('onExecutionFailedtestAsyncfun', onExecutionFailedtestAsyncfun);
    commonTaskSuite.addAsyncTest('onExecutionFailedtestmultAsyncfun', onExecutionFailedtestmultAsyncfun);
    commonTaskSuite.addAsyncTest('onFailCallbackExceptionTest', onFailCallbackExceptionTest);
    commonTaskSuite.addAsyncTest('multipleFailCallbackExceptionTest', multipleFailCallbackExceptionTest);

    let res = commonTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
