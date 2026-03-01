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

class StepSequence {
    constructor(numCheckpoints: int) {
        this.numCheckpoints = numCheckpoints;
        this.sequence = new Array<number>();
    }

    checkOrder() {
        arktest.assertEQ(this.sequence.length, this.numCheckpoints,
                 'Test failed. Expected ' + this.numCheckpoints + ' checkpoints, but got ' + this.sequence.length);
        for (let step = 0; step < this.sequence.length; ++step) {
            arktest.assertEQ(step, this.sequence[step], `Failed at step: ${step}. Got ${this.sequence[step]}`);
        }
    }

    checkpoint(value: int) {
        this.sequence.push(value);
    }

    private sequence: Array<number>;
    private numCheckpoints: int;
}

let executionOrder = new StepSequence(0);
let promiseIsPending: boolean

async function foo() {
    executionOrder.checkpoint(1);
    let p = bar();
    executionOrder.checkpoint(3);
    return p;
}

async function bar() {
    executionOrder.checkpoint(2);
    let emptyBody = () => { return 0; };
    promiseIsPending ? launch<Int, () => Int>(emptyBody).Await()
                     : await Promise.resolve(0);
    executionOrder.checkpoint(5);
}

async function executionOrderTest(isPending: boolean) {
    executionOrder = new StepSequence(6);
    promiseIsPending = isPending;
    executionOrder.checkpoint(0);
    let p = foo();
    executionOrder.checkpoint(4);
    await p;
    executionOrder.checkOrder();
}

async function testWithPendingPromise() {
    executionOrderTest(true);
}

async function testWithResolvedPromise() {
    executionOrderTest(false);
}

async function testPromiseBeforeAsync() {
    executionOrder = new StepSequence(4);
    let af = async () => {
        executionOrder.checkpoint(0);
        await Promise.resolve<Int>(0);
        executionOrder.checkpoint(3);
    };
    Promise.resolve<Int>(0).then(() => {
        executionOrder.checkpoint(2);
    });
    let p = af();
    executionOrder.checkpoint(1);
    await p;
    executionOrder.checkOrder();
}

async function dealPromise(asyncThen: boolean) {
    const p: Promise<string> = new Promise<string>((resolve, reject) => {
        resolve('resolve');
        executionOrder.checkpoint(0);
    });
    if (asyncThen) {
        p.then(async (res: string) => {
            executionOrder.checkpoint(2);
            await Promise.resolve();
            executionOrder.checkpoint(4);
        });
    } else {
        p.then((res: string) => {
            executionOrder.checkpoint(2);
        });
    }
    executionOrder.checkpoint(1);
}

async function testWithPromiseThen() {
    executionOrder = new StepSequence(4);
    let af = async () => {
        await dealPromise(false);
        executionOrder.checkpoint(3);
    }
    await af();
    executionOrder.checkOrder();
}

async function testWithAsyncPromiseThen() {
    executionOrder = new StepSequence(6);
    let af = async () => {
        await dealPromise(true);
        executionOrder.checkpoint(3)
    }
    await af().then((): void => {
        executionOrder.checkpoint(5)
    });
    executionOrder.checkOrder();
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.execution_order');
    testSuite.addAsyncTest('testWithPendingPromise', testWithPendingPromise);
    testSuite.addAsyncTest('testWithResolvedPromise', testWithResolvedPromise);
    testSuite.addAsyncTest('testPromiseBeforeAsync', testPromiseBeforeAsync);
    testSuite.addAsyncTest('testWithPromiseThen', testWithPromiseThen);
    testSuite.addAsyncTest('testWithAsyncPromiseThen', testWithAsyncPromiseThen);
    let res = testSuite.run();
    return res;
}