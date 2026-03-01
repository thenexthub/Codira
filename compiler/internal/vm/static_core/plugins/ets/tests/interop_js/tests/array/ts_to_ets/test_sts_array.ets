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

import {
    arr,
    arrConcat1,
    arrConcat2,
    arrCopyWithin1,
    arrCopyWithin2,
    arrCopyWithin3,
    arrFill,
    arrFlat,
    arrFlatMap,
    arrIndex,
    arrPop,
    arrPush,
    arrReduceRight,
    arrReverse,
    arrShift,
    arrSlice,
    arrSort,
    arrSplice1,
    arrSplice3,
    arrSplice4,
    arrToSorted,
    arrUnShift
} from 'test_array_ts';

let result = false
let localArray: Array<Number> = new Array<Number>(1, 2, 3, 4)
let array: Array<Number> = arr as Array<Number>
let arrayConcat1: Array<Number> = arrConcat1 as Array<Number>
let arrayConcat2: Array<Number> = arrConcat2 as Array<Number>
let arrayCopyWithin1: Array<Number> = arrCopyWithin1 as Array<Number>
let arrayCopyWithin2: Array<Number> = arrCopyWithin2 as Array<Number>
let arrayCopyWithin3: Array<Number> = arrCopyWithin3 as Array<Number>
let arrayFill: Array<Number> = arrFill as Array<Number>
let arrayFlat: Array<Array<Number>> = arrFlat as Array<Array<Number>>
let arrayFlatMap: Array<Number> = arrFlatMap as Array<Number>
let arrayIndex: Array<Number> = arrIndex as Array<Number>
let arrayPop: Array<Number> = arrPop as Array<Number>
let arrayPush: Array<Number> = arrPush as Array<Number>
let arrayReduceRight: Array<String> = arrReduceRight as Array<String>
let arrayReverse: Array<Number> = arrReverse as Array<Number>
let arrayShift: Array<Number> = arrShift as Array<Number>
let arraySlice: Array<Number> = arrSlice as Array<Number>
let arraySort: Array<Number> = arrSort as Array<Number>
let arraySplice1: Array<Number> = arrSplice1 as Array<Number>
let arraySplice3: Array<Number> = arrSplice3 as Array<Number>
let arraySplice4: Array<Number> = arrSplice4 as Array<Number>
let arrayToSorted: Array<Number> = arrToSorted as Array<Number>
let arrayUnShift: Array<Number> = arrUnShift as Array<Number>

const funcPredicate1 = (num: number, index: int, array: Array<Number>): boolean => num % 2 === 0

const funcPredicate2 = (num: number, index: int, array: Array<Number>): boolean => num < 6

const mapAndFlatten = (num: number, k: int, arr: Array<Number>): Number => num * num

const flatMapAndFlatten = (num: number): FixedArray<Number> => [num * num]

const processElement = (element: number, index: int, array: Array<Number>): void => result = (element === index + 1)

const sumReducer = (previousValue: number, currentValue: number, index: int, array: Array<Number>): number => {
    return previousValue + currentValue
}

const concatenateReducer = (previousValue: string, currentValue: string, index: int, array: Array<string>): string => {
    return previousValue + "" + currentValue
}

class A {
    style: string
    currency: string
    constructor(style: string, currency: string) {
        this.style = style
        this.currency = currency
    }
}

function indexAccess(): boolean {
    return array[0] === 1 && array[1] === 2 && array[2] === 3 && array[3] === 4
}

function testLength(): boolean {
    return array.length === 4
}

function typeofArray(): boolean {
    return typeof array === 'object'
}

function instanceofArray(): boolean {
    return array instanceof Array
}

function instanceofArrayObject(): boolean {
    let obj: Object = array as Object
    return obj instanceof Array
}

function testAt(): boolean {
    return array.at(2) as number === 3 && array.at(-1) as number === 4
}

function testConcat(): boolean {
    return localArray.concat(arrayConcat2).toString() === "1,2,3,4,5,6,7,8" && arrayConcat1.concat(arrayConcat2).toString() === "1,2,3,4,5,6,7,8" && arrayConcat1.concat(localArray).toString() === "1,2,3,4,1,2,3,4"
}

function testCopyWithin(): boolean {
    return arrayCopyWithin1.copyWithin(1).toString() === "1,1,2,3" && arrayCopyWithin2.copyWithin(0, 2).toString() === "3,4,3,4" && arrayCopyWithin3.copyWithin(0, 1, 3).toString() === "2,3,3,4"
}

function testEntries(): boolean {
    return true
}

function testEvery(): boolean {
return !array.every(funcPredicate1) && array.every(funcPredicate2)
}

function testFill(): boolean {
    return arrayFill.fill(8.0, 1, 3).toString() === "1,8,8,4"
}

function testFilter(): boolean {
    return array.filter(funcPredicate1).toString() === "2,4"
}

function testFind(): boolean {
    return array.find(funcPredicate1) as number === 2
}

function testFindIndex(): boolean {
    return array.findIndex(funcPredicate1) === 1
}

function testFindLast(): boolean {
    return array.findLast(funcPredicate1) as number === 4
}

function testFindLastIndex(): boolean {
    return array.findLastIndex(funcPredicate1) === 3
}

function testFlat(): boolean {
    return arrayFlat.flat<Number>().toString() === "1,2,3,4,5,6"
}

function testFlatMap(): boolean {
    return arrFlatMap.flatMap(flatMapAndFlatten).toString() === "1,4,9,16"
}

function testForEach(): boolean {
    array.forEach(processElement)
    return result
}

function testIncludes(): boolean {
    return array.includes(3) && !(array.includes(7))
}

function testIndexOf(): boolean {
    return arrayIndex.indexOf(1) === 0 && arrayIndex.indexOf(2) === 2
}

function testJoin(): boolean {
    return array.join("") === "1234"
}

function testKeys(): boolean {
    let idx = 0
    for (const iter of array.keys()) {
        if (idx++ !== iter) {
            return false
        }
    }
    return true
}

function testLastIndexOf(): boolean {
    return arrayIndex.lastIndexOf(2) === 3
}

function testMap(): boolean {
    return array.map(mapAndFlatten).toString() === "1,4,9,16"
}

function testPop(): boolean {
    let a: number = arrayPop.pop() as number
    return a == 4
}

function testPushOne(): boolean {
    arrayPush.push(5)
    let a: number = arrayPush.pop() as number
    return a === 5
}

function testPushArray(): boolean {
    arrayPush.push(1,2,3,4)
    let a: number = arrayPush.pop() as number
    return a === 4
}

function testReduce(): boolean {
    return array.reduce(sumReducer, 0) as number === 10
}

function testReduceRight(): boolean {
    let a: string = arrayReduceRight.reduceRight(concatenateReducer, "") as string
    return a === "4321"
}

function testReverse(): boolean {
    arrayReverse.reverse()
    return arrayReverse.toString() === "4,3,2,1"
}

function testShift(): boolean {
    let a: number = arrayShift.shift() as number
    return a === 1
}

function testSlice(): boolean {
    return arraySlice.slice(2).toString() === "3,4" && arraySlice.slice(1, 3).toString() === "2,3"
}

function testSome(): boolean {
    return array.some(funcPredicate2)
}

function testSort(): boolean {
    arraySort.sort()
    return arraySort.toString() === "1,2,3,4"
}

function testSplice(): boolean {
    return arraySplice1.splice(1).toString() === "2,3,4" && arraySplice3.splice(1, 1, 2).toString() === "2"  && arraySplice4.splice(1, 2, 6, 7).toString() === "2,3"
}

function testToLocaleString(): boolean {
    return array.toLocaleString("en-US", new A("currency", "USD")) === "$1.00,$2.00,$3.00,$4.00"
}

function testToReversed(): boolean {
    return array.toReversed().toString() === "4,3,2,1"
}

function testToSorted(): boolean {
    return arrayToSorted.toSorted().toString() === "1,2,3,4"
}

function testToSpliced(): boolean {
    return array.toSpliced(1).toString() === "1" && array.toSpliced(1, 2).toString() === "1,4" && array.toSpliced(1, 2, 3).toString() === "1,3,4" && array.toSpliced(1, 2, 6, 7).toString() === "1,6,7,4"
}

function testUnShift(): boolean {
    return arrayUnShift.unshift() === 4 && arrayUnShift.unshift(1) === 5 && arrayUnShift.unshift(1, 2) === 7 && arrayUnShift.unshift(1, 2, 3) === 10
}

function testValues(): boolean {
    let idx = 0
    for (const iter of array.values()) {
        if (array[idx++] !== iter) {
            return false
        }
    }
    return true
}

function testWith(): boolean {
    let a = array.with(1, 8.0)
    return a.toString() === "1,8,3,4"
}
