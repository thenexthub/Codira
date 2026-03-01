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
desc:  {function: Test_escompat_Array_misc }
---*/


function main(): int {
    let failures: int = 0

    failures += check((): int => { return testGoodNegIndex()}, "Test good negative index")
    failures += check((): int => { return testGoodPosIndex()}, "Test good positive index")

    failures += check((): int => { return testBadNegIndex()}, "Test bad negative index")
    failures += check((): int => { return testBadPosIndex()}, "Test bad positive index")

    failures += check((): int => { return testJoinEmpty()}, "Test join(0) method with empty source array")
    failures += check((): int => { return testJoin0()}, "Test join(0) method")
    failures += check((): int => { return testJoin1()}, "Test join(1) method")

    failures += check((): int => { return testSlice0()}, "Test slice(0) method")
    failures += check((): int => { return testSlice1()}, "Test slice(1) method")
    failures += check((): int => { return testSlice2()}, "Test slice(2) method")

    failures += check((): int => { return testIncludes()}, "Test includes() method")
    failures += check((): int => { return testIncludesInEmpty()}, "Test includes() method with emptyArray")

    failures += check((): int => { return testIndexOf()}, "Test indexOf method")
    failures += check((): int => { return testIndexOfMissed()}, "Test indexOf method with missed data")
    failures += check((): int => { return testIndexOfFromEmpty()}, "Test indexOf method with empty array")

    {% for ci in copyWithin -%}
    failures += check((): int => { return array{{.ci.name}}()}, "Test262: {{.ci.name}}")
    {%+ endfor %}


    return check(failures, "All tests run")
}

const source: Object[] = {{.item.copySource}}
const source2: Object[] = {{.item.concatSource}}

const SUCCESS = 0
const FAILURE = 1

function testIncludes(): int {
    const array = Array.from<object>(source)

    if (array.length != source.length) {
        console.println("Unexpected target length")
        return FAILURE
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.includes(source[i])) continue
        console.println("Missed expected data")
        return FAILURE
    }

    return SUCCESS
}


function testJoinEmpty():int {
    let empty: Object[] = []
    let src = new Array<Object>(empty)

    let target: String = src.join()
    if (target != "") {
        console.println("Unexpected empty join result")
        return FAILURE
    }
    return SUCCESS
}

function testJoin0(): int {
    const src = Array.from<object>(source2)
    let target: String = src.join()

    if (target != "{{.item.joinResult0}}") {
        console.println("Unexpected join(0) result: " + target + " vs " + "{{.item.joinResult0}}")
        return FAILURE
    }
    return SUCCESS
}

function testJoin1(): int {
    const src = Array.from<object>(source2)
    let target: String = src.join("+")

    if (target != "{{.item.joinResult1}}") {
        console.println("Unexpected join(1) result: " + target + " vs " + "{{.item.joinResult1}}")
        return FAILURE
    }
    return SUCCESS
}

function testSlice0(): int {
    const src = Array.from<object>(source2)

    let sliceStart = 0
    let sliceEnd = src.length
    let length = sliceEnd - sliceStart

    let target = src.slice()

    if (target.length != length) {
        console.println("Unexpected length change on slice(0)")
        return FAILURE
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue
        console.println("Bad slice copy")
        return FAILURE
    }
    return SUCCESS
}

function testSlice1(): int {
    let src = new Array<Object>(source2)

    let sliceStart = 0
    let sliceEnd = src.length
    let length = sliceEnd - sliceStart

    let target = src.slice(sliceStart)

    if (target.length != length) {
        console.println("Unexpected length change on slice(1)")
        return FAILURE
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue
        console.println("Bad slice copy")
        return FAILURE
    }
    return SUCCESS
}

function testSlice2(): int {
    let src = new Array<Object>(source2)

    let sliceStart: int = 0
    let sliceEnd = src.length
    let length = sliceEnd - sliceStart

    let target = src.slice(sliceStart, sliceEnd)

    if (target.length != length) {
        console.println("Unexpected length change on slice(2)")
        return FAILURE
    }


    for (let i = sliceStart; i < sliceEnd; i++) {
        if (target.at(i) == src.at(i)) continue
        console.println("Bad slice copy")
        return FAILURE
    }
    return SUCCESS
}

function testIncludesInEmpty(): int {
    let array: Array<Object> = new Array<Object>()

    if (array.length != 0) {
        console.println("Unexpected target length")
        return FAILURE
    }

    for (let i: int = 0; i < array.length; i++) {
        if (array.includes(source[i])) return FAILURE
    }

    return SUCCESS
}


function testIndexOf(): int {
    const array = Array.from<object>(source)
    let result = array.indexOf(source[3])
    if (result == 3) return SUCCESS

    console.println("Unexpected data returned")
    return FAILURE

}

function testIndexOfMissed(): int {
    const array = Array.from<object>(source)
    let test: Object = {{.item.fillValue}}
    let result = array.indexOf(test)
    if (result == -1) return SUCCESS
    console.println("Unexpected data returned")
    return FAILURE
}


function testIndexOfFromEmpty(): int {
    let array: Array<Object> = new Array<Object>()

    let result = array.indexOf(source[3])
    if (result == -1) return SUCCESS
    console.println("Unexpected data returned")
    return FAILURE
}

function testGoodPosIndex(): int {
    const array = Array.from<object>(source)
    let test = array.at(0)
    if (test == source[0]) return SUCCESS
    console.println("Bad at() result")
    return FAILURE
}

function testGoodNegIndex(): int {
    const array = Array.from<object>(source)
    let test = array.at(-array.length)
    if (test == source[0]) return SUCCESS
    console.println("Bad at() result")
    return FAILURE
}


function testBadNegIndex(): int {
    const array = Array.from<object>(source)
    try {
        let exp = array.at(-100)
        console.println("Unexpected result on bad negative index")
        return FAILURE
    } catch(e) {

    }
    return SUCCESS
}

function testBadPosIndex(): int {
    const array = Array.from<object>(source)
    try {
        let exp = array.at(100)
        console.println("Unexpected result on bad positive index")
        return FAILURE
    } catch(e) {

    }
    return SUCCESS
}

// currently not used tests *Flat*

function testFlatEmpty(): int {
    let arr: Array<Object> = new Array<Object>()

    let flatten = arr.flat<Object>()
    if (flatten.length == 0) return SUCCESS
    console.println("Unexpected flatten array length")
    return FAILURE
}

function testFlatFlat(): int {
    let arr: Array<Object> = new Array<Object>(source)

    let flatten = arr.flat<Object>()
    if (flatten.length == arr.length) return SUCCESS
    console.println("Unexpected flatten array length")
    return FAILURE
}

function testFlatNonFlat1(): int {
    let arr: Array<Object> = new Array<Object>(source)
    let length = arr.length

    arr.push(source)

    let flatten = arr.flat<Object>()

    if (flatten.length == length * 2) return SUCCESS
    console.println("Unexpected flatten array length: " + flatten.length + " expected: " + length * 2)
    return FAILURE
}

function testFlatNonFlat2(): int {
    let arr: Array<Object> = new Array<Object>(source)
    let length = arr.length

    arr.push(new Array<Object>(source))

    let flatten = arr.flat<Object>()


    if (flatten.length == length * 2) return SUCCESS
    console.println("Unexpected flatten array length: " + flatten.length + " expected: " + length * 2)
    return FAILURE
}

{% for ci in copyWithin %}
function array{{.ci.name}}(): int {
    let arr: Array<Object> = new Array<Object>(source)
    let target: Array<Object> = arr.copyWithin({{.ci.params|join(', ')}})

    return SUCCESS
}
{%- endfor %}

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
