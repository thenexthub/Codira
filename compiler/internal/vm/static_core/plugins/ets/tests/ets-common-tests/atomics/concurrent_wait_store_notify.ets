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

// 1. If this test prints "1 ok" then the order of events is as follows: wait -> store -> notify
// 2. "0 not-equal": store -> wait -> notify, or store -> notify -> wait
// All other outputs are invalid

import { launch } from 'std/concurrency'

let buf: ArrayBuffer = new ArrayBuffer(0);
let arr: Int32Array = new Int32Array;
let bytearr: Int8Array = new Int8Array;

function testMain() {
    buf = new ArrayBuffer(4);
    arr = new Int32Array(buf, 0, 1);
    bytearr = new Int8Array(buf, 0, 4)

    launch<Int, () => Int>(writer)

    let res = Atomics.wait(arr, 0, 0) as string
    arktest.assertTrue(res == 'ok' || res == 'not-equal')
}

function writer(): Int {
    Atomics.store(bytearr, 0, 5 as byte)
    let count = Atomics.notify(arr, 0)
    return 1
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics concurrent_wait_store_notify tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}