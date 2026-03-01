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

{% for item in escompat_Array %}
/*---
desc:  {function: Test_escompat_Array_modifications }
---*/


function main(): int {
    let failures: int = 0

    failures += check((): int => { return testPushEmpty()}, "Test push() method with empty Array<Object>")
    failures += check((): int => { return testPush()}, "Test push() method")
    failures += check((): int => { return testSplice1()}, "Test splice 1")
    failures += check((): int => { return testSplice2()}, "Test splice 2")
    failures += check((): int => { return testSplice3()}, "Test splice 3")
    failures += check((): int => { return testShift()}, "Test shift() method")
    failures += check((): int => { return testShiftEmpty()}, "Test shift() method with empty Array<Object>")
    failures += check((): int => { return testPop()}, "Test pop() method")
    failures += check((): int => { return testPopEmpty()}, "Test pop() method")


    return check(failures, "All tests run")
}

const source: Object[] = {{.item.copySource}}
const source2: Object[] = {{.item.concatSource}}

const SUCCESS = 0
const FAILURE = 1

function testShift(): int {
    const array = Array.from<object>(source)
    let length = array.length

    let shifted = array.shift()

    length--
    if (array.length != length) {
        console.println("Target array length mismatch: " + array.length + " but expected" + length)
        return FAILURE
    }

    if (shifted != source[0] as Object) {
        console.println("Unexpected value returned at shift")
        return FAILURE
    }

    for (let i: int = 0; i < array.length; i++) {
        let j = i+1
        if (array.at(i) == source[j]) continue
        console.println("Data mismatch")
        return FAILURE
    }

    return SUCCESS
}

function testShiftEmpty(): int {
    let array: Array<Object> = new Array<Object>()
    let length = array.length

    let result = array.shift()
    if (array.length != 0) {
        console.println("Array<Object> size mismatch")
        return FAILURE
    }
    if (result != null) {
        console.println("Unexpected data returned at shift empty " + result)
        return FAILURE
    }
    return SUCCESS
}


function testPop(): int {
    const array = Array.from<object>(source)
    let length = array.length

    let popped = array.pop()

    length--
    if (array.length != length) {
        console.println("Target array length mismatch: " + array.length + " but expected" + length)
        return FAILURE
    }

    if (popped != source[length] as Object) {
        console.println("Unexpected value returned at pop")
        return FAILURE
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.at(i) == source[i]) continue
        console.println("Data mismatch")
        return FAILURE
    }
    return SUCCESS
}


function testPopEmpty(): int {
    let array: Array<Object> = new Array<Object>()
    let length = array.length

    let result = array.pop()
    if (array.length != 0) {
        console.println("Array<Object> size mismatch")
        return FAILURE
    }
    if (result != null) {
        console.println("Unexpected data returned at pop empty" + result)
        return FAILURE
    }
    return SUCCESS
}


function testPushEmpty(): int {
    let array: Array<Object> = new Array<Object>()


    let length = array.push(source[0])
    if (length == 1) return SUCCESS
    console.println("Bad length after push")
    return FAILURE
}

function testPush(): int {
    const array = Array.from<object>(source)

    let length = array.length
    length++

    let test = array.push(source[0])

    if (test == length) return SUCCESS
    console.println("Bad length after push")
    return FAILURE
}

function testSplice1(): int {
    const arr = Array.from<object>(source)
    let length = arr.length

    arr.splice(2 as int)
    if (arr.length == 2) return SUCCESS
    console.println("Unexected size after splice")
    return FAILURE
}

function testSplice2(): int {
    const arr = Array.from<object>(source)
    let length = arr.length

    arr.splice(2 as int, 1 as int)
    if (arr.length == 4) return SUCCESS
    console.println("Unexected size after splice")
    return FAILURE
}

function testSplice3(): int {
    const arr = Array.from<object>(source)
    let length = arr.length

    arr.splice(-1 as int)
    if (arr.length == 4) return SUCCESS
    console.println("Unexected size after splice")
    return FAILURE
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
