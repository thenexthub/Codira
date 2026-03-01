/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

{% for item in escompat_Array %}
/*---
desc:  {function: Test_escompat_Array_methods }
---*/


function main(): int {
    let failures: int = 0

    failures += check((): int => { return testCreateFrom()}, "Test Create Array<Object> object from array")
    failures += check((): int => { return testCreateFrom2()}, "Test Create Array<Object> object from array with given function")
    failures += check((): int => { return testEvery()}, "Test Array<Object> every() method")
    failures += check((): int => { return testSome()}, "Test Array<Object> some() method")
    failures += check((): int => { return testSomeNegative()}, "Test Array<Object> some() method")
    failures += check((): int => { return testMap0()}, "Test create new Array<Object> object apply given function")
    failures += check((): int => { return testReverse()}, "Test Array<Object> reverse function")
    failures += check((): int => { return testToReversed()}, "Test Arrat toReversed function")
    failures += check((): int => { return testFill0()}, "Test Array<Object> fill(0) method")
    failures += check((): int => { return testFill3()}, "Test Array<Object> fill(3) method")
    failures += check((): int => { return testConcat()}, "Test Array<Object> concatenate method")


    return check(failures, "All tests run")
}

const source: Object[] = {{.item.copySource}}
const source2: Object[] = {{.item.concatSource}}

const SUCCESS = 0
const FAILURE = 1


function testCreateFrom(): int {
    let array: Array<Object> = Array.from<Object>(source)
    if (array.length != source.length) {
        console.println("Data length mismatch")
        return FAILURE
    }
    for (let i: int = 0; i < source.length; i++) {
        if (array.at(i) ==  source[i]) continue
        console.println("Data mismatch")
        return FAILURE
    }
    return SUCCESS
}

function testCreateFrom2(): int {
    const sourceArr = Array.from<object>(source)
    const array = Array.from<Object, Object>(sourceArr, (v: Object, _: int): Object => { return v } )
    if (array.length != source.length) {
        console.println("Data length mismatch")
        return FAILURE
    }
    for (let i: int = 0; i < source.length; i++) {
        if (array.at(i) ==  source[i]) continue
        console.println("Data mismatch")
        return FAILURE
    }
    return SUCCESS
}

function testMap0(): int {
    const array = Array.from<object>(source)
    let map: Object = {{.item.fillValue}}

    let target = array.map((item: Object): {{.item.copySourceItemType}} => { return item as {{.item.copySourceItemType}} })
    if (target.length != array.length) {
        console.println("Data length mismatch")
        return FAILURE
    }

    return SUCCESS
}

function testEvery(): int {
    const array = Array.from<object>(source2)
    let test: boolean = array.every((item: Object): boolean => { return item instanceof {{.item.itemType}}})
    if (test) return SUCCESS
    console.println("Unexpected every result")
    return FAILURE
}

function testSome(): int {
    const array = Array.from<object>(source2)
    let match: Object = source2[1]

    let test: boolean = array.some((item: Object): boolean => { return item == match})
    if (test) return SUCCESS
    console.println("Unexpected some result")
    return FAILURE
}

function testSomeNegative(): int {
    let array: Array<Object> = new Array<Object>(source2)
    let match: Object = {{.item.fillValue}}

    let test: boolean = array.some((item: Object): boolean => { return item == match})
    if (!test) return SUCCESS
    console.println("Unexpected negative some result")
    return FAILURE
}


function testFill0(): int {
    const array = Array.from<object>(source)
    array.fill({{.item.fillValue}})
    return checkValues(array, {{.item.fillValue}})
}

function testFill3(): int {
    const array = Array.from<object>(source)
    array.fill({{.item.fillValue}}, 1, 3)
    return checkValues(array, {{.item.fillValue}}, 1, 3)
}

function testConcat(): int {
    let array1: Array<Object> = new Array<Object>(source)
    let array2: Array<Object> = new Array<Object>(source2)

    let target: Array<Object> =  array1.concat(array2)
    if (target.length != array1.length * 2) {
        console.println("Expected array length mismatch")
        return FAILURE
    }
    let tl = array1.length

    for (let i = 0; i < tl; i++) {
        if ( target.at(i) != array1.at(i)) {
            console.println("produced array not contains this array")
            return FAILURE
        }
        if (target.at(i+tl) != array2.at(i)) {
            console.println("produced array not contains given array")
            return FAILURE
        }
    }

    return SUCCESS
}

function testReverse(): int {
    const src = Array.from<object>(source)
    src.reverse()
    let l = src.length

    for (let i = 0; i < l; i++) {
        if (src.at(i.toInt()) == source[l-i-1]) continue
        console.println("Bad data after reverse")
        return FAILURE
    }

    return SUCCESS
}

function testToReversed(): int {
    const src = Array.from<object>(source)
    let reversed = src.toReversed()

    let l = reversed.length

    for (let i = 0; i < l; i++) {
        if (reversed.at(i.toInt()) == source[l-i-1]) continue
        console.println("Bad data after reverse")
        return FAILURE
    }

    return SUCCESS
}

function checkValues(source: Array<Object>, value: Object): int {
    for (let i: int = 0; i < source.length; i++) {
        if (source.at(i) == value) continue
        return FAILURE
    }
    return SUCCESS
}

function checkValues(source: Array<Object>, value: Object, start: int, end: int): int {
    for (let i: int = start; i < end; i++) {
        if (source.at(i) == value) continue
        return FAILURE
    }
    return SUCCESS
}

function check(result: int, message: String): int {
    if (result == 0) {
        return SUCCESS
    }
    console.println("\nFAILED: " + message)
    return FAILURE
}

function check(fn: () => int, message: String): int {
    let result: int = 0
    try {
        result = fn()
    } catch(e) {
        console.println("FAILED: " + message + " => Error catched: " + e)
        return FAILURE
    }

    if (result == 0) {
        return SUCCESS
    }
    console.println("\nFAILED: " + message)
    return FAILURE
}

{%- endfor %}
