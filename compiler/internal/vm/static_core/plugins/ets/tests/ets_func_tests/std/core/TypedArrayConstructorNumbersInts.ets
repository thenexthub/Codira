/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

const success = 0
const failed = 1

function test(actual: String, expected: String, name: String): int {
    const result = actual == expected
    if (result) {
        console.println("PASSED: " + name)
        return success
    }
    console.println("FAILED: " + name + ", expected: " + expected + ", actual: " + actual)
    return failed
}

function testArrayConstructorsFromNumbersInts(): int {
    const nums: FixedArray<number> = [1, 2, 3, 4, 5, Infinity, -Infinity, NaN, -1, -65536]
    const ints: FixedArray<int> = [
        1 as int, 2 as int, 3 as int, 4 as int,
        5 as int, -1 as int, -65536 as int
    ]
    let failures: int = 0

    failures += test(
        (new Int8Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,-1,0",
        "Int8Array(number[])",

    )
    failures += test(
        (new Int8Array(ints)).toString(),
        "1,2,3,4,5,-1,0",
        "Int8Array(int[])"
    )

    failures += test(
        (new Int16Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,-1,0",
        "Int16Array(number[])"
    )
    failures += test(
        (new Int16Array(ints)).toString(),
        "1,2,3,4,5,-1,0",
        "Int16Array(int[])"
    )

    failures += test(
        (new Int32Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,-1,-65536",
        "Int32Array(number[])"
    )
    failures += test(
        (new Int32Array(ints)).toString(),
        "1,2,3,4,5,-1,-65536",
        "Int32Array(int[])"
    )

    failures += test(
        (new Float32Array(nums)).toString(),
        "1,2,3,4,5,Infinity,-Infinity,NaN,-1,-65536",
        "Float32Array(number[])"
    )
    failures += test(
        (new Float32Array(ints)).toString(),
        "1,2,3,4,5,-1,-65536",
        "Float32Array(int[])"
    )

    failures += test(
        (new Float64Array(nums)).toString(),
        "1,2,3,4,5,Infinity,-Infinity,NaN,-1,-65536",
        "Float64Array(number[])")
    
    failures += test(
        (new Float64Array(ints)).toString(),
        "1,2,3,4,5,-1,-65536",
        "Float64Array(int[])"
    )

    failures += test(
        (new Uint16Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,65535,0",
        "Uint16Array(number[])"
    )
    failures += test(
        (new Uint16Array(ints)).toString(),
        "1,2,3,4,5,65535,0",
        "Uint16Array(int[])"
    )

    failures += test(
        (new Uint32Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,4294967295,4294901760",
        "Uint32Array(number[])"
    )
    failures += test(
        (new Uint32Array(ints)).toString(),
        "1,2,3,4,5,4294967295,4294901760",
        "Uint32Array(int[])"
    )
    const buffer1 = new ArrayBuffer(17);
    const array1 = new Uint32Array(buffer1, 0, 2);
    array1[0] = 42;
    array1[1] = 100;
    failures += test(
        array1.toString(),
        "42,100",
        "Uint32Array(ArrayBuffer, byteOffset, length) with values"
    )
    const buffer2 = new ArrayBuffer(41);
    const array2 = new Uint32Array(buffer2, 8, 3);
    array2[0] = 1;
    array2[1] = 4294967295;
    array2[2] = 4294901760;
    failures += test(
        array2.toString(),
        "1,4294967295,4294901760",
        "Uint32Array(ArrayBuffer, byteOffset, length) with values"
    )


    failures += test(
        (new Uint8Array(nums)).toString(),
        "1,2,3,4,5,0,0,0,255,0",
        "Uint8Array(number[])"
    )
    failures += test(
        (new Uint8Array(ints)).toString(),
        "1,2,3,4,5,255,0",
        "Uint8Array(int[])"
    )

    failures += test(
        (new Uint8ClampedArray(nums)).toString(),
        "1,2,3,4,5,255,0,0,0,0",
        "Uint8ClampedArray(number[])"
    )
    failures += test(
        (new Uint8ClampedArray(ints)).toString(),
        "1,2,3,4,5,0,0",
        "Uint8ClampedArray(int[])"
    )
    return failures
}

function main(): int {
    let failures = 0
    failures += testArrayConstructorsFromNumbersInts()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
