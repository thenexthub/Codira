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

function checkSequence(actual: Array<number>, expectedLength: number): void {
    if (actual.length != expectedLength) {
        console.error('Test failed. Expected sequence length ' + expectedLength + ', but got ' + actual.length);
        throw new Error('Test failed.');
    }
    for (let i = 0; i < expectedLength; ++i) {
        if (actual[i] != i + 1) {
            console.error('Test failed. Expected element at ' + i + ' to be ' + i + ', but got ' + actual[i]);
            throw new Error('Test failed.');
        }
    }
}

let sequence = new Array<number>();
class Thenable implements PromiseLike<string> {
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<Awaited<U|E>> {
        sequence.push(3);
        return Promise.resolve(onFulfilled!('resolved')) as PromiseLike<Awaited<U|E>>;
    }
};
sequence.push(1);
let thenable = new Thenable();
let p = new Promise<string>((res) => {
    res(thenable);
})
sequence.push(2);

p.then<void, void>((v:Object): void => {
    sequence.push(4)
    checkSequence(sequence, 4)
    console.log('Test passed.');
}, (e: Error): void => {
    console.error('Test failed. The promise should not be rejected.');
})
