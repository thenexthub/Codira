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

let numberArray: FixedArray<number> = [1, 2, 3];
let boolArray: FixedArray<boolean> = [true, false, true];
class Obj {}
let objArray:FixedArray<Obj> = [new Obj(), new Obj(), new Obj()];
let str: string = 'ABC';
class Foo {
    data: FixedArray<number> = [1, 2, 3, 4, 5];
    $_iterator() : FooIterator {
        return new FooIterator(this);
    }
}

class FooIterator implements Iterator<number> {
    metadata: Foo;
    index: int = 0;
    constructor(metadata: Foo) {
        this.metadata = metadata;
    }
    next(): IteratorResult<number> {
        return {
            done: this.index >= this.metadata.data.length,
            value: this.index >= this.metadata.data.length ? undefined : this.metadata.data[this.index++]
        };
    }
}

let foo: Foo = new Foo()