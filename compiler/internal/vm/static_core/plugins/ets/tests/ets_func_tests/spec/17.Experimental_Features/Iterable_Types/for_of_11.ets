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
desc: 8.9 for-of Statements. 17.4, 17.5.1 - iterable
tags: []
---*/

class Foo<T> implements Iterable {
    values : T[]
    constructor(p_values: T[]) {this.value = p_values}
    [Symbol.iterator]():FooIterator<T>  {
        return new FooIterator(this.values);
    }
}

class FooIterator<T> implements Iterator<T> {
    private index: number;
    private done: boolean;

    constructor(values: T[]) {
        this.index = 0;
        this.done = false;
    }

    next(): IteratorResult<T, number | undefined> {
        if (this.done)
           return {
              done: this.done,
              value: undefined
           };

        if (this.index === this.values.length) {
            this.done = true;
            return {
                done: this.done,
                value: this.index
            };
        }

        const value = this.values[this.index];
        this.index++;
        return {
            done: false,
            value
        };
    }
}


function main() : int {
    const foo = new Foo([1,2,3]);
    for(const value of foo) {
        console.log(values);
    }

    return 0;
}

