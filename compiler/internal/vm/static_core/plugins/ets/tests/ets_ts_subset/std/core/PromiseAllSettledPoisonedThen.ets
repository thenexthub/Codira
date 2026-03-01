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
    constructor(error: Error) {
        this.error = error;
    }

    then<U, E>(_?: (value: string) => U | PromiseLike<U>, __?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        throw this.error;
    }

    private error: Error;
};

let error = new Error();
let thenable = new Thenable(error);
Promise.allSettled<string>([thenable]).then<void, void>((result: PromiseSettledResult<string>[]): void => {
    if (result.length != 1) {
        console.log('Test failed. Expected array of length 1');
        return;
    }
    if (result[0].status != PromiseStatus.rejected) {
        console.log('Test failed. Expected status \'rejected\', but got ' + result[0].status + '.');
        return;
    }
    if ((result[0] as PromiseRejectedResult).reason != error) {
        console.log('Test failed. Unexpected value.');
        return;
    }
    console.log('Test passed.');
}, (err: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
}).catch((e: Error): void => {
    console.log('Test failed: ' + e);
});
