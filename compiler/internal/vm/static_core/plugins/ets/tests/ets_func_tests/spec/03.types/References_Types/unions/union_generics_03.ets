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

/*---
desc: >-
    03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_generics_03
tags: []
---*/
type Margin = number | string | null | undefined

interface Comparable<T> {
    compare(l: T, r: T): int
}

class Item<T> implements Comparable<T> {
    item: T
    comparator: (l: T, r: T) => int
    Item(item: T, comparator: (l: T, r: T) => int ) {
        self.item = item
        self.comparator = comparator
    }
    compare(other: T): int {
        return self.comparator(this.item, other)
    }
}

function superMax<T extends Comparable<T>>(items: Item<T>[]): T | null {
    if (items.length == 0) {
        return null
    }
    let max: T = items[0]
    for (const item: T of items) {
        if (max.compare(item) < 0) {
            max = item
        }
    }
    return max
}

function getNumber(param: Margin): int {
    if (!param) {
        return 0
    }
    if (param instanceof string) {
        return (param as string).length()
    }
    return (param as number)
}

function compareMargin(left: Margin, right: Margin): int {
    let leftN: int = getNumber(left)
    let rightN: int = getNumber(right)
    return leftN - rightN
}

function main(): void {
    let arr: Array<Item<Margin> | null | undefined> = new Array([
        new Item<Margin>(5, compareMargin),
        new Item<number | string>("world", compareMargin),
        new Item<string>("hello world", compareMargin),
        null
    ])
    arr.push(new Item<number>(3, compareMargin))
    arr.push(undefined)
    let maxx: Margin = superMax<Margin>(arr)
    arktest.assertEQ( (maxx as number), 11)
}
