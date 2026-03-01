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

import { StdDebug } from 'std/debug'
import { launch } from 'std/concurrency'

type L = StdDebug.Logger

let buf: ArrayBuffer = new ArrayBuffer(0);
let buf_2: ArrayBuffer = new ArrayBuffer(0);
let arr: Int8Array = new Int8Array;
let arr_2: Int32Array = new Int32Array;

function testMain() {
    buf = new ArrayBuffer(1);
    buf_2 = new ArrayBuffer(4);
    arr = new Int8Array(buf, 0, 1);
    arr_2 = new Int32Array(buf_2, 0, 1);

    for (let i = 0; i < 5; i++) {
        launch<Int, (i: byte) => Int>(workerI8, i.toByte())
    }
    while (Atomics.load(arr, 0) != 5) {
    }
    L.log('Finished for Int8Array')

    for (let i = 0; i < 5; i++) {
        launch<Int, (i: int) => Int>(workerI32, i as int)
    }
    while (Atomics.load(arr_2, 0) != 5) {
    }
    L.log('Finished for Int32Array')
}

function workerI8(i: byte): Int {
    while (Atomics.compareExchange(arr, 0, i as byte, (i + 1).toByte()) != i) {
    }
    return 1
}

function workerI32(i: int): Int {
    while (Atomics.compareExchange(arr_2, 0, i as int, (i + 1) as int) != i) {
    }
    return 1
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics concurrent_compare_exchange tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}