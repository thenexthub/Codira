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

const INCREMENT = 5;
const EXPECTED_VALUE = 42;

let counter = 0;

function clearCounter() {
    counter = 0;
}

function foo(): number {
    return EXPECTED_VALUE;
}

async function fooAsync(): Promise<number> {
    return foo();
}

function add(num: int) {
    counter += num;
}

async function addAsync(num: number) {
    await Promise.resolve('a');
    counter += num;
}

function throwLaunchError() {
    throw new Error();
}

async function throwAsyncError(): Promise<void> {
    throw new Error();
}

function testFooLaunch(): number {
    let job : Job<number> = launch<number, ()=>number>(foo);
    let result = job.Await();

    return result;
}

function testAddSchedule(): void {
    clearCounter();
    let job: Job<void> = launch<void, (num: int)=>void>(add, INCREMENT);

    Coroutine.Schedule();
}

function recursiveFoo(): Job<number> {
    let job: Job<number> = launch<number, ()=>number>(testFooLaunch);

    return job;
}  

function testRecursiveLaunch(): number {
    let job: Job<Job<number>> = launch<Job<number>, ()=>Job<number>>(recursiveFoo);
    let result = job.Await().Await();

    return result;
}

function testLaunchThrow(): boolean {
    try {
        let job: Job<void> = launch<void, ()=>void>(throwLaunchError);
        job.Await();
    } catch (e) {
        return e instanceof Error;
    }

    return false;
}

function testFooAsync(): number {
    return waitForCompletion<number>(fooAsync);
}

function testAddAsyncSchedule(): void {
    clearCounter();

    addAsync(INCREMENT);
    Coroutine.Schedule();
}

async function recursiveAsyncFoo(): Promise<number> {
    return fooAsync();
}

function testRecursiveAsync(): number {
    return waitForCompletion<number>(recursiveAsyncFoo);
}

async function testAsyncThrow(): Promise<boolean> {
    try {
        let p: Promise<void> = throwAsyncError();
        await p;
    } catch (e) {
        return e instanceof Error;
    }

    return false;
}

function testACoroutineEaworker(): void {
    let ew = new EAWorker();
    ew.start();
    let job0 = ew.run<number>(testFooLaunch);
    let job1 = ew.run<void>(testAddSchedule);
    let job2 = ew.run<number>(testRecursiveLaunch);
    let job3 = ew.run<boolean>(testLaunchThrow);

    arktest.assertEQ(EXPECTED_VALUE, job0.Await());
    job1.Await();
    arktest.assertEQ(counter, INCREMENT);
    arktest.assertEQ(EXPECTED_VALUE, job2.Await());
    arktest.assertTrue(job3.Await());

    ew.join();
}

function testAJCoroutineEaworker(): void {
    let ew = new EAWorker();
    ew.start();
    let job0 = ew.run<number>(testFooAsync);
    let job1 = ew.run<void>(testAddAsyncSchedule);
    let job2 = ew.run<number>(testRecursiveAsync);
    let job3 = ew.run<boolean>(testLaunchThrow);

    arktest.assertEQ(EXPECTED_VALUE, job0.Await());
    job1.Await();
    arktest.assertEQ(counter, INCREMENT);
    arktest.assertEQ(EXPECTED_VALUE, job2.Await());
    arktest.assertTrue(job3.Await());

    ew.join();
}

function testACAJcoroutine(): void {
    let ew = new EAWorker();
    ew.start();
    let job1 = ew.run<number>(testFooLaunch);
    let job2 = ew.run<number>(testFooAsync);
    
    let res1 = job1.Await();
    let res2 = job2.Await();

    ew.join();

    arktest.assertEQ(res1, res2);
}

function testACoroutineCallAJCoroutine(): void {
    let ew = new EAWorker();
    ew.start();
    let job1 = ew.run<number>(() => {
        return waitForCompletion<number>(async (): Promise<number> => {
            let job2: Job<Promise<number>> = launch<Promise<number>, ()=>Promise<number>>(fooAsync)
            let p = job2.Await();
            let result = await p;
            return result;
        });
    });
    ew.join();
    arktest.assertEQ(EXPECTED_VALUE, job1.Await());
}

async function AsyncCallLaunch(): Promise<Job<number>> {
    let job: Job<number> = launch<number, ()=>number>(foo);

    return job;
}

async function testAJCoroutineCallACoroutine(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let job1 = ew.run<Promise<Job<number>>>(AsyncCallLaunch);
    let p = job1.Await();
    let job2: Job<number> = await p;
    let result = job2.Await();
    ew.join();

    arktest.assertEQ(EXPECTED_VALUE, result);
}

function testNestedEAWorker(): void {
    clearCounter();

    let e1 = new EAWorker();
    e1.start();
    let job1 = e1.run<number>(():number => {
        let e2 = new EAWorker();
        e2.start();
        let job2 = e2.run<number>(():number => {
            return foo();
        });
        e2.join();

        return job2.Await();
    });
    let result = job1.Await();

    e1.join();

    arktest.assertEQ(EXPECTED_VALUE, result);
}

function testOfPrioritySetting(): void {
    let e = new EAWorker();
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_DEFAULT);
    e.setPriority(WorkerPriority.PRIORITY_HIGH);
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_HIGH);
    e.start();
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_HIGH);
    e.join();
}

function main(): int {
    const suite = new arktest.ArkTestsuite('EAWorker Test Suite');

    suite.addTest('testACoroutineEaworker', testACoroutineEaworker);
    suite.addTest('testAJCoroutineEaworker', testAJCoroutineEaworker);
    suite.addTest('testACAJcoroutine', testACAJcoroutine);
    suite.addTest('testACoroutineCallAJCoroutine', testACoroutineCallAJCoroutine);
    suite.addAsyncTest('testAJCoroutineCallACoroutine', testAJCoroutineCallACoroutine);
    suite.addTest('testNestedEAWorker', testNestedEAWorker);
    
    return suite.run();
}
