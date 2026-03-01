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

type L = StdDebug.Logger

const kTotal = 6;
const asynclock = new AsyncLock();
let current = 0;
let event = new CompletableJob<void>();

async function AsyncFailing(): Promise<string> {
    throw Error('AsyncFailing');
}

function Failing(): string {
    throw Error('Failing');
}

function WaitAll(): void {
    event.Await();
}

// Called before or on the program completion
function PromiseHandler(reason: Error, obj: Object): void {
    // should be no-throw
    asynclock.lockAsync((): void => {
        current++;
        if (current == kTotal) {
            event.finish();
        } else if (current > kTotal) {
            console.error('More rejections than expected');
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
    },  AsyncLockMode.EXCLUSIVE);
}

let callWaitAll: boolean = true;

// called on program completion. Intended that job handlers aren't concurrent with each other
function JobHandler(reason: Error, obj: Object): void {
    if (callWaitAll) {
        callWaitAll = false;
        launch<void, () => void>(WaitAll);
    }
    // should be no-throw
    asynclock.lockAsync((): void => {
        current++;
        if (current == kTotal) {
            event.finish();
        } else if (current > kTotal) {
            console.error('More rejections than expected');
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
    },  AsyncLockMode.EXCLUSIVE);
}

function testUnhandledRejection(): void {
    StdProcess.on('unhandledJobRejection' as 'unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'Failing');
        JobHandler(reason, obj);
    });
    StdProcess.on('unhandledPromiseRejection',  (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        PromiseHandler(reason, obj);
    });

    let g = launch<string, () => string>(Failing);
    launch<Promise<string>, () => Promise<string>>(AsyncFailing);
    AsyncFailing();
    let p = Promise.reject(new Error('promise1'));
    let q = p.then(() => {
        L.log('Test failed: Unreachable code');
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    })
    Promise.reject(new Error('promise2'));
    Promise.reject();
}

function main(): int {
    StdProcess.off('unhandledJobRejection' as 'unhandledPromiseRejection');
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection');
    testSuite.addTest('testUnhandledRejection', testUnhandledRejection);
    let res = testSuite.run();
    return res;
}
