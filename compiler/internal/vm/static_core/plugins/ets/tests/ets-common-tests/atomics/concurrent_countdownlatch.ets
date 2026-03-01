/*
* Copyright (c) 2023-2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import {StdDebug} from 'std/debug'
import { launch } from 'std/concurrency'

type L = StdDebug.Logger

let N = 9

let buf_1: ArrayBuffer = new ArrayBuffer(0);
let buf_2: ArrayBuffer = new ArrayBuffer(0);
let arr_1: Int32Array = new Int32Array;
let arr_2: BigInt64Array = new BigInt64Array;
let bytearr_i8: Int8Array = new Int8Array;
let bytearr_i64: BigInt64Array = new BigInt64Array;

function testMain() {
    buf_1 = new ArrayBuffer(4)
    buf_2 = new ArrayBuffer(8)
    arr_1 = new Int32Array(buf_1, 0, 1)
    arr_2 = new BigInt64Array(buf_2, 0, 1)
    bytearr_i8 = new Int8Array(buf_1, 0, 4)
    bytearr_i64 = new BigInt64Array(buf_2, 0, 1)

    for (let i = 0; i < N; i++) {
        launch<Int, () => Int>(task1)
    }
    let cur: byte = Atomics.load(bytearr_i8, 0).toByte()
    while (cur != N) {
        L.log('waiting, finished tasks: ' + cur)
        Atomics.wait(arr_1, 0, cur)
        cur = Atomics.load(bytearr_i8, 0).toByte()
    }

    for (let i = 0; i < N; i++) {
        launch<Int, () => Int>(task2)
    }
    let cur_64: long = Atomics.load(bytearr_i64, 0).getLong()
    while (cur_64 != N) {
        L.log('waiting, finished tasks: ' + cur_64)
        Atomics.wait(arr_2, 0, new BigInt(cur_64))
        cur_64 = Atomics.load(bytearr_i64, 0).getLong()
    }
    L.log('all tasks have finished')
}

function task1(): Int {
    heavyComputation()
    L.log('Task finished. Notifying')
    Atomics.add(bytearr_i8, 0, 1 as byte)
    Atomics.notify(arr_1, 0)

    return 1
}

function task2(): Int {
    heavyComputation()
    L.log('Task finished. Notifying')
    Atomics.add(bytearr_i64, 0, new BigInt(1))
    Atomics.notify(arr_2, 0)

    return 1
}

function heavyComputation(): Int {
    let i = 1
    for (let j = 0; j < 10000; j++) {
        i += j
    }
    return i
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics concurrent_countdownlatch tests')
    suite.addTest('Function testMain', testMain)
    return suite.run()
}