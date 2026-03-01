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


function out_of_coro_test() {
    const coro_to_create = 2000;
    let arr = new Int32Array(new ArrayBuffer(1024))

    let jobs = new Array<Job<void>>();

    arktest.expectError(() => {
        let wait_func = (arr: Int32Array) => {Atomics.wait(arr, 0, 0);}
        let mainWaitJob = launch<void, (arr: Int32Array) => void> (wait_func, arr);
        let awaitIt = ()=>{ mainWaitJob.Await(); }

        for (let i = 1; i < coro_to_create; ++ i) {
            jobs.push(launch<void, () => void> (awaitIt));
        }
    }, new CoroutinesLimitExceedError('Unable to create a new coroutine: reached the limit for the number of existing coroutines.'));

    let eaw1 = new EAWorker();
    eaw1.start();

    let eaw2 = new EAWorker();
    eaw2.start();

    Atomics.store(arr, 0, 1);
    Atomics.notify(arr, 0);

    for (let j of jobs) {
        j.Await();
    }

    eaw1.quit();
    eaw2.run<void>(()=>{});
    eaw2.quit();
}

function main() {
    let testsuite = new arktest.ArkTestsuite("EAWorker");
    testsuite.addTest("out_of_coro_test", out_of_coro_test)
    return testsuite.run();
}

