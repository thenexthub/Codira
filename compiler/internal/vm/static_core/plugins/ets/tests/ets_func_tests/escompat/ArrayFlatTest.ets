/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

function compareString(first: string, second: string) {
    arktest.assertEQ(first, second)
}

function compareArray<T>(first: Array<T>, second: Array<T>) {
    arktest.assertEQ(first.length, second.length)
    for(let i = 0; i < first.length; i++) {
        if(first[i] instanceof Array) {
            compareArray<T>(first[i] as Array<T>, second[i] as Array<T>)
        } else {
            arktest.assertEQ(first[i], second[i])
        }
    }
}

function booleanTest() {
    const arr = new Array<boolean | Array<boolean>>(true, [false, true], false)
    compareArray<boolean>(arr.flat<boolean>(), [true,false,true,false])
}

function intTest() {
    const arr = [0, 1, [2, [3, [4, 5]]]]
    const arr1 = [1, [1, [1, undefined ,2], 2], 3]
    const arr2 = new Array<Number | Array<Array<Number> | Number>>(1, 2, 3, [4, [5, 6]])

    compareArray<int | Array<int | Array<int>>>(arr.flat<int | Array<int | Array<int>>>(), [0,1,2,[3,[4,5]]])
    compareArray<int | Array<int>>(arr.flat<int | Array<int>>(2), [0,1,2,3,[4,5]])
    compareArray<int | Array<int>>(arr2.flat<int | Array<int>>(), [1,2,3,4,[5,6]])
}

function stringTest() {
    const arr = ["1", "2", ["3", ["4"], "5"], "6"]
    const arr2 = new Array<String | Number | Array<bigint>>("1", 2, [3n])
    compareArray<String | Array<String>>(arr.flat<String | Array<String>>(), ["1", "2", "3", ["4"], "5", "6"])
    compareArray<String | Number | bigint>(arr2.flat<String | Number | bigint>(), ["1", 2, 3n])
}

function flatMapTest() {
    const arr = [1, 2, 1]
    const result = arr.flatMap<int | Array<int | Array<int>>>((num) => (num === 2 ? [[2, 2], 2] : 1))
    compareArray<int | Array<int | Array<int>>>(result, [1,[2,2],2,1] as Array<int | Array<int | Array<int>>>)

    const arr1 = [1, undefined, 1];
    const result1 = arr1.flatMap<int | Array<int>>((num) => (num === 2 ? [2, 2] : 1));
    compareArray<int | Array<int>>(result1, [1,1,1] as Array<int | Array<int>>)
}

class InterCls {}

class FlatTest {
    a1: string = "abc"
    a2: double = 1.234
    a3: InterCls = new InterCls()
}

function nestTest() {
    const arrNum = new Array<Number | Array<Array<Number> | Number>>(1, 2, 3, [4, [5, 6]])
    const numFlat1: Array<Number | Array<Number>> = arrNum.flat<Number | Array<Number>>(1)
    const numFlat2: Array<Number> = arrNum.flat<Number>(2)
    const targetNumFlat1 = new Array<Number | Array<Number>>(1, 2, 3, 4, [5, 6])
    const targetNumFlat2 = new Array<Number>(1, 2, 3, 4, 5, 6)
    compareArray<Number | Array<Number>>(numFlat1, targetNumFlat1)
    compareArray<Number>(numFlat2, targetNumFlat2)

    const arrStr = new Array<String | Array<Array<Array<String> | String> | String>>("a", ["b", "c"],
        [["d", ["e", "f"]], "g"], ["h", ["i", "j"]])
    const strFlat1: Array<String | Array<Array<String> | String>> =
        arrStr.flat<String | Array<Array<String> | String>>(1)
    const strFlat2: Array<String | Array<String>> = arrStr.flat<String | Array<String>>(2)
    const strFlat3: Array<String> = arrStr.flat<String>(3)
    const targetStrFlat1 =
        new Array<String | Array<Array<String> | String>>("a", "b", "c", ["d", ["e", "f"]], "g", "h", ["i", "j"])
    const targetStrFlat2 = new Array<String | Array<String>>("a", "b", "c", "d", ["e", "f"], "g", "h", "i", "j")
    const targetStrFlat3 = new Array<String>("a", "b", "c", "d", "e", "f", "g", "h", "i", "j")
    compareArray<String | Array<Array<String> | String>>(strFlat1, targetStrFlat1)
    compareArray<String | Array<String>>(strFlat2, targetStrFlat2)
    compareArray<String>(strFlat3, targetStrFlat3)

    const arrUnion =
        new Array<String | Number | Array<Array<String | Number> | Array<Number>>>("1", 2, [[3, 4] as Array<Number>, ["5", "6", 7]])
    const unionFlat1: Array<String | Number | Array<String | Number> | Array<Number>> =
        arrUnion.flat<String | Number | Array<String | Number> | Array<Number>>(1)
    const unionFlat2: Array<String | Number> = arrUnion.flat<String | Number>(2)
    const targetUnionFlat1 =
        Array<String | Number | Array<String | Number> | Array<Number>>("1", 2, [3, 4] as Array<Number>, ["5", "6", 7])
    const targetUnionFlat2 = new Array<String | Number>("1", 2, 3, 4, "5", "6", 7)
    compareArray<String | Number | Array<String | Number> | Array<Number>>(unionFlat1, targetUnionFlat1)
    compareArray<String | Number>(unionFlat2, targetUnionFlat2)

    const arrObj = Array<Array<Number> | Array<FlatTest> | Array<String>>([1, 2],
        [new FlatTest(), new FlatTest()], ["3", "4"])
    const objFlat1: Array<Number | FlatTest | String> = arrObj.flat<Number | FlatTest | String>(1)
    const targetObjFlat1 = new Array<Number | FlatTest | String>(1, 2, new FlatTest(), new FlatTest(), "3", "4")
    arktest.assertEQ(objFlat1.length, targetObjFlat1.length)
    arktest.assertEQ(objFlat1[0], targetObjFlat1[0])
    arktest.assertEQ((objFlat1[2] as FlatTest).a1, (targetObjFlat1[2] as FlatTest).a1)
    arktest.assertEQ(objFlat1[4], targetObjFlat1[4])
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array.flat tests')
    suite.addTest('for boolean', booleanTest)
    suite.addTest('for int', intTest)
    suite.addTest('for string', stringTest)
    suite.addTest('for flatMap', flatMapTest)
    suite.addTest('for nestType with depth', nestTest)
    return suite.run()
}