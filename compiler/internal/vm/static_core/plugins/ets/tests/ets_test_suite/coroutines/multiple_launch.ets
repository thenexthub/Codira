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

import { launch } from "std/concurrency"
import { Job } from "std/core"

type L = StdDebug.Logger

function returnValue(i: int): String {
    return 'value ' + i;
}

function launchCoroutines(n_cor: int): int {
    let p: (Job<String> | undefined)[] = new (Job<String> | undefined)[n_cor];
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch<String, (i: int) => String>(returnValue, i);
    }
    for (let i = 0; i < n_cor; ++i) {
        let val = (p[i]!).Await();
        if(val != returnValue(i)) {
            L.log('p[' + i + '] = ' + val + ' instead of ' + returnValue(i));
            return 1;
        }
    }
    return 0;
}

function throwError(): Object {
    throw new Error();
}

function launchCoroutinesWithError(n_cor: int): int { 
    let p: (Job<Object> | undefined)[] = new (Job<Object> | undefined)[n_cor];
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch<Object, () => Object>(throwError);
    }
    for (let i = 0; i < n_cor; ++i) {
        try {
            (p[i]!).Await();
        } catch (e) {
            if (!(e instanceof Error)) {
              L.log('Expected Error but another error has been thrown!: ' + e);
              return 1;
            }
            continue;
        }
        L.log('Was no error throwing');
        return 1;
    }
    return 0;
}

// !1581, molotkovmikhail - test should be start with coroutine pool to test it
function testMultipleLaunch(): void {
    // Create 5 coroutines and save 5 coroutines
    if(launchCoroutines(5) != 0) {
        arktest.assertTrue(false, 'First launch of 5 coroutines failed!!');
        return;
    }    

    // Reuse 5 coroutines
    if(launchCoroutines(5) != 0) {
        arktest.assertTrue(false, 'Second launch of 5 coroutines failed!!');
        return;
    }

    // Create 10 coroutines: reuse 5 coroutines and create new 5 ones. Save 10 coroutines 
    if(launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'First launch of 10 coroutines failed!!');
        return;
    }

    // Reuse 10 coroutines
    if(launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'Second launch of 10 coroutines failed!!');
        return;
    }

    // Reuse 5 coroutines and throw error in them
    if(launchCoroutinesWithError(5) != 0) {
        arktest.assertTrue(false, 'First launch of 5 throw-error coroutines failed!!');
        return;
    }

    // Reuse 10 coroutines
    if(launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'First launch of 10 coroutines after 5 throw-error ones failed!!');
        return;
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.multiple_launch');
    testSuite.addTest('testMultipleLaunch', testMultipleLaunch);
    let res = testSuite.run();
    return res;
}