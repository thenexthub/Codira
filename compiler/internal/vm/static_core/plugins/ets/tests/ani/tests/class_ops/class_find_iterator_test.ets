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

// test in namespace ops
namespace ops {
class Singleton {
    constructor() {
        this.weight = 0;
    }
    weight: int;
    data: int[] = [1, 2, 3]
    $_iterator() { // Function type is inferred from its body
        return new SingletonIterator(this)
    }
}

class SingletonIterator implements Iterator<int> {
    index = 0
    base: Singleton
    constructor (base: Singleton) {
        this.base = base
    }
    next(): IteratorResult<int> {
        return {
            done: this.index >= this.base.data.length,
            value: this.index >= this.base.data.length ? undefined : this.base.data[this.index++]
        }
    }
}
};

// test out of namespace ops
class Singleton {
     constructor() {
        this.weight = 1;
    }
    weight: int;
    data: int[] = [2, 4, 6]
    $_iterator() { // Function type is inferred from its body
        return new SingletonIterator(this)
    }
}

class SingletonIterator implements Iterator<int> {
    index = 0
    age = 0
    base: Singleton
    constructor (base: Singleton) {
        this.base = base
        this.age = 1
    }
    next(): IteratorResult<int> {
        return {
            done: this.index >= this.base.data.length,
            value: this.index >= this.base.data.length ? undefined : this.base.data[this.index++]
        }
    }
 }