/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

const BYTE_LENGTH = 128

let abint8 = new ArrayBuffer(BYTE_LENGTH)
let int8array = new Int8Array(abint8)
let abint16 = new ArrayBuffer(BYTE_LENGTH)
let int16array = new Int16Array(abint16)
let abint32 = new ArrayBuffer(BYTE_LENGTH)
let int32array = new Int32Array(abint32)
let abuint8 = new ArrayBuffer(BYTE_LENGTH)
let uint8array = new Uint8Array(abuint8)
let abuint16 = new ArrayBuffer(BYTE_LENGTH)
let uint16array = new Uint16Array(abuint16)
let abuint32 = new ArrayBuffer(BYTE_LENGTH)
let uint32array = new Uint32Array(abuint32)

let typeArrayList = [
    int8array,
    int16array,
    int32array,
    uint8array,
    uint16array,
    uint32array
]

function testLoad() {
    for (let i = 0; i < typeArrayList.length; i++) {
        let ta = typeArrayList[i]
        arktest.assertEQ(ta.byteOffset, 0)
        let arraySize = ta.length

        for (let j = 0; j < arraySize; j++) {
            Atomics.store(ta, j, 12)

            for (let k = 0; k < arraySize; k++) {
                let expected = (k === j) ? 12 : 0
                arktest.assertEQ(Atomics.load(ta, k), expected)
            }

            Atomics.store(ta, j, 0)
        }
    }
}

function main() {
    const suite = new arktest.ArkTestsuite('Function Atomics base_operation_load tests')
    suite.addTest('Function testLoad', testLoad)
    return suite.run()
}