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

const arrayLength : int = 11
const beginPlus : int = 1
const endMinus : int = 1

function create_array(): ArrayLike<Number> {
    let arr = new Array<Number>(arrayLength)
    for (let i: int = 0; i < (arr.length).toInt(); i++) {
        arr[i] = i
    }
    return arr
}

function check_array<T extends Number | BigInt>(data: ArrayLike<T>, data_rev: ArrayLike<T>): void {
    let length_rev = (data_rev.length).toInt()
    let length = (data.length).toInt()
    arktest.assertEQ(length, arrayLength, "Original array length error")
    arktest.assertEQ(length, length_rev + beginPlus + endMinus, "Inspected array length error")
    for (let i: int = 0; i < length_rev; i++) {
        let result : boolean = false
        if (typeof data[i] == "number") {
            result = ((data[length - i - beginPlus - 1] as Number) == data_rev[i])
        } else if (typeof data[i] == "bigint") {
            result = ((data[length - i - beginPlus - 1] as BigInt) == data_rev[i])
        }
        arktest.assertEQ(result, true, "Error element with index " + i)
    }
}

function sliceInt8ArrayTest(): void {
    let data = new Int8Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceInt16ArrayTest(): void {
    let data = new Int16Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceInt32ArrayTest(): void {
    let data = new Int32Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceBigInt64ArrayTest(): void {
    let data = new BigInt64Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceFloat32ArrayTest(): void {
    let data = new Float32Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceFloat64ArrayTest(): void {
    let data = new Float64Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceUint8ArrayTest(): void {
    let data = new Uint8Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceUint16ArrayTest(): void {
    let data = new Uint16Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceUint32ArrayTest(): void {
    let data = new Uint32Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceBigUint64ArrayTest(): void {
    let data = new BigUint64Array(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function sliceUint8ClampedArrayTest(): void {
    let data = new Uint8ClampedArray(create_array())
    let length = (data.length).toInt()
    let data_rev = data.slice(0 + beginPlus, length - endMinus)
    for (let i: int = 0; i < length; i++) {
        if (i >= length - i - 1) {
            break
        }
        let v = data[i]
        data[i] = data[length - i - 1]
        data[length - i - 1] = v
    }
    check_array(data, data_rev)
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("typedArrays.slice")

    testSuite.addTest("Int8Array: slice test", sliceInt8ArrayTest);
    testSuite.addTest("Int16Array: slice test", sliceInt16ArrayTest);
    testSuite.addTest("Int32Array: slice test", sliceInt32ArrayTest);
    testSuite.addTest("BigInt64Array: slice test", sliceBigInt64ArrayTest);
    testSuite.addTest("Float32Array: slice test", sliceFloat32ArrayTest);
    testSuite.addTest("Float64Array: slice test", sliceFloat64ArrayTest);
    testSuite.addTest("Uint8Array: slice test", sliceUint8ArrayTest);
    testSuite.addTest("Uint16Array: slice test", sliceUint16ArrayTest);
    testSuite.addTest("Uint32Array: slice test", sliceUint32ArrayTest);
    testSuite.addTest("BigUint64Array: slice test", sliceBigUint64ArrayTest);
    testSuite.addTest("Uint8ClampedArray: slice test", sliceUint8ClampedArrayTest);

    return testSuite.run();
}
