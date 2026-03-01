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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("Promisify API tests");

    suite.addAsyncTest("Promisify resolves with string", testUtilPromisifyResolvesWithString);
    suite.addAsyncTest("Promisify rejects with error", testUtilPromisifyRejectsWithError);
    suite.addAsyncTest("Promisify resolves with number", testUtilPromisifyResolvesWithNumber);
    suite.addAsyncTest("Promisify resolves with boolean", testUtilPromisifyResolvesWithBoolean);
    suite.addAsyncTest("Promisify resolves with another number", testUtilPromisifyResolvesWithAnotherNumber);

    return suite.run()
}

async function testUtilPromisifyResolvesWithString() {
    let func: Function = (val: Any, callback: (err: Error | null, ...value: FixedArray<Any>) => void) => {
        callback(null, val);
    }
    let value = util.promisify(func);
    let res = await value('Hello');
    arktest.assertEQ(new String(res), 'Hello');
}

async function testUtilPromisifyRejectsWithError() {
    let func: Function = (val: Any, callback: (err: Error | null, ...value: FixedArray<Any>) => void) => {
        if (val instanceof String) {
            callback(null, val);
        } else {
            callback(new Error('Invalid argument type, expected string'), null);
        }
    }
    let value = util.promisify(func);
    let exceptionCheck = (e: Error): boolean | string => {
        return e.message == 'Invalid argument type, expected string';
    }
    arktest.expectThrow(() => { await value([1, 2]); }, exceptionCheck);
}

async function testUtilPromisifyResolvesWithNumber() {
    let func: Function = (val: Any, callback: (err: Error | null, ...value: FixedArray<Any>) => void) => {
        callback(null, val);
    }
    let val = util.promisify(func);
    let res = await val(42);
    arktest.assertEQ(new String(res), '42');
}

async function testUtilPromisifyResolvesWithBoolean() {
    let func: Function = (val: Any, callback: (err: Error | null, ...value: FixedArray<Any>) => void) => {
        callback(null, val);
    }
    let value = util.promisify(func);
    let res = await value(true);
    arktest.assertEQ(Boolean(res), true);
}

async function testUtilPromisifyResolvesWithAnotherNumber() {
    let func: Function = (val: Any, callback: (err: Error | null, ...value: FixedArray<Any>) => void) => {
        callback(null, val);
    }
    let val = util.promisify(func);
    let res = await val(100);
    arktest.assertEQ(new String(res), '100');
}
