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

// This test checks if the coroutine statistics engine works correctly.

function sync_f(a: int): String {
    return 'Some dummy string:' + a
}

async function async_f(a: int): Promise<String> {
    return 'Some dummy string:' + a
}

async function testCoroStats(): Promise<void> {
    for (let i = 0; i < 10; ++i) {
        launch<String, (i: int) => String>(sync_f, 5);
    }
    for (let i = 0; i < 10; ++i) {
        launch<String, (i: int) => String>(sync_f, 5).Await();
    }
    for (let i = 0; i < 10; ++i) {
        async_f(5);
    }
    for (let i = 0; i < 10; ++i) {
        await async_f(5);
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.stats');
    testSuite.addAsyncTest('testCoroStats', testCoroStats);
    let res = testSuite.run();
    return res;
}