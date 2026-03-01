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

async function AsyncFailing(): Promise<string> {
    throw Error('AsyncFailing');
}

function Failing(): string {
    throw Error('Failing');
}

function SingleRejectedJob(): null {
    throw Error('This is a single rejected job');
}

function testAllhandledRejection(): void {
    StdProcess.on('unhandledJobRejection' as 'unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
    });

    StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
    });

    StdProcess.on('unhandledJobRejection' as 'unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'This is a single rejected job');

        StdProcess.on('unhandledJobRejection' as 'unhandledPromiseRejection', (reason: Error, obj: Object): void => {
            arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        });
    });

    let g = launch<string, () => string>(Failing);

    try {
        launch<string, () => string>(Failing).Await();
    } catch(e) {
    }

    AsyncFailing().catch((e: Any) => {});

    let p = Promise.reject(new Error('promise1'));
    let q = p.then(() => {
        L.log('Test failed: unreachable code');
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    });
    q.catch((e: Any) => {});

    let f = async (): Promise<void> => {
        try {
            await Promise.reject(new Error('promise2'));
        } catch (e) {
        }
    };

    f();

    try {
        g.Await();
    } catch (e) {
    }

    launch<null, () => null>(SingleRejectedJob);
}

function main(): int {
    StdProcess.off('unhandledJobRejection' as 'unhandledPromiseRejection')
    let testSuite = new arktest.ArkTestsuite('coroutines.allhandled_rejection');
    testSuite.addTest('testAllhandledRejection', testAllhandledRejection);
    let res = testSuite.run();
    return res;
}
