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
desc:  {function: Test_escompat_Array_reduce_sort }
---*/


function main(): int {
    let failures: int = 0

    failures += check((): int => { return testReduce()}, "Test reduce")
    failures += check((): int => { return testReduceWithStart()}, "Test reduce with start value")

    failures += check((): int => { return testReduceRight()}, "Test reduce backward")
    failures += check((): int => { return testReduceRightWithStart()}, "Test reduce backward with start value")

    failures += check((): int => { return testToSorted()}, "Test toSorted()")

    return check(failures, "All tests run")
}

const source: FixedArray<Object> = {{.item.copySource}}
const source2: FixedArray<Object> = {{.item.concatSource}}

const SUCCESS = 0
const FAILURE = 1

function testToSorted(): int {

    let unsorted: FixedArray<Object> = new FixedArray<Object>(source.length)
    let l:int = unsorted.length

    for (let i: int = 0; i < l; i++) {
        unsorted[i] = source[l-i-1]
    }

    const src = Array.from<object>(unsorted)

    let sorted = src.toSorted()

    for (let i: int = 0; i < src.length; i++) {
        if (sorted.at(i) == source[i]) continue
        console.print("Bad data order")
        return FAILURE
    }

    return SUCCESS
}

function testReduce(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduce((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString()
        tmp += val.toString()
        return tmp
    })
    let expected: String = "{{.item.reduce}}"
    if (expected == reduce as String) return SUCCESS
    console.println("Unexpected reduce: " + reduce)
    return FAILURE
}

function testReduceRight(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduceRight((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString()
        tmp += val.toString()
        return tmp
    })
    let expected: String = "{{.item.reduceRight}}"
    if (expected == reduce as String) return SUCCESS
    console.println("Unexpected reduce: " + reduce)
    return FAILURE
}


function testReduceWithStart(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduce((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString()
        tmp += val.toString()
        return tmp
    }, "Hello")
    let expected: String = "Hello{{.item.reduce}}"
    if (expected == reduce as String) return SUCCESS
    console.println("Unexpected reduce: " + reduce)
    return FAILURE
}

function testReduceRightWithStart(): int {
    const arr = Array.from<object>(source2)

    let reduce = arr.reduceRight((acc: Object, val: Object): Object => {
        let tmp: String = acc.toString()
        tmp += val.toString()
        return tmp
    }, "Hello")
    let expected: String = "Hello{{.item.reduceRight}}"
    if (expected == reduce as String) return SUCCESS
    console.println("Unexpected reduce: " + reduce)
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