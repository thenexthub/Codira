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

class Thenable implements PromiseLike<string> {
    then<U, E>(_?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.reject<E>(onRejected!(new Error('xyz')) as Error);
    }
}

let thenable = new Thenable();
Promise.allSettled([thenable]).then<void, void>((values: PromiseSettledResult<string>[]): void => {
    if (values.length != 1) {
        console.log('Test failed. Expected a string array of length 1 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != PromiseStatus.rejected) {
        console.log('Test failed. Expected all elements has status \'rejected\'.');
        return;
    }
    if ((values[0] as PromiseRejectedResult).reason.message != 'xyz') {
        console.log('Test failed. Unexpected reason of the first elements.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
});
