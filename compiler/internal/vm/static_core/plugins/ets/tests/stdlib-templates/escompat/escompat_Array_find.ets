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
desc:  {function: Test_escompat_Array_find }
---*/


function main(): int {
    let failures: int = 0

    failures += check((): int => { return testFindLastIndex()}, "Test findLastIndex method")
    failures += check((): int => { return testFindLastIndexMissed()}, "Test findLastIndex method on missed data")

    failures += check((): int => { return testFind()}, "Test find method")
    failures += check((): int => { return testFindMissData()}, "Test find missed data")
    failures += check((): int => { return testFindIndex()}, "Test find index method")
    failures += check((): int => { return testFindMissIndex()}, "Test find missed index method")

    failures += check((): int => { return testFindStatic()}, "Test static find method")
    failures += check((): int => { return testFindMissDataStatic()}, "Test static find missed data")
    failures += check((): int => { return testFindIndexStatic()}, "Test static find method")
    failures += check((): int => { return testFindMissIndex()}, "Test static find missed index method")

    failures += check((): int => { return testFindLast()}, "Test find backward")
    failures += check((): int => { return testFindLastMissed()}, "Test find backward missed")
    failures += check((): int => { return testFindLastStatic()}, "Test static find backward")
    failures += check((): int => { return testFindLastMissedStatic()}, "Test static find backward missed")

    return check(failures, "All tests run")
}

const source: Object[] = {{.item.copySource}}
const source2: Object[] = {{.item.concatSource}}

const SUCCESS = 0
const FAILURE = 1

function testFind(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2]

    let test = src.find((item: Object): boolean => { return item == match})
    if (test == match) return SUCCESS
    console.println("Missed expected")
    return FAILURE
}

function testFindMissData(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let test = src.find((item: Object): boolean => { return item == match})
    if (test == null) return SUCCESS
    console.println("Found some unexpected")
    return FAILURE
}

function testFindIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2]

    let index = src.findIndex((item: Object): boolean => { return item == match})
    if (index == 2) return SUCCESS
    console.println("Missed expected index")
    return FAILURE
}

function testFindMissIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let index = src.findIndex((item: Object): boolean => { return item == match})
    if (index == -1) return SUCCESS
    console.println("Found some unexpected")
    return FAILURE
}



function testFindStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2]

    let test = src.find((item: Object): boolean => { return item == match})
    if (test == match) return SUCCESS
    console.println("Missed expected")
    return FAILURE
}

function testFindMissDataStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let test = src.find((item: Object): boolean => { return item == match})
    if (test == null) return SUCCESS
    console.println("Found some unexpected")
    return FAILURE
}


function testFindIndexStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[2]

    let index = src.findIndex((item: Object): boolean => { return item == match})
    if (index == 2) return SUCCESS
    console.println("Missed expected index")
    return FAILURE
}

function testFindMissIndexStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let index = src.findIndex((item: Object): boolean => { return item == match})
    if (index == -1) return SUCCESS
    console.println("Found some unexpected")
    return FAILURE
}

function testFindLast(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3]

    let test = src.findLast((item: Object): boolean => { return item == match})
    if (test == match) return SUCCESS
    console.println("Missed expected")
    return FAILURE
}

function testFindLastStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3]

    let test = src.findLast((item: Object): boolean => { return item == match})
    if (test == match) return SUCCESS
    console.println("Missed expected")
    return FAILURE
}


function testFindLastIndex(): int {
    const src = Array.from<object>(source)
    let match: Object = source[3]

    let test = src.findLastIndex((item: Object): boolean => { return item == match})
    if (test == 3) return SUCCESS

    return FAILURE
}

function testFindLastIndexMissed(): int {
    const src = Array.from<object>(source)
    let match: Object = "Missed"

    let test = src.findLastIndex((item: Object): boolean => { return item == match})
    if (test == -1) return SUCCESS

    return FAILURE
}


function testFindLastMissed(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let test = src.findLast((item: Object): boolean => { return item == match})
    if (test == null) return SUCCESS
    console.println("Found some unexpected")
    return FAILURE
}

function testFindLastMissedStatic(): int {
    const src = Array.from<object>(source)
    let match: Object = {{.item.fillValue}}

    let test = src.findLast((item: Object): boolean => { return item == match})
    if (test == null) return SUCCESS
    console.println("Found some unexpected")
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