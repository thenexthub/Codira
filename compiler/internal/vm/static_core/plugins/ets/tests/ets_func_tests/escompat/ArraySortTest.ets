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

function main(): int {
    let arrayTestsuite = new arktest.ArkTestsuite("arrayTestsuite")
    arrayTestsuite.addTest("SortWithUndefined", SortWithUndefined)
    arrayTestsuite.addTest("SortWithUndefinedAndNull", SortWithUndefinedAndNull)
    arrayTestsuite.addTest("SortWithFloat", SortWithFloat)
    return arrayTestsuite.run()
}

function SortWithUndefined() {
    let myComparefn_1 = (x: number | undefined, y: number | undefined) => {
        if (x == undefined) {
            return -1
        }
        if (y == undefined) {
            return 1
        }
        return 0
    }
    let x: Array<number | undefined> = new Array<number | undefined>(undefined, 1.0, undefined, 5.0, 2.0, 3.0)
    x.sort(myComparefn_1)
    arktest.assertEQ(x.length, 6)
    arktest.assertEQ(x[0], 1)
    arktest.assertEQ(x[1], 5)
    arktest.assertEQ(x[2], 2)
    arktest.assertEQ(x[3], 3)
    arktest.assertEQ(x[4], undefined)
    arktest.assertEQ(x[5], undefined)
    let y: Array<int | undefined> = new Array<int | undefined>(3, undefined, 1, undefined, 5, 2)
    let myComparefn_2 = (x: int | undefined, y: int | undefined) => {
        if (x == undefined) {
            return 1
        }
        if (y == undefined) {
            return -1
        }
        return 0
    }
    y.sort(myComparefn_2)
    arktest.assertEQ(y.length, 6)
    arktest.assertEQ(y[0], 3)
    arktest.assertEQ(y[1], 1)
    arktest.assertEQ(y[2], 5)
    arktest.assertEQ(y[3], 2)
    arktest.assertEQ(y[4], undefined)
    arktest.assertEQ(y[5], undefined)
}

function SortWithUndefinedAndNull() {
    let arr: Array<number | undefined | null> = new Array<number | undefined | null>(1.0, 1000.0, 3.0, null, undefined, -2.0, undefined, 0.0)
    arr.sort()
    arktest.assertEQ(arr.length, 8)
    arktest.assertEQ(arr[0], -2)
    arktest.assertEQ(arr[1], 0)
    arktest.assertEQ(arr[2], 1)
    arktest.assertEQ(arr[3], 1000)
    arktest.assertEQ(arr[4], 3)
    arktest.assertEQ(arr[5], null)
    arktest.assertEQ(arr[6], undefined)
    arktest.assertEQ(arr[7], undefined)
}

function SortWithFloat() {
    const arr1: Float64Array = new Float64Array([2.2, 3.1, 1.0]);
    const result1 = arr1.sort((a: double, b: double): int => {
        return (a - b).toInt()
    })
    arktest.assertEQ(result1.toString(), [1,2.2,3.1].toString())

    const arr2 = [1.1, 3.2, -0.2];
    const combined = arr2.concat(Array.from(arr1));

    const result2 = combined.sort((a: double, b: double): int => {
        return Math.floor(a - b).toInt()
    })
    arktest.assertEQ(result2.toString(), [-0.2,1,1.1,2.2,3.1,3.2].toString())
}
