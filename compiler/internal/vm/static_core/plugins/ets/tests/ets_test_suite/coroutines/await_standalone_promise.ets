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

async function testAwaitStandalonePromise(): Promise<void> {
    let value = await Promise.resolve(42);
    arktest.assertEQ(value, 42, 'Awaited value of resolved promise is different\
                          from the expected value. Actual value: ' + value);
    try {
        await Promise.reject(new Error('Panda'));
    } catch (err) {
        arktest.assertTrue(err.toString().contains('Panda', 0), 'Awaited value of rejected promise is different\
                                                          from the expected value. Actual value: ' + err);
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.await_standalone_promise');
    testSuite.addAsyncTest('testAwaitStandalonePromise', testAwaitStandalonePromise);
    let res = testSuite.run();
    return res;
}
