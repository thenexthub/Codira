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
let p1Err = new Error('abc');
let p2Err = new Error('def');
let p1 = Promise.reject<Error>(p1Err);
let p2 = Promise.reject<Error>(p2Err);

sequence.push(1);

p1.catch<void>((): void => {
    sequence.push(3);
    checkSequence(sequence, 3);
});

Promise.any<Error>([p1, p2]).then<number>((): number => {
    sequence.push(5);
    checkSequence(sequence, 5);
    return 0;
}).then<void, void>((value): void => {}, (err): void => {
    if ((err as AggregateError).errors.length != 2) {
        console.log('Test failed. Expected number of errors 2, but got ' + (err as AggregateError).errors.length);
        throw new Error('Test failed');
    }
    if ((err as AggregateError).errors[0] != p1Err) {
        console.log('Test failed. Expected the first error is \'abc\', but got ' + (err as AggregateError).errors[0]);
        throw new Error('Test failed');
    }
    if ((err as AggregateError).errors[1] != p2Err) {
        console.log('Test failed. Expected the first error is \'def\', but got ' + (err as AggregateError).errors[1]);
        throw new Error('Test failed');
    }
    console.log('Test passed.');
});

p2.catch<void>((): void => {
    sequence.push(4);
    checkSequence(sequence, 4);
});

sequence.push(2);
