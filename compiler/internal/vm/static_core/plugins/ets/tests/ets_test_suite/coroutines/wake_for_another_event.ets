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

/**
 * This test checks functionallity that we can await for standalone promise
 * that will be resolved in then callback of another promise
 * that will be resolved in another coroutine
 */

async function foo() {}

let resolvePromiseFn: ((value: String) => void) | null = null;

async function testWakeForEvent() {
    let p1 = new Promise<String>((resolve: (value: String) => void): void => {
        resolvePromiseFn = resolve;
    });

    let p2 = foo();

    let p3 = p1.then<Number>((value: String): Number => {
        arktest.assertEQ(value, 'Panda');
        return 42;
    });

    p2.then<void>((): void => {
        resolvePromiseFn!('Panda');
    });

    let value = await p3;
    arktest.assertEQ(value, 42);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.wake_for_another_event');
    testSuite.addAsyncTest('testWakeForEvent', testWakeForEvent);
    let res = testSuite.run();
    return res;
}