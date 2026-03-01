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
class Fulfill implements PromiseLike<string> {
    then<U, E>(onFulfill?: (value: string) => U | PromiseLike<U>, _?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            return onFulfill!('abc');
        });
    }
}

class Reject implements PromiseLike<string> {
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<E>((): E | PromiseLike<E> => {
            onFulfilled!('def');
            return onRejected!(new Error('xyz'));
        });
    }
}

let thenable: PromiseLike<string>[] = [new Fulfill(), new Reject()];
Promise.allSettled(thenable).then<void, void>((values: PromiseSettledResult<string>[]): void => {
    if (values.length != 2) {
        console.log('Test failed. Expected a string array of length 2 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != PromiseStatus.fulfilled || values[1].status != PromiseStatus.fulfilled) {
        console.log('Test failed. Expected all elements has status \'fulfilled\'.');
        return;
    }
    let v: string = (values[0] as PromiseFulfilledResult<string>).value;
    if (v != 'abc') {
        console.log('Test failed. Expected the first elements has value \'abc\' but got ' + v + '.');
        return;
    }
    v = (values[1] as PromiseFulfilledResult<string>).value;
    if (v != 'def') {
        console.log('Test failed. Expected the first elements has value \'abc\' but got ' + v + '.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
}).catch((e: Error): void => {
    console.log('Test failed: ' + e);
});
