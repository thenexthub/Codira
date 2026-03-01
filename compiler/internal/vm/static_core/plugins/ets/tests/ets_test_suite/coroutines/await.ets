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

import {launch} from "std/concurrency";
import {Job} from "std/core";

let flag = false;

type L = StdDebug.Logger

function getString(): String {
    flag = true;
    return 'Paragliding is awesome!!\n';
}

function getInt(): int {
    flag = true;
    let ret: int = 0;
    return ret;
}

function getNumber(): number {
    flag = true;
    let ret: number = 0;
    return ret;
}

function getBoolean(): boolean {
    flag = true;
    let ret: boolean = false;
    return ret;
}

function getObject(): Object {
    flag = true;
    let func = (a : String) => { return a; }
    return func;
}

// template function for return types check
function checkReturnType<T>(fn: () => T): void {
    // reset flag
    flag = false;

    let p = launch<T, () => T>(fn);
    let result = p.Await();

    if (!flag) {
        arktest.assertTrue(false, 'Flag is not set!');
        return;
    } else {
        L.log('First await result: ' + result);
        let result2 = p.Await();
        L.log('Second await result: ' + result2);
        if (!(result === result2)) {
            arktest.assertTrue(false, 'await results do not match!');
            return;
        }
    }
}

function testAwaitForString() {
    checkReturnType(getString);
}

function testAwaitForInt() {
    checkReturnType(getInt);
}

function testAwaitForNumber() {
    checkReturnType(getNumber);
}

function testAwaitForBoolean() {
    checkReturnType(getBoolean);
}

function testAwaitForObject() {
    checkReturnType(getObject);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.await');
    testSuite.addTest('testAwaitForString', testAwaitForString);
    testSuite.addTest('testAwaitForInt', testAwaitForInt);
    testSuite.addTest('testAwaitForNumber', testAwaitForNumber);
    testSuite.addTest('testAwaitForBoolean', testAwaitForBoolean);
    testSuite.addTest('testAwaitForObject', testAwaitForObject);
    let res = testSuite.run();
    return res;
}
