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

import {StdDebug} from "std/debug"
import {CoroutineExtras} from "std/debug/concurrency"
import { launch } from "std/concurrency"

type L = StdDebug.Logger
type NullableType = Object | null

async function async_chain_element(caller_wid: int, counter: int): Promise<int> {
    let id_worker: int = CoroutineExtras.getWorkerId();
    if (caller_wid != id_worker) {
        return 1;
    }
    if (counter <= 0) {
        return 0;
    }
    return await async_chain_element(caller_wid, counter - 1);
}

async function async_function_chain(caller_wid: int, chain_len: int): Promise<int> {
    return await async_chain_element(caller_wid, chain_len);
}

function compare_worker_ids_async(chain_len: int): int {
    L.log('Testing async function chain');

    let id_main: int = CoroutineExtras.getWorkerId();
    let result = waitForCompletion(() => async_function_chain(id_main, chain_len));
    if (result == 0) {
        L.log('Successfully ran the async function chain')
    } else {
        L.log('Failed to run the async function chain')
    }
    return result;
}

function return_worker_id(): Int {
    return CoroutineExtras.getWorkerId();
}

function compare_worker_ids(): int {
    L.log('Testing launch in a separate worker');

    let id_main: int = CoroutineExtras.getWorkerId();
    let id_coro: int = (launch<Int, () => Int>(return_worker_id).Await()) as int;
    if (id_main != id_coro) {
        L.log('Successfully ran coro in a separate worker. Main WID: ' + id_main + ', Coro WID: ' + id_coro);
        return 0;
    } else {
        L.logError('Failed to run coro in a separate worker. Main WID: ' + id_main + ', Coro WID: ' + id_coro);
        return 1;
    }
}

function job(n: int): NullableType {
    L.log('Job with ' + n + ' iterations in a loop started');
    for (let i = 0; i < n; ++i) {
        //
    }
    return null;
}

function run_batch_launch(batch_size: int, iters: int): int {
    L.log('Testing batch launch of ' + batch_size + ' coroutines');
    for (let i = 0; i < batch_size; ++i) {
        launch<NullableType, (i: int) => NullableType>(job, iters);
    }
    return 0;
}

function await_chain(n: int): NullableType {
    if (n > 0) {
        launch<NullableType, (i: int) => NullableType>(await_chain, n-1).Await();
    }
    return null;
}

function run_await_chain(len: int): int {
    L.log('Testing await chain of ' + len + ' items');
    launch<NullableType, (i: int) => NullableType>(await_chain, len);
    return 0;
}

function simple(): NullableType {
    return null;
}

function run_batch_await(batch_size: int): int {
    L.log('Testing batch await of ' + batch_size + ' coroutines');
    for (let i = 0; i < batch_size; ++i) {
        let j = launch<NullableType, () => NullableType>(simple);
        j.Await();
    }
    return 0;
}

function testCompareWorkerIdsAsync(): void {
    // Test 1. Run an async function chain and check that all worker IDs are equal to MAIN's
    if (compare_worker_ids_async(5) != 0) {
        arktest.assertTrue(false, 'compare async function worker IDs test failed');
    }
}

function testCompareWorkerIds(): void {
    // Test 2. Run a coroutine and make sure that its worker ID is different from MAIN
    if (compare_worker_ids() != 0) {
        arktest.assertTrue(false, 'compare worker IDs test failed');
    }
}

function testRunBatchLaunch(): void {
    // Test 3. Run a batch of coroutines without await so they would be distributed to different workers
    if (run_batch_launch(100, 500) != 0) {
        arktest.assertTrue(false, 'batch launch test failed');
    }
}

function testRunAwaitChain(): void {
    // Test 4. Run a chain of several coroutines that await each other. Coroutines will run on different workers.
    if (run_await_chain(50) != 0) {
        arktest.assertTrue(false, 'await chain test failed');
    }
}

function testRunBatchAwait(): void {
    // Test 5. Run coroutines on different workers and await them.
    if (run_batch_await(100) != 0) {
        arktest.assertTrue(false, 'batch await test failed');
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_error');
    testSuite.addTest('testCompareWorkerIdsAsync', testCompareWorkerIdsAsync);
    testSuite.addTest('testCompareWorkerIds', testCompareWorkerIds);
    testSuite.addTest('testRunBatchLaunch', testRunBatchLaunch);
    testSuite.addTest('testRunAwaitChain', testRunAwaitChain);
    testSuite.addTest('testRunBatchAwait', testRunBatchAwait);
    let res = testSuite.run();
    return res;
}
