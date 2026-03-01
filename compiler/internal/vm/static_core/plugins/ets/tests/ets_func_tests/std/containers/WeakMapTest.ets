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

class Key {
    public readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

class Val {
    public readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

function weakMapOperations(): void {
    const weakMap = new WeakMap<Key, Val>()

    let k: Key | null = new Key(1)
    const v = new Val(k.id)

    weakMap.set(k, v)

    arktest.assertTrue(v === weakMap.get(k), 'WeakMap.get() returned unexpected value')
    arktest.assertTrue(weakMap.has(k), 'WeakMap.has() returned false')

    arktest.assertTrue(weakMap.delete(k), 'WeakMap.delete() returned false')

    // repeated removal
    arktest.assertFalse(weakMap.delete(k), 'repeated WeakMap.delete() returned true')

    arktest.assertFalse(weakMap.has(k), 'after detete WeakMap.has() returned true')
}

function newWeakMapUsingEntriesArray(): void {
    const key10 = new Key(10)
    const key20 = new Key(20)

    const entries: ([Key, Val])[] = [[key10, new Val(key10.id)], [key20, new Val(key20.id)]]
    const weakMap = new WeakMap<Key, Val>(entries)

    arktest.assertTrue(weakMap.has(key10))
    arktest.assertTrue(weakMap.has(key20))
}

function newWeakMapUsingEntriesIterable(): void {
    const key1 = new Key(100)
    const key2 = new Key(101)

    const entry1: [Key, Val] = [key1, new Val(key1.id)]
    const entry2: [Key, Val] = [key2, new Val(key2.id)]

    const entries: Iterable<[Key, Val]> = Array.of<[Key, Val]>(entry1, entry2)
    const weakMap = new WeakMap<Key, Val>(entries)

    arktest.assertTrue(weakMap.has(key1))
    arktest.assertTrue(weakMap.has(key2))
}

function newWeakMapUsingNullEntries(): void {
    const nullEntriesMap = new WeakMap<Key, Val>(null)

    const key1 = new Key(200)
    nullEntriesMap.set(key1, new Val(key1.id))

    arktest.assertTrue(nullEntriesMap.has(key1))
}

function newWeakMapUsingUndefinedEntries(): void {
    const undefEntriesMap = new WeakMap<Key, Val>(undefined)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('WeakMap API tests')
    suite.addTest('new WeakMap using entries array', newWeakMapUsingEntriesArray)
    suite.addTest('new WeakMap using entries iterable', newWeakMapUsingEntriesIterable)
    suite.addTest('WeakMap operations', weakMapOperations)
    suite.addTest('new WeakMap using null entries', newWeakMapUsingNullEntries)
    suite.addTest('new WeakMap using undefined entries', newWeakMapUsingUndefinedEntries)

    return suite.run()
}
