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
        console.log('Test failed. Expected sequence length ' + expectedLength + ', but got ' + actual.length);
        throw new Error('Test failed');
    }
    for (let i = 0; i < expectedLength; ++i) {
        if (actual[i] != i + 1) {
            console.log('Test failed. Expected element at ' + i + ' to be ' + i + ', but got ' + actual[i]);
            throw new Error('Test failed');
        }
    }
}

let sequence = new Array<number>();
let resolve: ((value: number) => void) | null = null;
let p = new Promise<number>((_resolve: (v: number) => void, _: (error: Error) => void) => {
    resolve = _resolve;
});

sequence.push(1);

resolve!(7);

p.then<void>((): void => {
    sequence.push(3);
    checkSequence(sequence, 3);
});

Promise.resolve().then<void>((): void => {
    // enqueue another then-handler
    p.then<void>((): void => {
        sequence.push(5);
        checkSequence(sequence, 5);
        console.log('Test passed.');
    });

    sequence.push(4);
    checkSequence(sequence, 4);
});

sequence.push(2);
