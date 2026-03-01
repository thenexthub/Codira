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
            (error: Error) => E | PromiseLike<E>): PromiseLike<Awaited<U|E>> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            return onFulfill!('abc');
        }) as PromiseLike<Awaited<U|E>>;
    }
}

class Reject implements PromiseLike<string> {
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<Awaited<U|E>> {
        return Promise.resolve().then<E>((): E | PromiseLike<E> => {
            onFulfilled!('def');
            return onRejected!(new Error('xyz'));
        }) as PromiseLike<Awaited<U|E>>;
    }
}

let thenable: PromiseLike<string>[] = [new Fulfill(), new Reject()];
Promise.all<string>(thenable).then<void, void>((values): void => {
    if (values.length != 2) {
        console.error('Test failed. Expected a string array of length 2 but got length ' + values.length + '.');
        return;
    }
    if (values[0] != 'abc' || values[1] != 'def') {
        console.error('Test failed. The promise should be resolved by the specified values.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.error('Test failed. The promise should not be rejected.');
});
