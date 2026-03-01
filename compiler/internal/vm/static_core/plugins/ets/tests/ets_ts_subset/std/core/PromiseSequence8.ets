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
let p = Promise.reject<number>(new Error('start error'));

p.finally((): void => {
    sequence.push(1);
}).then<void>((): void => {
    console.log('Test failed. The promise should be rejected.');
    throw new Error('Test failed');
}).catch<void>((err: Error): void => {
    sequence.push(2);
}).then<void>((): void => {
    checkSequence(sequence, 2);
    console.log('Test passed.');
});
