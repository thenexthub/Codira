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

class PoisonedIterator<T> implements Iterator<T> {
    constructor(error: Error) {
        this.error = error;
    }

    next(): IteratorResult<T> {
        throw this.error;
    }

    private error: Error;
}

class MyIterable<T> implements Iterable<T> {
    constructor(error: Error) {
        this.error = error;
    }

    [Symbol.iterator](): Iterator<T> {
        return new PoisonedIterator<T>(this.error);
    }

    private error: Error;
}

let error = new Error();
Promise.allSettled<Object>(new MyIterable<Object>(error))
        .then<void, void>((value: PromiseSettledResult<Object>[]): void => {
    console.log('Test failed. The promise should not be resolved.');
}, (err: Error): void => {
    if (err != error) {
        console.log('Test failed. The promise should be rejected by the provided error.');
        return;
    }
    console.log('Test passed.');
});
