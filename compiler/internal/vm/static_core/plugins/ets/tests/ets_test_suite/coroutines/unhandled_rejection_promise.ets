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

const asynclock = new AsyncLock();
const REJECTION_COUNT = 5;
const rejectionReason = 'get unhandled promise rejection';

let waiter = new Event();
let count = 0;

class Event {
    constructor() {
        this.promise = new Promise<boolean>((resolve: (val: boolean) => void) => {
            this.resolveFn = resolve;
        })
    }

    public async wait(): Promise<boolean> {
        return await this.promise;
    }

    public fire(val: boolean = true) {
        this.resolveFn!(val);
    }

    private promise: Promise<boolean>;
    private resolveFn: ((val: boolean) => void) | null = null;
}

async function countCheck() {
    await asynclock.lockAsync(() => {
        count++;
        if (count == REJECTION_COUNT) {
            arktest.assertEQ(count, REJECTION_COUNT);
            waiter.fire();
        }
    },  AsyncLockMode.EXCLUSIVE);
}

async function testUnhandledPromiseRejection(): Promise<void> {
    StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        arktest.assertEQ(reason.message, rejectionReason);
        countCheck();
    });

    new Promise<string>((res, rej) => {
        res('promise1 resolve');
    }).then(() => {
        throw new Error(rejectionReason);
    });

    new Promise<string>((res, rej) => {
        rej(new Error(rejectionReason));
    }).then(() => {
        throw new Error('promise2.then error');
    });

    new Promise<string>((res, rej) => {
        res('promise3 resolve');
    }).then(() => {
        throw new Error('promise3.then error');
    }).catch((e: Error) => {
        throw new Error(rejectionReason);
    });

    new Promise<string>((res, rej) => {
        rej(new Error('promise4 reject'));
    }).catch((e: Error) => {
        throw new Error(rejectionReason);
    });

    new Promise<string>((res, rej) => {
        throw new Error(rejectionReason);
    });

    waiter.wait();
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection_promise');
    testSuite.addAsyncTest('testUnhandledPromiseRejection', testUnhandledPromiseRejection);
    let res = testSuite.run();
    return res;
}
