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

import {CoroutineExtras, AtomicFlag} from "std/debug/concurrency"
import {StdDebug} from "std/debug"
import { launch } from "std/concurrency"
import { Job } from "std/core"

type L = StdDebug.Logger

let continue_execution: AtomicFlag = new AtomicFlag(false);

async function async_coro(): Promise<Int> {
    return CoroutineExtras.getWorkerId();
}

async function check_async_id(): Promise<int> {
    let id_current: int = CoroutineExtras.getWorkerId();
    let id_coro: int = (await async_coro()) as int;
    if (id_current != id_coro) {
        return 1;
    } else {
        return 0;
    }
}

function checker(id_main: int): Int {
    let id_current: int = CoroutineExtras.getWorkerId();
    while (continue_execution.get() != true) {
        // spin
    }
    return (id_main != id_current) ? 0 : 1;
}

function check_sync_id_nonmain(number_of_coros: int): int {
    let id_current: int = CoroutineExtras.getWorkerId();
    let promises: (Job<Int> | undefined)[] = new (Job<Int> | undefined)[number_of_coros];

    for (let i = 0; i < number_of_coros; ++i) {
        promises[i] = launch<Int, (i: int) => Int>(checker, id_current);
    }
    continue_execution.set(true);
    let result: int = 0;
    for (let i = 0; i < number_of_coros; ++i) {
        result += (promises[i]!).Await();
    }
    return result == 0 ? result : 1;
}

function check_default_policy(): int {
    L.log('Testing the DEFAULT affinity type');

    if (waitForCompletion(check_async_id) != 0) {
        L.logError('async function worker ID is different from launcher function worker ID');
        return 1;
    }
    return 0;
}

function check_nonmain_policy(): int {
    L.log('Testing the NON_MAIN affinity type');
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);

    if (waitForCompletion(check_async_id) != 0) {
        L.logError('async function worker ID is different from launcher function worker ID');
        return 1;
    }
    if (check_sync_id_nonmain(15) != 0) {
        L.logError('regular coroutine worker ID is the same as MAIN worker ID');
        return 1;
    }
    return 0;
}

function testAsyncDefaultPolicy(): void {
    // Test 1. Check that the async function will execute on the same worker
    if (check_default_policy() != 0) {
        arktest.assertTrue(false, 'test for the DEFAULT affinity type failed');
    }
}

function testAsyncNonMainPolicy(): void {
    /**
     * Test 2. Check that POLICY_NON_MAIN does not affect async functions,
     * while regular coroutines will execute on non-MAIN workers.
     */
    if (check_nonmain_policy() != 0) {
        arktest.assertTrue(false, 'test for the NON_MAIN affinity type failed');
    }
}

// to be implemented, not add to ArkTestsuite currently
function testExclusiveLaunchFlag(): void {
    /**
     * Test 3. Check that EXCLUSIVE launch flag works
     */
    // NOTE(konstanting): will be added later (see #14697)
}


function main(): int {
    // Call L's enableLogging method to enable logs.
    let testSuite = new arktest.ArkTestsuite('coroutines.affinity');
    testSuite.addTest('testAsyncDefaultPolicy', testAsyncDefaultPolicy);
    testSuite.addTest('testAsyncNonMainPolicy', testAsyncNonMainPolicy);
    let res = testSuite.run();
    return res;
}

