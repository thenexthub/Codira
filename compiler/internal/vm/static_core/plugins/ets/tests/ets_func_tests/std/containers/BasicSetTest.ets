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

const success : int = 0
const fail    : int = 1

function main(): int {
    let failures: int = 0
    failures += test(createEmptySet(),"Create Empty Set")
    failures += test(createSetAndAdd(), "Create Set and Add single item")

    failures += test(createSetAddAndClear(), "Create Set add and clear it")
    failures += test(createSetAddAndRemove(), "Create Set add and remove some key")

    failures += test(createSetAndGetValues(), "Get All Values from empty set")
    failures += test(createSetAddAndGetValues(), "Get All Values from non empty set")

    failures += test(createSetAddRemoveAndGetValues(), "Get Values after some value removed")
    failures += test(createSetAddClearAndGetValues(), "Get Values after all values removed")

    failures += test(createSetAddAndCheckHasValue(),"Check set has given value")
    failures += test(createSetFromBuiltinArray(), "Create set from builtin array")
    return test(failures, "All tests run")

}

function getIteratorLen<T>(iter: Iterator<T>): int {
    let size: int = 0
    while(!iter.next().done)
        size++

    return size
}

function createSetAddAndCheckHasValue(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'b')
    set.add(c'c')
    let result = set.has(c'c') ? 0 : 1
    return result
}

function createEmptySet(): int {
    let set: Set<char> = new Set<char>
    return set.size.toInt()
}

function createSetAndGetValues(): int {
    let set: Set<char> = new Set<char>
    let values = set.values()

    let result = getIteratorLen<char>(values) == 0 ? 0 : 1
    return result
}

function createSetAddAndGetValues(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'c')
    set.add(c'b')
    set.add(c'c')
    let values = set.values()

    let result = getIteratorLen<char>(values) == 3 ? 0 : 1
    return result
}

function createSetAddRemoveAndGetValues(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'b')
    set.add(c'c')
    set.delete(c'c')
    let values = set.values()
    let result = getIteratorLen<char>(values) == 2 ? 0 : 1
    return result
}

function createSetAddClearAndGetValues(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'b')
    set.add(c'c')
    set.add(c'c')
    set.clear()
    let values = set.values()
    let result = getIteratorLen<char>(values) == 0 ? 0 : 1
    return result
}

function createSetAddAndRemove(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'b')
    set.add(c'c')
    set.delete(c'c')
    let result = set.size == 2 ? 0 : 1
    return result
}

function createSetAddAndClear(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    set.add(c'b')
    set.add(c'c')
    set.clear()
    let result = set.size == 0 ? 0 : 1
    return result
}

function createSetAndAdd(): int {
    let set: Set<char> = new Set<char>
    set.add(c'a')
    let result = set.size == 1 ? 0 : 1
    return result
}

function createSetFromBuiltinArray(): int {
    let set = new Set<string> (
        [
            new String("foo"),
            new String("bar"),
            new String("bazz"),
            new String("bazz"),
            new String("spam"),
            new String("eggs")
        ]
    )
    let res = set.has("foo")
    res = res && set.has("bar")
    res = res && set.has("bazz")
    res = res && set.has("spam")
    res = res && set.has("eggs")
    if (res) {
        return success
    }
    console.log(`Error: expected elements: \`foo,bar,bazz,spam,eggs\`, but got: \`${set}\``)
    return fail
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println('PASSED: ' + message)
      return success
    }
    console.println('FAILED: ' + message)
    return fail
}
